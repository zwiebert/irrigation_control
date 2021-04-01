#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <mbedtls/base64.h>
#include <uout/so_target_desc.hh>
#include <app_uout/callbacks.h>
#include <uout/uo_types.h>
#include "net_http_server/http_server_setup.h"
#include "net_http_server/esp32/http_server_esp32.h"
#include <cli/cli.h>
#include "uout/status_json.hh"
#include "app_settings/config.h"
#include "cli/mutex.hh"
#include "app_cli/cli_app.h"
#include "utils_misc/int_types.h"
#include <uout/so_target_bits.h>
#include "webapp/content.h"

bool check_access_allowed(httpd_req_t *req); //XXX move this to a header file

#define URI_WAPP_HTML "/"
#define URI_WAPP_CSS "/f/css/wapp.css"
#define URI_WAPP_JS "/f/js/wapp.js"
#define URI_WAPP_JS_MAP "/f/js/wapp.js.map"
//#define URI_WAPP_CSS_MAP "/f/css/wapp.css.map"
//#define SERVE_BR

static const char *TAG="APP";

#ifdef USE_WS
/*
 *
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

static fd_set ws_fds;
static int ws_nfds;
struct ws_send_arg {
  httpd_handle_t hd;
  char *json;
  size_t json_len;
  int fd;
};

static void ws_async_broadcast(void *arg) {
  auto a = static_cast<ws_send_arg *>(arg);
  httpd_ws_frame_t ws_pkt = {  .final = true, .type = HTTPD_WS_TYPE_TEXT, .payload = (u8*)a->json, .len = a->json_len };

  for (int fd = 0; fd < ws_nfds; ++fd) {
    if (!FD_ISSET(fd, &ws_fds))
      continue;
    esp_err_t res = httpd_ws_send_frame_async(a->hd, fd, &ws_pkt);
    if (res != ESP_OK) {
      ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d (%s)", res, esp_err_to_name(res));
      FD_CLR(fd, &ws_fds);
      if (ws_nfds == fd + 1)
        ws_nfds = fd;
    }
  }
  free(a->json);
  free(a);
}

static esp_err_t ws_trigger_send(httpd_handle_t handle, const char *json, size_t len, int fd = -1) {
  struct ws_send_arg *arg = static_cast<struct ws_send_arg *>(malloc(sizeof(struct ws_send_arg)));
  arg->hd = handle;
  arg->json = strdup(json);
  arg->json_len = len;
  arg->fd = fd;
  return httpd_queue_work(handle, ws_async_broadcast, arg);
}

void ws_send_json(const char *json, ssize_t len) {
  ws_trigger_send(hts_server, json, len >= 0 ? len : strlen(json));
}

static int ws_write(void *req, const char *s, ssize_t len = -1, bool final = true) {
  if (len < 0)
    len = strlen(s);
  httpd_ws_frame_t ws_pkt = {  .final = final, .type = HTTPD_WS_TYPE_TEXT, .payload = (u8*)s, .len = len };
  if (auto res = httpd_ws_send_frame((httpd_req_t *)req, &ws_pkt); res != ESP_OK) {
    ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d (%s)", res, esp_err_to_name(res));
    return -1;
  }

  return len;
}

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
  ESP_LOGI(TAG, "ws_async_send");
    static const char * data = "Async data";
    struct async_resp_arg *resp_arg = static_cast<struct async_resp_arg *>(arg);
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.final = true;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}
#endif
////////////////////////// URI handlers /////////////////////////////////
static esp_err_t handle_uri_cmd_json(httpd_req_t *req) {
  char buf[256];
  int ret, remaining = req->content_len;
  int result = ESP_OK;

  if (!check_access_allowed(req))
    return ESP_FAIL;

  if ((ret = httpd_req_recv(req, buf, MIN(remaining, (sizeof buf)-1))) <= 0) {
    return ESP_FAIL;
  }

  { LockGuard lock(cli_mutex);
    buf[ret] = '\0';

    httpd_resp_set_type(req, "application/json") == ESP_OK || (result = ESP_FAIL);
#if 0
    TargetDesc td { req, static_cast<so_target_bits>(SO_TGT_HTTP | SO_TGT_FLAG_JSON), ht_write };
    cli_process_json(buf, td); // parse and process received command
    td.sj().write_json() >= 0 || (result = ESP_FAIL);
    httpd_resp_send_chunk(req, 0, 0);
#else
    TargetDesc td { static_cast<so_target_bits>(SO_TGT_HTTP | SO_TGT_FLAG_JSON)};
    cli_process_json(buf, td); // parse and process received command
    const char *json = td.sj().get_json() ? td.sj().get_json() : "{}";
    httpd_resp_sendstr(req, json) == ESP_OK || (result = ESP_FAIL);
#endif
  }

  return result;
}

#if 0
struct file_map { const char *uri, *type, *file, *file_gz, *file_br; u16 file_size, file_gz_size, file_br_size; };
const struct file_map uri_file_map[] = {
    { .uri = "/", .type = "text/html", .file_gz = build_wapp_html_gz, .file_gz_size = sizeof build_wapp_html_gz }, //
    { .uri = "/f/js/wapp.js", .type = "text/javascript", .file_gz = build_wapp_js_gz, .file_gz_size = sizeof build_wapp_js_gz  }, //
#ifdef SERVE_JS_MAP
    { .uri = "/f/js/wapp.js.map", .type = "text/javascript",  .file_br = build_wapp_js_map_br, .file_br_size = sizeof build_wapp_js_map_br }, //
#endif
    { .uri = "/f/css/wapp.css", .type = "text/css", .file_gz = build_wapp_css_gz, .file_gz_size = sizeof build_wapp_css_gz  }, //
    { .uri = "/f/cli/help/config", .file = cli_help_parmConfig }, //
    { .uri = "/f/cli/help/mcu", .file = cli_help_parmMcu }, //
    { .uri = "/f/cli/help/help", .file = cli_help_parmHelp }, //
    { .uri = "/f/cli/help/kvs", .file = cli_help_parmKvs }, //
    { .uri = "/f/cli/help/cmd", .file = cli_help_parmCmd }, //
    };


static esp_err_t handle_uri_get_file(httpd_req_t *req) {
  if (!check_access_allowed(req))
    return ESP_FAIL;

  for (int i = 0; i < sizeof(uri_file_map) / sizeof(uri_file_map[0]); ++i) {
    const struct file_map *fm = &uri_file_map[i];
    if (strcmp(req->uri, fm->uri) != 0)
      continue;
    const char *type = (fm->type) ? fm->type : "text/plain;charset=\"UTF-8\"";
    httpd_resp_set_type(req, type);

    if (fm->file_br) {
      httpd_resp_set_hdr(req, "content-encoding", "br");
      httpd_resp_send(req, fm->file_br, fm->file_br_size);
      return ESP_OK;
    }

    if (fm->file_gz) {
      httpd_resp_set_hdr(req, "content-encoding", "gzip");
      httpd_resp_send(req, fm->file_gz, fm->file_gz_size);
      return ESP_OK;
    }

    if (fm->file) {
      if (fm->file_size)
        httpd_resp_send(req, fm->file_gz, fm->file_size);
      else
        httpd_resp_sendstr(req, fm->file);
      return ESP_OK;
    }
  }

  return ESP_FAIL;
}

#else
struct file_map { const char *uri, *type, *file; u32 file_size; };

const struct file_map uri_file_map[] =  {
    { .uri = "/f/cli/help/config", .file = cli_help_parmConfig }, //
    { .uri = "/f/cli/help/mcu", .file = cli_help_parmMcu }, //
    { .uri = "/f/cli/help/help", .file = cli_help_parmHelp }, //
    { .uri = "/f/cli/help/kvs", .file = cli_help_parmKvs }, //
    { .uri = "/f/cli/help/cmd", .file = cli_help_parmCmd }, //
};


static esp_err_t respond_file(httpd_req_t *req, const struct file_map *fm, const char *content_encoding) {
  httpd_resp_set_type(req, fm->type ? fm->type : "text/plain;charset=\"UTF-8\"");
  if (content_encoding)
    httpd_resp_set_hdr(req, "content-encoding", content_encoding);
  if (fm->file_size)
    httpd_resp_send(req, fm->file, fm->file_size);
  else
    httpd_resp_sendstr(req, fm->file);
  return ESP_OK;
}

static esp_err_t handle_uri_get_file(httpd_req_t *req) {
  if (!check_access_allowed(req))
    return ESP_FAIL;

  struct file_map fm = {};
  const char *encoding = "gzip";

  if (strcmp(req->uri, URI_WAPP_HTML) == 0) {
    fm.file = build_wapp_html_gz;
    fm.file_size = build_wapp_html_gz_len;
    fm.type = "text/html";
  } else  if (strcmp(req->uri, URI_WAPP_CSS) == 0) {
    fm.file = build_wapp_css_gz;
    fm.file_size = build_wapp_css_gz_len;
    fm.type = "text/css";
  } else  if (strcmp(req->uri, URI_WAPP_JS) == 0) {
    fm.file = build_wapp_js_gz;
    fm.file_size = build_wapp_js_gz_len;
    fm.type = "text/javascript";
#ifdef URI_WAPP_JS_MAP
  } else  if (strcmp(req->uri, URI_WAPP_JS_MAP) == 0) {
    fm.type = "text/javascript";
#ifdef SERVE_BR
    fm.file = build_wapp_js_map_br;
    fm.file_size = build_wapp_js_map_br_len;
    encoding = "br";
#else
    fm.file = build_wapp_js_map_gz;
    fm.file_size = build_wapp_js_map_gz_len;
#endif
#endif
#ifdef URI_WAPP_CSS_MAP
  } else  if (strcmp(req->uri, URI_WAPP_CSS_MAP) == 0) {
    fm.type = "text/css";
#ifdef SERVE_BR
    fm.file = build_wapp_css_map_br;
    fm.file_size = build_wapp_css_map_br_len;
    encoding = "br";
#else
    fm.file = build_wapp_css_map_gz;
    fm.file_size = build_wapp_css_map_gz_len;
#endif
#endif
  } else {
    for (int i = 0; i < sizeof(uri_file_map) / sizeof(uri_file_map[0]); ++i) {
      if (strcmp(req->uri, uri_file_map[i].uri) == 0)
        return respond_file(req, &uri_file_map[i], "");
    }
    return ESP_FAIL;
  }

  return respond_file(req, &fm, encoding);

}

#endif

#ifdef USE_WS
static esp_err_t handle_uri_ws(httpd_req_t *req) {
  int fd = httpd_req_to_sockfd(req);
  FD_SET(fd, &ws_fds);
  if (ws_nfds <= fd)
    ws_nfds = 1 + fd;
  uint8_t buf[128] = { 0 };

  httpd_ws_frame_t ws_pkt = {.payload = buf };
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, (sizeof buf)-1);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d (%s)", ret, esp_err_to_name(ret));
      return ret;
  }

  if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) { // XXX: does nothing. remove this
    ESP_LOGI(TAG, "Close fd: %d", fd);
    FD_CLR(fd, &ws_fds);
  }


  {
    LockGuard lock(cli_mutex);
    TargetDescWs td { req, static_cast<so_target_bits>(SO_TGT_WS | SO_TGT_FLAG_JSON), ws_write };
    cli_process_json((char*)buf, td);// parse and process received command
  }
  return ret;
}
#endif

////////////////////////// URI definitions ////////////////////////////////
static const httpd_uri_t httpd_uris[] = {
    { .uri = "/cmd.json", .method = HTTP_POST, .handler = handle_uri_cmd_json },
    { .uri = "/f/*", .method = HTTP_GET, .handler = handle_uri_get_file },
    { .uri = "/", .method = HTTP_GET, .handler = handle_uri_get_file },
    { .uri = "/ws", .method = HTTP_GET, .handler = handle_uri_ws, .user_ctx = NULL, .is_websocket = true },
};


///////// public ///////////////
void hts_register_uri_handlers(httpd_handle_t server) {
  int i;

  ESP_LOGI(TAG, "Registering URI handlers");
  for (i = 0; i < sizeof(httpd_uris) / sizeof(httpd_uri_t); ++i) {
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &httpd_uris[i]));
  }

}

static void ws_send_json_cb(const uoCb_msgT msg) {
  if (auto json = uoCb_jsonFromMsg(msg))
    ws_send_json(json, -1);
}


void hts_setup_content() {
  hts_register_uri_handlers_cb = hts_register_uri_handlers;
#ifdef USE_WS
  uo_flagsT flags;
  flags.tgt.websocket = true;
  flags.evt.valve_change = true;
  flags.evt.async_http_resp = true;
  flags.fmt.json = true;
  uoCb_subscribe(ws_send_json_cb, flags);
#endif
}

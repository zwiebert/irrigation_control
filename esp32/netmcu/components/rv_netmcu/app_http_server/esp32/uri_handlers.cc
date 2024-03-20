#include "../web_content.hh"
#include "app_cli/cli_app.h"
#include <cli/cli.h>

#include <sys/param.h>

#include "net_http_server/http_server_setup.h"
#include "uout/status_json.hh"
#include "app_uout/callbacks.h"

#include "app_settings/config.h"

#include "utils_misc/int_types.h"
#include "cli/mutex.hh"
#include "net_http_server/esp32/http_server_esp32.h"

#include <mbedtls/base64.h>
#include <esp_check.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>

#include <fcntl.h>

static const char *TAG = "APP";

////////////////////////// URI handlers /////////////////////////////////
static esp_err_t handle_uri_cmd_json(httpd_req_t *req) {
  char buf[256];
  int ret, remaining = req->content_len;
  int result = ESP_OK;

  if (!check_access_allowed(req))
    return ESP_FAIL;

  if ((ret = httpd_req_recv(req, buf, MIN(remaining, (sizeof buf) - 1))) <= 0) {
    return ESP_FAIL;
  }

  {
    LockGuard lock(cli_mutex);
    buf[ret] = '\0';

    httpd_resp_set_type(req, "application/json") == ESP_OK || (result = ESP_FAIL);

    UoutWriter td { SO_TGT_HTTP | SO_TGT_FLAG_JSON };
    cli_process_json(buf, td); // parse and process received command
    httpd_resp_sendstr(req, td.sj().get_json()) == ESP_OK || (result = ESP_FAIL);
  }

  return result;
}

static esp_err_t handle_uri_get_file(httpd_req_t *req) {
  if (!check_access_allowed(req))
    return ESP_FAIL;

  if (auto fm = wc_getContent(req->uri); fm) {
    return respond_file(req, fm);
  }
  return ESP_FAIL;
}

#ifdef CONFIG_APP_USE_WS
static esp_err_t handle_uri_ws(httpd_req_t *req) {

  if (req->method == HTTP_GET) {
    ESP_LOGI(TAG, "Handshake done, the new connection was opened");
    return ESP_OK;
  }

  int fd = httpd_req_to_sockfd(req);
  FD_SET(fd, &ws_fds);
  if (ws_nfds <= fd)
    ws_nfds = 1 + fd;
  uint8_t buf[256] = { 0 };

  httpd_ws_frame_t ws_pkt = { .payload = buf };
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, (sizeof buf) - 1);

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
    UoutWriterWebsocket td { req, SO_TGT_WS | SO_TGT_FLAG_JSON, ws_write };
    cli_process_json((char*) buf, td); // parse and process received command
  }
  return ret;
}
#endif
////////////////////////// URI definitions ////////////////////////////////
static const httpd_uri_t httpd_uris[] = { //
    { .uri = "/cmd.json", .method = HTTP_POST, .handler = handle_uri_cmd_json }, //
        { .uri = "/f/*", .method = HTTP_GET, .handler = handle_uri_get_file }, //
        { .uri = "/", .method = HTTP_GET, .handler = handle_uri_get_file },
#ifdef CONFIG_APP_USE_WS
        { .uri = "/ws", .method = HTTP_GET, .handler = handle_uri_ws, .user_ctx = NULL, .is_websocket = true },
#endif
    };

///////// public ///////////////
void hts_register_uri_handlers(httpd_handle_t server) {
  for (int i = 0; i < sizeof(httpd_uris) / sizeof(httpd_uri_t); ++i) {
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
  flags.evt.uo_evt_flag_valveChange = true;
  flags.evt.async_http_resp = true;
  flags.fmt.json = true;
  uoCb_subscribe(ws_send_json_cb, flags);
#endif
}


#include "web_content.hh"
#include "app_cli/cli_app.h"


#include <iterator>
#include <cstring>


static const struct file_map uri_file_map[] = { //
    { .uri = "/", .type = "text/html", .wc = wapp_html_gz_fm }, //
        { .uri = "/f/js/wapp.js", .type = "text/javascript", .wc = wapp_js_gz_fm }, //
#ifdef CONFIG_APP_USE_HTTP_SERVE_JS_MAP
        { .uri = "/f/js/wapp.js.map", .type = "text/javascript", .wc = wapp_js_map_gz_fm }, //
#endif
        { .uri = "/f/css/wapp.css", .type = "text/css", .wc = wapp_css_gz_fm }, //
#ifdef CONFIG_APP_USE_HTTP_SERVE_CSS_MAP
        { .uri = "/f/css/wapp.css.map", .type = "text/css", .wc =  wapp_css_map_gz_fm }, //
#endif

        { .uri = "/f/cli/help/config", .type = "text/plain;charset=\"UTF-8\"", .wc = { .content = cli_help_parmConfig } }, //
        { .uri = "/f/cli/help/mcu", .type = "text/plain;charset=\"UTF-8\"", .wc = { .content = cli_help_parmMcu } }, //
        { .uri = "/f/cli/help/help", .type = "text/plain;charset=\"UTF-8\"", .wc = { .content = cli_help_parmHelp } }, //
        { .uri = "/f/cli/help/kvs", .type = "text/plain;charset=\"UTF-8\"", .wc = { .content = cli_help_parmKvs } }, //
        { .uri = "/f/cli/help/cmd", .type = "text/plain;charset=\"UTF-8\"", .wc = { .content = cli_help_parmCmd } }, //
};




const struct file_map *wc_getContent(const char *uri) {
  for (auto it = std::begin(uri_file_map); it != std::end(uri_file_map); ++it) {
    if (strcmp(uri, it->uri) == 0)
      return it;
  }
  return nullptr;
}


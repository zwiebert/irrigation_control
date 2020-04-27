#include "app_config/proj_app_cfg.h"
#include "so_out.h"
#include "so_print.h"

#include "app/common.h"
#include "app/ota.h"
#include "stm32/stm32_ota.h"
#include "app/rtc.h"
#include "cli_app/cli_config.h"
#include "cli_app/cli_imp.h" // FIXME?
#include "config/config.h"
#include "app/kvstore.h"
#include "net/ipnet.h"
#include "net/mqtt/app/mqtt.h"
#include "txtio/inout.h"
#include "userio/status_json.h"
#include "userio/status_json.h"
#include "userio_app/status_output.h"

#include "misc/int_macros.h"
#include "misc/int_types.h"

#include <string.h>
#include <stdio.h>

#define D(x)

extern u8 so_target;

bool so_output_message2(so_msg_t mt, const void *arg) {
  char buf[64];
  switch (mt) {
#ifdef USE_LAN
  case SO_CFG_LAN_PHY:
    so_out_x_reply_entry_s(mt, cfg_args_lanPhy[config_read_item_i8(CB_LAN_PHY, MY_LAN_PHY)]);
    break;
  case SO_CFG_LAN_PWR_GPIO:
    so_out_x_reply_entry_d(mt, config_read_item_i8(CB_LAN_PWR_GPIO, MY_LAN_PWR_GPIO));
    break;
#else
  case SO_CFG_LAN_PHY:
  case SO_CFG_LAN_PWR_GPIO:
    break;
#endif
#ifdef USE_WLAN
  case SO_CFG_WLAN_SSID:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_WIFI_SSID, buf, sizeof buf, MY_WIFI_SSID));
    break;
  case SO_CFG_WLAN_PASSWORD:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_WIFI_PASSWD, buf, sizeof buf, "") ? "*" : "");
    break;
#endif
#ifdef USE_NTP
  case SO_CFG_NTP_SERVER:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_NTP_SERVER, buf, sizeof buf, MY_NTP_SERVER));
    break;
#endif
#ifdef USE_MQTT
  case SO_CFG_MQTT_ENABLE:
    so_out_x_reply_entry_d(mt, !!config_read_item_i8(CB_MQTT_ENABLE, MY_MQTT_ENABLE));
    break;
  case SO_CFG_MQTT_URL:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_MQTT_URL, buf, sizeof buf, MY_MQTT_URL));
    break;
  case SO_CFG_MQTT_USER:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_MQTT_USER, buf, sizeof buf, MY_MQTT_USER));
    break;
  case SO_CFG_MQTT_PASSWORD:
    so_out_x_reply_entry_s(mt, *config_read_item_s(CB_MQTT_PASSWD, buf, sizeof buf, "") ? "*" : "");
    break;
  case SO_CFG_MQTT_CLIENT_ID:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_MQTT_CLIENT_ID, buf, sizeof buf, MY_MQTT_CLIENT_ID));
    break;
#else
  case SO_CFG_MQTT_ENABLE:
  case SO_CFG_MQTT_URL:
  case SO_CFG_MQTT_USER:
  case SO_CFG_MQTT_PASSWORD:
  case SO_CFG_MQTT_CLIENT_ID:
    break;
#endif

#ifdef USE_HTTP
  case SO_CFG_HTTP_ENABLE:
    so_out_x_reply_entry_d(mt, !!config_read_item_i8(CB_HTTP_ENABLE, MY_HTTP_ENABLE));
    break;
  case SO_CFG_HTTP_USER:
    so_out_x_reply_entry_s(mt, config_read_item_s(CB_HTTP_USER, buf, sizeof buf, MY_HTTP_USER));
    break;
  case SO_CFG_HTTP_PASSWORD:
    so_out_x_reply_entry_s(mt, *config_read_item_s(CB_HTTP_PASSWD, buf, sizeof buf, "") ? "*" : "");
    break;
#else
  case SO_CFG_HTTP_ENABLE:
  case SO_CFG_HTTP_USER:
  case SO_CFG_HTTP_PASSWORD:
    break;
#endif

  case SO_CFG_VERBOSE:
    so_out_x_reply_entry_d(mt, config_read_item_i8(CB_VERBOSE, MY_VERBOSE));
    break;

    default:
     return false;
  }
  return true;
}

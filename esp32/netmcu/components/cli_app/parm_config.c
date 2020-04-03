#include "app_config/proj_app_cfg.h"
#include "userio_app/status_output.h"
#include "config/config.h"
#include "app/rtc.h"
#include "cli_imp.h"
#include "cli/cli.h"
#include "net/mqtt/app/mqtt.h"
#include "net/http/server/http_server.h"
#include "app/kvstore.h"

#define ENABLE_RESTART 1 // allow software reset

const char * const cfg_keys[SO_CFG_size] = {
    "rtc", "network",
    "wlan-ssid", "wlan-password", "lan-phy", "lan-pwr-gpio", "ntp-server",
    "tz", "verbose",
    "mqtt-enable", "mqtt-url", "mqtt-user", "mqtt-password",
    "http-enable", "http-user", "http-password",
};

#ifdef USE_NETWORK
const char *const cfg_args_network[nwLEN] = {
    "none", "wlan", "ap", "lan"
};
#endif

#ifdef USE_LAN
const char *const cfg_args_lanPhy[lanPhyLEN] = {
    "none", "lan8270", "rtl8201", "ip101",
};
#endif

#ifdef CONFIG_GPIO_SIZE
//PIN_DEFAULT=0, PIN_INPUT, PIN_INPUT_PULLUP, PIN_OUTPUT, PIN_ERROR, PIN_READ, PIN_CLEAR, PIN_SET, PIN_TOGGLE

const char pin_state_args[] = "dipo ?01t";
#endif

const char cli_help_parmConfig[]  =
    "'config' sets or gets options. Use: config option=value ...; to set. Use: config option=? ...; to get, if supported\n\n"
    "rtc=(ISO_TIME|?)   set local time it NTP is not working.;\n"
#ifdef USE_NETWORK
    "network=(wlan|ap|lan) ap opens wlan access-point at 192.168.4.1 (restart required)\n"
#endif
#ifdef USE_WLAN
    "wlan-ssid=(SSID|?)\n"
    "wlan-password=PW\n"
#endif
#ifdef USE_NTP
    "ntp-server=(dhcp|IP4ADDR|NAME) default: ntp.pool.org (TODO: implement list of servers)\n"
#endif
#ifdef USE_MQTT
     "mqtt-enable=(0|1) enable MQTT\n"
     "mqtt-url=URL      broker/server URL (e.g. mqtt://192.168.1.42:7777)\n"
     "mqtt-user=NAME    user name on server\n"
     "mqtt-password=PW  user password on server\n"
#endif
#ifdef USE_HTTP
     "http-enable=(0|1) enable HTTP\n"
     "http-user=NAME    user name on server\n"
     "http-password=PW  user password on server\n"
#endif
#ifdef POSIX_TIME
    "tz=(POSIX_TZ|?)    time zone for RTC/NTP\n"
#endif
    "verbose=(0..5|?)   diagnose output verbosity level\n"
    "set-pw=password    set a config password. if set every config commands needs the pw option\n"
    "pw=PW              example: config pw=my_passw dst=eu;\n"
#if ENABLE_RESTART
    "restart            restart MCU\n"
#endif
#ifdef ACCESS_GPIO
    "gpioN=(i|p|o|0|1|d|?) Set gpio pin as input (i,p) or output (o,0,1) or use default\n"
#endif
//  "set-expert-password=\n"
;

int
process_parmConfig(clpar p[], int len) {
  int arg_idx;
  int errors = 0;
  so_msg_t so_key = SO_NONE;

  so_output_message(SO_CFG_begin, NULL);

  bool pw_ok = strlen(C.app_configPassword) == 0;

  for (arg_idx = 1; arg_idx < len; ++arg_idx) {
    const char *key = p[arg_idx].key, *val = p[arg_idx].val;

    if (key == NULL || val == NULL) {  // don't allow any default values in config
      return cli_replyFailure();
    } else if (strcmp(key, "pw") == 0) {
      if (val && strcmp(C.app_configPassword, val) == 0) {
        pw_ok = true;
        so_output_message(SO_CFGPASSWD_OK, NULL);
      } else {
        so_output_message(SO_CFGPASSWD_WRONG, NULL);
        return cli_replyFailure();
      }
    } else if (!pw_ok) {
      so_output_message(SO_CFGPASSWD_MISSING, NULL);
      return cli_replyFailure();


#if ENABLE_RESTART
    } else if (strcmp(key, "restart") == 0) {
      extern void  mcu_restart(void);
      mcu_restart();
#endif

    } else if (strcmp(key, "all") == 0) {
      if (*val == '?') {
        so_output_message(SO_CFG_all, "cj");
      }
    } else if (SO_NONE != (so_key = so_parse_config_key(key))) {
      if (0 == strcmp("?", val)) {
        so_output_message(so_key, NULL);
      } else
        switch (so_key) {

        case SO_CFG_RTC: {
          cli_replyResult(val ? rtc_set_by_string(val) : false);
        }
          break;

        case SO_CFG_VERBOSE: {
          NODEFAULT();
          enum verbosity level = atoi(val);
          C.app_verboseOutput = level;
          save_config_item(CB_VERBOSE);
        }
        break;
#ifdef USE_NETWORK
        case SO_CFG_NETWORK: {
          NODEFAULT();
          bool success = false;
          for (int i=0; i < nwLEN; ++i) {
#ifndef USE_LAN
            if (i == nwLan)  {
              continue;
            }
#endif
#ifndef USE_WLAN
            if (i == nwWlanSta || i == nwWlanAp)  {
              continue;
            }
#endif
            if (strcmp(val, cfg_args_network[i]) == 0) {
              C.network = i;
              save_config_item(CB_NETWORK_CONNECTION);
              success = true;
              break;
            }
          }
          if (!success)
            cli_replyFailure();
        }
        break;
#endif
#ifdef USE_WLAN
        case SO_CFG_WLAN_SSID: {
          if (strlen(val) < sizeof (C.wifi.SSID)) {
            strcpy (C.wifi.SSID, val);
            save_config_item(CB_WIFI_SSID);
          } else {
            cli_replyFailure();
          }
        }
        break;

        case SO_CFG_WLAN_PASSWORD: {
          if (strlen(val) < sizeof (C.wifi.password)) {
            strcpy (C.wifi.password, val);
            save_config_item(CB_WIFI_PASSWD);
          } else {
            cli_replyFailure();
          }
        }
        break;
#endif // USE_WLAN
#ifdef USE_LAN
        case SO_CFG_LAN_PHY: {
          NODEFAULT();
           bool success = false;
           u8 i;
           for (i=0; i < lanPhyLEN; ++i) {
             if (strcasecmp(val, cfg_args_lanPhy[i]) == 0) {
               C.lan.phy = i;
               save_config_item(CB_LAN_PHY);
               success = true;
               break;
             }
           }
           if (!success)
             cli_replyFailure();
         }
         break;
        case SO_CFG_LAN_PWR_GPIO: {
          NODEFAULT();
          C.lan.pwr_gpio = atoi(val);
          save_config_item(CB_LAN_PWR_GPIO);
        }
        break;
#endif // USE_LAN
#ifdef USE_NTP
        case SO_CFG_NTP_SERVER: {
          if (strlen(val) < sizeof (C.ntp.server)) {
            strcpy (C.ntp.server, val);
            save_config_item(CB_NTP_SERVER);
          } else {
            cli_replyFailure();
          }
        }
        break;
#endif

#ifdef USE_MQTT
        case SO_CFG_MQTT_ENABLE: {
          C.mqtt.enable = (*val == '1') ? 1 : 0;
          io_mqtt_enable(C.mqtt.enable);
          save_config_item(CB_MQTT_ENABLE);
        }
        break;

        case SO_CFG_MQTT_PASSWORD: {
          if (strlen(val) < sizeof (C.mqtt.password)) {
            strcpy (C.mqtt.password, val);
            save_config_item(CB_MQTT_PASSWD);
          } else {
            cli_replyFailure();
          }

        }
        break;

        case SO_CFG_MQTT_USER: {
          if (strlen(val) < sizeof (C.mqtt.user)) {
            strcpy (C.mqtt.user, val);
            save_config_item(CB_MQTT_USER);
          } else {
            cli_replyFailure();
          }
        }
        break;

        case SO_CFG_MQTT_URL: {
          if (strlen(val) < sizeof (C.mqtt.url)) {
            strcpy (C.mqtt.url, val);
            save_config_item(CB_MQTT_URL);
          } else {
            cli_replyFailure();
          }
        }
        break;
#endif //USE_MQTT

#ifdef USE_HTTP
        case SO_CFG_HTTP_ENABLE: {
          C.http.enable = (*val == '1') ? 1 : 0;
          hts_enable_http_server(C.http.enable);
          save_config_item(CB_HTTP_ENABLE);
        }
        break;

        case SO_CFG_HTTP_PASSWORD: {
          if (strlen(val) < sizeof (C.http.password)) {
            strcpy (C.http.password, val);
            save_config_item(CB_HTTP_PASSWD);
          } else {
            cli_replyFailure();
          }

        }
        break;

        case SO_CFG_HTTP_USER: {
          if (strlen(val) < sizeof (C.http.user)) {
            strcpy (C.http.user, val);
            save_config_item(CB_HTTP_USER);
          } else {
            cli_replyFailure();
          }
        }
        break;
#endif //USE_HTTP

        case SO_CFG_TZ: {
#ifdef POSIX_TIME
          strncpy(C.geo_tz, val, sizeof (C.geo_tz) -1);
          rtc_setup();
          save_config_item(CB_TZ);
#endif
        }
        break;

        default:
        break;
      }

#ifdef ACCESS_GPIO
    } else if (strncmp(key, "gpio", 4) == 0) {
      int gpio_number = atoi(key + 4);
      mcu_pin_state ps;

      if (*val == '?') {
        so_output_message(SO_CFG_GPIO_PIN, &gpio_number);
      } else if (!is_gpio_number_usable(gpio_number, true)) {
        reply_message("gpio:error", "gpio number cannot be used");
        ++errors;
      } else {
        const char *error = NULL;

        for (ps = 0; pin_state_args[ps] != 0; ++ps) {
          if (pin_state_args[ps] == *val) {
            break;
          }
        }

        switch (ps) {

          case PIN_DEFAULT:
          break;

          case PIN_CLEAR:
          case PIN_SET:
          case PIN_OUTPUT:
          error = mcu_access_pin(gpio_number, NULL, PIN_OUTPUT);
          if (!error && ps != PIN_OUTPUT) {
            error = mcu_access_pin(gpio_number, NULL, ps);
          }
          break;

          case PIN_INPUT:
          case PIN_INPUT_PULLUP:
          error = mcu_access_pin(gpio_number, NULL, ps);
          break;

          default:
          error = "unknown gpio config";
          ++errors;
        }

        if (error) {
          reply_message("gpio:failure", error);
        } else {
          C.gpio[gpio_number] = ps;
          save_config_item(CB_GPIO);
        }
      }
#endif

    } else if (strcmp(key, "set-pw") == 0) {
      if (strlen(val) < sizeof (C.app_configPassword)) {
        strcpy (C.app_configPassword, val);
        save_config_item(CB_CFG_PASSWD);
      } else {
        cli_replyFailure();
      }
    } else {
      ++errors;
      cli_warning_optionUnknown(key);
    }
  }

  so_output_message(SO_CFG_end, NULL);
  cli_replyResult(errors==0);
  return 0;
}

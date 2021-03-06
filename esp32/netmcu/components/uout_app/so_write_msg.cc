#include "app_config/proj_app_cfg.h"
#include "so_out.h"
#include "so_print.h"

#include "app/common.h"
#include "app/ota.h"
#include "stm32/stm32_ota.h"
#include "app/rtc.h"
#include "app/cli/cli_config.h"
#include "app/cli/cli_app.h" // FIXME?
#include "app_settings/config.h"
#include "app/kvstore.h"
#include "net/ipnet.h"
#include "app_mqtt/mqtt.h"
#include "txtio/inout.h"
#include "uout/status_json.hh"
#include "uout/status_json.hh"
#include "app/uout/status_output.h"
#include "app/uout/callbacks.h"
#include "utils_misc/int_macros.h"
#include "utils_misc/int_types.h"
#include "utils_misc/base64.h"
#include <utils_time/run_time.h>

#include <string.h>
#include <stdio.h>

void soMsg_FW_START_MSG_PRINT(const struct TargetDesc &td) {
  so_print_startup_info();
}

void soMsg_STATUS_OK(const struct TargetDesc &td) {
  td.so().print("status", "ok");
}

void soMsg_STATUS_ERROR(const struct TargetDesc &td) {
  td.so().print("status", "error");
}

void soMsg_MCU_begin(const struct TargetDesc &td) {
  td.so().x_open("mcu");
}

void soMsg_MCU_RUN_TIME(const struct TargetDesc &td) {
  td.so().print("run-time", run_time_s());
}
#ifdef MCU_ESP32
#include "esp_ota_ops.h"
void soMsg_MCU_VERSION(const struct TargetDesc &td) {
  char buf[64];
  const esp_app_desc_t *ad = esp_ota_get_app_description();

  td.so().print("chip", MCU_TYPE);
  td.so().print("firmware", (csu_copy_cat(buf, ad->project_name, "-", ad->version), buf));

  csu_copy_cat(buf, ad->date, "T", ad->time);
  char *p = buf;
  do
    if (*p == ' ')
      *p = '-';
  while (*++p);
  td.so().print("build-time", buf);
}
#endif

void soMsg_MCU_OTA(const struct TargetDesc &td, const char *url) {
#ifdef USE_OTA
  td.so().print("ota-url", url);
#endif
}
void soMsg_MCU_OTA_STATE(const struct TargetDesc &td) {
#ifdef USE_OTA
    td.so().print("ota-state", ota_getState());
#endif
}

void soMsg_MCU_STM32OTA(const struct TargetDesc &td, const char *url) {
#ifdef USE_OTA
    td.so().print("stm32ota-url", url);
#endif
}
void soMsg_MCU_STM32OTA_STATE(const struct TargetDesc &td) {
#ifdef USE_OTA
    td.so().print("stm32ota-state", stm32ota_getState());
#endif
}

void soMsg_MCU_BOOT_COUNT(const struct TargetDesc &td) {
  extern i32 boot_counter;
  td.so().print("boot-count", boot_counter);
}

void soMsg_MCU_end(const struct TargetDesc &td) {
  td.so().x_close();
}

/////////////////////////////////////////////////////////////////////////////////

void soMsg_KVS_begin(const struct TargetDesc &td) {
  td.so().x_open("kvs");
}

void soMsg_KVS_ZN_SINGLE(const struct TargetDesc &td, const char *key) {
  char buf[64];
  if (kvs_get_string(key, buf, sizeof buf)) {
    td.so().print(key, buf);
  }
}

void soMsg_KVS_ZN_ALL(const struct TargetDesc &td, const char *keyBase) {
  char buf[64];
  for (int i = 0; i < 14; ++i) {
    char key[16];
    snprintf(key, sizeof key, "%s%d", keyBase, i);
    if (kvs_get_string(key, buf, sizeof buf)) {
      td.so().print(key, buf);
    }
  }
}

void soMsg_KVS_end(const struct TargetDesc &td) {
  td.so().x_close();
}

/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
void soMsg_RVE_begin(const struct TargetDesc &td) {
  td.so().x_open("rve");
}

void soMsg_RVE_PUMP(const struct TargetDesc &td, const so_arg_on_t *state) {
  io_mqtt_publish_pump_status(state->on);
  td.so().print("pump", state->on);
}

void soMsg_RVE_RAIN(const struct TargetDesc &td, const so_arg_on_t *state) {
  io_mqtt_publish_rain_sensor_status(state->on);
  td.so().print("rain", state->on);
}

void soMsg_RVE_PRESS_CTL(const struct TargetDesc &td, const so_arg_on_t *state) {
  td.so().print("pc", state->on);
}

void soMsg_RVE_VALVES(const struct TargetDesc &td, const so_arg_valves_t *valves) {
  td.so().print("valve_state", valves->state_bits);
  td.so().print("valve_change", valves->changed_bits);

  u32 mask = valves->changed_bits;
  for (int i = 0; mask; ++i, (mask >>= 1)) {
    if (mask & 1) {
      uoAPp_publish_valve_change({i, GET_BIT(valves->state_bits, i)});
    }
  }
}

void soMsg_RVE_end(const struct TargetDesc &td) {
  td.so().x_close();
}

/////////////////////////////////////////////////////////////////////////////////
void soMsg_CFGPASSWD_OK(const struct TargetDesc &td) {
// io_puts("password ok\n");
}
void soMsg_CFGPASSWD_WRONG(const struct TargetDesc &td) {
  io_puts("wrong config password\n");
}
void soMsg_CFGPASSWD_MISSING(const struct TargetDesc &td) {
  io_puts("missing config password\n");
}

/////////////////////////////////////////////////////////////////////////////////
void soMsg_cfgpasswd_ok(const struct TargetDesc &td) {
  // td.write("password ok\n");
}
void soMsg_cfgpasswd_wrong(const struct TargetDesc &td) {
  td.write("wrong config password\n");
}

void soMsg_cfgpasswd_missing(const struct TargetDesc &td) {
  td.write("missing config password\n");
}


/////////////////////////////////////////////////////////////////////////////////

void soMsg_INET_PRINT_ADDRESS(const struct TargetDesc &td) {
#if defined USE_LAN || defined USE_WLAN
  char buf[20];
  ipnet_addr_as_string(buf, 20);
  td.write("tf: ipaddr: "), td.write(buf), td.write(";\n");
#endif
}

void soMsg_PBUF_begin(const struct TargetDesc &td) {
  td.so().x_open("pbuf");
}

void soMsg_PBUF_KV64(const struct TargetDesc &td, const so_arg_pbuf_t *pba) {
  char buf[64];
  size_t b64Len = 0;
  int err = mbedtls_base64_encode((uint8_t*) buf, sizeof buf, &b64Len, pba->buf, pba->buf_len);
  if (!err) {
    buf[b64Len] = '\0';
    td.so().print(pba->key, buf);
  }
}

void soMsg_PBUF_end(const struct TargetDesc &td) {
  td.so().x_close();
}


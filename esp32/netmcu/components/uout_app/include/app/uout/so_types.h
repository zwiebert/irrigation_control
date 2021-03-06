#pragma once

#include <uout/uo_types.h>
#include <stdint.h>


typedef enum {
  SO_NONE,

  SO_FW_START_MSG_PRINT, //1


  SO_CFG_all, SO_CFG_begin, //13
  SO_CFG_RTC,   SO_CFG_NETWORK,
  SO_CFG_WLAN_SSID, SO_CFG_WLAN_PASSWORD,
  SO_CFG_LAN_PHY, SO_CFG_LAN_PWR_GPIO,
  SO_CFG_NTP_SERVER, //19
  SO_CFG_TZ, SO_CFG_VERBOSE, //25
  SO_CFG_MQTT_ENABLE, SO_CFG_MQTT_URL, SO_CFG_MQTT_USER, SO_CFG_MQTT_PASSWORD, SO_CFG_MQTT_CLIENT_ID,//29
  SO_CFG_HTTP_ENABLE, SO_CFG_HTTP_USER, SO_CFG_HTTP_PASSWORD, //23
  SO_CFG_STM32_BOOTGPIO_INV,
  SO_CFG_end, // values between begin/end will be used for 'all=?'
  SO_CFG_GPIO_PIN,

  SO_KVS_begin, SO_KVS_ZN_ALL, SO_KVS_end,  SO_KVS_ZN_SINGLE,

  SO_CFGPASSWD_OK, SO_CFGPASSWD_WRONG, SO_CFGPASSWD_MISSING,

  SO_INET_PRINT_ADDRESS,

  SO_CMD_begin,
  SO_CMD_ZONE_ON, SO_CMD_ZONE_OFF,
  SO_CMD_end,

  SO_MCU_begin, SO_MCU_RUN_TIME, SO_MCU_VERSION, SO_MCU_OTA, SO_MCU_OTA_STATE, SO_MCU_STM32OTA, SO_MCU_STM32OTA_STATE, SO_MCU_BOOT_COUNT, SO_MCU_end,

  SO_RVE_begin, SO_RVE_PUMP, SO_RVE_RAIN, SO_RVE_VALVES, SO_RVE_PRESS_CTL, SO_RVE_end,

  SO_PBUF_begin, SO_PBUF_end, SO_PBUF_KV64,

  SO_STATUS_OK, SO_STATUS_ERROR,


} so_msg_t;

typedef struct {
  bool on;
} so_arg_on_t;

typedef struct {
  uint32_t state_bits, changed_bits;
} so_arg_valves_t;

typedef struct {
  const char *key;
  uint8_t *buf;
  int buf_len;
} so_arg_pbuf_t;

typedef struct {
  const char *key, *val;
} so_arg_kvs_t;

typedef struct {
  const char *key;
  int val;
} so_arg_kvd_t;


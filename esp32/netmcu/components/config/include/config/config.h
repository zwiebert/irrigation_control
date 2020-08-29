/*
 * config.h
 *
 *  Created on: 15.05.2019
 *      Author: bertw
 */

#ifndef MAIN_CONFIG_CONFIG_H_
#define MAIN_CONFIG_CONFIG_H_

#include "app_config/proj_app_cfg.h"
#include "config_kvs/config.h"


#ifdef USE_LAN
#include "net/ethernet.h"
#endif
#ifdef USE_MQTT
#include "net/mqtt/mqtt.h"
#endif
#ifdef USE_WLAN
#include "net/wifistation.h"
#endif
#ifdef USE_HTTP
#include "net/http/server/http_server.h"
#endif
#ifdef USE_NTP
#include "net/ntp.h"
#endif
#include <stdint.h>

#ifdef USE_NETWORK
enum nwConnection {
  nwNone, nwWlanSta, nwWlanAp, nwLan, nwLEN,
};
#endif

typedef struct {
	char app_configPassword[16];
#ifdef USE_NETWORK
  enum nwConnection network;
#endif
} config;

extern config C;

enum configAppItem {
  CBA_start = CB_size - 1,
  CB_CFG_PASSWD,  CB_TZ,
#ifdef USE_NETWORK
  CB_NETWORK_CONNECTION,
#endif
  CB_STM32_INV_BOOTPIN,
#ifdef USE_LPH
  CB_LPH,
#endif
//-----------
  CBA_size
};

// save C to persistent storage
void save_config_all();
void save_config_item(enum configItem item);
void save_config(uint32_t mask);

// restore C from persistent storage
void read_config_all();
void read_config_item(enum configItem item);
void read_config(uint32_t mask);

void config_setup_global();
void config_setup_mqttAppClient();
bool config_item_modified(enum configItem item);

const char *config_read_tz(char *d, unsigned d_size);
enum nwConnection  config_read_network_connection();
bool config_read_stm32_inv_bootpin();

bool config_read_lph(uint16_t lph[14]);
bool config_save_lph(uint16_t lph[14]);

bool config_save_pb64(enum configItem item, const char *pb64);
bool config_read_pb64(enum configItem item, char *pb64_buf, size_t buf_size);
bool config_save_pb(enum configItem item, const uint8_t *pb, size_t pb_len);
bool config_read_pb(enum configItem item, uint8_t *pb_buf, size_t *pb_size);

#endif /* MAIN_CONFIG_CONFIG_H_ */

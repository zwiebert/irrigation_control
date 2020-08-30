/*
 * cli_status.c
 *
 *  Created on: 15.06.2019
 *      Author: bertw
 */

#include "app_config/proj_app_cfg.h"

#include <string.h>
#include <stdbool.h>

#include "txtio/inout.h"
#include "net/mqtt/app/mqtt.h"
#include "app_config/proj_app_cfg.h"
#include "cli_imp.h"
#include "cli/cli.h"
#include "userio_app/status_output.h"
#include "userio/status_json.h"
#include "debug/debug.h"
#include "misc/int_macros.h"
#include <stdio.h>

#include <stm32/stm32.h>

#define D(x) x

#define STM32_STATUS "status "
#define STM32_STATUS_LEN (sizeof (STM32_STATUS) - 1)
#define KEY_VALVE_BITS "valve-bits"
#define KEY_VALVE_CHANGE_MASK "valve-change-mask"
#define KEY_RAIN_SENSOR "rain"
#define KEY_EVENT "event"
#define KEY_PUMP "pump"
#define KEY_PRESS_CTL "pc"
#define KEY_PB "pb"

#define BUF_SIZE 128
#define ZONE_COUNT 14
#define MAX_DURATION 60

int
process_parmStatus(clpar p[], int len) {
  int arg_idx;

  so_output_message(SO_RVE_begin, NULL);

  bool hasValveBits = false, hasValveChangeMask = false;
  u32 valveBits = 0, valveChangeMask = 0;

  for (arg_idx = 1; arg_idx < len; ++arg_idx) {
    const char *key = p[arg_idx].key, *val = p[arg_idx].val;
    D(db_printf("key=%s, val=%s\n", key, val));

    if (key == NULL) {
      return -1;
    } else if (strcmp(key, KEY_VALVE_BITS) == 0) {
      hasValveBits = true;
      valveBits = strtol(val, 0, 16);
    } else if (strcmp(key, KEY_VALVE_CHANGE_MASK) == 0) {
      hasValveChangeMask = true;
      valveChangeMask = strtol(val, 0, 16);
    } else if (strcmp(key, KEY_RAIN_SENSOR) == 0) {
      so_arg_on_t state = { .on = *val != '0' };
      so_output_message(SO_RVE_RAIN, &state);
    } else if (strcmp(key, KEY_EVENT) == 0) {
      io_mqtt_publish_stm32_event(val);
    } else if (strcmp(key, KEY_PB) == 0) {
       const char *b64 = val; // TODO: implement protobuf handler
    } else if (strcmp(key, KEY_PUMP) == 0) {
      so_arg_on_t state = { .on = *val != '0' };
      so_output_message(SO_RVE_PUMP, &state);
    } else if (strcmp(key, KEY_PRESS_CTL) == 0) {
      so_arg_on_t state = { .on = *val != '0' };
      so_output_message(SO_RVE_PRESS_CTL, &state);
    } else {
      cli_warning_optionUnknown(key);
    }
  }

  if (hasValveBits && hasValveChangeMask) {
    so_arg_valves_t valves = { .state_bits = valveBits, .changed_bits = valveChangeMask };
    so_output_message(SO_RVE_VALVES, &valves);
  }


  so_output_message(SO_RVE_end, NULL);

  return 0;
}


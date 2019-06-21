/*
 * cli_kvs.c
 *
 *  Created on: 12.06.2019
 *      Author: bertw
 */

#include "user_config.h"
#include "userio/status_output.h"
#include "config/config.h"
#include "main/rtc.h"
#include "cli_imp.h"
#include "userio/mqtt.h"
#include "userio/http_server.h"
#include "kvstore.h"
#include "debug/debug.h"

#define ENABLE_RESTART 1 // allow software reset


#ifdef CONFIG_GPIO_SIZE
//PIN_DEFAULT=0, PIN_INPUT, PIN_INPUT_PULLUP, PIN_OUTPUT, PIN_ERROR, PIN_READ, PIN_CLEAR, PIN_SET, PIN_TOGGLE

const char pin_state_args[] = "dipo ?01t";
#endif

const char help_parmKvs[]  =
    "'kvs' sets or gets key/value pairs\n\n"

;

#define ZONE_NAME_KEY "zn"
#define ZONE_NAME_KEY_LEN ((sizeof ZONE_NAME_KEY) - 1)

int ICACHE_FLASH_ATTR
process_parmKvs(clpar p[], int len) {
  int arg_idx;
  int errors = 0;
  so_msg_t so_key = SO_NONE;

  dbg_vpf(db_printf("process_parmKvs()\n"));

  so_output_message(SO_KVS_begin, NULL);

  bool pw_ok = strlen(C.app_configPassword) == 0;

  for (arg_idx = 1; arg_idx < len; ++arg_idx) {
    const char *key = p[arg_idx].key, *val = p[arg_idx].val;

    if (key == NULL || val == NULL) {  // don't allow any default values
      return reply_failure();

    } else if (strncmp(key, ZONE_NAME_KEY,ZONE_NAME_KEY_LEN) == 0) {
      //int zone_number = atoi(key + ZONE_NAME_KEY_LEN);
      if (*val == '?') {
        if (strlen(key) == ZONE_NAME_KEY_LEN) {
          so_output_message(SO_KVS_ZN_ALL, key);
        } else {
          so_output_message(SO_KVS_ZN_SINGLE, key);
        }
      } else {
        if (!kvs_store_string(key, val)) {
          ++errors;
        }
      }
    } else {
      ++errors;
      warning_unknown_option(key);
    }
  }

  so_output_message(SO_KVS_end, NULL);
  reply(errors==0);
  return 0;
}

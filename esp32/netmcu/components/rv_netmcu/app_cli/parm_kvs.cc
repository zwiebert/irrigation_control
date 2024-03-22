/*
 * cli_kvs.c
 *
 *  Created on: 12.06.2019
 *      Author: bertw
 */

#include "app_config/proj_app_cfg.h"
#include "app_uout/status_output.h"
#include "app_settings/config.h"
#include "app_misc/rtc.h"
#include <uout/cli_out.h>
#include "cli_imp.h"
#include "app_cli/cli_app.h"
#include "app_mqtt/mqtt.h"
#include "app_misc/kvstore.h"
#include "debug/log.h"
#include <ctype.h>
#include <string.h>

#define logtag "parm_kvs"
#define D(x) x

#define ENABLE_RESTART 1 // allow software reset

const char cli_help_parmKvs[] = "'kvs' sets or gets key/value pairs\n\n";

static const char *zoneKeysN[] = { "zn", "lph", 0 };

static bool match_zoneKeyN(const char *key) {
  for (int i = 0; zoneKeysN[i]; ++i) {
    if (key == strstr(key, zoneKeysN[i]))
      return true;
  }
  return false;
}

static bool match_kvsKey(const char *key) {
  return true; // XXX: allow any keys for now
}



int process_parmKvs(clpar p[], int len, class UoutWriter &td) {
  int arg_idx;
  int errors = 0;

  D(db_logi(logtag, "process_parmKvs()"));

  soMsg_KVS_begin(td);

  int needSecondPass = 0;

  // first pass: input parameters
  for (arg_idx = 1; arg_idx < len; ++arg_idx) {
    const char *key = p[arg_idx].key, *val = p[arg_idx].val;
    if (key == NULL || val == NULL) {  // don't allow any default values
      ++errors;
      continue;
    }

    if (*val == '?') {
      ++needSecondPass;
      continue; // handle this key/val pair in second pass
    }

    if (match_zoneKeyN(key)) {
      if (!kvs_store_string(key, val)) {
        ++errors;
      }
      continue;
    }

    if (match_kvsKey(key)) {
      if (!kvs_store_string(key, val)) {
        ++errors;
        continue;
      }
    }

  }

  // second pass: output parameters
  for (arg_idx = 1; arg_idx < len && needSecondPass > 0; ++arg_idx) {
    const char *key = p[arg_idx].key, *val = p[arg_idx].val;
    if (key == NULL || val == NULL)  // don't allow any default values
      continue;

    if (*val != '?')
      continue; // was handled in first pass

    --needSecondPass;

    if (match_zoneKeyN(key)) {
      if (!isdigit(key[strlen(key) - 1])) {

        soMsg_KVS_ZN_ALL(td, key);
      } else {
        soMsg_KVS_ZN_SINGLE(td, key);
      }
    }

    if (match_kvsKey(key)) {
      soMsg_KVS_ZN_SINGLE(td, key);
    }

  }

  soMsg_KVS_end(td);
  cli_replyResult(td, errors == 0);
  return 0;
}


#include "app_config/proj_app_cfg.h"

#include <string.h>


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


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifndef DISTRIBUTION
#define D(x) x
#else
#define D(x)
#endif

volatile bool cli_cmd_wait_for_response;

#define ONE_MINUTE (60)
#define ONE_HOUR (ONE_MINUTE * 60)
#define ONE_DAY (ONE_HOUR * 24)

const char cli_help_parmCmd[]  =
    "durN=[0-60]      activate zone N for up to 60 minutes (0=off). Example: cmd dur3=45;\n"
    "dur=?            request durations of all activated zones\n"
    "rem=?            request remaining times of all activated zones\n"
;

#define KEY_DURATION_PREFIX "dur"
#define KEY_DURATION_PREFIX_LEN ((sizeof KEY_DURATION_PREFIX) - 1)

#define RV_VERSION "rv-version"
#define CMD_ASK_VERSION "\"version\":\"?\","

#define BUF_SIZE 128
#define ZONE_COUNT 14
#define MAX_DURATION 60

int
process_parmCmd(clpar p[], int len) {
  int arg_idx;
  char buf[BUF_SIZE] = "{\"cmd\":{";
  int buf_idx = sizeof(buf);

  bool hasCmdLine = false;

  for (arg_idx = 1; arg_idx < len; ++arg_idx) {
    const char *key = p[arg_idx].key, *val = p[arg_idx].val;
    db_printf("key=%s, val=%s\n", key, val);

    if (key == NULL) {
      return -1;

    } else if (*val == '?') {
      if (strcmp(key, RV_VERSION) == 0) {
        strcat(buf, CMD_ASK_VERSION);
      } else {
        snprintf(buf + strlen(buf), sizeof buf - strlen(buf), "\"%s\":\"?\",", key);
      }
      hasCmdLine = true;

    } else if (strncmp(key, KEY_DURATION_PREFIX, KEY_DURATION_PREFIX_LEN) == 0) {
      int zone=-1, timer_number=0;
      sscanf((key + KEY_DURATION_PREFIX_LEN), "%d.%d", &zone, &timer_number);
      if (strchr(val, ',')) {
        float on = 0, off = 0;
        int repeats = 0;
        float period = 0;
        int dInterval = 0;
        float dhBegin = 0, dhEnd = 0;
        int ignoreRainSensor = 0;

        sscanf(val, "%f,%d,%f,%d,%f,%d,%f,%f", &on, &ignoreRainSensor, &off, &repeats, &period, &dInterval, &dhBegin, &dhEnd);

        buf_idx += sprintf(buf + strlen(buf), "\"dur%d.%d\":\"%d,%d,%d,%d,%d,%d,%d,%d\",", zone, timer_number, (int) (on * ONE_MINUTE), ignoreRainSensor, (int) (off * ONE_MINUTE), repeats,
            (int) (period * ONE_HOUR), dInterval, (int)(dhBegin * ONE_HOUR), (int)(dhEnd * ONE_HOUR));

        hasCmdLine = true;
      } else {
        float duration = 0;
        sscanf(val, "%f", &duration);
        if (0 <= zone && zone < ZONE_COUNT && 0 <= duration && duration <= MAX_DURATION) {
          buf_idx += sprintf(buf + strlen(buf), "\"dur%d.%d\":%d,", zone, timer_number, (int)(duration * 60));
          hasCmdLine = true;
        } else {
          db_printf("error: out of range");
        }
      }
    } else {
      cli_warning_optionUnknown(key);
    }
  }

  if (hasCmdLine) {
    buf[strlen(buf)-1] = '\0';
    strcat(buf, "}}\n");
    dbg_vpf(db_printf("cmd2stm32: <%s>\n", buf));
    if (stm32_mutexTake()) {
      stm32_write(buf, strlen(buf));
      stm32_mutexGive();
    }
  }

  return 0;
}





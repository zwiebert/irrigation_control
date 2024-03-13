/**
 * \file   app_main/esp32/main_loop_periodic.h
 * \brief  Run events in main task by setting event bits from periodic 100ms timer
 */
#include "app_config/proj_app_cfg.h"

#include "main.h"
#include <cli/cli.h>
#include "full_auto/weather.hh"

#include <app_settings/config.h>
#include "utils_misc/int_macros.h"
#include "utils_misc/int_types.h"
#include "utils_time/run_time.h"
#include "utils_time/ut_constants.hh"
#include "main_loop/main_queue.hh"
#include "net/ipnet.h"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/projdefs.h>
#include <freertos/timers.h>


//lfPerFlags mainLoop_PeriodicFlags;
/**
 * \brief Interval for periodic events. Will also restart MCU periodically to avoid memory fragmentation.
 * \param[in] loop_flags_periodic_100ms  global variable with loop_flagbits
 */
void tmr_loopPeriodic_start() {
  const int interval = pdMS_TO_TICKS(LOOP_PERIODIC_INTERVAL_MS);
  TimerHandle_t tmr = xTimerCreate("PerLoop100ms", interval, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
    static uint32_t count;
    ++count;


    mainLoop_callFun(lfPer100ms_mainFun);
#if 0
    if ((count & (BIT(7) - 1)) == 0) { // 12,8 secs
      app_safeMode_increment(true);
    }
#endif
    // forced daily reboot
    if ((count & (BIT(9) - 1)) == 0) { // 51,2 secs
      const time_t tnow = time(0);
      struct tm tms;
      if (localtime_r(&tnow, &tms)) { // tms is now valid


#ifdef CONFIG_APP_USE_WEATHER_AUTO
    static uint32_t last_poll;
    if (last_poll + (10 * 60 * 58) < count && tms.tm_min < 5) { // happens each 60 minutes
      if (fa_poll_weather_full_hour()) {
        last_poll = count;
      }
    }
#endif

        if (run_time_s() > SECS_PER_DAY)  {//

          if (tms.tm_hour == 23 && tms.tm_min >= 33)
              mainLoop_mcuRestart(0);  // XXX: restart every >=24 hours at 23:33

        }

      }





    }
  });
  if (!tmr || xTimerStart(tmr, 10 ) != pdPASS) {
    printf("PerLoopTimer start error");
  }
}

#include <string.h>
#include "app_config/proj_app_cfg.h"

#include "app/rtc.h"


#include <time.h>
#ifdef USE_POSIX_TIME
#include <sys/time.h>
#endif
#include "app_settings/config.h"
#include "app/common.h"

volatile time_t run_time_secs;

rtc_time_source_t rtc_last_time_source;


void set_system_time(time_t timestamp);


void  rtc_set_system_time(rtc_time_t stamp, rtc_time_source_t source) {
  rtc_last_time_source = source;
#ifdef USE_POSIX_TIME
  struct timeval tv = {.tv_sec = stamp, .tv_usec = 0 };
  settimeofday(&tv, NULL);
#else
  set_system_time(stamp);
#endif
}
rtc_time_t  rtc_time(void) {
  return time(NULL);
}


i8 
get_weekDay() {
  time_t timer = time(NULL);
  struct tm *tm = localtime(&timer);
  return tm->tm_wday;
}

i16 
get_yearDay() {
  time_t timer = time(NULL);
  struct tm *tm = localtime(&timer);
  return tm->tm_yday + 1;
}

int 
get_dst() {
  time_t timer = time(NULL);
  struct tm *tm = localtime(&timer);
  return tm->tm_isdst ? 1 : 0;
}

i16 
rtc_get_next_minute() {
  static int last_minute;

  time_t timer = time(NULL);
  struct tm *tm = localtime(&timer);
  if (last_minute == tm->tm_min)
    return -1;

  last_minute = tm->tm_min;
  return (i16)tm->tm_hour * 60 + tm->tm_min;
}


// 0123456789012345678
//"YYYY-MM-DDThh:mm:ss"
bool 
rtc_get_by_string(char *s) {
  time_t timer = time(NULL);
  struct tm t;
  localtime_r(&timer, &t);
#ifdef USE_POSIX_TIME
  strftime(s, 20, "%FT%H:%M:%S", &t);
#else
  isotime_r(&t, s);
  s[10] = 'T';
#endif
  return true;

}

time_t  time_iso2time(const char *dateTimeString) {
  if (strlen(dateTimeString) == 19) {
    char s[20], *token;

    strncpy(s, dateTimeString, 19);
    s[19] = '\0';
    struct tm t = { 0, };

    if ((token = strtok(s, "-"))) {
      t.tm_year = atoi(token) - 1900;
      if ((token = strtok(NULL, "-"))) {
        t.tm_mon = atoi(token) - 1;
        if ((token = strtok(NULL, "T "))) {
          t.tm_mday = atoi(token);
          if ((token = strtok(NULL, ":"))) {
            t.tm_hour = atoi(token);
            if ((token = strtok(NULL, ":"))) {
              t.tm_min = atoi(token);
              if ((token = strtok(NULL, "\0"))) {
                t.tm_sec = atoi(token);
                t.tm_isdst = -1;
                time_t timestamp = mktime(&t);
                return timestamp;
              }
            }
          }
        }
      }
    }
  }
  return -1;
}

bool 
rtc_set_by_string(const char *dateTimeString) {
  time_t timestamp = time_iso2time(dateTimeString);
  if (timestamp > 0) {
    rtc_set_system_time(timestamp, RTC_SRC_CLI); //FIXME: use real rtc_src
    return true;
  }
  return false;
}

i16 
eu_dst(const time_t *timer, i32 * z) {
  u32 t = *timer;
  if ((u8) (t >> 24) >= 194)
    t -= 3029443200U;
  t = (t + 655513200) / 604800 * 28;
  if ((u16) (t % 1461) < 856)
    return 3600;
  else
    return 0;
}

i16 
always_dst(const time_t *timer, i32 * z) {
  return 3600;
}

time_t rtc_timezone_in_secs() {
  time_t utc, local;
  utc = time(&local);
  localtime(&local);
  return local - utc;
}

void 
rtc_setup() {
  char buf[64];
  setenv("TZ", config_read_tz(buf, sizeof buf), 1);
  tzset();
}



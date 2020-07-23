#include "main.h"

EventGroupHandle_t loop_event_group;
#define EVT_BITS  ((1 << lf_Len) - 1)

#ifndef USE_EG
volatile uint32_t loop_flags;
#endif
uint32_t loop_flags_periodic;

typedef void (*lfa_funT)(void);

static const lfa_funT lfa_table[lf_Len] = {
    lfa_gotIpAddr, lfa_lostIpAddr,
#if defined USE_AP_FALLBACK || defined USE_WLAN_AP
    lfa_createWifiAp,
#endif
    lfa_syncStm32Time,
#ifdef USE_WDG
    ping_loop, watchDog_loop,
#endif
#ifdef USE_TCPS
    tcps_loop,
#endif
#ifndef USE_STM32COM
    stm32loop,
#endif
    cli_loop,
    lfa_mcuRestart,
};

#ifndef USE_EG
void loop_checkFlags() {
  u32 lf = loop_flags;
  for (int i = 0; lf; ++i, (lf >>= 1)) {
    if (!GET_BIT(lf, 0))
      continue;

    CLR_BIT(loop_flags, i);

    if (lfa_table[i])
      (lfa_table[i])();
  }
}
#else
void loop_eventBits_setup() {
  loop_event_group = xEventGroupCreate();
}
u32 loop_eventBits_wait() {
  EventBits_t bits = xEventGroupWaitBits(loop_event_group, EVT_BITS, pdTRUE, pdFALSE, portMAX_DELAY);
  return bits;
}

void loop_eventBits_check() {
  u32 lf = loop_eventBits_wait();
  for (int i = 0; lf; ++i, (lf >>= 1)) {
    if (!GET_BIT(lf, 0))
      continue;

    if (lfa_table[i])
      (lfa_table[i])();
  }
}
#endif

static void tmr_checkNetwork_cb(TimerHandle_t xTimer) {
  if (!ipnet_isConnected()) {
    lf_setBit(lf_createWifiAp);
  }
}

void tmr_checkNetwork_start() {
  TimerHandle_t tmr;
  int interval = pdMS_TO_TICKS(1000 * CHECK_NETWORK_INTERVAL);
  tmr = xTimerCreate("CheckNetworkTimer", interval, pdFALSE, (void*) lf_createWifiAp, tmr_checkNetwork_cb);
  if (xTimerStart(tmr, 10 ) != pdPASS) {
    printf("CheckNetworkTimer start error");
  }
}

static void tmr_pingLoop_cb(TimerHandle_t xTimer) {
  if (!wifi_ap_active) {
    lf_setBit(lf_pingLoop);
  }
}

void tmr_pingLoop_start() {
  TimerHandle_t tmr;
  int interval = pdMS_TO_TICKS(1000 * PING_INTERVAL);
  tmr = xTimerCreate("PingLoopTimer", interval, pdTRUE, (void*) lf_pingLoop, tmr_pingLoop_cb);
  if (xTimerStart(tmr, 10 ) != pdPASS) {
    printf("PingLoopTimer start error");
  }
}

static void tmr_loopPeriodic_cb(TimerHandle_t xTimer) {
  lf_setBits(loop_flags_periodic);
}

void tmr_loopPeriodic_start() {
  TimerHandle_t tmr;
  int interval = pdMS_TO_TICKS(LOOP_PERIODIC_INTERVAL_MS);
  tmr = xTimerCreate("PingLoopTimer", interval, pdTRUE, (void*) 0, tmr_loopPeriodic_cb);
  if (xTimerStart(tmr, 10 ) != pdPASS) {
    printf("PingLoopTimer start error");
  }
}

void stm32loop() {
  if (xSemaphoreTakeRecursive(uart_mutex, 0)) {
    stm32_checkForInput();
    xSemaphoreGiveRecursive(uart_mutex);
  }
}

void loop(void) {
#ifdef USE_EG
  loop_eventBits_check();
#else
  loop_checkFlags();
#endif
}


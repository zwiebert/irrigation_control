/*
 * watch_dog.cc
 *
 *  Created on: 04.06.2019
 *      Author: bertw
 */
#include "user_config.h"
#include <time.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/memorymap.h>
#include "real_time_clock.h"

#include "watch_dog.hh"
#include "peri/uart.h"

static unsigned call_time, response_time;
static unsigned missed_responses;

#define ESP32_RESET_PIN_PORT GPIOB
#define ESP32_RESET_PIN GPIO9

static void watchDog_gotResponse() {
	response_time = runTime();
}

static void watchDog_sendCall() {
	esp32_write(WDG_CALL, WDG_CALL_LEN);
	call_time = runTime();
	response_time = 0;
}

static void watchDog_resetEsp32() {
	gpio_clear(ESP32_RESET_PIN_PORT, ESP32_RESET_PIN);
	for (unsigned long i = 0; i < 4500; ++i) {
		__asm__("nop");
	}
	gpio_set(ESP32_RESET_PIN_PORT, ESP32_RESET_PIN);
}

void watchDog_loop() {

	if (call_time + WDG_INTERVAL < runTime()) {
		watchDog_sendCall();
		return;
	}

	if (call_time <= response_time && response_time <= call_time + WDG_RESPONSE_DELAY) {
		missed_responses = 0;
		return; // response received in time
	}

	if (call_time + WDG_RESPONSE_DELAY < runTime()) {
		if (WDG_MAX_MISSED_RESPONSES < ++missed_responses) {
			response_time = call_time = runTime();
			missed_responses = 0;
			watchDog_resetEsp32();
			return;
		}
		watchDog_sendCall();
	}


}



bool watchDog_checkCommandLine(const char *cmdLine) {
	if (strcmp (cmdLine, WDG_RESPONSE) == 0) {
		watchDog_gotResponse();
		return true;
	}
	return false;
}

void watchDog_setup() {
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set(ESP32_RESET_PIN_PORT, ESP32_RESET_PIN);
	gpio_set_mode(ESP32_RESET_PIN_PORT, GPIO_MODE_OUTPUT_2_MHZ,
			GPIO_CNF_OUTPUT_OPENDRAIN, ESP32_RESET_PIN);

}
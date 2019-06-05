/*
 * app.cc
 *
 *  Created on: 04.06.2019
 *      Author: bertw
 */

#include "user_config.h"
#include "rv_timer.hh"
#include "app_cxx.h"
#include "watch_dog.hh"

extern "C" void app_switch_valve(int valve_number, bool state);

RvTimers rvt = RvTimers(app_switch_valve);

extern "C" void cxx_loop() {
	rvt.loop();
#ifdef USE_WDG
	watchDog_loop();
#endif
}

extern "C" void cxx_setup() {
#ifdef USE_WDG
	watchDog_setup();
#endif
	// rvt.set(0,2,4,3,60)->run();
	// rvt.set(1,3,5,4,130)->run();
	// rvt.set(2,3)->run();
	//rvt.set(6, 30, 0, 0, 60 * 2)->run();
	//rvt.set(7, 30, 0, 0, 60 * 2)->run();
}

#ifndef RV_TIMER_HH
#define RV_TIMER_HH

#include <rain_sensor.hh>
#include <app_cxx.hh>
#include "user_config.h"
#include "misc/int_macros.h"
#include "real_time_clock.h"
#include <time.h>
#include <stdio.h>
#ifdef MOD_TEST
#include <iostream>
volatile time_t start_time;
volatile int x;
void breaker() {
  // start_time = time(0);
}

#endif

const int RV_TIMER_COUNT = 20;
const int RV_VALVE_COUNT = 12;
const int RV_MAX_LPH = 1200;

extern int Lph[RV_VALVE_COUNT];

typedef void (*switch_valve_cb)(int valve_number, bool state);
typedef void (*switch_valves_cb)(uint16_t valve_bits, uint16_t valve_mask);

class RvTimer;
class RvTimers;

class RvTimer {
  friend class RvTimers;
public:
  struct SetArgs {
    int on_duration = 0, off_duration = 0, repeats = 0, period = 0;
    int mDaysInterval = 0, mTodSpanBegin = 0, mTodSpanEnd = 0;
    int mIgnoreRainSensor = false;
    char *toJSON(char *buf, int buf_size) {
      if (0 <= snprintf(buf, buf_size, "{\"d1\":%d,\"ir\":%d,\"d0\":%d,\"r\":%d,\"per\":%d,\"di\":%d,\"sb\":%d,\"se\":%d}",
          on_duration, mIgnoreRainSensor, off_duration, repeats, period,
          mDaysInterval, mTodSpanBegin, mTodSpanEnd))
        return buf;

      return 0;
    }
  };

public:

  RvTimer *pred = 0, *succ = 0;
  void unlinkThis() {
    if (pred)
      pred->succ = succ;
    if (succ)
      succ->pred = pred;
  }
  RvTimer *getNext() {
    return succ->mValveNumber == -2 ? 0 : succ;
  }
private:
  int mValveNumber = -1;
  int mTimerNumber = 0; // multiple time per valve with different numbers

  SetArgs mArgs;

  bool isRunOnce() {
    return mArgs.on_duration && !mArgs.off_duration && !mArgs.repeats && !mArgs.period;
  }
  bool hasTodSpan() {
    return mArgs.mTodSpanBegin || mArgs.mTodSpanEnd;
  }

  time_t mNextRun = 0, mNextOnOff = 0;
  time_t mLastRun = 0;

  bool isDisabledByInterval(time_t now = time(0)) {

    if (mLastRun && mArgs.mDaysInterval && mLastRun + mArgs.mDaysInterval * ONE_DAY < now)
      return true;

    return false;
  }

  bool isDisabledByRain() {
    if (mArgs.mIgnoreRainSensor)
      return false;
    if (rs.getState())
      return true; // raining now or was raining (adjust sensor directly)

    return false;
  }

  bool isDisabledByTimeOfDay(time_t now = time(0)) {
    if (mArgs.mTodSpanBegin == mArgs.mTodSpanEnd)
      return false;

    struct tm *tm_now = localtime(&now);
    int dsecs = tm_now->tm_hour * ONE_HOUR + tm_now->tm_min * ONE_MINUTE + tm_now->tm_sec;
    if (!(mArgs.mTodSpanBegin <= dsecs && dsecs <= mArgs.mTodSpanEnd))
      return true;

    return false;
  }

  bool mIsRunning = false, mIsOn = false;
  int mDoneOn = 0;

  switch_valve_cb mSwitchValveCb = 0;
public:
  RvTimer(bool isListHead = false) {
    if (isListHead) {
      mValveNumber = -2;
      this->pred = this->succ = this;
    }
  }
  char *argsToJSON(char *buf, int buf_size) {
    if (0 <= snprintf(buf + strlen(buf), buf_size - strlen(buf), "\"timer%d.%d\":", getValveNumber(), getTimerNumber())) {
      return mArgs.toJSON(buf + strlen(buf), buf_size - strlen(buf));
    }
    return 0;
  }
  bool isListHead() {
    return mValveNumber == -2;
  }

  int getValveNumber() {
    return mValveNumber;
  }
  int getTimerNumber() {
    return mTimerNumber;
  }
  bool match(int valve_number, int id = 0) {
    return valve_number == mValveNumber && id == mTimerNumber;
  }

  void register_callback(switch_valve_cb cb, int valve_number) {
    mSwitchValveCb = cb;
    mValveNumber = valve_number;
  }

  void switch_valve(bool state) {
    if (mSwitchValveCb) {
      mSwitchValveCb(mValveNumber, state);
    }
  }

  void run() {
    mNextOnOff = time(0);
    mIsRunning = true;
    mDoneOn = 0;
  }

  void scheduleRun() {
    mNextRun = time(0);
  }

  void stop();

  void pause();

  int get_duration() {
    return mArgs.on_duration;
  }
  int get_remaining() {
    return isOn() ? mNextOnOff - time(0) : mArgs.on_duration;
  }

  void set(SetArgs &args, int id) {
    mTimerNumber = id;
    mArgs = args;
    if (rs.getState() && !(args.period || args.mDaysInterval || args.mIgnoreRainSensor)) {
      mArgs.on_duration = 0; // force timer to stop
    }
  }

  void set(int on_duration, int id) {
    SetArgs args;
    args.on_duration = on_duration;
    set(args, id);
  }

  bool isReadyToRun() {
    time_t now = time(0);
    if (mIsRunning)
      return false;

    if (mArgs.on_duration == 0)
      return true; // run this 0-timer, so it can be stopped and removed from list

    if (!(mNextRun && mNextRun < now))
      return false;
    if (isDisabledByRain())
      return false;
    if (isDisabledByInterval())
      return false;
    if (isDisabledByTimeOfDay())
      return false;

    return true;
   // return !mIsRunning && mNextRun && mNextRun < now && !isDisabledByRain() && !isDisabledByInterval(now) && !isDisabledByTimeOfDay(now);
  }
  bool isReadyToOnOff() {
    return mIsRunning && mNextOnOff && mNextOnOff < time(0);
  }
  bool shouldStopBecauseRain() {
    if (mArgs.mIgnoreRainSensor)
      return false;
    if (!(mIsRunning && !mIsOn))
      return false;
    if (!rs.getState())
      return false;
    return true;
  }

  bool isDone() {
    return !mIsRunning && mDoneOn >= mArgs.repeats && !mNextRun;
  }
  bool isRunning() {
    return mIsRunning;
  }
  bool isOn() {
    return mIsRunning && mIsOn;
  }
  bool isOff() {
    return mIsRunning && !mIsOn;
  }

  void changeOnOff();

  bool checkState() {
    if (isReadyToRun())
      return true;
    if (isReadyToOnOff())
      return true;
    if (shouldStopBecauseRain())
      return true;

    return false;
  }
};

class RvTimers {
  static uint16_t valve_bits, valve_mask;
  static void switch_valve_2(int valve_number, bool state) {
    SET_BIT(valve_mask, valve_number);
    if (state)
      SET_BIT(valve_bits, valve_number);
  }
private:
  int lph = 0; // XXX: don't count timers for the same valve more than once

  class Timers {
  private:
    RvTimer mTimers[RV_TIMER_COUNT];
  public:
    RvTimer mFreeTimers, mUsedTimers;
    Timers() :
        mFreeTimers(true), mUsedTimers(true) {

      for (int i = 0; i < RV_TIMER_COUNT; ++i) {
        timer_to_list(&mFreeTimers, &mTimers[i]);
      }
    }

    void timer_to_list(RvTimer *list, RvTimer *timer);
  } mRvTimers;

  switch_valve_cb mSvCb; // XXX
  switch_valves_cb mSvsCb;

public:

  RvTimers(switch_valve_cb cb, switch_valves_cb cb2) :
      mSvCb(cb), mSvsCb(cb2) {

  }

  void register_callback(switch_valve_cb cb) {
    mSvCb = cb; // XXX
  }

  RvTimer *set(RvTimer::SetArgs &args, int valve_number, int id) {
    RvTimer *timer = 0;
    for (RvTimer *t = mRvTimers.mUsedTimers.getNext(); t; t = t->getNext()) {
      if (t->match(valve_number, id)) {
        if (t->isOn())
          lph -= Lph[valve_number];

        t->stop();

        timer = t;
        break;
      }
    }

    if (!timer) {
      timer = mRvTimers.mFreeTimers.getNext();
      if (timer)
        mRvTimers.timer_to_list(&mRvTimers.mUsedTimers, timer);
    }

    if (timer) {
      timer->register_callback((mSvCb ? mSvCb : switch_valve_2), valve_number);
      timer->set(args, id);
      return timer;
    }
    return 0;
  }

  RvTimer *set(int valve_number, int on_duration, int id = 0) {
    RvTimer::SetArgs args;
    args.on_duration = on_duration;
    return set(args, valve_number, id);
  }

  void unset(int valve_number, int id) {
    for (RvTimer *t = mRvTimers.mUsedTimers.getNext(); t; t = t->getNext()) {
      if (t->match(valve_number, id)) {
        mRvTimers.timer_to_list(&mRvTimers.mFreeTimers, t);
      }
    }
  }

  void loop();

public:
  RvTimer *getTimerList() {
    return &mRvTimers.mUsedTimers;
  }
};

#ifdef MOD_TEST
#include <stdlib.h>
#include <iostream>

void switch_valve(int valve_number, bool state) {
  std::cout << (time(0) - start_time) << ": valve " << valve_number << " switched " << (state ? "on" : "off") << std::endl;
  std::cout.flush();
}

extern "C" void app_switch_valve(int valve_number, bool state);

int main() {
  start_time = time(0);
  RvTimers rvt = RvTimers(switch_valve);

  // rvt.set(0,2,4,3,60)->run();
  // rvt.set(1,3,5,4,130)->run();
  // rvt.set(2,3)->run();

  rvt.set(6, 30, 0, 0, 60*2)->run();
  rvt.set(7, 30, 0, 0, 60*2)->run();
  while (1) {
    rvt.loop();
    //sleep(100);
  }
}

#endif

#endif
// Local Variables:
// compile-command: "/c/MinGW/bin/g++ -ggdb rv_timer.cc"
// End:

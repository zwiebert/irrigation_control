/**
 * \file   rv/rv.hh
 * \brief  Settings and forward declarations of the RV app
 */

#pragma once


class RvTimerPause;
class RvTimerData;
class RvTimer;
class RvTimers;
class RainSensor;
class RvZone;
class RvZones;

extern RvTimers rvt;
extern RainSensor rs;
extern RvZones rvz;

//int getLph(int valve_number);
//bool setLph(int valve_number, int lph);

inline constexpr int PAUSE_SECS_PER_LITER = 8;
inline constexpr int PAUSE_AFTER_LITER = 100;
inline constexpr int PAUSE_LPH_THRESHOLD = 100;
inline constexpr int ZONE_LPH_DEFAULT = 1000;

inline constexpr int RV_TIMER_COUNT = 20;
inline constexpr int RV_VALVE_COUNT = 14;
inline constexpr int RV_MAX_LPH = 1600;

/***********************************************************************
 * Filename: time_ctrl.cpp
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * 
 ***********************************************************************/

#include "time_ctrl.h"

RTC_DATA_ATTR bool TimeCtrl::sunriseFlag = false;
RTC_DATA_ATTR bool TimeCtrl::sunsetFlag = false;
RTC_DATA_ATTR time_t TimeCtrl::lastEvent = 0;

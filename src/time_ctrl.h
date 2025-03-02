/***********************************************************************
 * Filename: time_ctrl.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines the TimeCtrl class that controls automatic opening and
 *     closing of a feeder based on sunrise and sunset times.
 * 
 ***********************************************************************/


#pragma once

#include "Arduino.h"
#include "parameters.h"
#include "feeder_ctrl.h"
#include "deep_sleep_ctrl.h"

#define TIME_CONTROL_TASK_PERIOD_MS 10000

class TimeCtrl
{
private:
    static RTC_DATA_ATTR bool sunriseFlag;
    static RTC_DATA_ATTR bool sunsetFlag;
    static RTC_DATA_ATTR time_t lastEvent;

public:
    static void Init(void)
    {
        active_tasks[Time_Task] = true;
        SetTimezone(PopisCasu.Get().c_str());
        PosledniCasOtevreni.Set(PosledniCasOtevreni_S.Get());
        PosledniCasZavreni.Set(PosledniCasZavreni_S.Get());
    }

    static void Task(void)
    {
        time_t currentTime = Now();
        time_t sunriseTime = CasVychodu.Get();
        time_t sunsetTime = CasZapadu.Get();
        if (sunriseTime == 0 && sunsetTime == 0)
        {
            active_tasks[Time_Task] = false;
            return;
        }

        active_tasks[Time_Task] = true;

        time_t adjustedSunriseTime = sunriseTime + ZpozdeniOtevreni.Get() * 60;
        time_t adjustedSunsetTime = sunsetTime + ZpozdeniZavreni.Get() * 60;

        if (adjustedSunriseTime < currentTime && adjustedSunsetTime < currentTime && sunriseTime != 0 && sunsetTime != 0)
        {
            if (adjustedSunriseTime < adjustedSunsetTime)
            {
                CasVychodu.Set(sunriseTime + 24 * 60 * 60);
                adjustedSunriseTime += 24 * 60 * 60;
            }
            else
            {
                CasZapadu.Set(sunsetTime + 24 * 60 * 60);
                adjustedSunsetTime += 24 * 60 * 60;
            }
        }

        if (sunriseTime != 0)
        {
            CasOtevreni.Set(adjustedSunriseTime);
        }

        if (sunsetTime != 0)
        {
            CasZavreni.Set(adjustedSunsetTime);
        }

        if (sunsetTime != 0 && currentTime >= adjustedSunsetTime && !sunsetFlag)
        {
            if ((lastEvent + 60 * 60) < currentTime || currentTime < lastEvent)
            {
                sunsetFlag = true;
                sunriseFlag = false;
                if (AutomatikaZavreni.Get())
                {
                    FeederCtrl::EventSourced(ev_close, povel_casem);
                    lastEvent = currentTime;
                }
            }
        }
        else if (sunriseTime != 0 && currentTime >= adjustedSunriseTime && !sunriseFlag)
        {
            if ((lastEvent + 60 * 60) < currentTime || currentTime < lastEvent)
            {
                sunriseFlag = true;
                sunsetFlag = false;

                if (AutomatikaOtevreni.Get())
                {
                    FeederCtrl::EventSourced(ev_open, povel_casem);
                    lastEvent = currentTime;
                }
            }
        }

        active_tasks[Time_Task] = false;
    }
};
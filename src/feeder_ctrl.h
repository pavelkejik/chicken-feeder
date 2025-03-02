/***********************************************************************
 * Filename: feeder_ctrl.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Declares the FeederCtrl class, which serves as the core of the 
 *     finite state machine for feeder control.
 *
 ***********************************************************************/

#pragma once

#include "common.h"
#include "parameters.h"
#include "freertos/FreeRTOS.h"
#include <mutex>
#include "debounce.h"
#include "deep_sleep_ctrl.h"

#define FEEDER_CONTROL_TASK_PERIOD_MS 100
#define MAX_OPEN_CLOSE_TIME_S 100

#define NOSTATE nmr_states

typedef enum
{
    ev_stop = 0,
    ev_open = 1,
    ev_close = 2,
    ev_overload_open = 3,
    ev_overload_close = 4,
    ev_no_current = 5,
    ev_timer_expired = 6,
    nmr_events = 7,
    no_event = 8,
    first_event = 0
} FeederEvents_t;

typedef FeederState_t (*state_fun_t)(void);

class FeederCtrl
{
private:
    static std::atomic<uint32_t> events;
    static const state_fun_t state_fun[nmr_states][nmr_events];
    static uint32_t timer;
    static void execute(void);

public:
    static void Init(void);
    static void Task(void)
    {
        execute();
    }
    static void Event(FeederEvents_t event)
    {
        // std::lock_guard<std::mutex> lock(events_mutex);
        active_tasks[Position_Task] = true;
        events |= 1 << event;
    }

    static void EventSourced(FeederEvents_t event, FeederCmdSource_t source)
    {
        // std::lock_guard<std::mutex> lock(events_mutex);
        active_tasks[Position_Task] = true;
        events |= 1 << event;
        PovelOd.Set(source);
    }

    static bool IsState(FeederState_t state);
    static void SetState(FeederState_t state);
    static void TimerStart(uint32_t secs);
    static void TimerStop();
};

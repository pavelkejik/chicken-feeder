/***********************************************************************
 * Filename: feeder_ctrl.cpp
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines event processing functions for the finite state machine 
 *     in the FeederCtrl class.
 *
 ***********************************************************************/

#include <feeder_ctrl.h>
#include "led.h"
#include "motor.h"
#include "log.h"
#include "error.h"
#include "deep_sleep_ctrl.h"
#include "weight.h"

#define TIME_SCHEDULE(_t_secs) ((uint32_t)((_t_secs) * 1000 / FEEDER_CONTROL_TASK_PERIOD_MS))

std::atomic<uint32_t> FeederCtrl::events;
uint32_t FeederCtrl::timer;

extern Motor motor;
extern Weight weight;

const char *const cmd_source_txt[] = {
    "",
    "(Tlacitkem)",
    "(Casem)",
    "(ESP-NOW)",
    "(Doplneni krmiva)",
    ""};

FeederState_t noaction()
{
    return NOSTATE;
}

FeederState_t feeder_error()
{
    SystemLog::PutLog("Chyba krmitka", v_error);
    motor.Stop();
    FeederCtrl::TimerStop();
    return ChybaKrmitka;
}

FeederState_t closed()
{
    SystemLog::PutLog("Zavreno", v_info);
    motor.Stop();
    error_close.Clear();
    PosledniCasZavreni_S.Set(Now());
    FeederCtrl::TimerStop();
    return Zavreno;
}

FeederState_t opened()
{
    SystemLog::PutLog("Otevreno", v_info);
    motor.Stop();
    error_open.Clear();
    PosledniCasOtevreni_S.Set(Now());
    FeederCtrl::TimerStop();
    weight.RunMeasure();
    return Otevreno;
}

FeederState_t check_open()
{
    motor.Close();
    FeederCtrl::TimerStart(2);
    return OvereniOtevreni;
}

FeederState_t check_open_current()
{
    if (AktualniProud_mA.Get() > ZERO_CURRENT_THRESHOLD_mV)
    {
        return opened();
    }
    else
    {
        return feeder_error();
    }
}

FeederState_t check_close()
{
    motor.Open();
    FeederCtrl::TimerStart(1);
    return OvereniZavreni;
}

FeederState_t check_close_current()
{

    if (AktualniProud_mA.Get() > ZERO_CURRENT_THRESHOLD_mV)
    {
        return closed();
    }
    else
    {
        return feeder_error();
    }
}

FeederState_t stop()
{
    SystemLog::PutLog("Stop", v_info);

    motor.StopFast();
FeederCtrl::TimerStop();
    return NeznaznamaPoloha;
}

FeederState_t break_close()
{
    // SystemLog::PutLog("Stop", v_info);

    motor.Stop();

    FeederCtrl::TimerStart(3);

    return StopZavirani;
}

FeederState_t break_open()
{
    motor.Stop();

    FeederCtrl::TimerStart(2);

    return StopOtevirani;
}

FeederState_t repeat_open()
{
    SystemLog::PutLog("Opakovani otevreni", v_info);
    motor.Open();
    FeederCtrl::TimerStart(MAX_OPEN_CLOSE_TIME_S);
    return Otevirani;
}

FeederState_t repeat_close()
{
    SystemLog::PutLog("Opakovani zavreni", v_info);
    motor.Close();
    FeederCtrl::TimerStart(MAX_OPEN_CLOSE_TIME_S);
    return Zavirani;
}

FeederState_t unable_open()
{
    motor.Stop();

    if (error_open.Check(true))
    {
        return feeder_error();
    }

    delay(500);
    FeederCtrl::TimerStart(3);
    motor.Close();
    return UvolneniOtevirani;
}

FeederState_t to_close()
{
    SystemLog::PutLog("Zavirani " + String(cmd_source_txt[PovelOd.Get()]), v_info);
    motor.Close();
    FeederCtrl::TimerStart(MAX_OPEN_CLOSE_TIME_S);
    return Zavirani;
}

FeederState_t unable_close()
{
    motor.Stop();

    if (error_close.Check(true))
    {
        return feeder_error();
    }

    delay(500);
    FeederCtrl::TimerStart(3);
    motor.Open();
    return UvolneniZavirani;
}

FeederState_t to_open()
{
    SystemLog::PutLog("Otevirani " + String(cmd_source_txt[PovelOd.Get()]), v_info);
    motor.Open();
    FeederCtrl::TimerStart(MAX_OPEN_CLOSE_TIME_S);
    return Otevirani;
}

//                     Stop      Otevrit    Zavrit    pretizeni otev     pretizeni zav    bez proudu      time expired
#undef State
#define State(_state_, _ev_stop, _ev_open, _ev_close, _ev_overload_up, _ev_overload_down, _ev_no_current, _ev_timer_expired) \
    {_ev_stop, _ev_open, _ev_close, _ev_overload_up, _ev_overload_down, _ev_no_current, _ev_timer_expired},

const state_fun_t FeederCtrl::state_fun[nmr_states][nmr_events] =
    {
#include "feeder_ctrl_table.h"
};

void FeederCtrl::execute()
{
    if (EndTimer(timer))
    {
        Event(ev_timer_expired);
    }

    // std::lock_guard<std::mutex> lock(events_mutex);
    if (events != 0)
    {
        for (int i = first_event; i < nmr_events; i++)
        {
            uint32_t event_mask = 1 << i;
            if (events & event_mask)
            {
                FeederState_t state = state_fun[StavKrmitka.Get()][i]();
                if (state != NOSTATE)
                {
                    SetState(state);
                }
                events &= ~event_mask;
            }
        }
    }
    else
    {
        if (motor.IsStopped() && !IsTimerRunning(timer))
        {
            active_tasks[Position_Task] = false;
        }
    }
}

void FeederCtrl::Init(void)
{
    active_tasks[Position_Task] = true;
}

bool FeederCtrl::IsState(FeederState_t state)
{
    return (FeederState_t)StavKrmitka.Get() == state;
}
void FeederCtrl::SetState(FeederState_t state)
{
    StavKrmitka.Set(state);
}

void FeederCtrl::TimerStart(uint32_t secs)
{
    StartTimer(timer, TIME_SCHEDULE(secs));
}

void FeederCtrl::TimerStop()
{
    StopTimer(timer);
}
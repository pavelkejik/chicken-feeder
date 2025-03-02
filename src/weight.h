/***********************************************************************
 * Filename: weight.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines the Weight class that performs weight measurements for
 *     monitoring the feed level and manages automatic refilling. It uses
 *     a HX711-like load cell to measure the weight and triggers appropriate
 *     events based on weight thresholds and calibration.
 ***********************************************************************/


#pragma once

#include "Arduino.h"
#include "pin_map.h"
#include "parameters.h"
#include "deep_sleep_ctrl.h"
#include "log.h"
#include "debounce.h"
#include "feeder_ctrl.h"

#define Channel_A_gain_128 1
#define Channel_B_gain_32 2
#define Channel_A_gain_64 3

#define cyclespermicro 160
#define microcycles(n) (n * cyclespermicro)

class Weight
{
private:
    static RTC_DATA_ATTR uint32_t weightCnt;

    void ClkPulse(void)
    {
        uint32_t startCounter, counter, cpu_cycles;
        int cyclediff, totalcycles, corrected;

        startCounter = ESP.getCycleCount();
        totalcycles = startCounter + microcycles(2);

        portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
        taskENTER_CRITICAL(&myMutex);

        digitalWrite(PDCLK, HIGH);

        while (ESP.getCycleCount() < totalcycles)
        {
            __asm__ __volatile__("nop");
        }
        digitalWrite(PDCLK, LOW);
        taskEXIT_CRITICAL(&myMutex);
    }

    int32_t doConversion(uint8_t mode)
    {
        byte index;
        uint32_t conv_res = 0L;

        while (digitalRead(PDO))
        {
            delay(1);
        }

        for (uint8_t i = 0; i < 24; i++)
        {
            ClkPulse();
            conv_res = (conv_res << 1) | digitalRead(PDO);
            delayMicroseconds(10);
        }
        for (uint8_t i = 0; i < mode; i++)
        {
            ClkPulse();
            delayMicroseconds(10);
        }

        if (conv_res >= 0x800000)
            conv_res = conv_res | 0xFF000000L;

        return (int32_t)conv_res;
    }

public:
    void Init(void)
    {
        gpio_hold_dis((gpio_num_t)PDCLK);
        FillingUpdate();
        if (StavKrmitka.Get() == Otevreno)
        {
            if (AktualniVaha_proc.Get() < UrovenDoplneni_proc.Get())
            {
                weightCnt += PeriodaKomunikace_S.Get();
            }
            else if (weightCnt >= PeriodaKomunikace_S.Get())
            {
                weightCnt -= PeriodaKomunikace_S.Get();
            }
        }
        else
        {
            weightCnt = 0;
        }
    }

    int32_t Measure(void)
    {
        Wake();
        doConversion(Channel_A_gain_64);

        int32_t curr = 0;
        for (int i = 0; i < 2; i++)
        {
            delay(10);
            curr += doConversion(Channel_A_gain_64);
        }
        curr /= 2;
        Sleep();

        Serial.printf("Aktualni vaha: %d\n", curr);

        return curr;
    }

    void FillingUpdate(void)
    {
        int32_t full = VahaPlne.Get();
        int32_t empty = VahaPrazdne.Get();
        if ((full > 0) && (empty > 0) && (full > empty))
        {
            if (StavKrmitka.Get() == Otevreno)
            {
                int32_t delta_x = (AktualniVaha.Get() - empty) * 100;
                delta_x /= (full - empty);
                delta_x = (delta_x < 0) ? 0 : (delta_x > 100) ? 100
                                                              : delta_x;
                AktualniVaha_proc.Set(delta_x);
            }
        }
        else
        {
            AktualniVaha_proc.Set(0);
        }
    }

    void Task(void)
    {
        bool empty_tmp = KalibracePrazdne.Get() == kalibrace_proved;
        bool full_tmp = KalibracePlne.Get() == kalibrace_proved;

        if (empty_tmp && full_tmp)
        {
            empty_tmp = false;
            full_tmp = false;
            KalibracePlne.Set(kalibrace_neni);
            KalibracePrazdne.Set(kalibrace_neni);
        }

        if ((empty_tmp || full_tmp) && (StavKrmitka.Get() != Otevreno))
        {
            empty_tmp = false;
            full_tmp = false;
            KalibracePlne.Set(kalibrace_neni);
            KalibracePrazdne.Set(kalibrace_neni);
        }

        int32_t tmp = Measure();
        AktualniVaha.Set(tmp);

        if (empty_tmp)
        {
            SystemLog::PutLog("Kalibrace prazdneho krmitka");
            VahaPrazdne.Set(tmp);
            KalibracePrazdne.Set(kalibrace_provedena);
        }
        else if (full_tmp)
        {
            SystemLog::PutLog("Kalibrace plneho krmitka");
            VahaPlne.Set(tmp);
            KalibracePlne.Set(kalibrace_provedena);
        }
        FillingUpdate();

        if (((weightCnt / 60) >= CasProDoplneni_M.Get()) && (StavKrmitka.Get() == Otevreno))
        {
            if (AutomatikaDoplnovani.Get())
            {
                FeederCtrl::EventSourced(ev_close, povel_vaha);
            }
            weightCnt = 0;
        }
        if ((StavKrmitka.Get() == Zavreno) && (PovelOd.Get() == povel_vaha))
        {
            FeederCtrl::EventSourced(ev_open, povel_vaha);
        }
    }

    void RunMeasure(void)
    {
        active_tasks[Write_Command_Task] = true;
        xSemaphoreGive(write_cmd_sem);
    }

    void Sleep(void)
    {
        digitalWrite(PDCLK, LOW);
        digitalWrite(PDCLK, HIGH);
    }

    void Wake(void)
    {
        pinMode(PDCLK, OUTPUT);
        digitalWrite(PDCLK, HIGH);
        digitalWrite(PDCLK, LOW);
        pinMode(PDO, INPUT);
        delay(2);
        while (digitalRead(PDO))
        {
            delay(1);
        }
    }
};
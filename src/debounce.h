/***********************************************************************
 * Filename: debounce.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines the Debounce class to filter out noise in digital signals.
 *
 ***********************************************************************/

#pragma once
#include "Arduino.h"

#define CNT_MAX_DEFAULT 100

class Debounce
{
private:
    uint16_t max;
    const uint8_t inc;
    const uint8_t dec;
    int16_t cnt;
    bool matured;
    bool matured_event;

public:
    Debounce(uint8_t incr = 1, uint8_t decr = 1, uint16_t maxv = CNT_MAX_DEFAULT) : inc(incr), dec(decr), cnt(0), max(maxv), matured(false), matured_event(false){};

    bool Inp(bool in)
    {
        if (in)
        {
            cnt += inc;

            if (cnt >= max)
            {
                cnt = max;
                if (!matured)
                {
                    matured_event = true;
                }
                matured = true;
            }
        }
        else
        {
            cnt -= dec;

            if (cnt <= 0)
            {
                cnt = 0;
                matured = false;
            }
        }
        return matured;
    }

    bool Out()
    {
        return matured;
    }

    void SetMax(uint16_t maxv)
    {
        if (max != maxv)
        {
            max = maxv;
            if (max <= cnt)
            {
                cnt = max;
            }
        }
    }

    void Reset()
    {
        matured = false;
        matured_event = false;
        cnt = 0;
    }

    bool InpEvent(bool in)
    {
        Inp(in);
        bool ret = matured_event;
        matured_event = false;
        return ret;
    }

    uint16_t GetMaturity(void)
    {
        return ((cnt * 100) / max);
    }

    uint16_t GetCnt(void)
    {
        return max - cnt;
    }
};
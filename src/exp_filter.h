/***********************************************************************
 * Filename: exp_filter.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Declares the ExpFilter class, which implements an exponential 
 *     filter for smoothing input values.
 *
 ***********************************************************************/

#pragma once
#include "Arduino.h"

class ExpFilter
{
private:
    int32_t val;
    const int32_t coef;

public:
    ExpFilter(int32_t coeficient = 16) : coef(coeficient), val(INT32_MAX){};

    int16_t Input(int32_t curr_val)
    {
        if (val == INT32_MAX)
        {
            val = curr_val * coef;
        }
        else
        {
            val += (curr_val * coef - val);
        }
        if (val > (INT16_MAX * coef))
        {
            val = INT16_MAX * coef;
        }
        else if (val < (INT16_MIN * coef))
        {
            val = INT16_MIN * coef;
        }

        return (int16_t)(val / coef);
    }

    int16_t Out(void)
    {
        return (int16_t)(val / coef);
    }

    void Reset(void)
    {
        val = INT32_MAX;
    }
};
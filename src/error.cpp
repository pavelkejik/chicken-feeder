/***********************************************************************
 * Filename: error.cpp
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Implements the Error class and instantiate Error class objects.
 *
 ***********************************************************************/

#include "error.h"
#include "parameter_values.h"
#include "Preferences.h"
#include "parameters.h"
#include "feeder_ctrl.h"

#undef ERROR
#define ERROR(_name, _debup, _debdown, _blocked_fun, _clearing_by, _txt, _error_code, _verbose, _event) \
    Error _name(_debup, _debdown, _blocked_fun, _clearing_by, _txt, _error_code, _verbose, _event);

#include "error_table.h"

#undef ERROR

#define ERROR(_name, _debup, _debdown, _blocked_fun, _clearing_by, _txt, _error_code, _verbose, _event) &_name,

Error *const Error::err_inst[] = {
#include "error_table.h"
};

size_t Error::nmr_inst = sizeof(Error::err_inst) / sizeof(Error *);

uint8_t Error::block_active;

#undef ERROR

bool Error::IsActive(void)
{
    return deb.Out();
}

void Error::SetErrCode(void)
{
    if (!ChybovyKod.Get())
    {
        ChybovyKod.Set(error_code);
        LogHistory.Set(error_code);
    }
}

void Error::ClearErrCode(void)
{
    if (ChybovyKod.Get() == error_code)
    {
        ChybovyKod.Set(0);
    }
}

bool Error::Check(bool val)
{
    bool isAct = IsActive();
    bool noErrorClr = isAct && ((clearing & SELF_CLEAR) == 0);
    if (!noErrorClr)
    {
        if (deb.Inp(val))
        {
            if (!isAct)
            {
                block_active |= block;
                if (verb != v_empty)
                {
                    SystemLog::PutLog("[Kod: " + String(error_code) + "] " + String(txt), verb);
                }
                FeederCtrl::Event(event);
                SetErrCode();
            }
        }
        else if ((clearing & SELF_CLEAR) != 0)
        {
            RestoreError();
            ClearErrCode();
        }
    }
    return deb.Out();
}

void Error::RestoreError()
{
    uint8_t block_active_new = 0;
    for (int i = 0; i < nmr_inst; i++)
    {
        if (err_inst[i]->IsActive())
        {
            block_active_new |= err_inst[i]->block;
        }
    }
    block_active = block_active_new;
}

void Error::Clear(void)
{
    deb.Reset();
    ClearErrCode();
}

void Error::ClearAll(void)
{
}

uint8_t Error::ClearAction(uint8_t event)
{
    uint8_t block_active_new = 0;

    if (IsActive())
    {
        if (event & clearing)
        {
            Clear();
        }
        else
        {
            block_active_new = block;
            SetErrCode();
        }
    }
    else
    {
        ClearErrCode();
    }
    return block_active_new;
}

void Error::ClearActionAll(uint8_t event)
{
    for (int i = 0; i < nmr_inst; i++)
    {
        err_inst[i]->ClearAction(event);
    }
    RestoreError();
}

bool Error::IsBlocked(uint8_t action)
{
    return (action & block_active) != 0;
}
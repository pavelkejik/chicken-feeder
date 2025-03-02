/***********************************************************************
 * Filename: error.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Declares the Error class, which provides methods for managing 
 *     error states and events in the system.
 *
 ***********************************************************************/

#pragma once

#include "debounce.h"
#include "parameters.h"
#include "log.h"
#include "time.h"
#include "feeder_ctrl.h"


enum
{
    MOVE_UP = 1,
    MOVE_DOWN = 2,
    MOVE_CMD = 4,
    SELF_CLEAR = 8,
    MOVE_CTRL_BLOCK = 16,
};

class Error
{
private:
    Debounce deb;
    const uint8_t block;
    const uint8_t clearing;
    static Error *const err_inst[];
    static size_t nmr_inst;
    const char *txt;
    static uint8_t block_active;
    const ErrorState_t error_code;
    const Verbosity_t verb;
    const FeederEvents_t event;

public:
    Error(uint8_t debup, uint8_t debdown, uint8_t blck, uint8_t clear, const char *err_txt, ErrorState_t err_code, Verbosity_t verbose, FeederEvents_t ev) : deb(debup, debdown), block(blck), clearing(clear), txt(err_txt), error_code(err_code), verb(verbose), event(ev){};

    bool IsActive(void);

    void SetErrCode(void);

    void ClearErrCode(void);

    bool Check(bool val);

    static void RestoreError();

    void Clear(void);

    static void ClearAll(void);

    uint8_t ClearAction(uint8_t event);

    static void ClearActionAll(uint8_t event);

    static bool IsBlocked(uint8_t action);
};

#undef ERROR

#define ERROR(_name,_debup,_debdown,_blocked_fun, _clearing_by, _txt, _error_code, _verbose, _event) extern Error _name;
#include "error_table.h"
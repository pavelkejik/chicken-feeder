/***********************************************************************
 * Filename: parameter_values.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines constants, enumerations, and flags for parameter values used
 *     throughout the application.
 *
 ***********************************************************************/

#pragma once

#define MAIN_REVISION 21

#define FLAGS_NONE 0
#define BOOL_FLAG 0x01
#define COMM_PERIOD_FLAG 0x02
#define CHART_FLAG 0x04
#define FW_VERSION_FLAG 0x08

#define COMMAND_FLAG 0x10
#define STATE_FLAG 0x20


#define ERR_HISTORY_CNT 16

typedef enum
{
	PAR_U16 = 1,
	PAR_S16 = 2,
	PAR_U32 = 3,
	PAR_S32 = 4,
	PAR_STRING = 5,
} ParType_t;

typedef enum
{
	Manualni = 0,
	Casem = 1
} AutoOvladani_t;

typedef enum
{
	Uzivatel = 0,
	Servis = 1,
} ActiveAccess_t;

typedef enum
{
	NormalniMod = 0,
	Parovani = 1,
	Vybrano = 2,
	Sparovano = 3,
} DeviceState_t;

typedef enum
{
	NeznaznamaPoloha = 0,
	Otevirani = 1,
	Otevreno = 2,
	UvolneniOtevirani = 3,
	OvereniOtevreni = 4,

	Zavirani = 5,
	Zavreno = 6,
	UvolneniZavirani = 7,
	OvereniZavreni = 8,

	StopZavirani = 9,
	StopOtevirani = 10,

	ChybaKrmitka = 11,
	nmr_states
} FeederState_t;

typedef enum
{
	NeniChyba = 0,
	NadProudOtev = 1,
	NadProudZav = 2,
	NeniProud = 3,
	ChybaZavirani = 4,
	ChybaOtevirani = 5,
	MAX_ERROR = 100
} ErrorState_t;

typedef enum
{
	v_empty = 0,
	v_error = 1,
	v_warning = 2,
	v_info = 3,
} Verbosity_t;

typedef enum
{
	vypnuto = 0,
	povoleno = 1,
} AutoControl_t;

// Odpovida FeederEvents_t
typedef enum
{
	manual_stop = 0,
	manual_otevrit = 1,
	manual_zavrit = 2,
} FeederEventsManual_t;

typedef enum
{
	kalibrace_neni = 0,
	kalibrace_provedena = 1,
	kalibrace_proved = 2,
} TareState_t;

typedef enum
{
	povel_neni = 0,
	povel_tlacitkem = 1,
	povel_casem = 2,
	povel_espnow = 3,
	povel_vaha = 4,
	povel_neznamy = 5
} FeederCmdSource_t;

typedef enum
{
	rst_Unknown = 0,
	rst_Software = 1,
	rst_Watchdog = 2,
	rst_Brownout = 3,
	rst_Poweron = 4,
	rst_External = 5,
	rst_Deepsleep = 6,

} ResetReason_t;
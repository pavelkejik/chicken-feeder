
/****************************************************************************
 * Filename: parameters_table.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     This header file contains the definitions for all parameters and
 *     registers used throughout the application. It defines a structured way
 *     to declare system parameters that are essential for operational control,
 *     configuration, and status monitoring of various components.
 *
 *     The parameters are defined using the following macros that encapsulate
 *     their properties and behavioral characteristics:
 *     - DefPar_Ram: Define a volatile parameter stored in RAM.
 *     - DefPar_Nv: Define a non-volatile parameter stored in persistent memory.
 *     - DefPar_Fun: Define a custom parameter type derived from the Register class.
 *
 *     In this table use following macros:
 *     - DefPar_Ram(_alias_,_regadr_,_def_,_min_, _max_,_type_,_dir_,_lvl_,_atr_)
 *     - DefPar_Nv (_alias_,_regadr_,_def_,_min_, _max_,_type_,_dir_,_lvl_,_atr_)
 *     - DefPar_Fun(_alias_,_regadr_,_def_,_min_, _max_,_type_,_dir_,_lvl_,_atr_,_fun_)
 * 
 * 
 *     Parameters are defined with attributes:
 *     - _alias_: Name of the parameter instance derived from the Register class.
 *     - _regadr_: Modbus register address. Parameters can occupy mutliple registers.
 *     - _def_: Default value, overwritten by stored value if valid for non-volatile types.
 *     - _min_, _max_: Define the permissible range of values.
 *     - _type_: Data type of the parameter, e.g., S16_, U16_, S32_, U32_, STRING_.
 *     - _dir_: Access direction (Par_R for read-only, Par_RW for read/write, Par_W for write-only).
 *     - _lvl_: Access level required to modify the parameter:
 *         - Par_Public: Accessible without privileges.
 *         - Par_Installer: Requires installer-level privileges.
 *         - Par_ESPNow: Accessible over ESP-Now, suitable for sharing definitions/values.
 *     - _atr_: Additional user-defined attributes or flags.
 *     - _fun_: Specific parameter type derived from the Register class (used with DefPar_Fun).
 ****************************************************************************/

#ifdef PAR_DEF_INCLUDES

#include "error.h"
#include "log.h"
#include "motor.h"
#include "feeder_ctrl.h"
#undef PAR_DEF_INCLUDES

#else /*PAR_DEF_INCLUDES*/


/*
-----------------------------------------------------------------------------------------------------------
  @ Stav krmitka

-----------------------------------------------------------------------------------------------------------
*/
DefPar_Ram( StavZarizeni,  1,     NormalniMod,    NormalniMod ,     Sparovano, U16_,   Par_R  ,    Par_Public,    FLAGS_NONE )
DefPar_RTC( StavKrmitka,  2,     NeznaznamaPoloha,     NeznaznamaPoloha,     ChybaKrmitka, U16_,   Par_R  ,    Par_Public | Par_ESPNow,    STATE_FLAG )
DefPar_Fun( ManualniOvladani, 3,  manual_stop,   manual_stop,    manual_zavrit, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    COMMAND_FLAG, event_reg)
DefPar_Ram( KalibracePrazdne, 4,  kalibrace_neni,   kalibrace_neni,    kalibrace_proved, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE)
DefPar_Ram( KalibracePlne, 5,  kalibrace_neni,   kalibrace_neni,    kalibrace_proved, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE)

DefPar_Ram( ChybovyKod, 6,  NeniChyba,     NeniChyba,    MAX_ERROR, U16_,   Par_R  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )
DefPar_RTC( AktualniVaha, 7,  0,     -1000000,    10000000, S32_,   Par_R  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )
DefPar_RTC( AktualniVaha_proc, 9,  0,     5,    200, U16_,   Par_R  ,    Par_Public | Par_ESPNow,    CHART_FLAG )

DefPar_Fun( PosledniCasOtevreni, 10,  0,    0,   6 , STRING_,   Par_R  ,    Par_Public | Par_ESPNow,    FLAGS_NONE, time_reg)
DefPar_Fun( PosledniCasZavreni, 13,  0,    0,   6 , STRING_,   Par_R  ,    Par_Public | Par_ESPNow,    FLAGS_NONE, time_reg)

DefPar_Fun( CasOtevreni, 16,  0,    0,   6 , STRING_,   Par_R  ,    Par_Public | Par_ESPNow,    FLAGS_NONE, time_reg)
DefPar_Fun( CasZavreni, 19,  0,    0,   6 , STRING_,   Par_R  ,    Par_Public | Par_ESPNow,    FLAGS_NONE, time_reg)

DefPar_RTC( NapetiBaterie_mV, 22,  0,    5,   300, U16_,   Par_R  ,    Par_Public | Par_ESPNow,    CHART_FLAG)

// DefPar_Ram( LedDimming,  20,     127,     0,     255, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )


DefPar_Ram( AktualniProud_mA, 100,  0,     0,    2000, S16_,   Par_R  ,    Par_Public,    FLAGS_NONE )
DefPar_RTC( PovelOd, 101,  povel_neni,   povel_neni,    povel_neznamy, U16_,   Par_R  ,    Par_Public,    FLAGS_NONE)
DefPar_RTC( CasVychodu, 102,  0,    0,   0 , S32_,   Par_R  ,    Par_Public,    FLAGS_NONE)
DefPar_RTC( CasZapadu, 104,  0,    0,   0 , S32_,   Par_R  ,    Par_Public,    FLAGS_NONE)
DefPar_RTC( PosledniCasOtevreni_S, 106,  0,    0,   0 , S32_,   Par_R  ,    Par_Public,    FLAGS_NONE)
DefPar_RTC( PosledniCasZavreni_S, 108,  0,    0,   0 , S32_,   Par_R  ,    Par_Public,    FLAGS_NONE)
/*
-----------------------------------------------------------------------------------------------------------
  @ Konfigurace

-----------------------------------------------------------------------------------------------------------
*/
DefPar_Nv( AutomatikaOtevreni,   23,  povoleno, vypnuto,povoleno, U16_,   Par_RW  ,   Par_Public | Par_ESPNow, BOOL_FLAG)
DefPar_Nv( AutomatikaZavreni,   24,  povoleno, vypnuto,povoleno, U16_,   Par_RW  ,   Par_Public | Par_ESPNow, BOOL_FLAG)
DefPar_Nv( ZpozdeniOtevreni, 25,  0,    -180,    180, S16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )
DefPar_Nv( ZpozdeniZavreni, 26,  0,    -180,    180, S16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )

DefPar_Nv( MotorMaxProud_mA, 27,  90,    10,    1000, S16_,   Par_RW  ,    Par_Installer | Par_ESPNow,    FLAGS_NONE )

DefPar_Nv( VahaPrazdne, 28,  0,    0,    1000000L, S32_,   Par_RW  ,    Par_Installer | Par_ESPNow,    FLAGS_NONE )
DefPar_Nv( VahaPlne, 30,  0,    0,    2000000L, S32_,   Par_RW  ,    Par_Installer | Par_ESPNow,    FLAGS_NONE )

DefPar_Nv( UrovenDoplneni_proc, 32,  40,     0,    100, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )
DefPar_Nv( AutomatikaDoplnovani,   33,  povoleno, vypnuto,povoleno, U16_,   Par_RW  ,   Par_Public | Par_ESPNow, BOOL_FLAG)
DefPar_Nv( CasProDoplneni_M, 34,  5,    2,    180, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    FLAGS_NONE )


/*
-----------------------------------------------------------------------------------------------------------
  @ ESP-NOW pripojeni

-----------------------------------------------------------------------------------------------------------
*/
DefPar_Nv( PeriodaKomunikace_S, 35,  10,    2,    3600, U16_,   Par_RW  ,    Par_Public | Par_ESPNow,    COMM_PERIOD_FLAG)
DefPar_Fun( MasterMacAdresa, 200,  255,    0,    0, U16_,   Par_RW  ,    Par_Installer,    FLAGS_NONE, mac_reg_nv)
DefPar_RTC( WiFiKanal, 203,  1,     1,    13, U16_,   Par_R,    Par_Public,    FLAGS_NONE )

/*
-----------------------------------------------------------------------------------------------------------
  @ Datum a cas

-----------------------------------------------------------------------------------------------------------
*/
DefPar_Nv( PopisCasu, 300,  0,    0,    46, STRING_,   Par_RW  ,    Par_Public,    FLAGS_NONE )
DefPar_Ram( AktualniCas, 323,  0,    0,    20, STRING_,   Par_R  ,    Par_Public,    FLAGS_NONE )

/*
-----------------------------------------------------------------------------------------------------------
  @ Zaznamy

-----------------------------------------------------------------------------------------------------------
*/

DefPar_Fun(LogHistory,   400,  NeniChyba, NeniChyba,MAX_ERROR, U16_,   Par_R  ,   Par_Public, FLAGS_NONE,log_reg)


// -----------------------------------------------------------------------------------------------------------
//   @ System info
// -----------------------------------------------------------------------------------------------------------
// */

DefPar_Ram(VerzeFW, 36,MAIN_REVISION, MAIN_REVISION,	UINT16_MAX,U16_ ,Par_R, Par_Public | Par_ESPNow,FW_VERSION_FLAG)
DefPar_Ram(RestartCmd, 37,vypnuto, vypnuto,	povoleno,U16_ ,Par_RW, Par_Installer | Par_ESPNow,BOOL_FLAG)

DefPar_Ram(CompDate, 1001,0, 0,	30,STRING_ ,Par_R, Par_Public,FLAGS_NONE)
DefPar_Ram(ResetReason, 1016,rst_Poweron, rst_Unknown,	rst_Deepsleep,U16_ ,Par_R, Par_Public,FLAGS_NONE)

#endif /*PAR_DEF_INCLUDES*/

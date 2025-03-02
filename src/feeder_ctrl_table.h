/***********************************************************************
 * Filename: feeder_ctrl_table.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines the finite state machine for feeder control, using a macro 
 *     that associates each state with the appropriate actions for various 
 *     events such as stopping, opening, closing, detecting overloads,
 *     and handling errors.
 *
 ***********************************************************************/

//                            Stop            Otevrit         Zavrit            pretizeni otev     pretizeni zav     bez proudu        time expired
State(NeznaznamaPoloha,      noaction,       to_open,         to_close,         noaction,          noaction,         noaction,         noaction)
State(Otevirani,             stop,           noaction,        break_open,       unable_open,       noaction,         check_open,       unable_open)
State(Otevreno,              noaction,       noaction,        to_close,         noaction,          noaction,         noaction,         noaction)
State(UvolneniOtevirani,     stop,           noaction,        break_open,       noaction,          repeat_open,      repeat_open,      repeat_open)
State(OvereniOtevreni,       noaction,       noaction,        break_open,       noaction,          noaction,         noaction,         check_open_current)



State(Zavirani,              stop,           break_close,     noaction,         noaction,          unable_close,     check_close,      unable_close)
State(Zavreno,               noaction,       to_open,         noaction,         noaction,          noaction,         noaction,         noaction)
State(UvolneniZavirani,      stop,           break_close,     noaction,         repeat_close,      noaction,         repeat_close,     repeat_close)
State(OvereniZavreni,        noaction,       break_close,     noaction,         noaction,          noaction,         noaction,         check_close_current)


State(StopZavirani,          noaction,       noaction,        to_close,         noaction,          noaction,         noaction,         to_open)
State(StopOtevirani,         noaction,       to_open,         noaction,         noaction,          noaction,         noaction,         to_close)

State(ChybaKrmitka,          stop,           stop,            stop,             noaction,          noaction,         noaction,         noaction)
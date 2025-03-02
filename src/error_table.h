/***********************************************************************
 * Filename: error_table.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Defines a set of errors using the ERROR macro, which includes 
 *     information about debounce timing, blocking functions, clearing 
 *     conditions, descriptions, error codes, verbosity levels, and 
 *     corresponding events.
 *
 ***********************************************************************/

// ERROR(_name,_debup,_debdown,_blocked_fun, _clearing_by, _txt, _error_code, _verbose, _event)
ERROR(ovrl_open,5,20,MOVE_UP | MOVE_CTRL_BLOCK, MOVE_UP | MOVE_DOWN, "Nadproud otevirani",NadProudOtev, v_warning, ev_overload_open)
ERROR(ovrl_close,5,20,MOVE_DOWN | MOVE_CTRL_BLOCK,  MOVE_UP | MOVE_DOWN, "Nadproud zavirani",NadProudZav, v_warning, ev_overload_close)
ERROR(no_current,5,100,MOVE_DOWN | MOVE_UP, MOVE_UP | MOVE_DOWN | SELF_CLEAR, "Zadny proud", NeniProud, v_warning, ev_no_current)
ERROR(error_close,25,100, MOVE_CTRL_BLOCK, MOVE_UP | MOVE_DOWN, "Nelze zavrit", ChybaZavirani, v_error, no_event)
ERROR(error_open,25,100, MOVE_CTRL_BLOCK, MOVE_UP | MOVE_DOWN, "Nelze otevrit", ChybaOtevirani, v_error, no_event)
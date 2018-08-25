/* ***************************************************************************
 *
 *       Filename:  hsignal.h
 *    Description:  logger
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#ifndef _HSIGNAL_H
#define _HSIGNAL_H 1

#include <hcommon.h>
#include <hlog.h>


typedef  e_error_code_t  (*pf_reload_t)(void *p_arg);
typedef  e_error_code_t  (*pf_term_t)  (void *p_arg);


typedef struct _st_signal_handle_t
{
    /*   RELOAD PROCESS  FUNCTION   */
    void            *p_rarg;
    pf_reload_t      pf_reload;

    /*   TERM PROCESS FUNCTION      */
    void            *p_targ;
    pf_term_t        pf_term;
} st_signal_handle_t;
typedef st_signal_handle_t *pst_signal_handle_t;


/* ------------------------------------------------------------------------
 *   log  handler  API
 * ------------------------------------------------------------------------ */
e_error_code_t
init_signal      (pst_signal_handle_t   *pp_handle,
                  pf_reload_t   p_func1, void *p_arg1,
                  pf_term_t     p_func2, void *p_arg2);
e_error_code_t
destroy_signal   (pst_signal_handle_t   *pp_handle);

#endif

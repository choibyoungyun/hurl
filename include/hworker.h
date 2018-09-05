/* ***************************************************************************
 *
 *       Filename:  hworker.h
 *    Description:  worker
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#ifndef _HWORKER_H
#define _HWORKER_H  1

#include <hcommon.h>
#include <hlog.h>
#include <hsignal.h>
#include <hutil.h>
#include <heigw.h>
#include <hcurl.h>
#include <hfcgi.h>
#include <hauth.h>


/* ------------------------------------------------------------------------
 *  worker handler
 * ------------------------------------------------------------------------ */
#define WORKER_CONFIG_DEFAULT_DELAY     "1000"  /* 1000 mili-second     */
#define WORKER_CONFIG_DEFAULT_INTERVAL  "100"   /* 100  mili-second     */

#define WORKER_CONFIG_SECTION_NAME      "WORKER"
#define WORKER_CONFIG_DELAY_NAME        "DELAY"
#define WORKER_CONFIG_INTERVAL_NAME     "INTERVAL"


typedef struct _st_worker_handle_t *pst_worker_handle_t;
typedef struct _st_worker_handle_t
{
    char                    cfname   [DEFAULT_STRING_BUF_LEN];
    char                    csection [DEFAULT_STRING_BUF_LEN];

    int                     interval;
    int                     delay;

    e_error_code_t          (*pf_config)(pst_worker_handle_t);
    void                    (*pf_show)  (pst_worker_handle_t, char *);
} st_worker_handle_t;


typedef struct _st_process_handle_t *pst_process_handle_t;
typedef struct _st_process_handle_t
{
    e_bool_t                stop_flag;
    e_bool_t                reload_flag;
    e_bool_t                standby_flag;


    /*   process name           */
    char                    pname  [DEFAULT_STRING_BUF_LEN];
    int                     pindex;

    /* configuration file       */
    char                    cfname [DEFAULT_STRING_BUF_LEN];

    pst_worker_handle_t     p_worker;
    pst_signal_handle_t     p_sig;
    pst_log_handle_t        p_trace;

    /*  client required handle  */
    pst_auth_handle_t       p_auth;
    pst_http_handle_t       p_http;
    pst_eigw_handle_t       p_eigw;

    /*  server required handle  */
    pst_fcgi_handle_t       p_fcgi;

    e_error_code_t          (*pf_period)   (struct _st_process_handle_t *);
    e_error_code_t          (*pf_job)      (struct _st_process_handle_t *);

    e_error_code_t          (*pf_config)   (struct _st_process_handle_t *);
    e_error_code_t          (*pf_active)   (struct _st_process_handle_t *);
    e_error_code_t          (*pf_inactive) (struct _st_process_handle_t *);
} st_process_handle_t;


/* ------------------------------------------------------------------------
 *   process handler API
 * ------------------------------------------------------------------------ */
e_error_code_t
init_process         (pst_process_handle_t p_cb);
e_error_code_t
destroy_process      (pst_process_handle_t p_cb);


e_error_code_t
init_worker          (pst_worker_handle_t *pp_cb,
                      char                *p_cfname,
                      char                *p_csection);
e_error_code_t
destroy_worker       (pst_worker_handle_t *pp_cb);
e_error_code_t
do_worker            (pst_process_handle_t p_cb);

e_error_code_t
init_haf             (pst_process_handle_t p_cb);
void
notify_haf           (void);

#endif

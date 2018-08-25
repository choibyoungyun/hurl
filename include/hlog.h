/* ***************************************************************************
 *
 *       Filename:  hlog.h
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


#ifndef _HLOG_H
#define _HLOG_H 1

#include <hcommon.h>
#include <hutil.h>

/* ------------------------------------------------------------------------
 *  LOG LEVEL  REDEFINE  (Gobal Type: __endLogLevel
 * ------------------------------------------------------------------------ */
typedef enum _e_log_level_t
{
    DEBUG_FORCE     = LOG_F,
    DEBUG_CRITICAL  = LOG_1,
    DEBUG_ERROR     = LOG_2,
    DEBUG_NORMAL    = LOG_3,
    DEBUG_INFO      = LOG_4,
    DEBUG_LOW       = LOG_5
} e_log_level_t;


/* ------------------------------------------------------------------------
 *  log control block
 * ------------------------------------------------------------------------ */
#define LOG_CONFIG_SECTION_NAME     "LOG"
#define LOG_CONFIG_LEVEL_NAME       "LEVEL"
#define LOG_CONFIG_SIZE_NAME        "SIZE"
#define LOG_CONFIG_FILECNT_NAME     "FILECNT"
#define LOG_CONFIG_DIR_NAME         "DIR"
#define LOG_CONFIG_PREFIX_NAME      "PREFIX"

#define LOG_DEFAULT_SIZE            "100"
#define LOG_DEFAULT_LEVEL           "5"
#define LOG_DEFAULT_FILECNT         "10"


typedef struct _st_log_handle_t *pst_log_handle_t;
typedef struct _st_log_handle_t
{
    char            cfname   [DEFAULT_STRING_BUF_LEN];
    char            csection [DEFAULT_STRING_BUF_LEN];

    int             level;
    int             size;
    int             max_num;

    char            dname   [DEFAULT_STRING_BUF_LEN];
    char            prefix  [DEFAULT_STRING_BUF_LEN];

    e_error_code_t  (*pf_config)(pst_log_handle_t);
    void            (*pf_show)  (pst_log_handle_t, char *);
} st_log_handle_t;


/* ------------------------------------------------------------------------
 *   log  handler  API
 * ------------------------------------------------------------------------ */
e_error_code_t
init_logger      (pst_log_handle_t  *pp_handle,
                  char              *p_name,
                  char              *p_cfname);
e_error_code_t
destroy_logger   (pst_log_handle_t  *pp_handle);

#endif

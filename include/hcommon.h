/* ***************************************************************************
 *
 *       Filename:  hcommon.h
 *    Description:  common macro & definion
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _HCOMMON_H
#define _HCOMMON_H  1

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <time.h>
#include <libgen.h>
#include <iconv.h>
#include <errno.h>
#include <stddef.h>


#include <cmn/def.h>
#include <cmn/msg.h>
#include <cmn/lib/core/base/base.h>
#include <cmn/lib/core/signal/signal.h>
#include <cmn/lib/util/cfg/cfg.h>
#include <cmn/lib/util/log/log.h>


#ifndef PKG_BASE_VARIABLE
 #define PKG_BASE_VARIABLE           "PKG_ROOT"
#endif

#ifndef DEFAULT_STRING_BUF_LEN
 #define DEFAULT_STRING_BUF_LEN      256
#endif

#ifndef FNAME_STRING_BUF_LEN
 #define FNAME_STRING_BUF_LEN        DEFAULT_STRING_BUF_LEN
#endif


/* ------------------------------------------------------------------------
 *  macro defintion for function sigle point return.
 * ------------------------------------------------------------------------ */

#ifndef try_exception
    #define try_exception(condition, label)  if ( (condition) ) { goto label; }
#endif

#ifndef try_goto
    #define try_goto(label)                  { goto label; }
#endif

#ifndef try_catch
    #define try_catch(label)                 goto  try_finally_label;  label:
#endif

#ifndef try_catch_through
    #define try_catch_through(label)         label:
#endif

#ifndef try_finally
    #define try_finally                      try_finally_label:
#endif

#ifndef try_assert
    #define try_assert(condition) { \
            if ((condition)) \
            { \
                abort(); \
             } \
     }
#endif


/* ------------------------------------------------------------------------
 * user defined memory
 * ------------------------------------------------------------------------ */
#ifdef _USER_MALLOC
 #define MALLOC(x)
 #define CALLOC(x,y)
 #define REALLOC(x,y)
 #define FREE(x)
#else
 #define MALLOC(x)    malloc((size_t)(x))
 #define CALLOC(x,y)  calloc((x),(y))
 #define REALLOC(x,y) realloc((x),(y))
 #define FREE(x)      {if (x) {free((void *)(x)); x=NULL;}}
#endif


#ifdef _DEBUG
 #define HTTP_LOG(...)  { \
     fprintf(stdout,__VA_ARGS__); fflush (stdout); \
 }
#else
 #define HTTP_LOG(...)
#endif


/* ------------------------------------------------------------------------
 *  for preventing unused variable error
 * ------------------------------------------------------------------------ */
#ifndef UNUSED
 #define UNUSED(expr)    (void)(expr);
#endif



/* ------------------------------------------------------------------------
 * error code definition
 * ------------------------------------------------------------------------ */
typedef enum _e_error_code_t
{
    /* APP  ERROR  */
    E_FAILURE               =  -1,
    E_SUCCESS               =  SUCC,
    E_DELAY_JOB,
    E_IMMEDIATE_JOB,
    E_INVALID_ARG,
    E_INVALID_CONFIG,
    E_ALLOC_HANDLE,
    E_ENV_NOTFOUND,
    E_TIMEOUT,
    E_BUSY,
    E_CALLBACK_FUNCTION,
    E_SOCK_DISCONNECT,

    E_FILE_NOTFOUND         = 100,
    E_FILE_OPEN,
    E_FILE_WRITE,
    E_FILE_READ,
    E_FILE_MMAP,

    /* SQL  ERROR  */
    E_SQL_DISCONNECT        = 200,
    E_SQL_NOT_FOUND,            /* NOT FOUND  RECORD            */
    E_SQL_DUP,                  /* DUPLICATION RECORD           */
    E_SQL_NO_RESOURCE,          /* Alloc Handle                 */
    E_SQL_MANY_COLUMN,          /* INSUFFICIENT COLUMNS ARRAY   */
    E_SQL_REMAIN_RESULT,        /* INSUFFICIENT COLUMNS ARRAY   */


    /* APPLICATION(CURL or EIGW) PROTOCOAL    */
    E_PROTOCOL_HTTP_INTERNAL = 900,
    E_PROTOCOL_HTTP_SETOPT,
    E_PROTOCOL_SPEC,
    E_PROTOCOL_INVALID_HEADER,
    E_PROTOCOL_INVALID_HEADER_FRAME,
    E_PROTOCOL_INVALID_HEADER_LENGTH,
    E_PROTOCOL_INVALID_HEADER_TYPE,
    E_PROTOCOL_INVALID_HEADER_NAME,
    E_PROTOCOL_INVALID_HEADER_CLIENTID,
    E_PROTOCOL_INVALID_BODY,
    E_PROTOCOL_INVALID_BODY_URI,
    E_PROTOCOL_INVALID_BODY_PROTOCOL,
    E_PROTOCOL_INVALID_BODY_METHOD,
    E_PROTOCOL_TOO_BIG_RESPONSE
} e_error_code_t;
typedef e_error_code_t  e_http_error_code_t;
typedef e_error_code_t  *pe_http_error_code_t;


/* ------------------------------------------------------------------------
 * ON/OFF flag definition
 * ------------------------------------------------------------------------ */
typedef enum _e_bool_t
{
    BOOL_TRUE   = 1,
    BOOL_FALSE  = 0
} e_bool_t;
typedef _e_bool_t       e_http_bool_t;
typedef e_http_bool_t   *pe_http_bool_t;


/* ------------------------------------------------------------------------
 * HTTP STATUS CODE
 * ------------------------------------------------------------------------ */
typedef enum _e_http_result_state_t
{
    HTTP_RESULT_NOK     = -1,
    HTTP_RESULT_UNKNOWN = 0,
    HTTP_RESULT_OK      = 200
} e_http_result_state_t;

#endif

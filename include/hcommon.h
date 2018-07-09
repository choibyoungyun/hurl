/* ------------------------------------------------------------------------
 *  fname       : hcommon.h
 *  version     :
 *  date        :
 *  author      :
 *  description :
 * ------------------------------------------------------------------------ */

#ifndef _HTTP_COMMON_H_
#define _HTTP_COMMON_H_

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


#ifdef _JDIN_APP
#include <cmn/def.h>
#include <cmn/msg.h>
#include <cmn/lib/core/base/base.h>
#include <cmn/lib/core/signal/signal.h>
#include <cmn/lib/util/cfg/cfg.h>
#include <cmn/lib/util/log/log.h>


#define PKG_BASE_VARIABLE           "PKG_ROOT"
#define DEFAULT_STRING_BUF_LEN      256
#endif

/* ************************************************************************
 *  macro defintion for function sigle point return.
 **************************************************************************/

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


/* ************************************************************************
 * define memeory management subsystem
 **************************************************************************/
#ifdef _USER_MALLOC
 #define MALLOC(x)
 #define CALLOC(x,y)
 #define REALLOC(x,y)
 #define FREE(x)
#else
 #define MALLOC(x)    malloc((x))
 #define CALLOC(x,y)  calloc((x),(y))
 #define REALLOC(x,y) realloc((x),(y))
 #define FREE(x)      free((void *)(x))
#endif


#ifdef _DEBUG
    #define HTTP_LOG(...)  {\
    fprintf(stdout,__VA_ARGS__); fflush (stdout); \
}
#else
 #include <cmn/lib/util/log/log.h>
 #define    HTTP_LOG(...)  Log(DEBUG_INFO,__VA_ARGS__)
#endif


/*  for preventing unused variable error */
#define UNUSED(expr)    do {(void)(expr);} while(0);


/* ************************************************************************
 * error code definition
 **************************************************************************/
typedef enum _e_error_code_t
{
    /* APP  ERROR  */
    E_FAILURE       =  -1,
    E_SUCCESS       =  0,
    E_DELAY_JOB,
    E_IMMEDIATE_JOB,
    E_INVALID_ARG,
    E_ALLOC_HANDLE,
    E_ENV_NOTFOUND,
    E_TIMEOUT,
    E_BUSY,
    E_CALLBACK_FUNCTION,
    E_SOCK_DISCONNECT,

    E_FILE_NOTFOUND = 100,
    E_FILE_OPEN,
    E_FILE_WRITE,
    E_FILE_READ,
    E_FILE_MMAP,

    /* SQL  ERROR  */
    E_SQL_DISCONNECT = 1000,
    E_SQL_NOT_FOUND,            /* NOT FOUND  RECORD            */
    E_SQL_DUP,                  /* DUPLICATION RECORD           */
    E_SQL_NO_RESOURCE,          /* Alloc Handle                 */
    E_SQL_MANY_COLUMN,          /* INSUFFICIENT COLUMNS ARRAY   */
    E_SQL_REMAIN_RESULT,        /* INSUFFICIENT COLUMNS ARRAY   */

    /* HTTP ERROR (for hcurl library) */
    E_HTTP_NOTFOUND_REQUEST = 2000
} e_error_code_t;
typedef e_error_code_t  e_http_error_code_t;
typedef e_error_code_t  *pe_http_error_code_t;




/* **************************************************************************
 * ON/OFF flag definition
 * **************************************************************************/
typedef enum _e_bool_t
{
    BOOL_TRUE   = 1,
    BOOL_FALSE  = 0
} e_bool_t;
typedef _e_bool_t       e_http_bool_t;
typedef e_http_bool_t   *pe_http_bool_t;

#endif

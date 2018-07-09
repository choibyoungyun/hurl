/* ****************************************************************************
 *       Filename:  http2_get.c
 *    Description:
 *        Version:  1.0
 *        Created:  06/19/18 15:51:06
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *   Description :
 * ****************************************************************************/

#include <hcommon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* somewhat unix-specific */
#include <sys/time.h>
#include <unistd.h>

/* curl stuff */
#include <curl/curl.h>

#include <uv.h>

#ifndef CURLPIPE_MULTIPLEX
/* This little trick will just make sure that we don't enable pipelining for
   libcurls old enough to not have this symbol. It is _not_ defined to zero in
   a recent libcurl header. */
#define CURLPIPE_MULTIPLEX 0
#endif



#define HTTP_MAX_MCHUNK_SIZE        256
typedef struct _st_http_mchunk_t    *pst_http_mchunk_t;
typedef struct _st_http_mchunk_t
{
    int                 max_size;
    int                 now_size;
    char                *p_mem;
} st_http_mchunk_t;


/* **************************************************************************
 * HTTP HANDLE MODE
 * **************************************************************************/
typedef enum _e_http_handle_mode_t
{
    HTTP_SIMPLE_MODE  = 0,
    HTTP_SOCKET_MODE
} e_http_handle_mode_t;


typedef struct _st_http_request_t   *pst_http_request_t;
typedef struct _st_http_request_t
{
    CURL                *p_context;

    /*  http response info */
    char                *p_done_url;
    long                rsp_code;
    st_http_mchunk_t    rsp_header;
    st_http_mchunk_t    rsp_body;

    int                 index;
    e_http_bool_t       is_pending;

    e_http_error_code_t (*pf_resp)    (long    code,
                                       char   *p_header,
                                       char   *p_body);
    e_http_error_code_t (*pf_set    ) (pst_http_request_t);
    e_http_error_code_t (*pf_perform) (pst_http_request_t);
} st_http_request_t;


#define HTTP_MAX_REQ_POOL_SIZE      2
typedef struct _st_http_handle_t   *pst_http_handle_t;
typedef struct _st_http_handle_t
{
    e_http_handle_mode_t    mode;
    /* UV  loop handle                          */
    uv_loop_t               *p_loop;
    uv_timer_t              timer;

    /* CURL multi handle                        */
    CURLM                   *p_context;

    /* last result code. (include/curl/multi.h) */
    CURLMcode               result_ecode;

    /* number of still running request          */
    int                     still_running;


    /* HTTP REQUEST POOL                        */
    int                     pool_size;
    pst_http_request_t      p_pool;
    int                     pool_last_used;
    int                     pool_last_free;  /* not used */


    /* handle function definition               */
    e_http_error_code_t (*pf_perform) (pst_http_handle_t, pst_http_request_t);
    e_http_error_code_t (*pf_set    ) (pst_http_handle_t);
    e_http_error_code_t (*pf_add    ) (pst_http_handle_t, pst_http_request_t);
    e_http_error_code_t (*pf_remove ) (pst_http_handle_t, pst_http_request_t);
} st_http_handle_t;


typedef struct _st_http_context_t    *pst_http_context_t;
typedef struct _st_http_context_t
{
    uv_poll_t           poll_handle;

    pst_http_handle_t   p_handle;
    curl_socket_t       sockfd;
}st_http_context_t;


/* ---------------------------------------------------------------------
 * HTTP HANDLE CONTEXT API
 * --------------------------------------------------------------------- */
e_http_error_code_t
init_http_handle        (pst_http_handle_t      *pp_handle,
                         e_http_handle_mode_t   mode,
                         e_http_error_code_t (*pf_set ) (pst_http_handle_t));
e_http_error_code_t
destory_http_handle     (pst_http_handle_t  p_handle);
e_http_error_code_t
perform_http_handle     (pst_http_handle_t  p_handle,
                         pst_http_request_t p_req);
/*
e_http_error_code_t
setopt_http_handle      (pst_http_handle_t  p_handle);
*/


/* ---------------------------------------------------------------------
 * HTTP REQUEST  CONTEXT API
 * --------------------------------------------------------------------- */
e_http_error_code_t
init_http_request       (pst_http_handle_t  p_handle,
                        pst_http_request_t *p_req,
                        e_http_error_code_t (*pf_set)  (pst_http_request_t),
                        e_http_error_code_t (*pf_resp) (long, char *, char *));
e_http_error_code_t
destory_http_request    (pst_http_handle_t  p_handle,
                         pst_http_request_t p_req);


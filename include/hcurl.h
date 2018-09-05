/* **************************************************************************
 *       Filename:  hcurl.h
 *    Description:
 *        Version:  1.0
 *        Created:  06/19/18 15:51:06
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *   Description :
 * **************************************************************************/

#ifndef _HCURL_H
#define _HCURL_H

#include <hcommon.h>
#include <hlog.h>
#include <mpool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* somewhat unix-specific */
#include <sys/time.h>
#include <unistd.h>

/* curl stuff */
#include <curl/curl.h>

#ifdef _UV_LIB_SUPPORT
 #include <uv.h>
#endif

#ifndef CURLPIPE_MULTIPLEX
/* This little trick will just make sure that we don't enable pipelining for
   libcurls old enough to not have this symbol. It is _not_ defined to zero in
   a recent libcurl header. */
#define CURLPIPE_MULTIPLEX 0
#endif


typedef enum _e_http_result_state_t
{
    HTTP_RESULT_OK  = 200,
    HTTP_RESULT_NOK = 999
} e_http_result_state_t;


/* **************************************************************************
 * HTTP HANDLE MODE
 * **************************************************************************/
typedef enum _e_http_handle_mode_t
{
    HTTP_SIMPLE_MODE  = 0,
    HTTP_SOCKET_MODE
} e_http_handle_mode_t;


/* ------------------------------------------------------------------------
 * DEFINE HTTP RESPONSE
 * -----------------------------------------------------------------------*/
typedef struct _st_http_response_t  *pst_http_response_t;
typedef struct _st_http_response_t
{
    char                    *p_done_url;
    long                    status_code;
    char                    *p_content_type;
    double                  content_length;

    pst_mchunk_handle_t     p_header;
    pst_mchunk_handle_t     p_body;
} st_http_response_t;


/* ------------------------------------------------------------------------
 * DEFINE HTTP REQUEST
 * -----------------------------------------------------------------------*/
#define HTTP_MAX_METHOD_LEN     32
#define HTTP_MAX_URI_LEN        256

typedef struct _st_http_request_t   *pst_http_request_t;
typedef struct _st_http_request_t
{
    /*  http handle   pointer       */
    void                    *p_multi;

    CURL                    *p_context;

    /*  http request infomation     */
    char                    method [HTTP_MAX_METHOD_LEN];
    char                    uri    [HTTP_MAX_URI_LEN];
    /*  http add header list        */
    struct curl_slist       *p_header;

    /*  http response info          */
    e_bool_t                is_done;
    st_http_response_t      rsp;

    CURLcode                err_code;
    char                    err_string [256];

    /*  user define  pointer        */
    void                    *p_user1;   /*  user add value 1    */
    void                    *p_user2;   /*  user add value 2    */

    /*  member function             */
    e_http_error_code_t     (*pf_resp)    (pst_http_request_t);
    e_http_error_code_t     (*pf_set    ) (pst_http_request_t);
    e_http_error_code_t     (*pf_perform) (pst_http_request_t);
} st_http_request_t;


#define HTTP_CONFIG_SECTION_NAME                "HTTP"

/* -----------------------------------------------------------------------
 * DEFINE HTTP VERSION
 * CURL_HTTP_VERSION_NONE               = 0
 * CURL_HTTP_VERSION_1_0                = 1
 * CURL_HTTP_VERSION_1_1                = 2
 * CURL_HTTP_VERSION_2_0                = 3
 * CURL_HTTP_VERSION_2TLS               = 4
 * CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE  = 5
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_VERSION_NAME                    "PROTOCOL_VERSION"
#define HTTP_CONFIG_VERSION_DEFAULT                 "3"


/* -----------------------------------------------------------------------
 * DEFINE HTTP PENDING
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_SIZE_PENDING_QUEUE_NAME         "SIZE_OF_PENDING_QUEUE"
#define HTTP_CONFIG_SIZE_PENDING_QUEUE_DEFAULT      "32"

/* -----------------------------------------------------------------------
 * DEFINE DEBUG OPTION
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_DEBUG_VERBOSE_NAME              "DEBUG_VERBOSE"
#define HTTP_CONFIG_DEBUG_VERBOSE_DEFAULT           "0"

/* -----------------------------------------------------------------------
 * DEFINE TIMER OPTION
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_TIMER_REQ_TIMEOUT_MS_NAME       "REQ_TIMEOUT_MS"
#define HTTP_CONFIG_TIMER_REQ_TIMEOUT_MS_DEFAULT    "1000"
#define HTTP_CONFIG_TIMER_CON_TIMEOUT_MS_NAME       "CON_TIMEOUT_MS"
#define HTTP_CONFIG_TIMER_CON_TIMEOUT_MS_DEFAULT    "3000"



/* -----------------------------------------------------------------------
 * DEFINE DEBUG OPTION
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_CONNECTIONS_MAX_HOST_NAME       "MAX_HOST_CONNECTIONS"
#define HTTP_CONFIG_CONNECTIONS_MAX_TOTAL_NAME      "MAX_TOTAL_CONNECTIONS"
#define HTTP_CONFIG_CONNECTIONS_MAX_HOST_DEFAULT    "1"
#define HTTP_CONFIG_CONNECTIONS_MAX_TOTAL_DEFAULT   "0"


/* -----------------------------------------------------------------------
 * DEFINE SSL OPTION
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_SSL_VERIFY                      "SSL_VERIFY"
#define HTTP_CONFIG_SSL_CA_PATH_NAME                "SSL_CA_PATH"
#define HTTP_CONFIG_SSL_CA_INFO_NAME                "SSL_CA_INFO"
#define HTTP_CONFIG_SSL_CRL_FILE_NAME               "SSL_CRL_FILE"
#define HTTP_CONFIG_SSL_CERT_FILE_NAME              "SSL_CERT_FILE"
#define HTTP_CONFIG_SSL_CERT_TYPE_NAME              "SSL_CERT_TYPE"
#define HTTP_CONFIG_SSL_KEY_NAME                    "SSL_KEY_FILE"
#define HTTP_CONFIG_SSL_KEY_PASSWD_NAME             "SSL_KEY_PASSWD"
#define HTTP_CONFIG_SSL_KEY_TYPE_NAME               "SSL_KEY_TYPE"

#define HTTP_CONFIG_SSL_VERIFY_DEFAULT      "0"
#define HTTP_CONFIG_SSL_CA_PATH_DEFAULT     "/etc/pki/tsl/certs"
#define HTTP_CONFIG_SSL_CERT_FILE_DEFAULT   "/etc/pki/tsl/certs/ca-bundle.crt"
#define HTTP_CONFIG_SSL_CERT_TYPE_DEFAULT   "PEM"


/* -----------------------------------------------------------------------
 * DEFINE HTTP HEADER
 * ----------------------------------------------------------------------- */
#define HTTP_CONFIG_HEADER_AGENT_NAME           "HEADER_USER_AGENT"
#define HTTP_CONFIG_HEADER_AGENT_DEFAULT        "TAS-JDIN"

typedef struct _st_http_header_t        *pst_http_header_t;
typedef struct _st_http_header_t
{
    char                    agent_name   [256];
} st_http_header_t;



typedef struct _st_http_connection_t    *pst_http_connection_t;
typedef struct _st_http_connection_t
{
    unsigned int            max_host;
    unsigned int            max_total;
}st_http_connection_t;



typedef struct _st_http_timer_t    *pst_http_timer_t;
typedef struct _st_http_timer_t
{
    unsigned int            req_timeout_ms;
    unsigned int            con_timeout_ms;
}st_http_timer_t;


typedef struct _st_http_ssl_t      *pst_http_ssl_t;
typedef struct _st_http_ssl_t
{
    /* ssl option                               */
    e_bool_t                verify;

    char                    ca_path     [256];
    char                    ca_info     [256];

    char                    crl_fname   [256];

    char                    cert_fname  [256];
    char                    cert_type   [16];

    char                    key_fname   [256];
    char                    key_passwd  [128];
    char                    key_type    [16];
} st_http_ssl_t;

typedef struct _st_http_handle_t   *pst_http_handle_t;
typedef struct _st_http_handle_t
{
    char                    cfname   [256];
    char                    csection [128];

    e_http_handle_mode_t    mode;

#ifdef _UV_LIB_SUPPORT
    /* UV  loop handle                          */
    uv_loop_t               *p_loop;
    uv_timer_t              timer;
    curl_socket_t           sockfd;
#endif

    /* CURL multi handle                        */
    CURLM                   *p_context;

    /* OAUTH handle                             */
    void                    *p_auth;

    /* last result code. (include/curl/multi.h) */
    CURLMcode               err_code;
    char                    err_string [256];

    /* last perform tick                        */
    int                     last_tick;

    /* number of still running request          */
    int                     still_running;

    /* memory pool  for http request/response   */
    int                     size_of_pqueue;
    pst_mpool_handle_t      p_pending_queue;

    /*  http version                            */
    int                     hversion;

    /*  debug option                            */
    e_bool_t                verbose;

    /* http request timeout (mili-second)       */
    st_http_timer_t         timer;
    st_http_connection_t    connections;
    st_http_ssl_t           ssl;

    /*  DEFAULT  header                         */
    st_http_header_t        header;

    /* handle function definition               */
    e_http_error_code_t (*pf_perform) (pst_http_handle_t, pst_http_request_t);
    e_http_error_code_t (*pf_done)    (pst_http_handle_t);
    e_http_error_code_t (*pf_set    ) (pst_http_handle_t);
    e_http_error_code_t (*pf_add    ) (pst_http_handle_t, pst_http_request_t);
    e_http_error_code_t (*pf_remove ) (pst_http_handle_t, pst_http_request_t);
    void                (*pf_show )   (pst_http_handle_t, char *);
} st_http_handle_t;


#ifdef _UV_LIB_SUPPORT
typedef struct _st_http_context_t    *pst_http_context_t;
typedef struct _st_http_context_t
{
    uv_poll_t           poll_handle;

    pst_http_handle_t   p_handle;
    curl_socket_t       sockfd;
}st_http_context_t;
#endif


/* ---------------------------------------------------------------------
 * HTTP HANDLE CONTEXT API
 * --------------------------------------------------------------------- */
e_http_error_code_t
init_http_handle        (pst_http_handle_t      *pp_handle,
                         char                   *p_fname,
                         char                   *p_section,
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
                        e_http_error_code_t (*pf_resp) (pst_http_request_t));
e_http_error_code_t
destory_http_request    (pst_http_handle_t  p_handle,
                         pst_http_request_t p_req);
#endif /*  _H_HCURL_H_ */

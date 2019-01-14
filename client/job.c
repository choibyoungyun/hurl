/* ***************************************************************************
 *
 *       Filename:  job.c
 *    Description:  job
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hworker.h>
#include <hauth.h>


/* **************************************************************************
 *  @brief      send http response to EIGW
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_error_code_t
send_response_to_app_server (pst_http_request_t p_http)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_eigw_handle_t   p_handle = (pst_eigw_handle_t)  p_http->p_user1;
    pst_eigw_request_t  p_req    = (pst_eigw_request_t) p_http->p_user2;
    st_eigw_response_t  rsp;


    /* -------------------------------------------------------------------
     * set  content length
     * ------------------------------------------------------------------- */
    p_http->rsp.content_length = MCHUNK_NOW_SIZE(p_http->rsp.p_body);

    /* -------------------------------------------------------------------
     *  ISSUE. TOO BIG SIZE BODY (32K)
     * ------------------------------------------------------------------- */
    if (p_http->rsp.p_body->now_size
            > EIGW_MAX_MSG_BODY_DATA_LENGTH(st_eigw_response_t))
    {
        sprintf (p_http->err_string,
                 "fail, too long size body [now:%d, max:%d]",
                 p_http->rsp.p_body->now_size,
                 EIGW_MAX_MSG_BODY_DATA_LENGTH(st_eigw_response_t));
        p_http->rsp.status_code      = HTTP_RESULT_UNKNOWN;
        p_http->rsp.p_body->now_size = 0;
    }


    if ((p_http->rsp.status_code == HTTP_RESULT_UNKNOWN)
            || (p_http->rsp.status_code >= E_PROTOCOL_SPEC))
    {
        if (p_http->rsp.status_code == HTTP_RESULT_UNKNOWN)
        {
            p_http->rsp.status_code = E_PROTOCOL_SPEC;
        }
        Log (DEBUG_ERROR,
                "fail, rsp SRC[%X:%ld] [%04d %s %s (err:%s)]\n",
                p_req->header.unGwRteVal,
                p_req->header.ulSeq,
                p_http->rsp.status_code,
                p_req->body.method,
                p_req->body.data + p_req->body.ind2,
                p_http->err_string);
    }
    else
    {
        if (p_http->rsp.content_length <= 0)
        {
            /* -----------------------------------------------------------
             * copy header to body for sending to EIGW
             * ----------------------------------------------------------- */
            (p_http->rsp.p_body->pf_reset) (p_http->rsp.p_body);
            (p_http->rsp.p_body->pf_add)   (p_http->rsp.p_body,
                                        MCHUNK_MEM (p_http->rsp.p_header),
                                        MCHUNK_NOW_SIZE (p_http->rsp.p_header));
        }
    }

    e_code = (p_handle->pf_encode)(p_req,
                                   &rsp,
                                   p_http->rsp.status_code,
                                   p_http->rsp.p_content_type,
                                   p_http->rsp.content_length,
                                   p_http->rsp.p_header,
                                   p_http->rsp.p_body,
                                   p_http->err_string);

    e_code = (p_handle->pf_send)(p_handle, p_req, &rsp);
    if (e_code != E_SUCCESS)
    {
        Log (DEBUG_ERROR, "%s", EIGW_HANDLE_ERROR_STRING (p_handle));
        if (p_req)
        {
            Log (DEBUG_ERROR,
                    "fail, discard rsp SRC[%08x:%ld] [HEAD %s %s %s length(%s)]\n",
                    rsp.header.unGwRteVal,
                    rsp.header.ulSeq,
                    rsp.body.method,
                    rsp.body.status_code,
                    IS_AVAILABLE_EIGW_RSP_BODY_DATA (&rsp, rsp.body.ind2)
                    ? rsp.body.data + rsp.body.ind2 : "NULL",
                    IS_AVAILABLE_EIGW_RSP_BODY_DATA (&rsp, rsp.body.ind3)
                    ? rsp.body.data + rsp.body.ind3 : "NULL");

        }
    }
    else
    {
        Log (DEBUG_LOW, "%s", EIGW_HANDLE_ERROR_STRING (p_handle));
        Log (DEBUG_INFO,
                "succ, send rsp SRC[%08x:%ld] [HEAD %s %s %s length(%s)]\n",
                rsp.header.unGwRteVal,
                rsp.header.ulSeq,
                rsp.body.method,
                rsp.body.status_code,
                IS_AVAILABLE_EIGW_RSP_BODY_DATA (&rsp, rsp.body.ind2)
                ? rsp.body.data + rsp.body.ind2 : "NULL",
                IS_AVAILABLE_EIGW_RSP_BODY_DATA (&rsp, rsp.body.ind3)
                ? rsp.body.data + rsp.body.ind3 : "NULL");

        Log (DEBUG_LOW,
                "succ, send rsp SRC[%08x:%ld] [BODY %s]\n",
                p_req->header.unGwRteVal,
                p_req->header.ulSeq,
                IS_AVAILABLE_EIGW_RSP_BODY_DATA (&rsp, rsp.body.ind4)
                ? rsp.body.data + rsp.body.ind4 : "NULL");
    }


    if (p_http->rsp.status_code == 401)
    {
        pst_auth_handle_t p_auth;

        p_auth = (pst_auth_handle_t)
                 (HTTP_HANDLE_FROM_REQUEST(p_http)->p_auth);
        (void) (p_auth->pf_clean)(p_auth);
    }


    if (p_req) FREE (p_req);

    return (e_code);
}




/* **************************************************************************
 *  @brief      setopt_http_header
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
setopt_http_header (pst_eigw_request_t     p_app_req,
                    pst_http_request_t     p_http_req,
                    pst_auth_handle_t      p_auth)
{
    e_error_code_t      e_code    = E_SUCCESS;
    CURL                *p_handle = p_http_req->p_context;
    char                tmp [HTTP_MAX_HEADER_LEN] = {0,};


    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (CONTENT-TYPE)
     * -------------------------------------------------------------------- */
    if (IS_AVAILABLE_EIGW_REQ_BODY_DATA(p_app_req, p_app_req->body.ind6))
    {
        snprintf (tmp, sizeof (tmp) - 1, "%s:%s",
                 "Content-Type",
                  p_app_req->body.data + p_app_req->body.ind3);
        p_http_req->p_header = curl_slist_append (p_http_req->p_header,
                                                 (const char *)tmp);
    }

    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (USER-DEFINED HEADER)
     * -------------------------------------------------------------------- */
    if (IS_AVAILABLE_EIGW_REQ_BODY_DATA(p_app_req, p_app_req->body.ind5))
    {
        p_http_req->p_header = curl_slist_append (p_http_req->p_header,
                  p_app_req->body.data + p_app_req->body.ind5);
    }

    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (OAUTH HEADER)
     * - COMPARE NF NAME
     * -------------------------------------------------------------------- */
    if (p_auth && ((p_auth->pf_verify)(p_auth,
                    p_app_req->body.data + p_app_req->body.ind1) == E_SUCCESS))
    {
        p_http_req->p_header = curl_slist_append (p_http_req->p_header,
                                                  p_auth->auth_header);
    }

    p_http_req->err_code = curl_easy_setopt (p_handle,
                                             CURLOPT_HTTPHEADER,
                                             p_http_req->p_header);
    try_exception (p_http_req->err_code != CURLE_OK,
                        exception_setopt_http_request);

    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (USER AGENT)
     * -------------------------------------------------------------------- */
    p_http_req->err_code = curl_easy_setopt (p_handle,
                                           CURLOPT_USERAGENT,
               HTTP_HANDLE_FROM_REQUEST(p_http_req)->header.agent_name);

    try_exception (p_http_req->err_code != CURLE_OK,
                            exception_setopt_http_request);

    try_catch (exception_setopt_http_request)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief      setopt_http_request
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
setopt_http_request (pst_eigw_request_t     p_app_req,
                     pst_http_request_t     p_http_req,
                     pst_auth_handle_t      p_auth)
{
    e_error_code_t      e_code    = E_SUCCESS;
    CURL                *p_handle = p_http_req->p_context;


    /* --------------------------------------------------------------------
     *  PROTOCOL VERSION
     * -------------------------------------------------------------------- */
    curl_easy_setopt (p_handle, CURLOPT_HTTP_VERSION,
                     (HTTP_HANDLE_FROM_REQUEST(p_http_req))->hversion);
    if ((HTTP_HANDLE_FROM_REQUEST(p_http_req))->hversion
            == CURL_HTTP_VERSION_1_1 )
    {
        curl_easy_setopt (p_handle, CURLOPT_PIPEWAIT, 1L);
    }

    /* --------------------------------------------------------------------
     *  TIMER OPTION
     *  ----------------
     *  curl_easy_setopt (p_handle, CURLOPT_CONNECTTIMEOUT_MS, 1);
     *  curl_easy_setopt (p_handle, CURLOPT_TIMEOUT, 1);
     *  curl_easy_setopt (p_handle, CURLOPT_TIMEOUT_MS, 1);
     * -------------------------------------------------------------------  */
    curl_easy_setopt(p_handle, CURLOPT_CONNECTTIMEOUT_MS,
            HTTP_HANDLE_FROM_REQUEST(p_http_req)->timer.con_timeout_ms);

    curl_easy_setopt(p_handle, CURLOPT_TIMEOUT_MS,
            HTTP_HANDLE_FROM_REQUEST(p_http_req)->timer.req_timeout_ms);

    /* --------------------------------------------------------------------
     *  REQUEST URI
     * -------------------------------------------------------------------- */
    p_http_req->err_code = curl_easy_setopt (p_handle,
                                             CURLOPT_URL,
                                             p_http_req->uri);
    try_exception (p_http_req->err_code != CURLE_OK,
                        exception_setopt_http_request);

    /* --------------------------------------------------------------------
     *  REQUEST METHOD
     * -------------------------------------------------------------------- */
    p_http_req->err_code = curl_easy_setopt (p_handle,
                                             CURLOPT_CUSTOMREQUEST,
                                             p_http_req->method);
    try_exception (p_http_req->err_code != CURLE_OK,
                        exception_setopt_http_request);


    /* --------------------------------------------------------------------
     *  REQUEST HEADER
     * -------------------------------------------------------------------- */
    e_code = setopt_http_header (p_app_req, p_http_req, p_auth);
    try_exception (e_code != E_SUCCESS, exception_setopt_http_request);


    /* --------------------------------------------------------------------
     *  REQUEST BODY
     * -------------------------------------------------------------------- */
    if (IS_AVAILABLE_EIGW_REQ_BODY_DATA(p_app_req, p_app_req->body.ind6))
    {
        p_http_req->err_code = curl_easy_setopt (p_handle,
                                                 CURLOPT_POSTFIELDS,
                                  p_app_req->body.data + p_app_req->body.ind6);
        try_exception (p_http_req->err_code != CURLE_OK,
                            exception_setopt_http_request);
    }


    try_catch (exception_setopt_http_request)
    {
        HTTP_REQUEST_INTERNAL_ERROR (p_http_req);
        Log (DEBUG_ERROR,
                "fail, set http option (%d, %s)\n",
                HTTP_REQUEST_ERROR_CODE(p_http_req),
                HTTP_REQUEST_ERROR_STRING(p_http_req));

        e_code = E_PROTOCOL_HTTP_SETOPT;
    }
    try_finally;

    return (e_code);
}




/* *************************************************************************
 *  @brief      send_request_to_http_server
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param
 *  @retval     E_SUCCESS/else
 * ************************************************************************/
static
e_error_code_t
send_request_to_http_server (pst_eigw_handle_t  p_eigw,
                             pst_http_handle_t  p_http,
                             pst_auth_handle_t  p_auth)
{
    e_error_code_t      e_code = E_SUCCESS;
    pst_http_request_t  p_uri  = NULL;


    try_assert (p_eigw == NULL);

    /*  init http request handle         */
    do
    {

        e_code = init_http_request (p_http,
                                    &p_uri,
                                    NULL,
                                    send_response_to_app_server);
        if (e_code == E_BUSY)
        {
            Log (DEBUG_LOW,
                    "info, busy http memory pool [%p: total(%d)]\n",
                   p_http->p_pending_queue,
                   p_http->p_pending_queue->num_of_block);

            (void) (p_http->pf_perform) (p_http, NULL);
            usleep(5);
            continue;
        }
    } while (e_code != E_SUCCESS);

    /*  add pending information         */
    p_uri->p_user1 = (void *)p_eigw;
    p_uri->p_user2 = (void *)p_eigw->p_req;


    /* set http option                  */
    strcpy (p_uri->uri, p_eigw->p_req->body.data + p_eigw->p_req->body.ind2);
    strcpy (p_uri->method, p_eigw->p_req->body.method);
    if ((e_code = setopt_http_request (p_eigw->p_req,
                                       p_uri,
                                       p_auth)) != E_SUCCESS)
    {
        (void) send_response_to_app_server (p_uri);
        (p_http->p_pending_queue->pf_free)(p_http->p_pending_queue, p_uri);
        p_eigw->p_req = NULL;

        return (E_SUCCESS);
    }

    /*  send http request               */
    e_code = (p_http->pf_perform)(p_http, p_uri);
    if (e_code != E_SUCCESS)
    {
        Log (DEBUG_ERROR,
                "fail, perform http handle (%d, %s)\n",
                HTTP_HANDLE_ERROR_CODE (p_http),
                HTTP_HANDLE_ERROR_STRING (p_http));

        /*  generate core file          */
        try_assert (1);
    }

    p_eigw->p_req = NULL;

    return (e_code);
}




/* *************************************************************************
 *  @brief      recv_request_from_eigw
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param
 *  @retval     E_SUCCESS/else
 * ************************************************************************/
static
e_error_code_t
recv_request_from_app_server (pst_eigw_handle_t    p_handle)
{
    e_error_code_t      e_code = E_SUCCESS;
    pst_eigw_request_t  p_msg  = NULL;
    int                 length = 0;


    if ((e_code = (p_handle->pf_recv)(p_handle)) != E_SUCCESS)
    {
        return (e_code);
    }

    length = (int) ntohs(p_handle->p_req->header.usLength);
    p_msg = (pst_eigw_request_t) MALLOC (length);
    if (p_msg == NULL)
    {
        p_handle->p_req = NULL;
        return (E_ALLOC_HANDLE);
    }
    memcpy (p_msg, p_handle->p_req, length);
    p_handle->p_req = (pst_eigw_request_t)p_msg;

    /* ----------------------------------------------------------------
     * Logging low-level applicatioin request
     * ---------------------------------------------------------------- */
    Log (DEBUG_LOW, "%s", EIGW_HANDLE_ERROR_STRING(p_handle));

    Log (DEBUG_INFO, "succ, recv req SRC[%08x:%d] [HEAD %s %s %s (H:%s)]\n",
                    p_msg->header.unGwRteVal,
                    p_msg->header.ulSeq,
                    p_msg->body.method,
                    p_msg->body.data + p_msg->body.ind2,
                    p_msg->body.data + p_msg->body.ind3,
                    IS_AVAILABLE_EIGW_REQ_BODY_DATA(p_msg, p_msg->body.ind5)
                    ? p_msg->body.data + p_msg->body.ind5 : "NULL");

    Log (DEBUG_LOW,  "succ, recv req SRC[%08x:%d] [BODY %s]\n",
                    p_msg->header.unGwRteVal,
                    p_msg->header.ulSeq,
                    IS_AVAILABLE_EIGW_REQ_BODY_DATA(p_msg, p_msg->body.ind6)
                    ? p_msg->body.data + p_msg->body.ind6 : "NULL");

    return (e_code);
}




/* *************************************************************************
 *  @brief      pre-check handle state
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param
 *  @retval     E_SUCCESS/else
 * ************************************************************************/
static
e_error_code_t
preprocess_job (pst_process_handle_t    p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    static int      i      = 0;


    /*  1. GET ACCESS-TOKEN (OAUTH2)    */
    if ((p_handle->p_auth->pf_verify)(p_handle->p_auth, NULL) != E_SUCCESS)
    {
        (p_handle->p_auth->pf_send)(p_handle->p_auth, p_handle->p_http);
    }

    if ((p_handle->p_auth->pf_verify)(p_handle->p_auth, NULL) != E_SUCCESS)
    {
        return (E_FAILURE);
    }


    /*  2. CONNECT TO EIGW              */
    if (EIGW_HANDLE_SOCKET_FD (p_handle->p_eigw) < 0)
    {
        if ((p_handle->p_eigw->pf_connect)(p_handle->p_eigw) != E_SUCCESS)
        {
            if ((i++%100) == 0)
            {
                Log (DEBUG_ERROR,
                        "%s", EIGW_HANDLE_ERROR_STRING(p_handle->p_eigw));
            }
            return (E_FAILURE);
        }

        Log (DEBUG_CRITICAL,
                "succ, connect    EIGW  [ip:%s, port:%s, sockfd:%d]\n",
                EIGW_HANDLE_REMOTE_IP   (p_handle->p_eigw),
                EIGW_HANDLE_REMOTE_PORT (p_handle->p_eigw),
                EIGW_HANDLE_SOCKET_FD   (p_handle->p_eigw));
    }


    /*  3. heartbeat  with EIGW                 */
    if ((time (NULL) - p_handle->p_eigw->last_tick)
            > p_handle->p_eigw->hb_interval)
    {
        if ((p_handle->p_eigw->pf_heartbeat)(p_handle->p_eigw) != E_SUCCESS)
        {
            Log (DEBUG_ERROR,
                    "%s", EIGW_HANDLE_ERROR_STRING (p_handle->p_eigw));
        }
    }


    if (p_handle->p_http->still_running != 0)
    {
        (void) (p_handle->p_http->pf_perform) (p_handle->p_http, NULL);
    }

    return (e_code);
}




/* *************************************************************************
 *  @brief          DO JOB
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param
 *  @retval
 * ************************************************************************/

e_error_code_t
do_job   (pst_process_handle_t   p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;


    /*  check current handle state       */
    if ((e_code = preprocess_job (p_handle)) != E_SUCCESS)
    {
        return (E_DELAY_JOB);
    }

    /*  recv request from EIGW          */
    e_code = recv_request_from_app_server (p_handle->p_eigw);
    if (e_code != E_SUCCESS)
    {
        if ((e_code == E_TIMEOUT)
                || (e_code == E_PROTOCOL_INVALID_BODY)
                || (e_code == E_BUSY))
            e_code = E_IMMEDIATE_JOB;
        else
        {
            Log (DEBUG_CRITICAL,
                    "%s", EIGW_HANDLE_ERROR_STRING(p_handle->p_eigw));
            e_code = E_DELAY_JOB;
        }

        return (e_code);
    }


    /*  send request to HTTP          */
    e_code = send_request_to_http_server (p_handle->p_eigw,
                                          p_handle->p_http,
                                          p_handle->p_auth);
    if (e_code == E_SUCCESS) e_code = E_IMMEDIATE_JOB;


    return (e_code);
}

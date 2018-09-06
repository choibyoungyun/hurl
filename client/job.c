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
    p_http->rsp.content_length = p_http->rsp.p_body->now_size;

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
            if (p_http->rsp.p_body->max_size < p_http->rsp.p_header->now_size)
            {
                char *p_tmp = NULL;
                p_tmp = (char *)malloc (p_http->rsp.p_header->now_size + 1);

                if (p_http->rsp.p_body->p_mem != NULL)
                {
                    free (p_http->rsp.p_body->p_mem);
                }
                p_http->rsp.p_body->p_mem = p_tmp;
                p_http->rsp.p_body->max_size = p_http->rsp.p_header->now_size + 1;
                p_http->rsp.p_body->now_size = 0;
            }

            memcpy (p_http->rsp.p_body->p_mem,
                    p_http->rsp.p_header->p_mem,
                    p_http->rsp.p_header->now_size);
            p_http->rsp.p_body->now_size = p_http->rsp.p_header->now_size;
        }

        Log (DEBUG_INFO,"info, rsp SRC[%x:%ld] [%04d %s %s clength(%d:%d)]\n",
                        p_req->header.unGwRteVal,
                        p_req->header.ulSeq,
                        p_http->rsp.status_code,
                        p_req->body.method,
                        p_req->body.data + p_req->body.ind2,
                        (int)p_http->rsp.content_length,
                        p_http->rsp.p_body->now_size);

        Log (DEBUG_INFO,"info, rsp SRC[%x:%ld] [header:%s]\n",
                        p_req->header.unGwRteVal,
                        p_req->header.ulSeq,
                        p_http->rsp.p_header->p_mem);

        Log (DEBUG_LOW, "info, rsp SRC[%x:%ld] [body  :%s]\n",
                        p_req->header.unGwRteVal,
                        p_req->header.ulSeq,
                        p_http->rsp.p_body->p_mem);
    }

    e_code = (*p_handle->pf_encode)(p_req,
                                    &rsp,
                                    p_http->rsp.status_code,
                                    p_http->rsp.p_content_type,
                                    p_http->rsp.content_length,
                                    p_http->rsp.p_header,
                                    p_http->rsp.p_body,
                                    p_http->err_string);

    e_code = (*p_handle->pf_send)(p_handle, p_req, &rsp);


    if (p_http->rsp.status_code == 401)
    {
        pst_auth_handle_t p_auth;

        p_auth = (pst_auth_handle_t)
                 (HTTP_HANDLE_FROM_REQUEST(p_http)->p_auth);
        (void) (p_auth->pf_clean)(p_auth);
    }

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
    char                tmp [128] = {0,};


    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (CONTENT-TYPE)
     * -------------------------------------------------------------------- */
    snprintf (tmp, sizeof (tmp) - 1, "%s:%s",
             "Content-Type",
              p_app_req->body.data + p_app_req->body.ind3);
    p_http_req->p_header = curl_slist_append (p_http_req->p_header,
                                             (const char *)tmp);

    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (USER-DEFINED HEADER)
     * -------------------------------------------------------------------- */
    if (p_app_req->body.ind5 > 0)
    {
        p_http_req->p_header = curl_slist_append (p_http_req->p_header,
                  p_app_req->body.data + p_app_req->body.ind5);
    }

    /* --------------------------------------------------------------------
     * REQUEST APPLICATION HEADER (OAUTH HEADER)
     * -------------------------------------------------------------------- */
    if (p_auth && ((p_auth->pf_verify)(p_auth) == E_SUCCESS))
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
    if (p_app_req->body.ind6 > 0)
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
    int                 alloc_size = 0;


    UNUSED (alloc_size);
    try_exception (p_eigw == NULL, exception_null_eigw_request);

    /*  all http request handle         */
    e_code = init_http_request (p_http,
                                &p_uri,
                                NULL,
                                send_response_to_app_server);
    try_exception (e_code == E_BUSY, exception_busy_http_handle);


    /*  add pending information         */
    p_uri->p_user1 = (void *)p_eigw;
    alloc_size     = offsetof(st_eigw_response_t, body.data)
                            + p_eigw->p_req->body.ind3;
    memcpy (p_uri->p_user2, p_eigw->p_req, alloc_size);


    /*  validate application request    */
    strcpy (p_uri->uri, p_eigw->p_req->body.data + p_eigw->p_req->body.ind2);
    strcpy (p_uri->method, p_eigw->p_req->body.method);

    /* set http option                  */
    try_exception ((e_code = setopt_http_request (p_eigw->p_req,
                                                  p_uri,
                                                  p_auth))
                   != E_SUCCESS,
                   exception_setopt_http_request);

    p_eigw->p_req = NULL;


    /*  send http request               */
    e_code = perform_http_handle (p_http, p_uri);
    if (e_code != E_SUCCESS)
    {
        Log (DEBUG_ERROR,
                "fail, perform http handle (%d, %s)\n",
                HTTP_HANDLE_ERROR_CODE (p_http),
                HTTP_HANDLE_ERROR_STRING (p_http));

        /*  generate core file          */
        try_assert (1);
    }


    try_catch (exception_null_eigw_request)
    {
    }
    try_catch (exception_setopt_http_request)
    {
        (void) send_response_to_app_server (p_uri);
        (*p_http->p_pending_queue->pf_free)(p_http->p_pending_queue, p_uri);
        p_eigw->p_req = NULL;

        e_code = E_SUCCESS;
    }
    try_catch (exception_busy_http_handle)
    {
        Log (DEBUG_INFO,
                "info, busy http handle [pending req: %d] \n",
                p_http->still_running);
        e_code = E_BUSY;
    }
    try_finally;

    if (p_http->still_running != 0)
    {
        (void) (*p_http->pf_perform)(p_http, NULL);
    }

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
    return ((*p_handle->pf_recv)(p_handle));
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


    if ((p_handle->p_auth->pf_verify)(p_handle->p_auth) != E_SUCCESS)
    {
        (p_handle->p_auth->pf_send)(p_handle->p_auth, p_handle->p_http);
    }


    if ((time (NULL) - p_handle->p_eigw->last_tick)
            > p_handle->p_eigw->hb_interval)
    {
        (void) (*p_handle->p_eigw->pf_heartbeat)(p_handle->p_eigw);
    }

    return (e_code);
}




/* *************************************************************************
 *  @brief          MIGRATE IPCC.TCD to BILL.TBL_HIST_TCALL
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

    if ((p_handle->p_auth->pf_verify)(p_handle->p_auth) != E_SUCCESS)
    {
        return (E_DELAY_JOB);
    }


    /*  recv request from EIGW          */
    if (p_handle->p_eigw->p_req == NULL)
    {
        e_code = recv_request_from_app_server (p_handle->p_eigw);
        try_exception (e_code != E_SUCCESS, exception_busy_eigw_handle);

        try_exception (GET_MSGN_NAME(p_handle->p_eigw->p_req->header.unMsgName)
                       == EIGW_MSG_NAME_HEARTBEAT,
                       exception_heartbeat_eigw_handle);
    }

    /*  send request to HTTP          */
    do
    {
        e_code = send_request_to_http_server (p_handle->p_eigw,
                                              p_handle->p_http,
                                              p_handle->p_auth);
        if (e_code == E_SUCCESS) e_code = E_IMMEDIATE_JOB;
    } while (e_code == E_BUSY);



    try_catch (exception_busy_eigw_handle)
    {
        if ((e_code == E_TIMEOUT)
                || (e_code == E_PROTOCOL_INVALID_BODY)
                || (e_code == E_BUSY))
            e_code = E_IMMEDIATE_JOB;
        else
            e_code = E_DELAY_JOB;
    }
    try_catch (exception_heartbeat_eigw_handle)
    {
        p_handle->p_eigw->p_req = NULL;
        p_handle->p_eigw->last_tick = time (NULL);
        Log (DEBUG_NORMAL,
                "succ, recv heartbeat response from eigw [result:%d]\n",
                (int) p_handle->p_eigw->p_req->header.sRet);
        e_code = E_IMMEDIATE_JOB;
    }
    try_finally;


    if (p_handle->p_http->still_running != 0)
    {
        (void) send_request_to_http_server (NULL,
                                            p_handle->p_http,
                                            p_handle->p_auth);
        e_code = E_IMMEDIATE_JOB;
    }

    return (e_code);
}

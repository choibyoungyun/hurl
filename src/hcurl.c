/* ***************************************************************************
 *       Filename:  http2_get.c
 *    Description:
 *        Version:  1.0
 *        Created:  06/19/18 15:51:06
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *   Description :
 * **************************************************************************/

#include <hurl.h>

#ifdef _OLD_FUNCTION
/* **************************************************************************
 *	@brief      get_http_request_using_contect
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
get_http_request_using_contect (pst_http_handle_t   p_handle,
                                pst_http_request_t  *pp_req,
                                CURL                *p_context)
{
    /*
    e_http_error_code_t e_code = E_SUCCESS;
    int                     i  = 0;
    pst_http_request_t      p_now = NULL;


    i = p_handle->pool_last_free;
    do
    {
        p_now = p_handle->p_pool + i;
        if (p_now->p_context == p_context)
        {
            p_handle->pool_last_free  = i;
            *pp_req = p_now;

            return (e_code);
        }

        i = (i == (p_handle->pool_size-1)) ? 0 : i+1;
    }
    while (i != p_handle->pool_last_free);

    return (E_DATA_NOTFOUND);

    UNUSED (p_handle);
    curl_easy_getinfo (p_context, CURLINFO_PRIVATE, pp_req);
    */
    return (E_SUCCESS);
}
#endif







/* **************************************************************************
 *	@brief      alloc_http_mchunk
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

static
e_http_error_code_t
alloc_http_mchunk (pst_http_mchunk_t    p_mchunk,
                   char                 *p_ptr,
                   int                  add_size)
{
    p_mchunk->p_mem = (char*) REALLOC (p_mchunk->p_mem,
                                      p_mchunk->now_size + add_size + 1);
    if (p_mchunk->p_mem == NULL)
    {
        return (E_ALLOC_HANDLE);
    }

    memcpy (p_mchunk->p_mem + p_mchunk->now_size,
            p_ptr,
            add_size);
    p_mchunk->now_size       += add_size;
    p_mchunk->p_mem[p_mchunk->now_size] = 0x00;
    if (p_mchunk->now_size >  p_mchunk->max_size)
    {
        p_mchunk->max_size = p_mchunk->now_size;
    }

    return (E_SUCCESS);
}





/* **************************************************************************
 *	@brief      process_http_reponse
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

size_t
process_http_rsp_header (char      *p_ptr,
                     size_t    size,
                     size_t    nmemb,
                     void      *p_user_data)
{
    pst_http_request_t  p_req   = (pst_http_request_t)p_user_data;
    pst_http_mchunk_t   p_chunk = &p_req->rsp_header;

    if (alloc_http_mchunk (p_chunk, p_ptr, size * nmemb) != E_SUCCESS)
        return (0);

    return(size * nmemb);
}




/* **************************************************************************
 *	@brief      process_http_reponse
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

size_t
process_http_rsp_body (char      *p_ptr,
                       size_t    size,
                       size_t    nmemb,
                       void      *p_user_data)
{
    pst_http_request_t  p_req    = (pst_http_request_t)p_user_data;
    pst_http_mchunk_t   p_chunk  = &p_req->rsp_body;

#ifdef _DEBUG_
    HTTP_LOG ("succ, receive http body [index: %d, size:%d, nmemb:%d]\n",
               p_req->index, (int)size, (int)nmemb);
#endif

    if (alloc_http_mchunk (p_chunk, p_ptr, size * nmemb) != E_SUCCESS)
        return (0);

    return(size * nmemb);
}




/* **************************************************************************
 *	@brief      process_http_rsp_done
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

static
e_http_error_code_t
process_http_rsp_done (pst_http_handle_t p_handle)
{
    e_http_error_code_t e_code = E_SUCCESS;
    struct  CURLMsg    *p_msg  = NULL;
    int     msgs_in_queue      = 0;
    pst_http_request_t  p_req  = NULL;


    /* msgs_in_queue :
     * The integer pointed to with msgs_in_queue will contain
     * the number of remaining messages after this function was called.
     * */

    /* call curl_multi_perform or curl_multi_socket_action first, then loop
     * through and check if there are any transfers that have completed */
    do
    {
        msgs_in_queue = 0;
        p_msg         = NULL;
        p_msg = curl_multi_info_read (p_handle->p_context,
                                      &msgs_in_queue);

        if ((p_msg != NULL) && (p_msg->msg == CURLMSG_DONE))
        {
           (void) curl_easy_getinfo (p_msg->easy_handle,
                                     CURLINFO_PRIVATE,
                                     &p_req);

           (void) curl_easy_getinfo (p_msg->easy_handle,
                                     CURLINFO_EFFECTIVE_URL,
                                     &p_req->p_done_url);
           (void) curl_easy_getinfo (p_msg->easy_handle,
                                     CURLINFO_RESPONSE_CODE,
                                     &p_req->rsp_code);
            if (p_req->pf_resp != NULL)
            {
                (*p_req->pf_resp)(p_req->rsp_code,
                                  p_req->rsp_header.p_mem,
                                  p_req->rsp_body.p_mem);
            }

             p_req->is_pending = BOOL_FALSE;
        }
    } while (p_msg != NULL);


    return (e_code);
}




/* **************************************************************************
 *	@brief      setopt_http_request
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
default_setopt_http_request (pst_http_request_t   p_req)
{
    e_http_error_code_t e_code    = E_SUCCESS;
    CURL                *p_handle = p_req->p_context;


    /*
    curl_easy_setopt (p_handle, CURLOPT_HEADER, 1L);
    */
    curl_easy_setopt (p_handle, CURLOPT_HEADERDATA, p_req);
    curl_easy_setopt (p_handle, CURLOPT_HEADERFUNCTION,
                                process_http_rsp_header);

    /* write to this file */
    curl_easy_setopt (p_handle, CURLOPT_WRITEDATA, p_req);
    curl_easy_setopt (p_handle, CURLOPT_WRITEFUNCTION,
                                process_http_rsp_body);

    /* HTTP/2 please */
    curl_easy_setopt (p_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);


    /*  Private option */
    curl_easy_setopt (p_handle, CURLOPT_PRIVATE, p_req);


#if (CURLPIPE_MULTIPLEX > 0)
    /* wait for pipe connection to confirm */
    curl_easy_setopt (p_handle, CURLOPT_PIPEWAIT, 1L);
#endif

    /*  set timeout option                  */
    curl_easy_setopt (p_handle, CURLOPT_CONNECTTIMEOUT, 300);
    curl_easy_setopt (p_handle, CURLOPT_TIMEOUT, 1);


    /* we use a self-signed test server, skip verification during debugging */
    curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    return (e_code);
}




/* **************************************************************************
 *	@brief      init_http_request
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

e_http_error_code_t
init_http_request (pst_http_handle_t    p_handle,
                   pst_http_request_t   *pp_req,
                   e_http_error_code_t  (*pf_option) (pst_http_request_t),
                   e_http_error_code_t  (*pf_resp)   (long    code,
                                                      char   *p_header,
                                                      char   *p_body))
{
    e_http_error_code_t e_code = E_SUCCESS;
    pst_http_request_t  p_now  = NULL;
    int                     i  = 0;
    CURLMcode           mc;


    i = p_handle->pool_last_used;
    do
    {
        p_now = p_handle->p_pool + i;
        if (p_now->is_pending != BOOL_TRUE)
        {
            /*  remove CURL from CURL MULTI HANDLE */
            mc = curl_multi_remove_handle (p_handle->p_context,
                                           p_now->p_context);
            if (mc != CURLM_OK)
            {
                HTTP_LOG ("fail, %s\n", curl_multi_strerror (mc));
                abort();
            }

            p_now->is_pending = BOOL_TRUE;
            p_now->rsp_header.now_size = 0;
            p_now->rsp_body.now_size = 0;
            if (pf_option != NULL)
            {
                p_now->pf_set = pf_option;
            }
            e_code = default_setopt_http_request(p_now);
            e_code = (*p_now->pf_set)(p_now);
            if (e_code != E_SUCCESS)
            {
                e_code = E_CALLBACK_FUNCTION;
            }

            p_now->pf_resp = pf_resp;
            p_handle->pool_last_used  = i;

            *pp_req = p_now;

            return (e_code);
        }

        i = (i == (p_handle->pool_size-1)) ? 0 : i+1;
    }
    while (i != p_handle->pool_last_used);

    return (E_BUSY);
}



e_http_error_code_t
destroy_http_request (pst_http_request_t    p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;
    curl_easy_cleanup (p_req->p_context);

    return (e_code);
}



/* **************************************************************************
 *	@brief      add_http_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
add_http_handle (pst_http_handle_t  p_handle,
                 pst_http_request_t p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;

    if (curl_multi_add_handle (p_handle->p_context, p_req->p_context)
            != CURLM_OK)
        e_code = E_FAILURE;

    return (e_code);
}


static
e_http_error_code_t
remove_http_handle (pst_http_handle_t  p_handle,
                    pst_http_request_t p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;

    if (curl_multi_remove_handle (p_handle->p_context, p_req->p_context)
            != CURLM_OK)
        e_code = E_FAILURE;

    return (e_code);
}




/* **************************************************************************
 *	@brief      perform_multi_wait_http_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/E_FAILURE
 * **************************************************************************/

e_http_error_code_t
perform_multi_wait_http_handle (pst_http_handle_t  p_handle,
                                pst_http_request_t  p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;
    struct timeval      timeout;
    int                 rc;   /* select() return code */
    fd_set              fdread, fdwrite, fdexcep;
    int                 maxfd = -1;
    long                curl_timeo = -1;


    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    if (p_req != NULL)
    {
        try_exception ((*p_handle->pf_add)(p_handle, p_req) != E_SUCCESS,
                       exception_multi_add_handle);
    }

    try_assert (curl_multi_timeout (p_handle->p_context, &curl_timeo)
                != CURLM_OK);
    if (curl_timeo == -1)
    {
        curl_timeo = 980;
    }

    timeout.tv_sec = curl_timeo / 1000;
    if(timeout.tv_sec > 1)
    {
        timeout.tv_sec = 1;
    }
    else
    {
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    try_exception (curl_multi_fdset (p_handle->p_context,
                                     &fdread,
                                     &fdwrite, &fdexcep, &maxfd)
                   != CURLM_OK,
                   exception_curl_multi_fdset);

    rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    if (rc > -1)
    {
        (void) curl_multi_perform(p_handle->p_context,
                                  &p_handle->still_running);

        (void) process_http_rsp_done (p_handle);
    }


    try_catch (exception_multi_add_handle)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_curl_multi_fdset)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      simple_setopt_http_multi_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

static
e_http_error_code_t
simple_setopt_http_handle (pst_http_handle_t  p_handle)
{
    e_http_error_code_t e_code = E_SUCCESS;
    CURLMcode   curl_e_code    = CURLM_OK;


    /*  1. PIPELINING OPTION (HTTP1.1, HTTP/2.0)    */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_PIPELINING,
                                         CURLPIPE_MULTIPLEX))
                    != CURLM_OK,
                    exception_curl_multi_setopt);

    /*  2. set curl multi-handle option */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_MAX_HOST_CONNECTIONS,
                                         1L))
                    != CURLM_OK,
                    exception_curl_multi_setopt);
    p_handle->result_ecode = curl_e_code;


    try_catch (exception_curl_multi_setopt)
    {
        p_handle->result_ecode = curl_e_code;
        HTTP_LOG ("fail, %s\n", curl_multi_strerror (curl_e_code));
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}





/* **************************************************************************
 *	@brief      perform_multi_socket_http_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/E_FAILURE
 * **************************************************************************/

static
void
perform_multi_socket_http_handle (uv_poll_t *p_req,
                                  int        status,
                                  int        events)
{
    int                 running_handles = 0;
    int                 flags           = 0;
    pst_http_context_t  p_context       = NULL;


    UNUSED (status);

    if(events & UV_READABLE) flags |= CURL_CSELECT_IN;
    if(events & UV_WRITABLE) flags |= CURL_CSELECT_OUT;

    p_context = (pst_http_context_t) p_req->data;

    curl_multi_socket_action (p_context->p_handle->p_context,
                              p_context->sockfd, flags,
                              &running_handles);


    (void) process_http_rsp_done (p_context->p_handle);

    return;
}




/* **************************************************************************
 *	@brief      on_timeout
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param      [IN] req  - uv timer object
 *  @retval     none
 * **************************************************************************/

static
void
on_timeout (uv_timer_t *req)
{
    int running_handles;


    UNUSED (req);
    /*  NOTICE !!
    curl_multi_socket_action (curl_handle, */

    curl_multi_socket_action (NULL,
                              CURL_SOCKET_TIMEOUT,
                              0,
                              &running_handles);
    /*
    process_http_rsp_done ();
    */

    return;
}




/* **************************************************************************
 *	@brief      start_timeout
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param      [IN] multi      - multi handle
 *              [IN] timeout_ms
 *              [IN] userp      - private callback pointer
 *  @retval     none
 * **************************************************************************/

static
int
start_timeout (CURLM    *multi,
               long     timeout_ms,
               void     *userp)
{
    pst_http_handle_t   p_handle = (pst_http_handle_t) userp;

    UNUSED (multi);
    if (timeout_ms < 0)
    {
        uv_timer_stop(&p_handle->timer);
    }
    else
    {
        if(timeout_ms == 0)
        {
            /* 0 means directly call socket_action, but we'll do it in a bit */
            timeout_ms = 1;
        }
        uv_timer_start(&p_handle->timer, on_timeout, timeout_ms, 0);
    }

    return 0;
}




/* **************************************************************************
 *	@brief      init_curl_context
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
static
pst_http_context_t
init_curl_context (pst_http_handle_t  p_handle,
                   curl_socket_t      sockfd)
{
    pst_http_context_t  p_context = NULL;

    p_context = (pst_http_context_t) MALLOC (sizeof *p_context);
    p_context->p_handle = p_handle;
    p_context->sockfd   = sockfd;

    uv_poll_init_socket (p_handle->p_loop, &p_context->poll_handle, sockfd);
    p_context->poll_handle.data = p_context;

    return p_context;
}

static
void
curl_close_cb (uv_handle_t  *p_handle)
{
    pst_http_context_t  p_context = (pst_http_context_t)p_handle->data;

    FREE (p_context);
}


static
void
destroy_curl_context (pst_http_context_t    p_context)
{
    uv_close ((uv_handle_t *) &p_context->poll_handle, curl_close_cb);
    return;
}


/* **************************************************************************
 *	@brief      handle_socket
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param      [IN]    easy handle
 *              [IN]    socket
 *              [IN]    describes the socket
 *              [IN]    private callback pointer
 *              [IN]    private socket pointer
 *  @retval     none
 * **************************************************************************/

static
int
handle_socket (CURL           *easy,
               curl_socket_t  s,
               int            action,
               void           *userp,
               void           *socketp)
{
    pst_http_handle_t   p_handle       = (pst_http_handle_t)userp;
    pst_http_context_t  p_curl_context = NULL;
    int                 events = 0;

    UNUSED (easy);

    switch(action)
    {
        case CURL_POLL_IN:
        case CURL_POLL_OUT:
        case CURL_POLL_INOUT:
            p_curl_context = socketp ?
                            (pst_http_context_t) socketp
                            : init_curl_context (p_handle, s);

            curl_multi_assign (p_handle->p_context,
                               s,
                               (void *) p_curl_context);

            if (action != CURL_POLL_IN)  events |= UV_WRITABLE;
            if (action != CURL_POLL_OUT) events |= UV_READABLE;

            uv_poll_start (&p_curl_context->poll_handle,
                            events,
                            perform_multi_socket_http_handle);
            break;

        case CURL_POLL_REMOVE:
            if (socketp != NULL)
            {
                uv_poll_stop (&((pst_http_context_t)socketp)->poll_handle);
                destroy_curl_context((pst_http_context_t) socketp);
                curl_multi_assign(p_handle->p_context, s, NULL);
            }
            break;
        default:
            abort();
    }

    return (0);
}




/* **************************************************************************
 *	@brief      socket_setopt_http_multi_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

static
e_http_error_code_t
socket_setopt_http_handle (pst_http_handle_t  p_handle)
{
    e_http_error_code_t e_code = E_SUCCESS;
    CURLMcode      curl_e_code = CURLM_OK;


    /*  1. PIPELINING OPTION (HTTP1.1, HTTP/2.0)    */
    try_exception ((curl_e_code = curl_multi_setopt (p_handle->p_context,
                                                     CURLMOPT_PIPELINING,
                                                     CURLPIPE_MULTIPLEX))
                   != CURLM_OK,
                   exception_curl_multi_setopt);

    /*  2. set curl multi-handle option */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_MAX_HOST_CONNECTIONS,
                                         1L))
                    != CURLM_OK,
                    exception_curl_multi_setopt);

    /*  3. set curl socket function     */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_SOCKETFUNCTION,
                                         handle_socket))
                    != CURLM_OK,
                    exception_curl_multi_setopt);
    try_exception ((curl_e_code = curl_multi_setopt (p_handle->p_context,
                                                     CURLMOPT_SOCKETDATA,
                                                     (void *)p_handle))
                   != CURLM_OK,
                   exception_curl_multi_setopt);

    /*  4. set curl timer function     */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_TIMERFUNCTION,
                                         start_timeout))
                    != CURLM_OK,
                    exception_curl_multi_setopt);
    try_exception ((curl_e_code = curl_multi_setopt (p_handle->p_context,
                                                     CURLMOPT_TIMERDATA,
                                                     (void *)p_handle))
                   != CURLM_OK,
                   exception_curl_multi_setopt);
    p_handle->result_ecode = curl_e_code;


    try_catch (exception_curl_multi_setopt)
    {
        p_handle->result_ecode = curl_e_code;
        HTTP_LOG ("fail, %s\n", curl_multi_strerror (curl_e_code));
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      perform_http_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

e_http_error_code_t
perform_http_handle (pst_http_handle_t  p_handle,
                     pst_http_request_t  p_req)
{
    return ((*p_handle->pf_perform)(p_handle, p_req));
}




/* **************************************************************************
 *	@brief      init_http_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

e_http_error_code_t
init_http_handle (pst_http_handle_t                *pp_handle,
                  e_http_handle_mode_t              mode,
                  e_http_error_code_t (*pf_option) (pst_http_handle_t))
{
    pst_http_handle_t   p_handle = NULL;
    int                 i        = 0;


    /*  1. alloc http handle    */
    p_handle = (pst_http_handle_t) MALLOC (sizeof (st_http_handle_t));
    if (p_handle == NULL)
    {
        HTTP_LOG ("fail, memory_alloc\n");
        return (E_ALLOC_HANDLE);
    }
    p_handle->mode = mode;

    /*  2. alloc http context   */
    p_handle->p_context = curl_multi_init();
    if (p_handle->p_context == NULL)
    {
        HTTP_LOG ("fail, alloc http handle\n");
        FREE (p_handle);
        return (E_ALLOC_HANDLE);
    }

    /*  3. register handle function */
    p_handle->pf_set     = simple_setopt_http_handle;
    p_handle->pf_perform = perform_multi_wait_http_handle;
    if (pf_option != NULL)
    {
        p_handle->pf_set = pf_option;
    }
    p_handle->pf_add     = add_http_handle;
    p_handle->pf_remove  = remove_http_handle;
    if (p_handle->mode == HTTP_SOCKET_MODE)
    {
        p_handle->pf_set     = socket_setopt_http_handle;
    }


    (void) simple_setopt_http_handle (p_handle);
    if ((*p_handle->pf_set)(p_handle) != E_SUCCESS)
    {
        FREE (p_handle);
        return (E_ALLOC_HANDLE);
    }
    p_handle->pool_size      = HTTP_MAX_REQ_POOL_SIZE;
    p_handle->pool_last_used = 0;
    p_handle->pool_last_free = 0;
    p_handle->p_pool     = (pst_http_request_t)
                           CALLOC (sizeof (st_http_request_t),
                           p_handle->pool_size);
    if (p_handle->p_pool == NULL)
    {
        HTTP_LOG ("fail, alloc http request handle\n");
        FREE (p_handle);
        return (E_ALLOC_HANDLE);
    }

    for(i=0; i < p_handle->pool_size; i++)
    {
        p_handle->p_pool[i].p_context  = curl_easy_init();
        p_handle->p_pool[i].index      = i;
        p_handle->p_pool[i].is_pending = BOOL_FALSE;

        p_handle->p_pool[i].rsp_header.p_mem = NULL;
        p_handle->p_pool[i].rsp_body.p_mem   = NULL;
    }

    (*pp_handle) = p_handle;

    return (E_SUCCESS);
}




/* **************************************************************************
 *	@brief      destory_http_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

e_http_error_code_t
destory_http_handle (pst_http_handle_t    p_handle)
{
    if (p_handle == NULL)
    {
        return (E_INVALID_ARG);
    }

    (void) curl_multi_cleanup (p_handle->p_context);
    (void) FREE (p_handle);

    return (E_SUCCESS);
}

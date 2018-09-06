/* ***************************************************************************
 *
 *       Filename:  hcurl.c
 *    Description:  http curl
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hcurl.h>
#include <heigw.h>


/* **************************************************************************
 *  @brief          show_http_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
void
show_http_handle (pst_http_handle_t  p_handle,
                  char              *p_module)
{
    char    *p_name;

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [http req timeout     : %u ms]\n",
            p_name,
            p_handle->timer.req_timeout_ms);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [http conn timeout    : %u ms]\n",
            p_name,
            p_handle->timer.con_timeout_ms);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [max host connections : %u ]\n",
            p_name,
            p_handle->connections.max_host);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [max total connections: %u ]\n",
            p_name,
            p_handle->connections.max_total);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   verify         : %s]\n",
            p_name,
            p_handle->ssl.verify ? "ON" : "OFF");

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   ca path        : %s]\n",
            p_name,
            p_handle->ssl.ca_path);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   ca info        : %s]\n",
            p_name,
            p_handle->ssl.ca_info);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   cert           : %s]\n",
            p_name,
            p_handle->ssl.cert_fname);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   key            : %s]\n",
            p_name,
            p_handle->ssl.key_fname);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   key passwd     : %c%cxxx]\n",
            p_name,
            p_handle->ssl.key_passwd[0] == 0x00
                                ? '-' : p_handle->ssl.key_passwd[0],
            p_handle->ssl.key_passwd[1] == 0x00
                                ? '-' : p_handle->ssl.key_passwd[1]);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [ssl   key type       : %s]\n",
            p_name,
            p_handle->ssl.key_type);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [header   agent name  : %s]\n",
            p_name,
            p_handle->header.agent_name);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [pending buffer count : %d]\n",
            p_name,
            p_handle->size_of_pqueue);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [debug vorbose        : %d]\n",
            p_name,
            p_handle->verbose );

    return;
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
    pst_mchunk_handle_t p_chunk = p_req->rsp.p_header;

    if ((*p_chunk->pf_add)(p_chunk, p_ptr, size * nmemb) != E_SUCCESS)
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
    pst_mchunk_handle_t p_chunk  = p_req->rsp.p_body;

    HTTP_LOG ("succ, receive http body [size:%d, nmemb:%d]\n",
               (int)size, (int)nmemb);

    if ((*p_chunk->pf_add)(p_chunk, p_ptr, size * nmemb) != E_SUCCESS)
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
    CURLcode            c_code = CURLE_OK;
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
            if ((c_code = curl_easy_getinfo (p_msg->easy_handle,
                                            CURLINFO_PRIVATE,
                                            &p_req)) == CURLE_OK)
            {

                (void)
                curl_easy_getinfo (p_msg->easy_handle, CURLINFO_EFFECTIVE_URL,
                                   &p_req->rsp.p_done_url);

                /* --------------------------------------------------------
                 *  The stored value will be zero
                 *  if no server response code has been received
                 *  (HTTP_RESULT_UNKNOWN = 0)
                 * -------------------------------------------------------- */
                (void)
                curl_easy_getinfo (p_msg->easy_handle, CURLINFO_RESPONSE_CODE,
                                   &p_req->rsp.status_code);

                (void)
                curl_easy_getinfo (p_msg->easy_handle, CURLINFO_CONTENT_TYPE,
                                   &p_req->rsp.p_content_type);

                (void)
                curl_easy_getinfo (p_msg->easy_handle,
                                   CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                                   &p_req->rsp.content_length);
            }
            else
            {
                p_req->rsp.status_code = HTTP_RESULT_UNKNOWN;
                p_req->err_code        = c_code;
                HTTP_REQUEST_INTERNAL_ERROR(p_req);
            }

            /*  timer off (option) */
            (void) curl_easy_setopt  (p_msg->easy_handle,
                                      CURLOPT_TIMEOUT,   0L);

            if (p_req->pf_resp != NULL)
            {
                (*p_req->pf_resp)(p_req);
            }

            /*  for removing from  multi-block */
            if (p_req->p_header) curl_slist_free_all (p_req->p_header);
            p_req->p_header = NULL;
            p_req->is_done = BOOL_TRUE;

            (*p_handle->p_pending_queue->pf_free)(p_handle->p_pending_queue, p_req);
        }
    } while (p_msg != NULL);


    return (e_code);
}




/* **************************************************************************
 *	@brief     DEBUG LOG FUNCTION
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

static
int
log_http_request (CURL          *p_handle,
                  curl_infotype type,
                  char          *p_data,
                  size_t        size,
                  void          *userp)
{
    const char   *text;

    /* prevent compiler warning */
    UNUSED (p_handle);
    UNUSED (userp);
    UNUSED (size);


    switch (type)
    {
        case CURLINFO_TEXT:
            Log (DEBUG_LOW,
                    "info, (handle:%x:%x) %s",
                    userp, p_handle, p_data);
            /* FALLTHROUGH */
        default: /* in case a new one is introduced to shock us */
            return 0;

        case CURLINFO_HEADER_OUT:
            Log (DEBUG_LOW,
                    "info, (handle:%x:%x) [OUT_HEADER] %s",
                    userp, p_handle, p_data);
            break;
        case CURLINFO_DATA_OUT:
            Log (DEBUG_LOW,
                    "info, (handle:%x:%x) [OUT_BODY  ] %s\n",
                    userp, p_handle, p_data);
            break;
        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            Log (DEBUG_LOW,
                    "info, (handle:%x:%x) [IN_HEADER ] %s",
                    userp, p_handle, p_data);
            break;
        case CURLINFO_DATA_IN:
            Log (DEBUG_LOW,
                    "info, (handle:%x:%x) [IN_BODY   ] %s\n",
                    userp, p_handle, p_data);
            break;
        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }

    UNUSED (text);
    /*
    dump(text, num, (unsigned char *)data, size, 1);
    */

    return (0);
}




/* **************************************************************************
 *	@brief      PROCESS SOCKET CLOSE EVENT
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

int
process_http_socket_close (void *p_arg, curl_socket_t   item)
{
    pst_http_request_t  p_req = (pst_http_request_t)p_arg;

    Log (DEBUG_CRITICAL,
            "info, socket close [handle(%x:%x), uri(%s %s)]\n",
            p_req->p_multi,
            p_req->p_context,
            p_req->method,
            p_req->uri);

    close ((int) item);

    return (0);
}




/* **************************************************************************
 *	@brief      SET EASY HANDLE DEBUG OPTION
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
setopt_debug_http_request_default (pst_http_request_t   p_req)
{
    e_error_code_t  e_code    = E_SUCCESS;
    CURL            *p_handle = p_req->p_context;


    /* --------------------------------------------------------------------
     * please be verbose
     * curl_easy_setopt (p_handle, CURLOPT_DEBUGFUNCTION, NULL);
    * --------------------------------------------------------------------- */
    if (((pst_http_handle_t)(p_req->p_multi))->verbose == BOOL_TRUE)
    {
        try_exception (curl_easy_setopt (p_handle, CURLOPT_VERBOSE, 1L)
                       != CURLE_OK,
                       exception_setopt_debug_option);

        try_exception (curl_easy_setopt (p_handle, CURLOPT_DEBUGFUNCTION,
                                         log_http_request)
                       != CURLE_OK,
                       exception_setopt_debug_option);

        try_exception (curl_easy_setopt (p_handle, CURLOPT_DEBUGDATA,
                                         p_req->p_multi)
                       != CURLE_OK,
                       exception_setopt_debug_option);
    }
    else
    {
        curl_easy_setopt (p_handle, CURLOPT_VERBOSE, 0L);
    }

    try_catch (exception_setopt_debug_option)
    {
        Log (DEBUG_ERROR,
                "fail, http request(easy handle) invalid DEBUG option\n");
        e_code = E_INVALID_ARG;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      SET EASY HANDLE SSL OPTION
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
setopt_ssl_http_request_default (pst_http_request_t   p_req)
{
    e_http_error_code_t e_code    = E_SUCCESS;
    pst_http_handle_t   p_multi   = (pst_http_handle_t)(p_req->p_multi);
    CURL                *p_handle = p_req->p_context;


    /* ---------------------------------------------------------------------
     *   VERIFY PEER
     *  -------------------------------------------------------------------- */
    if (((pst_http_handle_t)(p_req->p_multi))->ssl.verify == BOOL_TRUE)
    {
        try_exception (curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYPEER, 1L)
                       != CURLE_OK,
                       exception_setopt_ssl_option);
    }
    else
    {
        try_exception (curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYPEER, 0L)
                       != CURLE_OK,
                       exception_setopt_ssl_option);
        return (E_SUCCESS);
    }


    /* ---------------------------------------------------------------------
     *  CA INFORMATION
     *  -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_CAINFO,
                                     p_multi->ssl.ca_info)
                   != CURLE_OK,
                   exception_setopt_ssl_option);
    if (p_multi->ssl.ca_path[0] != 0x00)
    {
        try_exception (curl_easy_setopt (p_handle, CURLOPT_CAPATH,
                                         p_multi->ssl.ca_path)
                       != CURLE_OK,
                       exception_setopt_ssl_option);
    }

    if (p_multi->ssl.crl_fname[0] != 0x00)
    {
        try_exception (curl_easy_setopt (p_handle, CURLOPT_CRLFILE,
                                         p_multi->ssl.crl_fname)
                       != CURLE_OK,
                       exception_setopt_ssl_option);
    }

    /* ---------------------------------------------------------------------
     *  support key filename
     *  -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_SSLCERT,
                                     p_multi->ssl.cert_fname)
                    != CURLE_OK,
                    exception_setopt_ssl_option);

    /* ---------------------------------------------------------------------
     *  support key type : "PEM", "DER", "ENG"
     *  -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_SSLCERTTYPE,
                                     p_multi->ssl.cert_type)
                   != CURLE_OK,
                   exception_setopt_ssl_option);


    /* ---------------------------------------------------------------------
     *  support key filename
     *  -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_SSLKEY,
                                     p_multi->ssl.key_fname)
                   != CURLE_OK,
                   exception_setopt_ssl_option);

    /* ---------------------------------------------------------------------
     *  support key passwd
     *  -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_KEYPASSWD,
                                     p_multi->ssl.key_passwd)
                   != CURLE_OK,
                   exception_setopt_ssl_option);

    /* ---------------------------------------------------------------------
     *  support key type : "PEM", "DER", "ENG"
     *  -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_SSLKEYTYPE,
                                     p_multi->ssl.key_type)
                   != CURLE_OK,
                   exception_setopt_ssl_option);


    /* --------------------------------------------------------------------
     *  unsupport VERIFYHOST option
     *  LOG: CURLOPT_SSL_VERIFYHOST no longer supports 1 as value
     *  curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYHOST, 1L);
     *  ------------------------------------------------------------------- */

    try_catch (exception_setopt_ssl_option)
    {
        Log (DEBUG_ERROR,
                "fail, http request(easy handle) invalid SSL option\n");
        e_code = E_INVALID_ARG;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      SET EASY HANDLE FUNCTION(HEADER/BODY) OPTION
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
setopt_function_http_request_default (pst_http_request_t   p_req)
{
    e_http_error_code_t e_code    = E_SUCCESS;
    CURL                *p_handle = p_req->p_context;


    /* --------------------------------------------------------------------
     * curl_easy_setopt (p_handle, CURLOPT_HEADER, 1L);
     * -------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_HEADERDATA, p_req)
                   != CURLE_OK,
                   exception_setopt_default_option);

    try_exception (curl_easy_setopt (p_handle, CURLOPT_HEADERFUNCTION,
                                     process_http_rsp_header)
                   != CURLE_OK,
                   exception_setopt_default_option);


    /* ---------------------------------------------------------------------
     * write to this file
     * --------------------------------------------------------------------- */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_WRITEDATA, p_req)
                   != CURLE_OK,
                   exception_setopt_default_option);
    try_exception (curl_easy_setopt (p_handle, CURLOPT_WRITEFUNCTION,
                                     process_http_rsp_body)
                   != CURLE_OK,
                   exception_setopt_default_option);


    /* --------------------------------------------------------------------
     * curl_easy_setopt (p_handle,
     *                   CURLOPT_OPENSOCKETFUNCTION, opensocket_callback);
     * curl_easy_setopt (p_handle,
     *                   CURLOPT_OPENSOCKETDATA, &fd);
     *
     * curl_easy_setopt (p_handle,
     *                   CURLOPT_CLOSESOCKETFUNCTION, opensocket_callback);
     * curl_easy_setopt (p_handle,
     *                   CURLOPT_OPENSOCKETDATA, &fd);
     * --------------------------------------------------------------------- */
    /*
    try_exception (curl_easy_setopt (p_handle, CURLOPT_CLOSESOCKETDATA,p_req)
                   != CURLE_OK,
                   exception_setopt_default_option);
    try_exception (curl_easy_setopt (p_handle, CURLOPT_CLOSESOCKETFUNCTION,
                                     process_http_socket_close)
                   != CURLE_OK,
                   exception_setopt_default_option);
                   */



    try_catch (exception_setopt_default_option)
    {
        Log (DEBUG_ERROR,
                "fail, http request(easy handle) invalid FUNCTION option\n");
        e_code = E_INVALID_ARG;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      setopt_http_request_default
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
setopt_http_request_default (pst_http_request_t   p_req)
{
    e_http_error_code_t e_code    = E_SUCCESS;
    CURL                *p_handle = p_req->p_context;


    /* HTTP/2 please */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_HTTP_VERSION,
                                     CURL_HTTP_VERSION_2_0)
                   != CURLE_OK,
                   exception_setopt_default_option);

    /*  Private option */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_PRIVATE, p_req)
                   != CURLE_OK,
                   exception_setopt_default_option);

    /*  set interface error string */
    try_exception (curl_easy_setopt (p_handle, CURLOPT_ERRORBUFFER,
                                     p_req->err_string)
                   != CURLE_OK,
                   exception_setopt_default_option);

#if (CURLPIPE_MULTIPLEX > 0)
    /* wait for pipe connection to confirm */
    curl_easy_setopt (p_handle, CURLOPT_PIPEWAIT, 0L);
#endif

    e_code = setopt_function_http_request_default (p_req);
    e_code = setopt_ssl_http_request_default (p_req);
    e_code = setopt_debug_http_request_default (p_req);


    try_catch (exception_setopt_default_option)
    {
        Log (DEBUG_ERROR,
                "fail, http request(easy handle) invalid general option\n");
        e_code = E_INVALID_ARG;
    }
    try_finally;

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
                   e_http_error_code_t  (*pf_resp)   (pst_http_request_t))
{
    e_http_error_code_t e_code = E_SUCCESS;
    pst_http_request_t  p_now  = NULL;
    CURL                *p_tmp = NULL;


    e_code = (*p_handle->p_pending_queue->pf_alloc)(p_handle->p_pending_queue,
                                           (void **)&p_now);
    try_exception (e_code != E_SUCCESS, exception_alloc_pool);


    /*  remove CURL from CURL MULTI HANDLE */
    if (p_now->is_done == BOOL_TRUE)
    {
        p_handle->err_code = curl_multi_remove_handle (p_handle->p_context,
                                                 p_now->p_context);
        if (p_handle->err_code != CURLM_OK)
        {
            Log (DEBUG_CRITICAL,
                    "fail, %s\n", curl_multi_strerror (p_handle->err_code));
            abort();
        }

        try_assert((p_tmp = curl_easy_duphandle (p_now->p_context)) == NULL);
        (void) curl_easy_cleanup (p_now->p_context);
        p_now->p_context = p_tmp;
    }

    p_now->p_multi                = p_handle;
    p_now->is_done                = BOOL_FALSE;
    p_now->rsp.p_header->now_size = 0;
    p_now->rsp.p_body->now_size   = 0;
    p_now->rsp.status_code        = HTTP_RESULT_UNKNOWN;
    p_now->pf_resp                = pf_resp;

    /* --------------------------------------------------------------
     *  NOT USED
     * e_code = setopt_http_request_default(p_now);
     * ------------------------------------------------------------ */

    /*  used defined option */
    if (pf_option != NULL)
    {
        p_now->pf_set = pf_option;
        try_exception ((e_code = (*p_now->pf_set)(p_now)) != E_SUCCESS,
                       exception_user_callback);
    }

    (*pp_req) = p_now;


    try_catch (exception_alloc_pool)
    {
        Log (DEBUG_ERROR,
                "info, busy http memory pool [%p: total(%d)]\n",
               p_handle->p_pending_queue,
               p_handle->p_pending_queue->num_of_block);
        e_code = E_BUSY;
    }
    try_catch (exception_user_callback)
    {
        (void) (*p_handle->p_pending_queue->pf_free)(p_handle->p_pending_queue,
                                             (void **)&p_now);
        e_code = E_CALLBACK_FUNCTION;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      destroy_http_request
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/

e_http_error_code_t
destroy_http_request (pst_http_request_t    p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;

    (void) curl_easy_cleanup (p_req->p_context);
    p_req->p_context = NULL;

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

    p_handle->err_code = curl_multi_add_handle (p_handle->p_context,
                                          p_req->p_context);
    if (p_handle->err_code != CURLM_OK)
    {
        strcpy (p_handle->err_string,
                curl_multi_strerror (p_handle->err_code));
        e_code = E_FAILURE;
    }

    return (e_code);
}


static
e_http_error_code_t
remove_http_handle (pst_http_handle_t  p_handle,
                    pst_http_request_t p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;

    p_handle->err_code = curl_multi_remove_handle (p_handle->p_context,
                                             p_req->p_context);
    if (p_handle->err_code != CURLM_OK)
    {
        strcpy (p_handle->err_string,
                curl_multi_strerror (p_handle->err_code));
        e_code = E_FAILURE;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      perform_http_handle_simple
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/E_FAILURE
 * **************************************************************************/

e_http_error_code_t
perform_http_handle_simple (pst_http_handle_t  p_handle,
                            pst_http_request_t  p_req)
{
    e_http_error_code_t e_code = E_SUCCESS;
    int                 now_running;
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

        now_running = p_handle->still_running + 1;
        try_exception ((p_handle->err_code
                        = curl_multi_perform(p_handle->p_context,
                                             &p_handle->still_running))
                        != CURLM_OK,
                        exception_curl_multi_perform);
        if (p_handle->still_running < now_running)
        {
            (void) process_http_rsp_done (p_handle);
        }
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
    try_exception ((p_handle->err_code
                    = curl_multi_fdset (p_handle->p_context,
                                        &fdread,
                                        &fdwrite, &fdexcep, &maxfd))
                   != CURLM_OK,
                   exception_curl_multi_fdset);

    rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    if (rc >= 0)
    {
        now_running = p_handle->still_running;
        try_exception ((p_handle->err_code
                        = curl_multi_perform(p_handle->p_context,
                                             &p_handle->still_running))
                        != CURLM_OK,
                        exception_curl_multi_perform);

        if (p_handle->still_running < now_running)
        {
            (void) process_http_rsp_done (p_handle);
        }
    }


    try_catch (exception_multi_add_handle)
    {
        strcpy (p_handle->err_string,
                curl_multi_strerror (p_handle->err_code));
        e_code = E_FAILURE;
    }
    try_catch (exception_curl_multi_fdset)
    {
        strcpy (p_handle->err_string,
                curl_multi_strerror (p_handle->err_code));
        e_code = E_FAILURE;
    }
    try_catch (exception_curl_multi_perform)
    {
        strcpy (p_handle->err_string,
                curl_multi_strerror (p_handle->err_code));
        try_assert (p_handle->err_code == CURLM_INTERNAL_ERROR);
        e_code = E_FAILURE;
    }
    try_finally;

    /*
    (void) process_http_rsp_done (p_handle);
    */

    return (e_code);
}




/* **************************************************************************
 *	@brief      setopt_http_handle_simple
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

static
e_http_error_code_t
setopt_http_handle_simple (pst_http_handle_t  p_handle)
{
    e_http_error_code_t e_code = E_SUCCESS;


    /*  1. PIPELINING OPTION (HTTP1.1, HTTP/2.0)    */
    try_exception ((p_handle->err_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_PIPELINING,
                                         CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX))
                    != CURLM_OK,
                    exception_curl_multi_setopt);

    /*  2. set curl multi-handle option */
    try_exception ((p_handle->err_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_MAX_HOST_CONNECTIONS,
                                         p_handle->connections.max_host))
                    != CURLM_OK,
                    exception_curl_multi_setopt);


    try_catch (exception_curl_multi_setopt)
    {
        strcpy (p_handle->err_string,
                curl_multi_strerror (p_handle->err_code));
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
    p_handle->last_tick = time (NULL);

    return ((*p_handle->pf_perform)(p_handle, p_req));
}




/* **************************************************************************
 *  @brief          load_http_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
load_http_handle_option (pst_http_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];

    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SIZE_PENDING_QUEUE_NAME,
                        HTTP_CONFIG_SIZE_PENDING_QUEUE_DEFAULT,
                        buf);
    p_handle->size_of_pqueue = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_VERSION_NAME,
                        HTTP_CONFIG_VERSION_DEFAULT,
                        buf);
    p_handle->hversion = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_DEBUG_VERBOSE_NAME,
                        HTTP_CONFIG_DEBUG_VERBOSE_DEFAULT,
                        buf);
    if (buf[0] == '1')
        p_handle->verbose = BOOL_TRUE;
    else
        p_handle->verbose = BOOL_FALSE;


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_TIMER_REQ_TIMEOUT_MS_NAME,
                        HTTP_CONFIG_TIMER_REQ_TIMEOUT_MS_DEFAULT,
                        buf);
    p_handle->timer.req_timeout_ms = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_TIMER_CON_TIMEOUT_MS_NAME,
                        HTTP_CONFIG_TIMER_CON_TIMEOUT_MS_DEFAULT,
                        buf);
    p_handle->timer.con_timeout_ms = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_CONNECTIONS_MAX_HOST_NAME,
                        HTTP_CONFIG_CONNECTIONS_MAX_HOST_DEFAULT,
                        buf);
    p_handle->connections.max_host = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_CONNECTIONS_MAX_TOTAL_NAME,
                        HTTP_CONFIG_CONNECTIONS_MAX_TOTAL_DEFAULT,
                        buf);
    p_handle->connections.max_total = atoi (buf);



    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_VERIFY,
                        HTTP_CONFIG_SSL_VERIFY_DEFAULT,
                        buf);
    if (buf[0] == '1')
        p_handle->ssl.verify = BOOL_TRUE;
    else
        p_handle->ssl.verify = BOOL_FALSE;


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_CA_PATH_NAME,
                        HTTP_CONFIG_SSL_CA_PATH_DEFAULT,
                        p_handle->ssl.ca_path);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_CA_INFO_NAME,
                        NULL,
                        p_handle->ssl.ca_info);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_CRL_FILE_NAME,
                        NULL,
                        p_handle->ssl.crl_fname);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_CERT_FILE_NAME,
                        NULL,
                        p_handle->ssl.cert_fname);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_CERT_TYPE_NAME,
                        HTTP_CONFIG_SSL_CERT_TYPE_DEFAULT,
                        p_handle->ssl.cert_type);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_KEY_NAME,
                        NULL,
                        p_handle->ssl.key_fname);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_KEY_PASSWD_NAME,
                        NULL,
                        p_handle->ssl.key_passwd);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_SSL_KEY_TYPE_NAME,
                        NULL,
                        p_handle->ssl.key_type);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        HTTP_CONFIG_HEADER_AGENT_NAME,
                        HTTP_CONFIG_HEADER_AGENT_DEFAULT,
                        p_handle->header.agent_name);


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      pre_alloc_easy_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param      p_mem [IN] - http request  (curl easy)
 *  @param      p_arg [IN] - http handle   (curl multi)
 *  @retval     SUCC(0)
 * **************************************************************************/
static
e_error_code_t
pre_alloc_easy_handle (void *p_mem, void *p_arg)
{
    int                 user_size = 0;
    pst_http_request_t  p_tmp = (pst_http_request_t)p_mem;


    /*  init curl easy handle                   */
    p_tmp->p_context = curl_easy_init();
    if (p_tmp->p_context == NULL)
        return (E_FAILURE);
    p_tmp->p_multi = (pst_http_handle_t)p_arg;


    /*  set easy handle default option          */
    if (setopt_http_request_default(p_tmp) != E_SUCCESS)
        return (E_FAILURE);


    /*  alloc easy handle body buffer           */
    if (init_mchunk_handle (&(p_tmp->rsp.p_header), 0) != E_SUCCESS)
    {
        return (E_FAILURE);
    }

    if (init_mchunk_handle (&(p_tmp->rsp.p_body), 0) != E_SUCCESS)
    {
        return (E_FAILURE);
    }


    /* -----------------------------------------------------------
     * ISSUE : remove Application Dependency"
     *  application pending information buffer
     * ----------------------------------------------------------- */
    user_size = MEMORY_ALIGNED(offsetof (st_eigw_request_t, body.data) + 256,4);
    p_tmp->p_user2 = MALLOC (user_size);
    if (p_tmp->p_user2 == NULL)
    {
        return (E_FAILURE);
    }

    return (E_SUCCESS);
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
                  char                             *p_fname,
                  char                             *p_section,
                  e_http_handle_mode_t              mode,
                  e_http_error_code_t (*pf_option) (pst_http_handle_t))
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_http_handle_t   p_handle = NULL;


    /*  1. alloc http handle    */
    try_exception ((p_handle =
                    (pst_http_handle_t) malloc (sizeof (st_http_handle_t)))
                   == NULL,
                   exception_alloc_handle);
    memset (p_handle, 0x00, sizeof (*p_handle));
    p_handle->mode      = mode;
    p_handle->last_tick = time (NULL);

    /*  2. alloc http context   */
    try_exception ((p_handle->p_context = curl_multi_init()) == NULL,
                   exception_init_multi);

    /*  3. load config option   */
    p_handle->size_of_pqueue = atoi (HTTP_CONFIG_SIZE_PENDING_QUEUE_DEFAULT);
    if (p_fname != NULL)
    {
        /*  define current message  */
        strcpy (p_handle->cfname,   p_fname);
        if (p_section)
            strcpy (p_handle->csection, p_section);
        else
            strcpy (p_handle->csection,
                    (const char *) HTTP_CONFIG_SECTION_NAME);

        try_exception (load_http_handle_option (p_handle) != E_SUCCESS,
                       exception_init_handle);
    }

    /*  4. register handle function */
    p_handle->pf_set     = setopt_http_handle_simple;
    p_handle->pf_perform = perform_http_handle_simple;
#ifdef _UV_LIB_SUPPORT
    if (p_handle->mode == HTTP_SOCKET_MODE)
    {
        p_handle->pf_set     = setopt_http_handle_socket;
        p_handle->pf_perform = perform_http_handle_socket;
    }
#endif

    if (pf_option != NULL)
    {
        /*  overloading default option  */
        p_handle->pf_set = pf_option;
    }
    p_handle->pf_add     = add_http_handle;
    p_handle->pf_remove  = remove_http_handle;
    p_handle->pf_done    = process_http_rsp_done;
    p_handle->pf_show    = show_http_handle;


    try_exception (((*p_handle->pf_set)(p_handle) != E_SUCCESS) != E_SUCCESS,
                   exception_init_handle);

    try_exception ((e_code = init_mpool_handle (&(p_handle->p_pending_queue),
                                                p_handle->size_of_pqueue,
                                                sizeof (st_http_request_t),
                                                pre_alloc_easy_handle,
                                                (void *)p_handle))
                   != E_SUCCESS,
                   exception_init_mpool);

    (*pp_handle) = p_handle;


    try_catch (exception_alloc_handle)
    {
        Log (DEBUG_CRITICAL, "fail, alloc http handle \n");
        e_code = E_ALLOC_HANDLE;
    }
    try_catch (exception_init_multi)
    {
        Log (DEBUG_CRITICAL, "fail, init  http handle (curl_multi_init)\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_catch (exception_init_mpool)
    {
        Log (DEBUG_CRITICAL, "fail, init  http handle (mpool)\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_catch (exception_init_handle)
    {
        Log (DEBUG_CRITICAL, "fail, init  http handle [%s]\n",
                p_handle->err_string);

        if (p_handle->p_context)
        {
            (void) curl_multi_cleanup (p_handle->p_context);
        }

        if (p_handle)
        {
            free (p_handle);
        }
        e_code = E_ALLOC_HANDLE;
    }
    try_finally;

    return (e_code);
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

    curl_multi_cleanup (p_handle->p_context);
    FREE (p_handle);

    return (E_SUCCESS);
}




#ifdef _UV_LIB_SUPPORT

/* **************************************************************************
 *	@brief      perform_http_handle_socket
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/E_FAILURE
 * **************************************************************************/

static
void
perform_http_handle_socket (uv_poll_t *p_req,
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
                            perform_http_handle_socket);
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
 *	@brief      setopt_http_multi_handle_socket
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     SUCC(0)
 * **************************************************************************/

static
e_http_error_code_t
setopt_http_handle_socket (pst_http_handle_t  p_handle)
{
    e_http_error_code_t e_code = E_SUCCESS;
    CURLMcode      curl_e_code = CURLM_OK;


    /*  1. PIPELINING OPTION (HTTP1.1, HTTP/2.0)    */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_PIPELINING,
                                         CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX))
                    != CURLM_OK,
                    exception_curl_multi_setopt);

    /*  2. set curl multi-handle option */
    try_exception ((curl_e_code
                    = curl_multi_setopt (p_handle->p_context,
                                         CURLMOPT_MAX_HOST_CONNECTIONS,
                                         (long) p_handle->connections.max_host))
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
    p_handle->err_code = curl_e_code;


    try_catch (exception_curl_multi_setopt)
    {
        p_handle->err_code = curl_e_code;
        HTTP_LOG ("fail, %s\n", curl_multi_strerror (curl_e_code));
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}
#endif


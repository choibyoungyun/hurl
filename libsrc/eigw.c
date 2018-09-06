/* ***************************************************************************
 *
 *       Filename:  eigw.c
 *    Description:  eigw interface
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 *      show_eigw_handle                ()
 *      alloc_pending_queue_eigw_handle ()
 *      free_pending_queue_eigw_handle  ()
 *      encode_eigw_handle              ()
 *      set_errstring_eigw_handle       ()
 *      validate_body_eigw_handle       ()
 *      validate_header_eigw_handle     ()
 *      recv_eigw_handle                ()
 *      send_eigw_handle                ()
 *      heartbeat_eigw_handle           ()
 *      bind_eigw_handle                ()
 *      connect_eigw_handle             ()
 *      disconnect_eigw_handle          ()
 *      init_eigw_handle                ()
 *      destroy_socket_handle           ()
 * ***************************************************************************/

#include <heigw.h>


/* **************************************************************************
 *  @brief          show_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         void
 * **************************************************************************/
static
void
show_eigw_handle (pst_eigw_handle_t  p_handle,
                  char              *p_module)
{
    char    *p_name;

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [client identifier    : %02X%02X%02X%02X]\n",
            p_name,
            p_handle->client_id.system_id,
            p_handle->client_id.node_type,
            p_handle->client_id.node_id,
            p_handle->client_id.module_id);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [heartbeat interval   : %d]\n",
            p_name,
            p_handle->hb_interval);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [stream buf size      : %d]\n",
            p_name,
            p_handle->p_rbuf->bucket_size);


    (*p_handle->p_sock->pf_show)(p_handle->p_sock, p_name);

    return;
}




/* **************************************************************************
 *	@brief      alloc pending queue
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_error_code_t
alloc_pending_queue_eigw_handle (pst_eigw_handle_t    p_handle,
                                 pst_eigw_request_t   *pp_req)
{
    if (p_handle->p_pending_queue)
        return ((*p_handle->p_pending_queue->pf_alloc)(p_handle->p_pending_queue,
                                                       (void **)pp_req));
    return (E_SUCCESS);
}




/* **************************************************************************
 *	@brief      free memeory
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_error_code_t
free_pending_queue_eigw_handle (pst_eigw_handle_t    p_handle,
                                pst_eigw_request_t   *pp_req)
{
    e_error_code_t  e_code = E_SUCCESS;

    (*pp_req) = NULL;

    if (p_handle->p_pending_queue)
    {
        e_code
            = (*p_handle->p_pending_queue->pf_free)(p_handle->p_pending_queue,
                                                    *pp_req);
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      encode eigw handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
encode_eigw_handle (pst_eigw_request_t      p_req,
                    pst_eigw_response_t     p_rsp,
                    long                    status_code,
                    char                   *p_content_type,
                    double                  content_length,
                    pst_mchunk_handle_t     p_header,
                    pst_mchunk_handle_t     p_body,
                    char                   *p_err_string)
{
    short               *p_now;
    unsigned short      ind_now, ind_next;


    UNUSED (p_header);
    /* ----------------------------------------------------------------
     * set application response message header
     * ---------------------------------------------------------------- */
    memcpy   (&p_rsp->header, &p_req->header, sizeof (p_rsp->header));
    SET_MSGN_TYPE(p_rsp->header.unMsgName, MSG_TYPE_RSP);
    if (status_code != HTTP_RESULT_OK)
    {
        p_rsp->header.sRet     = (short) status_code;
    }
    else
    {
        p_rsp->header.sRet     = SUCC;
    }


    /* ----------------------------------------------------------------
     * set application response message body
     * ---------------------------------------------------------------- */
    memcpy   (p_rsp->body.method,
              p_req->body.method,
              sizeof (p_rsp->body.method));
    snprintf (p_rsp->body.status_code,
              sizeof (p_rsp->body.status_code),
             "%ld", status_code);


    p_now = (short *) ((char *)p_rsp
            + offsetof (st_eigw_response_t, body.ind1));

    /* ----------------------------------------------------------------
     * NETWORK INDETIFIER
     * ---------------------------------------------------------------- */
    ind_now = 0;
    strcpy (p_rsp->body.data + ind_now,
            p_req->body.data + p_req->body.ind1);
    ind_next = ind_now + strlen (p_rsp->body.data + ind_now) + 1;
    p_rsp->body.data [ind_next - 1] = '\0';
    *p_now = ind_now;

    /* ----------------------------------------------------------------
     * CONTENT-TYPE
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    if (p_content_type != NULL)
    {
        strcpy (p_rsp->body.data + ind_now, p_content_type);
    }
    else if (status_code >= E_PROTOCOL_SPEC)
    {
        strcpy (p_rsp->body.data + ind_now, "text/plain");
    }
    else
    {
        strcpy (p_rsp->body.data + ind_now, "unknown");
    }
    ind_next = ind_now + strlen (p_rsp->body.data + ind_now) + 1;
    p_rsp->body.data [ind_next - 1] = '\0';
    *(p_now + 1) = ind_now;


    /* ----------------------------------------------------------------
     * CONTENT-LENGTH
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    if (content_length != -1)
    {
        sprintf (p_rsp->body.data + ind_now,
                 "%ld", (long) content_length);
    }
    else if (status_code >= E_PROTOCOL_SPEC)
    {
        sprintf (p_rsp->body.data + ind_now,
                 "%ld", (long) strlen (p_err_string));
    }
    else if (p_body)
    {
        sprintf (p_rsp->body.data + ind_now,
                 "%ld", (long) p_body->now_size);
    }
    else
    {
        sprintf (p_rsp->body.data + ind_now,
                 "%ld", (long)-1);
    }
    ind_next = ind_now + strlen (p_rsp->body.data + ind_now) + 1;
    p_rsp->body.data [ind_next - 1] = '\0';
    *(p_now + 2) = ind_now;


    ind_now = ind_next;
    if (p_body && p_body->now_size != 0)
    {
        memcpy (p_rsp->body.data + ind_now,
                p_body->p_mem,
                p_body->now_size);
        content_length = p_body->now_size;
    }
    else if (status_code >= E_PROTOCOL_SPEC)
    {
        strcpy (p_rsp->body.data + ind_now, p_err_string);
        content_length = strlen (p_err_string);
    }
    ind_next = ind_now + content_length + 1;
    p_rsp->body.data [ind_next - 1] = '\0';
    *(p_now + 3) = ind_now;

    p_rsp->header.usLength = (unsigned short)
                             ((unsigned short) offsetof (st_eigw_response_t, body.data)
                             + ind_next);

    return (E_SUCCESS);
}



/* **************************************************************************
 *	@brief          set error string
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param    [OUT] p_handle   - handle
 *  @param    [IN ] p_req      - request message
 *  @param    [IN ] p_err_code - error code
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
set_errstring_eigw_handle (pst_eigw_handle_t   p_handle,
                           pst_eigw_request_t  p_req,
                           e_error_code_t      err_code)
{
    st_eigw_client_id_t client_id;

    switch (err_code)
    {
        case E_PROTOCOL_INVALID_BODY_URI:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message URI      (URI:NULL)");
            break;
        case E_PROTOCOL_INVALID_BODY_PROTOCOL:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message PROTOCOL (URI:%s)",
                        p_req->body.data + p_req->body.ind2);
            break;
        case E_PROTOCOL_INVALID_BODY_METHOD:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message METHOD   (METHOD:%s)",
                        p_req->body.method[0] == 0x00
                         ? "NULL" : p_req->body.method);
            break;
        case E_PROTOCOL_INVALID_HEADER_FRAME:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message frame     (frame:0x%02X%02X)",
                        p_req->header.cFrame[0],
                        p_req->header.cFrame[1]);
            break;
        case E_PROTOCOL_INVALID_HEADER_LENGTH:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message length    (len:%d)",
                        ntohs (p_req->header.usLength));
            break;
        case E_PROTOCOL_INVALID_HEADER_CLIENTID:
            memcpy (&client_id,
                    &p_req->header.unGwRteVal, sizeof (client_id));
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message client id (id:%02X%02X%02X%02X)",
                        client_id.system_id,
                        client_id.node_type,
                        client_id.node_id,
                        client_id.module_id);
            break;
        case E_PROTOCOL_INVALID_HEADER_NAME:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message name      (name:%d)",
                        p_req->header.unMsgName);
            break;
        case E_PROTOCOL_INVALID_HEADER_TYPE:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, invalid message type      (name:%d, type:%d)",
                        p_req->header.unMsgName,
                        GET_MSGN_TYPE(p_req->header.unMsgName));
            break;
        default:
            snprintf (p_handle->err_string, sizeof (p_handle->err_string) - 1,
                        "fail, undefined  body error    (%d:%ld)",
                        p_req->header.unGwRteVal,
                        p_req->header.ulSeq);

            break;
    }

    return (E_SUCCESS);
}




/* **************************************************************************
 *	@brief          validate message body
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param    [OUT] pp_handle  - handle
 *  @param    [IN ] p_fnmae    - config file full path
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
validate_body_eigw_handle (pst_eigw_handle_t    p_handle,
                           pst_eigw_request_t   p_req)
{
    e_error_code_t      e_code = E_SUCCESS;


    /*  validate message frame          */
    e_code = E_PROTOCOL_INVALID_BODY_URI;
    try_exception ((p_req->body.ind2 <= 0)
                    || (p_req->body.data[p_req->body.ind2] == 0x00),
                    exception_invalid_message_body);


    e_code = E_PROTOCOL_INVALID_BODY_PROTOCOL;
    try_exception (((p_req->body.ind3 - p_req->body.ind2) < 6)
                  ||
                  (strncmp (p_req->body.data + p_req->body.ind2, "http", 4)
                   &&
                   strncmp (p_req->body.data + p_req->body.ind2, "https", 5)),
                  exception_invalid_message_body);

    e_code = E_PROTOCOL_INVALID_BODY_METHOD;
    try_exception (p_req->body.method[0] == 0x00,
                    exception_invalid_message_body);

    /* ----------------------------------------------------------------
     * Logging low-level applicatioin request
     * ---------------------------------------------------------------- */
    Log (DEBUG_INFO, "info, req SRC[%ld:%d] [HEAD %s %s %s (H:%s)]\n",
                    p_req->header.unGwRteVal,
                    p_req->header.ulSeq,
                    p_req->body.method,
                    p_req->body.data + p_req->body.ind2,
                    p_req->body.data + p_req->body.ind3,
                    p_req->body.data [p_req->body.ind5] == 0x00
                    ? "NONE" : p_req->body.data + p_req->body.ind5);

    Log (DEBUG_INFO, "info, req SRC[%ld:%d] [BODY %s]\n",
                    p_req->header.unGwRteVal,
                    p_req->header.ulSeq,
                    p_req->body.data [p_req->body.ind6] == 0x00
                    ? "NULL" : p_req->body.data + p_req->body.ind6);

    e_code = E_SUCCESS;


    try_catch (exception_invalid_message_body)
    {
        st_eigw_response_t  rsp;
        (void) set_errstring_eigw_handle (p_handle, p_req, e_code);
        (void) (*p_handle->pf_encode)(p_req,
                                      &rsp,
                                      e_code,
                                      (char *) "text/plain",
                                      strlen (p_handle->err_string),
                                      NULL,
                                      NULL,
                                      p_handle->err_string);
        (*p_handle->pf_send) (p_handle, NULL, &rsp);
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      validate_header_eigw_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_error_code_t
validate_header_eigw_handle (pst_eigw_handle_t   p_handle,
                             pst_eigw_request_t  p_req)
{
    e_error_code_t      e_code = E_SUCCESS;
    unsigned short      frame  = 0x00;


    /*  validate message frame          */
    e_code = E_PROTOCOL_INVALID_HEADER_FRAME;
    memcpy ((void *)&frame, (void *)p_req->header.cFrame, 2);
    try_exception (frame != EIGW_HEADER_FRAME_VALUE,
                   exception_invalid_message_header);

    /*  validate client identifier          */
    e_code = E_PROTOCOL_INVALID_HEADER_CLIENTID;
    try_exception (memcmp (&p_req->header.unGwRteVal,
                           &p_handle->client_id,
                           sizeof (p_req->header.unGwRteVal))
                   != 0,
                   exception_invalid_message_header);

    /*  validate message type               */
    e_code = E_PROTOCOL_INVALID_HEADER_TYPE;
    try_exception ((GET_MSGN_TYPE(p_req->header.unMsgName) != MSG_TYPE_REQ)
                && (GET_MSGN_TYPE(p_req->header.unMsgName) != MSG_TYPE_RSP)
                && (GET_MSGN_TYPE(p_req->header.unMsgName) != MSG_TYPE_RPT),
                exception_invalid_message_header);

    /*  validate message name               */
    e_code = E_PROTOCOL_INVALID_HEADER_NAME;
    try_exception (((GET_MSGN_TYPE(p_req->header.unMsgName) == MSG_TYPE_REQ)
                    && (GET_MSGN_NAME(p_req->header.unMsgName)
                            != EIGW_MSG_NAME_REST_REQ))
            ||
            ((GET_MSGN_TYPE(p_req->header.unMsgName) == MSG_TYPE_RSP)
                && (GET_MSGN_NAME(p_req->header.unMsgName)
                    != EIGW_MSG_NAME_HEARTBEAT)),
             exception_invalid_message_header);

    /*  validate message length (too small) */
    e_code = E_PROTOCOL_INVALID_HEADER_LENGTH;
    if (GET_MSGN_TYPE(p_req->header.unMsgName) == MSG_TYPE_REQ)
    {
        try_exception ((ntohs (p_req->header.usLength)
                         < offsetof(st_eigw_request_t, body.data))
                        ||
                        (ntohs (p_req->header.usLength)
                         > EIGW_MAX_MSG_REQUEST_LENGTH),
                        exception_invalid_message_header);
    }
    e_code = E_SUCCESS;


    try_catch (exception_invalid_message_header)
    {
        st_eigw_response_t  rsp;

        (void) set_errstring_eigw_handle (p_handle, p_req, e_code);
        (void) (*p_handle->pf_encode)(p_req,
                                      &rsp,
                                      e_code,
                                      (char *) "text/plain",
                                      strlen (p_handle->err_string),
                                      NULL,
                                      NULL,
                                      p_handle->err_string);
        (*p_handle->pf_send) (p_handle, NULL, &rsp);
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          recv_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
is_already_received_message  (char      *p_buf,
                              int       received_bytes,
                              int       *p_read_bytes)
{
    e_error_code_t      e_code = E_FAILURE;
    pst_eigw_request_t  p_now  = (pst_eigw_request_t)p_buf;

    if (ntohs (p_now->header.usLength) <= received_bytes)
    {
        *p_read_bytes = (int) ntohs(p_now->header.usLength);
        return (E_SUCCESS);
    }

    /*  ----------------------------------------------------------
     *  for processing invalid message header
     *  --------------------------------------------------------- */
    if (ntohs (p_now->header.usLength) > sizeof (p_now->body.data))
    {
        *p_read_bytes = received_bytes;
        return (E_SUCCESS);
    }

    return (e_code);
}


static
e_error_code_t
recv_eigw_socket_handle (void  *p_handle,
                         char  *p_buf,
                         int   total_bytes,
                         int   *p_read_bytes)
{
    e_error_code_t              e_code = E_SUCCESS;
    pst_tcp_socket_handle_t     p_sock = (pst_tcp_socket_handle_t)p_handle;

    p_sock->p_rcvbuf = p_buf;
    e_code = (*p_sock->pf_recv)(p_sock,
                                total_bytes,
                                p_read_bytes);

    return (e_code);
}


static
e_error_code_t
recv_eigw_handle (pst_eigw_handle_t p_handle)
{
    e_error_code_t          e_code      = E_SUCCESS;
    pst_stream_handle_t     p_rbuf    = p_handle->p_rbuf;
    pst_eigw_request_t      p_req       = NULL;
    int                     rbytes      = 0;


    UNUSED (p_req);
    /* ------------------------------------------------------------
     * pre-checking socket state && initialize stream buffer
     * ------------------------------------------------------------ */
    if (p_handle->p_sock->sfd == -1)
    {
        try_exception ((e_code = (*p_handle->pf_connect)(p_handle))
                       != E_SUCCESS,
                       exception_connect_eigw);
    }

    e_code = (p_rbuf->pf_read)(p_rbuf,
                               sizeof (st_eigw_request_header_t),
                               is_already_received_message,
                               recv_eigw_socket_handle,
                               p_handle->p_sock,
                               &rbytes,
                      (char **)&p_req);
    try_exception (e_code != E_SUCCESS, exception_recv_from_eigw);

    try_exception ((e_code = (p_handle->pf_validate_header)(p_handle, p_req))
                   != E_SUCCESS,
                   exception_invalid_header_eigw);

    if ((p_handle->p_req == NULL)
        && (GET_MSGN_TYPE(p_req->header.unMsgName) == MSG_TYPE_REQ)
        && (GET_MSGN_NAME(p_req->header.unMsgName) == EIGW_MSG_NAME_REST_REQ))
    {
        try_exception ((e_code = alloc_pending_queue_eigw_handle (p_handle,
                                                         &p_handle->p_req))
                       != E_SUCCESS,
                       exception_alloc_pending_queue);

        try_exception ((e_code = (p_handle->pf_validate_body)(p_handle, p_req))
                       != E_SUCCESS,
                       exception_invalid_body_eigw);
    }
    p_handle->p_req =  p_req;

    Log (DEBUG_LOW,
            "succ, recv message from eigw [SRC:%x SEQ:%lu len:%hd name:%x]\n",
            p_handle->p_req->header.unGwRteVal,
            p_handle->p_req->header.ulSeq,
            p_handle->p_req->header.usLength,
            p_handle->p_req->header.unMsgName);


    try_catch (exception_connect_eigw)
    {
        e_code = E_SOCK_DISCONNECT;
    }
    try_catch (exception_alloc_pending_queue)
    {
        e_code = E_BUSY;
    }
    try_catch (exception_invalid_header_eigw)
    {
        (*p_handle->pf_disconnect)(p_handle);
        e_code = E_SOCK_DISCONNECT;
    }
    try_catch (exception_invalid_body_eigw)
    {
        e_code = E_PROTOCOL_INVALID_BODY;
    }
    try_catch (exception_recv_from_eigw)
    {
        if (e_code != E_TIMEOUT)
        {
            Log (DEBUG_ERROR,
                    "fail, recv message from eigw [%d,%s]\n",
                    p_handle->p_sock->err_no,
                    p_handle->p_sock->err_string);

            if (e_code == E_SOCK_DISCONNECT)
            {
                /* reset stream buffer */
                (*p_handle->pf_disconnect)(p_handle);
            }
        }
    }
    try_finally;


    return (e_code);
}




/* **************************************************************************
 *  @brief          send_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
send_eigw_handle (pst_eigw_handle_t     p_handle,
                  pst_eigw_request_t    p_req,
                  pst_eigw_response_t   p_rsp)
{
    e_error_code_t      e_code     = E_SUCCESS;
    pst_eigw_request_t  p_tmp      = p_req;
    int                 send_bytes = p_rsp->header.usLength;


    if (p_handle->p_sock->sfd == -1)
    {
        if (p_req != NULL)
        {
            Log (DEBUG_ERROR,
                    "fail, discard http response(not connect app) [%s:%s]\n",
                    p_rsp->body.status_code,
                    p_req->body.data + p_req->body.ind2);
        }

    }
    else
    {
        p_rsp->header.usLength = htons (p_rsp->header.usLength);
        p_handle->p_sock->p_sndbuf = (char *)p_rsp;
        e_code = (*p_handle->p_sock->pf_send)(p_handle->p_sock,
                                              send_bytes);
        try_exception (e_code != E_SUCCESS, exception_send_response);

        p_handle->last_tick = time (NULL);
        Log (DEBUG_LOW,
                "succ, send message to   eigw [SRC:%x SEQ:%d len:%d name:%x]\n",
                p_rsp->header.unGwRteVal,
                p_rsp->header.ulSeq,
                ntohs (p_rsp->header.usLength),
                p_rsp->header.unMsgName);
    }


    try_catch (exception_send_response)
    {
        Log (DEBUG_ERROR,
                "fail, send    EIGW  [ip:%s, port:%s, errno:%d, str:%s]\n",
                p_handle->p_sock->remote_ip,
                p_handle->p_sock->remote_port,
                p_handle->p_sock->err_no,
                p_handle->p_sock->err_string);

        if (p_req != NULL)
        {
            Log (DEBUG_ERROR,
                    "fail, discard http response(not connect app) [%s:%s]\n",
                    p_rsp->body.status_code,
                    p_req->body.data + p_req->body.ind2);
        }
    }
    try_finally;


    if (p_req != NULL)
    {
        e_code = free_pending_queue_eigw_handle (p_handle, &p_tmp);
    }

    return (e_code);
}




/* **************************************************************************
 *  @brief          heartbeat_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
heartbeat_eigw_handle (pst_eigw_handle_t p_handle)
{
    e_error_code_t              e_code  = E_SUCCESS;
    st_eigw_request_hb_t        req;

    try_exception (time (NULL) - p_handle->last_tick < p_handle->hb_interval,
                   exception_heartbeat_bypass);

    /* ------------------------------------------------------------------
     * HEART REQ/RESP
     *   memset (&req, 0x00, sizeof (req));
     * ------------------------------------------------------------------ */
    req.header.cFrame[0] = 0xFE;
    req.header.cFrame[1] = 0xFE;
    req.header.unMsgName = MK_MSGN (MSG_TYPE_REQ,EIGW_MSG_NAME_HEARTBEAT);
    req.header.usLength  = htons(sizeof (req));
    GET_EIGW_SEND_SEQ(p_handle, req.header.ulSeq);

    p_handle->p_sock->p_sndbuf = (char *)&req;
    e_code = (*p_handle->p_sock->pf_send)(p_handle->p_sock,
                                          sizeof (req));
    try_exception (e_code != E_SUCCESS, exception_send_heartbeat_request);


    try_catch (exception_send_heartbeat_request)
    {
        Log (DEBUG_ERROR,
                "fail, send heartbeat request  to   eigw \n");
        e_code = E_FAILURE;
    }
    try_catch (exception_heartbeat_bypass)
    {
        ;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          bind_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
bind_eigw_handle (pst_eigw_handle_t p_handle)
{
    e_error_code_t              e_code  = E_SUCCESS;
    st_eigw_request_bind_t      req;
    int                         now_length, total_length;
    st_eigw_response_bind_t     rsp;
    time_t                      tick;


    /* ------------------------------------------------------------------
     * BIND REQ/RESP
     * memset (&req, 0x00, sizeof (req));
     * ------------------------------------------------------------------ */
    req.header.cFrame[0] = 0xFE;
    req.header.cFrame[1] = 0xFE;
    req.header.unMsgName = MK_MSGN (MSG_TYPE_REQ,EIGW_MSG_NAME_BIND);
    req.header.usLength  = htons(sizeof (req));
    GET_EIGW_SEND_SEQ(p_handle, req.header.ulSeq);

    p_handle->p_sock->p_sndbuf = (char *)&req;
    e_code = (*p_handle->p_sock->pf_send)(p_handle->p_sock,
                                          sizeof (req));
    try_exception (e_code != E_SUCCESS, exception_send_bind_request);


    total_length = 0;
    now_length   = 0;
    tick         = time (NULL);
    do
    {
        p_handle->p_sock->p_rcvbuf = (char *)(&rsp) + total_length;
        e_code = (*p_handle->p_sock->pf_recv)(p_handle->p_sock,
                                              sizeof (rsp) - total_length,
                                              &now_length);
        try_exception ((e_code != E_SUCCESS) && (e_code != E_TIMEOUT),
                       exception_recv_bind_socket);

        try_exception (time(NULL) - tick > 5, exception_recv_bind_timeout);
        total_length += now_length;
    } while (total_length < (int)sizeof (req.header));
    try_exception (rsp.header.sRet != SUCC, exception_recv_bind_response);

    Log (DEBUG_NORMAL,
            "succ, bind       EIGW  [result:%d]\n",
            (int) rsp.header.sRet);

    try_catch (exception_recv_bind_socket);
    try_catch_through (exception_send_bind_request)
    {
        snprintf (p_handle->err_string,
                  sizeof (p_handle->err_string) - 1,
                  "fail, bind       EIGW  [errno:%d, err_string:%s]",
                  p_handle->p_sock->err_no,
                  p_handle->p_sock->err_string);
        e_code = E_FAILURE;
    }
    try_catch (exception_recv_bind_response)
    {
        snprintf (p_handle->err_string,
                  sizeof (p_handle->err_string) - 1,
                  "fail, bind       EIGW  [result:%d]",
                  (int) rsp.header.sRet);
        e_code = E_FAILURE;
    }
    try_catch (exception_recv_bind_timeout)
    {
        snprintf (p_handle->err_string,
                  sizeof (p_handle->err_string) - 1,
                  "fail, bind       EIGW  [timeout(5)]");
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          connect_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
connect_eigw_handle (pst_eigw_handle_t p_handle)
{
    e_error_code_t              e_code  = E_SUCCESS;


    if (p_handle->p_sock == NULL)
    {
        try_exception ((e_code = init_socket_handle (&(p_handle->p_sock),
                                                p_handle->cfname,
                                        (char *)EIGW_CONFIG_SECTION_NAME))
                       != E_SUCCESS,
                       exception_init_socket);
    }
    try_exception ((e_code = (*p_handle->p_sock->pf_connect)(p_handle->p_sock))
                   != E_SUCCESS,
                   exception_connect_eigw_handle);


    try_exception ((e_code = bind_eigw_handle (p_handle)) != E_SUCCESS,
                   exception_bind_eigw_handle);

    Log (DEBUG_CRITICAL,
            "succ, connect    EIGW  [ip:%s, port:%s]\n",
            p_handle->p_sock->remote_ip,
            p_handle->p_sock->remote_port);

    try_catch (exception_init_socket)
    {
        (*p_handle->p_sock->pf_close)(p_handle->p_sock);
    }
    try_catch (exception_connect_eigw_handle)
    {
        static int i = 0;

        if (i%100 == 0)
        {
            Log (DEBUG_LOW,
                    "fail, connect    EIGW  [ip:%s, port:%s, errno:%d, str:%s]\n",
                    p_handle->p_sock->remote_ip,
                    p_handle->p_sock->remote_port,
                    p_handle->p_sock->err_no,
                    p_handle->p_sock->err_string);
        }
        i++;
    }
    try_catch (exception_bind_eigw_handle)
    {
        Log (DEBUG_ERROR,
                "%s\n", p_handle->err_string);
        Log (DEBUG_ERROR,
                "fail, bind       EIGW  [ip:%s, port:%s, errno:%d, str:%s]\n",
                p_handle->p_sock->remote_ip,
                p_handle->p_sock->remote_port,
                p_handle->p_sock->err_no,
                p_handle->p_sock->err_string);
        (*p_handle->pf_disconnect)(p_handle);
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          disconnect_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
disconnect_eigw_handle (pst_eigw_handle_t p_handle)
{
    e_error_code_t  e_code  = E_SUCCESS;

    e_code = (*p_handle->p_sock->pf_close)(p_handle->p_sock);
    (*p_handle->p_rbuf->pf_reset)(p_handle->p_rbuf);

    Log (DEBUG_CRITICAL,
            "succ, disconnect EIGW  [ip:%s, port:%s]\n",
            p_handle->p_sock->remote_ip,
            p_handle->p_sock->remote_port);

    return (e_code);
}




/* **************************************************************************
 *  @brief          set_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
set_eigw_handle (pst_eigw_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];


    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);

    /* NOT USED  */
    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        EIGW_CONFIG_SECTION_NAME,
                        EIGW_CONFIG_MODULE_ID_NAME,
                        EIGW_CONFIG_MODULE_ID_DEFAULT, buf);
    p_handle->client_id.module_id = (char)atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        EIGW_CONFIG_SECTION_NAME,
                        EIGW_CONFIG_HB_INTERVAL_NAME,
                        EIGW_CONFIG_HB_INTERVAL_DEFAULT, buf);
    p_handle->hb_interval = atoi (buf);
    p_handle->last_tick   = time (NULL);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        EIGW_CONFIG_SECTION_NAME,
                        EIGW_CONFIG_PENDING_BUF_COUNT_NAME,
                        EIGW_CONFIG_PENDING_BUF_COUNT_DEFAULT, buf);
    p_handle->size_of_pqueue = atoi (buf);


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
        Log (DEBUG_CRITICAL,
                "fail, [not found file:%s]\n", p_handle->cfname);
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          init_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle  - handle
 *  @param    [IN ] p_fnmae    - config file full path
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_eigw_handle (pst_eigw_handle_t *pp_handle,
                  char              *p_fname,
                  char              *p_section)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_eigw_handle_t   p_handle = NULL;


    try_exception ((p_handle
                        = (pst_eigw_handle_t)MALLOC(sizeof (st_eigw_handle_t)))
                    == NULL,
                    exception_alloc_handle);
    memset (p_handle, 0x00, sizeof (*p_handle));

    /*  define current message  */
    strcpy (p_handle->cfname,   p_fname);
    if (p_section)
        strcpy (p_handle->csection, p_section);

    else
        strcpy (p_handle->csection, (const char *)EIGW_CONFIG_SECTION_NAME);

    p_handle->p_req      = NULL;
    p_handle->p_rsp      = NULL;


    /*  define  member function */
    p_handle->pf_connect         = connect_eigw_handle;
    p_handle->pf_disconnect      = disconnect_eigw_handle;
    p_handle->pf_recv            = recv_eigw_handle;
    p_handle->pf_send            = send_eigw_handle;
    p_handle->pf_encode          = encode_eigw_handle;
    p_handle->pf_heartbeat       = heartbeat_eigw_handle;
    p_handle->pf_validate_header = validate_header_eigw_handle;
    p_handle->pf_validate_body   = validate_body_eigw_handle;
    p_handle->pf_show            = show_eigw_handle;


    try_exception (pthread_mutex_init (&p_handle->send_lock, NULL) != 0,
                   exception_init_pthread_mutex);

    if (p_fname != NULL)
    {
        set_eigw_handle (p_handle);
    }


    try_exception ((e_code = init_socket_handle (&(p_handle->p_sock),
                                                 p_handle->cfname,
                                      (char *)EIGW_CONFIG_SECTION_NAME))
                    != E_SUCCESS,
                   exception_init_socket_handle)


    try_exception ((e_code = init_stream_handle (&p_handle->p_rbuf,
                                                 p_handle->cfname,
                                                 p_handle->csection))
                   != E_SUCCESS,
                   exception_init_stream_handle);

    /* -------------------------------------------------------------
     * NOT USED PENDING QUEUE
     * (void) init_mpool_handle (&(p_handle->p_pending_queue),
     *                           p_handle->size_of_pqueue,
     *                           sizeof (st_eigw_request_t),
     *                           NULL,
     *                           NULL);
     * try_exception (p_handle->p_pending_queue == NULL,
     *                exception_init_mpool_handle);
     *
     *
     * try_catch (exception_init_mpool_handle);
     * {
     *  (void) pthread_mutex_destroy (&p_handle->send_lock);
     *  (void) destroy_socket_handle (&p_handle->p_sock);
     *  (void) destroy_stream_handle (&p_handle->p_rbuf);
     *  FREE (p_handle);
     *  Log (DEBUG_CRITICAL,
     *          "fail, alloc EIGW handle (penidng memory pool)\n");
     *  e_code = E_ALLOC_HANDLE;
     * }
     * ------------------------------------------------------------- */

    (*pp_handle) = p_handle;


    try_catch (exception_alloc_handle)
    {
        Log (DEBUG_CRITICAL,
                "fail, alloc EIGW handle\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_catch (exception_init_pthread_mutex)
    {
        FREE (p_handle);
        Log (DEBUG_CRITICAL,
                "fail, alloc EIGW handle (init mutex)\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_catch (exception_init_socket_handle);
    {
        (void) pthread_mutex_destroy (&p_handle->send_lock);
        FREE (p_handle);
        Log (DEBUG_CRITICAL,
                "fail, alloc EIGW handle (socket handle)\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_catch (exception_init_stream_handle)
    {
        (void) pthread_mutex_destroy (&p_handle->send_lock);
        (void) destroy_socket_handle (&p_handle->p_sock);
        FREE (p_handle);
        Log (DEBUG_CRITICAL,
                "fail, alloc EIGW handle (init mutex)\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          destroy_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
destroy_eigw_handle (pst_eigw_handle_t  p_handle)
{
    return (destroy_socket_handle (&(p_handle->p_sock)));
}

/* ***************************************************************************
 *       Filename:  eigw.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/07/18 17:26:38
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <heigw.h>


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
send_notification_to_eigw (pst_eigw_handle_t     p_handle,
                           pst_eigw_request_t    p_req,
                           pst_eigw_response_t   p_rsp)
{
    e_error_code_t      e_code     = E_SUCCESS;
    int                 send_bytes = p_req->header.usLength;

    UNUSED (p_rsp);
    GET_EIGW_SEND_SEQ(p_handle, p_req->header.ulSeq);
    if (p_handle->p_sock->sfd == -1)
    {
        Log (DEBUG_ERROR,
                "fail, discard http notifictaion (not connect app) [%s]\n",
                p_req->body.data + p_req->body.ind2);
    }
    else
    {
        p_req->header.usLength = htons (p_req->header.usLength);
        p_handle->p_sock->p_sndbuf = (char *)p_req;
        e_code = (*p_handle->p_sock->pf_send)(p_handle->p_sock,
                                              send_bytes);
        try_exception (e_code != E_SUCCESS, exception_send_request);

        p_handle->last_tick = time (NULL);
    }

    try_catch (exception_send_request)
    {

    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          register_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
register_eigw_handle (pst_eigw_handle_t     p_handle)
{
    p_handle->pf_send = send_notification_to_eigw;

    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          process_recv_message_from_eigw
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
void *
recv_message_from_eigw (void *p_arg)
{
    e_error_code_t      e_code = E_SUCCESS;
    pst_eigw_handle_t   p_handle = (pst_eigw_handle_t)p_arg;

    Log (DEBUG_CRITICAL,
            "succ, startup EIGW recv thread complete\n");
    while (1)
    {
        p_handle->p_req = NULL;
        e_code = (*p_handle->pf_recv)(p_handle);
        if (e_code == E_SUCCESS)
        {
            usleep (1000);
            continue;
        }
        sleep (1);
    }

    return (NULL);
}




/* **************************************************************************
 *  @brief          startup_eigw_rthread
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
startup_eigw_rthread (pst_eigw_handle_t     p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    pthread_t       tid;
    pthread_attr_t  attr;

    try_exception (pthread_attr_init (&attr) != 0,
                   exception_pthread_attr_init);
    try_exception (pthread_attr_setdetachstate (&attr,
                                                PTHREAD_CREATE_DETACHED)
                   != 0,
                   exception_pthread_setdetachstate);
    try_exception (pthread_create (&tid, &attr,
                                   (void *(*)(void *))recv_message_from_eigw,
                                   (void *)p_handle)
                   != 0,
                   exception_pthread_create);


    try_catch (exception_pthread_attr_init)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_pthread_setdetachstate)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_pthread_create)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}


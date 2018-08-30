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


typedef struct _st_job_handle_t *pst_job_handle_t;
typedef struct _st_job_handle_t
{
    pthread_t           tid;
    pthread_attr_t      attr;

    pst_fcgi_handle_t   p_fcgi;
    pst_eigw_handle_t   p_eigw;

    pst_socket_handle_t p_sock;
} st_job_handle_t;




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
void *
process_fcgi_notify (void *p_arg)
{
    e_error_code_t      e_code = E_SUCCESS;
    pst_job_handle_t    p_job  = (pst_job_handle_t)p_arg;
    pst_eigw_handle_t   p_eigw = p_job->p_eigw;
    pst_fcgi_notify_t   p_notification;
    st_fcgi_handle_t    fcgi;
    pst_eigw_request_t  p_req;


    Log (DEBUG_CRITICAL,
            "info, startup   FCGI thread [%s:%s]\n",
            p_job->p_sock->remote_ip,
            p_job->p_sock->remote_port);


    memcpy (&fcgi, p_job->p_fcgi, sizeof (fcgi));
    fcgi.p_sock = p_job->p_sock;
    if (init_stream_handle (&fcgi.p_rbuf,
                            fcgi.cfname,
                            fcgi.csection) != E_SUCCESS)
    {
        try_exception (1, exception_process_fcgi_notify);
    }

    while (1)
    {
        e_code = (*fcgi.pf_recv)(&fcgi, &p_notification);
        if (e_code == E_TIMEOUT)
        {
            usleep (1000);
            continue;
        }
        try_exception (e_code != E_SUCCESS, exception_recv_notify);

        p_req = (pst_eigw_request_t)p_notification->body.data;

        /*  -------------------------------------------------
         *  ISSUE: Policy Function
         *  ------------------------------------------------- */

        p_req->header.usLength = ntohs (p_req->header.usLength);
        (*p_eigw->pf_send)  (p_eigw, p_req, NULL);
    }


    try_catch (exception_process_fcgi_notify)
    {

    }
    try_catch (exception_recv_notify)
    {

    }
    try_finally;


    destroy_socket_handle (&fcgi.p_sock);
    destroy_stream_handle (&fcgi.p_rbuf);
    free (p_job);

    Log (DEBUG_CRITICAL,
            "info, terminate FCGI thread [%s:%s]\n",
            p_job->p_sock->remote_ip,
            p_job->p_sock->remote_port);

    return (NULL);
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

    UNUSED (p_handle);

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
    e_error_code_t  e_code      = E_SUCCESS;
    pst_fcgi_handle_t           p_fcgi = p_handle->p_fcgi;
    pst_socket_handle_t         p_remote;
    pst_job_handle_t            p_job;


    /*  check current handle state       */
    e_code = preprocess_job (p_handle);

    e_code = p_fcgi->p_sock->pf_accept (p_fcgi->p_sock, &p_remote);
    try_exception (e_code != E_SUCCESS, exception_fcgi_accept);

    p_job = (pst_job_handle_t) MALLOC (sizeof (*p_job));
    p_job->p_sock = p_remote;
    p_job->p_eigw = p_handle->p_eigw;
    p_job->p_fcgi = p_handle->p_fcgi;

    try_exception (pthread_attr_init (&p_job->attr) != 0,
                   exception_pthread_attr_init);
    try_exception (pthread_attr_setdetachstate (&p_job->attr,
                                                PTHREAD_CREATE_DETACHED)
                   != 0,
                   exception_pthread_setdetachstate);
    try_exception (pthread_create (&p_job->tid, &p_job->attr,
                                   (void *(*)(void *))process_fcgi_notify,
                                   (void *)p_job)
                   != 0,
                   exception_pthread_create);

    try_catch (exception_fcgi_accept)
    {
    }
    try_catch (exception_pthread_attr_init)
    {
    }
    try_catch (exception_pthread_setdetachstate)
    {
    }
    try_catch (exception_pthread_create)
    {
    }
    try_finally;

    return (e_code);
}

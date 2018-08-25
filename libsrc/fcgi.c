/* ***************************************************************************
 *
 *       Filename:  fcgi.c
 *    Description:  fcgi interface
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hfcgi.h>




/* **************************************************************************
 *  @brief          show_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
void
show_fcgi_handle (pst_fcgi_handle_t  p_handle,
                  char              *p_module)
{
    char    *p_name;

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    (*p_handle->p_sock->pf_show)(p_handle->p_sock, p_name);

    return;
}




/* **************************************************************************
 *  @brief          is_alreay_recv
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

#ifdef _OLD
static
e_error_code_t
is_already_recv (pst_fcgi_handle_t p_handle,
                 pst_fcgi_notify_t *pp_notification)
{
    UNUSED (p_handle);
    UNUSED (pp_notification);
    pst_fcgi_notify_t p_now      = NULL;

    p_now = (pst_fcgi_notify_t)(p_handle->p_rcvbuf + p_handle->last_location);


    if ((p_handle->curr_location - p_handle->last_location)
            > offsetof (st_fcgi_notify_t, header.pid))
    {
        if (ntohl (p_now->header.length)
                <= (long) (p_handle->curr_location - p_handle->last_location))
        {
            p_handle->last_location += ntohl (p_now->header.length);
            (*pp_notification) = p_now;

            Log (DEBUG_INFO,
                    "succ, recv message from fcgi [last:%d, current:%d]\n",
                    p_handle->last_location,
                    p_handle->curr_location);
            return (E_SUCCESS);
        }
    }
    return (E_FAILURE);
}
#endif


/* **************************************************************************
 *  @brief          recv_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
is_already_recv (char   *p_buf, int received_bytes, int *p_read_bytes)
{
    pst_fcgi_notify_t p_now  = (pst_fcgi_notify_t)p_buf;

    if (ntohl (p_now->header.length) <= (long) received_bytes)
    {
        *p_read_bytes = ntohl (p_now->header.length);
        return (E_SUCCESS);
    }

    return (E_FAILURE);
}


/* **************************************************************************
 *  @brief          recv_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
recv_fcgi_socket_handle (void  *p_handle,
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
recv_fcgi_handle (pst_fcgi_handle_t  p_handle,
                  pst_fcgi_notify_t  *pp_notification)
{
    e_error_code_t  e_code       = E_SUCCESS;
    int             recv_length  = 0;
    char            *p_tmp       = NULL;


    e_code = (p_handle->p_rbuf->pf_read)(p_handle->p_rbuf,
                                    sizeof (st_fcgi_notify_header_t),
                                         is_already_recv,
                                         recv_fcgi_socket_handle,
                                         p_handle->p_sock,
                                         &recv_length,
                                         &p_tmp);
    try_exception ((e_code != E_TIMEOUT) && (e_code != E_SUCCESS),
                   exception_recv_handle);

    if (e_code == E_SUCCESS)
    {
        *pp_notification = (pst_fcgi_notify_t)p_tmp;
    }

    try_catch (exception_recv_handle)
    {

    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          send_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
send_fcgi_handle (pst_fcgi_handle_t     p_handle)
{
    e_error_code_t  e_code     = E_SUCCESS;

    UNUSED (p_handle);
    return (e_code);
}




/* **************************************************************************
 *  @brief          connect_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
connect_fcgi_handle (pst_fcgi_handle_t p_handle)
{
    return ((*p_handle->p_sock->pf_connect)(p_handle->p_sock));
}




/* **************************************************************************
 *  @brief          disconnect_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
disconnect_fcgi_handle (pst_fcgi_handle_t p_handle)
{
    return ((*p_handle->p_sock->pf_close)(p_handle->p_sock));
}



/* **************************************************************************
 *  @brief          connect_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
accept_fcgi_handle (pst_fcgi_handle_t p_handle)
{
    UNUSED (p_handle);
    return (E_SUCCESS);
    /*
    return ((*p_handle->p_sock->pf_accept)(p_handle->p_sock));
    */
}




/* **************************************************************************
 *  @brief          set_socket
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
set_fcgi_handle (pst_fcgi_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;


    try_exception ((e_code = init_socket_handle (&(p_handle->p_sock),
                                                 p_handle->cfname,
                                                 p_handle->csection))
                   != E_SUCCESS,
                   exception_alloc_socket_handle);

#ifdef _OLD
    char            buf [DEFAULT_STRING_BUF_LEN] = {0,};
    if (Is_FileExist (p_handle->cfname) != TRUE)
    {
        p_handle->rcvbuf_length = atoi (FCGI_CONFIG_RBUF_LEN_DEFAULT);
    }
    else
    {
        memset (buf, 0x00, sizeof (buf));
        (void) ReadConfFile (p_handle->cfname,
                             p_handle->csection,
                             FCGI_CONFIG_RBUF_LEN_NAME,
                             FCGI_CONFIG_RBUF_LEN_DEFAULT, buf);

        p_handle->rcvbuf_length = atoi (buf);
    }

    p_handle->p_rcvbuf = (char *) MALLOC (p_handle->rcvbuf_length);
    if (p_handle->p_rcvbuf == NULL)
    {
        return (E_ALLOC_HANDLE);
    }
#endif

    try_catch (exception_alloc_socket_handle)
    {
        ;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          init_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle  - handle
 *  @param    [IN ] p_fnmae    - config file full path
 *  @param    [IN ] p_section  - config file section name
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_fcgi_handle (pst_fcgi_handle_t *pp_handle,
                  char              *p_fname,
                  char              *p_section)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_fcgi_handle_t   p_handle = NULL;

    try_exception ((p_handle = (pst_fcgi_handle_t)malloc (sizeof (*p_handle)))
                    == NULL,
                    exception_init_fcgi_handle);
    memset (p_handle, 0x00, sizeof (*p_handle));


    p_handle->pf_set        = set_fcgi_handle;
    p_handle->pf_connect    = connect_fcgi_handle;
    p_handle->pf_disconnect = disconnect_fcgi_handle;
    p_handle->pf_accept     = accept_fcgi_handle;
    p_handle->pf_send       = send_fcgi_handle;
    p_handle->pf_recv       = recv_fcgi_handle;
    p_handle->pf_show       = show_fcgi_handle;


    strcpy (p_handle->cfname, p_fname);
    if (p_section != NULL)
    {
        strcpy (p_handle->csection, p_section);
    }
    try_exception ((*p_handle->pf_set)(p_handle) != E_SUCCESS,
                   exception_init_fcgi_handle);

    (void) init_stream_handle (&p_handle->p_rbuf, p_fname, p_section);

    (*pp_handle) = p_handle;

    try_catch (exception_init_fcgi_handle)
    {
        Log (DEBUG_CRITICAL, "fail, alloc FCGI handle\n");
        e_code = E_ALLOC_HANDLE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          destroy_fcgi_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
destroy_fcgi_handle (pst_fcgi_handle_t  *pp_handle)
{
    if (pp_handle && *pp_handle)
    {
        FREE (*pp_handle);
        (*pp_handle) = NULL;
    }

    return (E_SUCCESS);
}

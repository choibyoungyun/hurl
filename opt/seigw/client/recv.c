/* ***************************************************************************
 *       Filename:  tcp_server.c
 *    Description:  libuv test sample
 *        Version:  1.0
 *        Created:  07/05/18 18:18:19
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <seigw.h>


static int      num_of_receive_message;

extern
e_error_code_t
recv_bind_request_from_udr (uv_stream_t         *p_client,
                            ssize_t             nread,
                            const uv_buf_t      *buf);


/* **************************************************************************
 *	@brief      alloc_recv_buffer
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

static
void
alloc_recv_buffer (uv_handle_t   *p_handle,
                   size_t        suggested_size,
                   uv_buf_t      *p_buf)
{
    UNUSED (p_handle);

    p_buf->len  = suggested_size;
    p_buf->base = (char *)malloc (p_buf->len);
    if (p_buf->base == NULL)
    {
        fprintf (stdout, "fail, alloc recv buffer\n");
        exit(-1);
    }
    HTTP_LOG ("succ, alloc recv buffer [%p]\n", p_buf->base);

    return;
}



/* **************************************************************************
 *	@brief      request_resetapi
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

static
e_error_code_t
get_message_from_buf (pst_eigw_response_t   p_msg,
                      int                   bytes,
                      int                   *p_size)
{

    /*  compare header size */
    if (bytes < (int)sizeof (st_eigw_response_header_t))
    {
        return (E_FAILURE);
    }
    else if (bytes < ntohs (p_msg->header.usLength))
    {
        return (E_FAILURE);
    }

    *p_size = ntohs (p_msg->header.usLength);
    if (*p_size == 0)
    {
        fprintf (stdout,"fail, invalid length\n");
        exit(-1);
    }

    return (E_SUCCESS);
}


/* **************************************************************************
 *	@brief      request_resetapi
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

static
void
recv_message_from_udr (uv_stream_t         *p_client,
                        ssize_t             nread,
                        const uv_buf_t      *buf)
{
    e_error_code_t      e_code = E_SUCCESS;
    char                *p_base = NULL;
    char                *p_now  = NULL;

    static char         already_buf[65536];
    static  int         already_size = 0;
    static  int         recv_count;
    static  int         is_bind;

    int                 recv_bytes     = 0;
    int                 message_length = 0;
    int                 message_name;


    HTTP_LOG ("info, recv response [bytes:%d]\n", (int)nread);
    try_exception (nread < 0, exception_disconnect_client);
    try_exception (is_bind == 0, exception_not_ready_client);


    recv_bytes = already_size + nread;
    try_exception ((p_base = (char *)malloc (recv_bytes)) == NULL,
                   exception_alloc_buffer);
    HTTP_LOG ("succ, alloc process buffer [%d]\n", recv_bytes);

    memcpy (p_base, already_buf, already_size);
    memcpy (p_base + already_size, buf->base, nread);
    already_size = 0;


    p_now = p_base;
    while (p_now < (p_base + recv_bytes))
    {
        message_length = 0;
        e_code = get_message_from_buf ((pst_eigw_response_t)p_now,
                                       p_base + recv_bytes - p_now,
                                       &message_length);
        if ( e_code != E_SUCCESS)
            break;


        message_name = ((pst_eigw_request_t)p_now)->header.unMsgName;
        if (GET_MSGN_NAME(message_name) == EIGW_MSG_NAME_HEARTBEAT)
        {
            fprintf (stdout, "info, recv heartgeat\n");
            p_now = p_now + message_length;
            continue;
        }

        if (num_of_receive_message == 1)
        {
            (void)
            decode_udr_response_msg ((pst_eigw_response_t)p_now);
        }
        else if ((num_of_receive_message > 1)
                && ((recv_count%1000) == 0) && (recv_count != 0))
        {
            fprintf (stdout, "%d [%d] [seq:%ld]\n",
                      recv_count, (int) time(NULL),
                      ((pst_eigw_response_t)p_now)->header.ulSeq);
            fflush  (stdout);
        }

        recv_count ++;
        p_now = p_now + message_length;
    }

    already_size = p_base + recv_bytes - p_now;
    memcpy (already_buf, p_now, already_size);
    HTTP_LOG ("fail, save unprocessed buffer [%d]\n", already_size);


    if (recv_count == num_of_receive_message)
    {
        exit (0);
    }

    try_catch (exception_disconnect_client)
    {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) p_client, on_close);
    }
    try_catch (exception_alloc_buffer)
    {
        fprintf (stdout,
                "fail, alloc process buffer [%d]\n", recv_bytes);
        exit (-1);
    }
    try_catch (exception_not_ready_client)
    {
        uv_thread_t  send_tid;

        e_code = recv_bind_request_from_udr (p_client, nread, buf);
        if (e_code != E_SUCCESS)
        {
            fprintf (stdout,
                    "fail, invalid bind request \n");
            exit(-1);
        }
        is_bind = 1;

        uv_thread_create (&send_tid,
                          send_message_to_udr_start,
                          p_client);
        uv_thread_join (&send_tid);
    }
    try_finally;

    if (p_base != NULL)    free (p_base);
    if (buf->base != NULL) free (buf->base);

    return;
}




/* **************************************************************************
 *	@brief      recv_message_from_udr_start
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

void
recv_message_from_udr_start (uv_stream_t    *p_client,
                             int            num_of_recv)
{

    num_of_receive_message = num_of_recv;

    uv_read_start (p_client,
                   alloc_recv_buffer,
                   recv_message_from_udr);
    return;
}

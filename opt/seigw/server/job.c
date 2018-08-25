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

extern
void
recv_message_from_udr_start (uv_stream_t    *p_client);

static  uv_loop_t *loop;

/* **************************************************************************
 *	@brief      on_close
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
void
on_close (uv_handle_t *p_handle)
{
    FREE (p_handle);
    return;
}



/* **************************************************************************
 *	@brief      on_new_connection
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

void
on_new_connection (uv_stream_t *server, int status)
{
    uv_tcp_t    *p_client = NULL;

    if (status < 0)
    {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    p_client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init (loop, p_client);

    if (uv_accept(server, (uv_stream_t*) p_client) == 0)
    {
        (void) recv_message_from_udr_start ((uv_stream_t *)p_client);
    }
    else
    {
        uv_close ((uv_handle_t*) p_client, on_close);
    }

    return;
}




/* **************************************************************************
 *	@brief      do job
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

e_error_code_t
do_job (int         port)
{
    uv_tcp_t            server;
    int                 r;
    struct  sockaddr_in addr;

    /* ---------------------------------------------------
     * set input parameter
     *  - test case
     *  - number of transmission for perf
     * --------------------------------------------------- */
    loop = uv_default_loop();
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", port, &addr);
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    r = uv_listen((uv_stream_t*) &server,
                   SEIGW_DEFAULT_BACKLOG,
                   on_new_connection);
    if (r)
    {
        fprintf(stderr, "err, listen %s\n", uv_strerror(r));
        return (E_FAILURE);
    }

    return (e_error_code_t)uv_run(loop, UV_RUN_DEFAULT);
}

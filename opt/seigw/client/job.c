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

extern e_error_code_t
encode_udr_request_msg (char *, pst_eigw_request_t p_msg);

extern e_error_code_t
decode_udr_response_msg (pst_eigw_response_t p_msg);


static  uv_loop_t               *loop;
struct  sockaddr_in             addr;

char            test_case [128];
int             num_of_transmissions = 0;
st_recv_buf_t   recv_buf;


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
        (void) recv_message_from_udr_start ((uv_stream_t *)p_client,
                                            num_of_transmissions);
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
do_job (int         port,
        char        *p_tcase,
        char        *p_num_of_trans)
{
    uv_tcp_t            server;
    int                 r;


    /* ---------------------------------------------------
     * set input parameter
     *  - test case
     *  - number of transmission for perf
     * --------------------------------------------------- */
    strcpy (test_case, p_tcase);
    num_of_transmissions = 1;
    if ((p_num_of_trans != NULL) &&  (*p_num_of_trans != 0x00))
        num_of_transmissions = atoi (p_num_of_trans);

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

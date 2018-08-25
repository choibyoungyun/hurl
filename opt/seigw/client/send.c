/* ***************************************************************************
 *       Filename:  send.c
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



extern int  num_of_transmissions;
extern char test_case [128];


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
send_request_to_udr_done (uv_write_t *p_req, int status)
{
    if (status != 0)
    {
        fprintf (stdout,
                "fail, write error %s\n", uv_strerror(status));
    }

    free (((pst_write_req_t)p_req)->buf.base);
    free (p_req);
}




/* **************************************************************************
 *	@brief      send_message_to_udr_start
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

void
send_request_to_udr (uv_stream_t            *p_client,
                     pst_eigw_request_t     p_msg,
                     int                    count)
{
    static unsigned long seq = 0;
    pst_write_req_t     p_req;
    int                 i;
    int                 length = ntohs (p_msg->header.usLength);



    for (i=0; i < count; i++)
    {
        try_exception ((p_req = (pst_write_req_t) malloc(sizeof(*p_req)))
                       == NULL,
                       exception_alloc_write_buffer);

        try_exception ((p_req->buf.base  = (char *)malloc (length)) == NULL,
                       exception_alloc_write_buffer);

        p_msg->header.ulSeq   = seq++;
        memcpy (p_req->buf.base, p_msg, length);
        p_req->buf.len    = length;

        (void) uv_write ((uv_write_t*) p_req,
                         p_client,
                         &p_req->buf,
                         1,
                         send_request_to_udr_done);
    }


    try_catch (exception_alloc_write_buffer)
    {
        fprintf (stdout,"fail, alloc send uv buffer\n");
        exit (-1);
    }
    try_finally;

    return;
}




/* **************************************************************************
 *	@brief      send_message_to_udr_start
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

void
send_message_to_udr_start (void    *p_client)
{
    st_eigw_request_t    msg;
    int                  i;


    HTTP_LOG ("info, start send request [%d] \n", num_of_transmissions);

    if (encode_udr_request_msg (test_case, &msg) != E_SUCCESS)
    {
        exit (-1);
    }


    for (i=0; i < (num_of_transmissions/SEIGW_MAX_NUM_OF_SEND_BUF); i++)
    {
        (void)
        send_request_to_udr ((uv_stream_t *)p_client,
                             &msg,
                             SEIGW_MAX_NUM_OF_SEND_BUF);
    }

    (void)
    send_request_to_udr ((uv_stream_t *)p_client,
                         &msg,
                         (num_of_transmissions % SEIGW_MAX_NUM_OF_SEND_BUF));

    HTTP_LOG ("info, complete send request [%d] \n", num_of_transmissions);

    return;
}

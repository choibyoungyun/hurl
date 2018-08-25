/* ***************************************************************************
 *       Filename:  bind.c
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


static
void
send_bind_response_to_udr_done (uv_write_t *p_req, int status)
{


    if (status != 0)
    {
        fprintf (stdout,
                "fail, write error %s\n", uv_strerror(status));
        fflush (stdout);
    }

    free (((pst_write_req_t)p_req)->buf.base);
    free (p_req);

    HTTP_LOG ("succ, bind client\n" );
}




/* **************************************************************************
 *	@brief      send_bind_response_to_udr
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
e_error_code_t
send_bind_response_to_udr (uv_stream_t              *p_client,
                           pst_eigw_response_bind_t p_msg)
{
    e_error_code_t      e_code = E_SUCCESS;
    pst_write_req_t     p_req  = NULL;


    try_exception ((p_req = (pst_write_req_t) malloc(sizeof(*p_req))) == NULL,
                   exception_alloc_object);

    p_req->buf.len   = sizeof (st_eigw_response_bind_t);
    try_exception ((p_req->buf.base  = (char *)malloc (p_req->buf.len))
                   == NULL,
                   exception_alloc_object);

    memcpy (p_req->buf.base, p_msg, sizeof (p_req->buf.len));
    (void) uv_write ((uv_write_t*) p_req,
                     p_client,
                     &p_req->buf,
                     1,
                     send_bind_response_to_udr_done);

    try_catch (exception_alloc_object)
    {
        fprintf (stdout,"fail, alloc send object\n");
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      recv_bind_request_from_udr
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
e_error_code_t
recv_bind_request_from_udr (uv_stream_t            *p_client,
                            pst_eigw_request_t      p_req)
{
    e_error_code_t  e_code = E_SUCCESS;
    st_eigw_response_bind_t rsp;

    memcpy (&rsp.header, &p_req->header, sizeof (rsp.header));
    if (GET_MSGN_NAME(p_req->header.unMsgName) == EIGW_MSG_NAME_HEARTBEAT)
    {
        rsp.header.unMsgName = MK_MSGN(MSG_TYPE_RSP, EIGW_MSG_NAME_HEARTBEAT);
    }
    else if (GET_MSGN_NAME(p_req->header.unMsgName)
            == EIGW_MSG_NAME_BIND)
    {
        rsp.header.unMsgName = MK_MSGN(MSG_TYPE_RSP, EIGW_MSG_NAME_BIND);
    }
    rsp.header.sRet     = SUCC;
    rsp.header.usLength = htons (sizeof (rsp));
    e_code = send_bind_response_to_udr (p_client, &rsp);

    fprintf (stdout, "==>send %lu\n", rsp.header.ulSeq);
    return (e_code);
}

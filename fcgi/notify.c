/* ***************************************************************************
 *       Filename:  notify.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/06/18 16:11:45
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <hcommon.h>
#include <hworker.h>
#include <hfcgi.h>
#include <heigw.h>

#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>



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
encode_eigw_handle   (pst_eigw_request_t      p_req,
                      int                     seq)
{
    short               *p_now;
    int                 ind_now, ind_next;
    char                *p_env;


    p_req->header.cFrame[0]    = 0xFE;
    p_req->header.cFrame[1]    = 0xFE;
    p_req->header.unMsgName = MK_MSGN (MSG_TYPE_RPT, EIGW_MSG_NAME_REST_NOTIFY);
    p_req->header.ulSeq         = (unsigned long) seq;
    p_req->header.unGwRteVal    = 0;
    p_req->header.sRet          = SUCC;
    p_req->header.ucVersion     = '0';
    p_req->header.ucReserved    = '0';


    p_now = (short *) ((char *)p_req
            + offsetof (st_eigw_request_t, body.ind1));

    /* ----------------------------------------------------------------
     * NETWORK FUNCTION
     * ---------------------------------------------------------------- */
    ind_now = 0;
    strcpy (p_req->body.data + ind_now, "unknown");
    ind_next = ind_now + strlen (p_req->body.data + ind_now) + 1;
    p_req->body.data [ind_next - 1] = '\0';
    *p_now = ind_now;

    /* ----------------------------------------------------------------
     * CLIENT_IP ---> URI
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    strcpy (p_req->body.data + ind_now,
            ((p_env = getenv("REMOTE_ADDR"))  == NULL)
            ? (char *)"NULL" : p_env);
    ind_next = ind_now + strlen (p_req->body.data + ind_now) + 1;
    p_req->body.data [ind_next - 1] = '\0';
    *(p_now+1) = ind_now;


    /* ----------------------------------------------------------------
     * CONTENT-TYPE
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    strcpy (p_req->body.data + ind_now,
            ((p_env = getenv("CONTENT_TYPE")) == NULL)
                ? (char *)"NULL" : p_env);
    ind_next = ind_now + strlen (p_req->body.data + ind_now) + 1;
    p_req->body.data [ind_next - 1] = '\0';
    *(p_now+2) = ind_now;

    /* ----------------------------------------------------------------
     * CONTENT-LENGTH
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    strcpy (p_req->body.data + ind_now,
            ((p_env = getenv("CONTENT_LENGTH")) == NULL)
                ? (char *)"0" : p_env);
    ind_next = ind_now + strlen (p_req->body.data + ind_now) + 1;
    p_req->body.data [ind_next - 1] = '\0';
    *(p_now+3) = ind_now;

    /* ----------------------------------------------------------------
     * REQUEST URI --->  HEADER
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    strcpy (p_req->body.data + ind_now,
            ((p_env = getenv("SCRIPT_NAME"))  == NULL)
            ? (char *)"NULL" : p_env);
    ind_next = ind_now + strlen (p_req->body.data + ind_now) + 1;
    p_req->body.data [ind_next - 1] = '\0';
    *(p_now+4) = ind_now;


    /* ----------------------------------------------------------------
     * BODY
     * ---------------------------------------------------------------- */
    ind_now = ind_next;
    p_env = getenv ("CONTENT_LENGTH");
    if (p_env != NULL)
    {
        fread (p_req->body.data + ind_now, atoi (p_env), 1, stdin);
    }
    else
    {
        strcpy (p_req->body.data + ind_now, "body");
    }
    ind_next = ind_now + strlen (p_req->body.data + ind_now) + 1;
    p_req->body.data [ind_next - 1] = '\0';
    *(p_now+5) = ind_now;

    p_req->header.usLength = offsetof (st_eigw_request_t, body.data)
                        + ind_next;
    p_req->header.usLength = htons (p_req->header.usLength);

    return (E_SUCCESS);
}




/* **************************************************************************
 *	@brief      process_put_method
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @retval     void
 * **************************************************************************/

e_error_code_t
process_put_method (pst_fcgi_handle_t    p_handle,
                    char                 *p_err_string)
{
    e_error_code_t       e_code = E_SUCCESS;
    st_fcgi_notify_t     notification;
    st_eigw_request_t    req;
    static unsigned long seq          = 0;
    int                total_length = 0;


    sprintf (notification.header.pid,"%d",getpid ());
    sprintf (notification.header.seq, "%ld",seq++);

    memset (&req, 0x00, sizeof (req));
    (void) encode_eigw_handle (&req, seq);
    memcpy (notification.body.data, &req, ntohs(req.header.usLength));

    notification.header.length = offsetof (st_fcgi_notify_t, body.data)
                                 + ntohs (req.header.usLength);
    total_length               = notification.header.length;
    notification.header.length = htonl (notification.header.length);

    if (p_handle->p_sock == NULL)
    {

        strcpy (p_err_string,
                "fail, internal error (socket is NULL for backend gateway interface)\n");
        return (E_FAILURE);
    }

    if (p_handle->p_sock->sfd < 0)
    {
        e_code = (*p_handle->p_sock->pf_connect)(p_handle->p_sock);
        if (e_code != E_SUCCESS)
        {
            sprintf (p_err_string,
                    "fail, internal error (connect to notification gateway [%s:%s] [%d,%s])\n",
                    p_handle->p_sock->remote_ip,
                    p_handle->p_sock->remote_port,
                    p_handle->p_sock->err_code,
                    p_handle->p_sock->err_string);
            return (e_code);
        }
    }


    p_handle->p_sock->p_sndbuf = (char *)&notification;
    e_code = (*p_handle->p_sock->pf_send)(p_handle->p_sock,
                                          total_length);
    if (e_code != E_SUCCESS)
    {
        sprintf (p_err_string,
                "fail, internal error (sending message to notification gateway [%s:%s] [%d,%s])\n",
                  p_handle->p_sock->remote_ip,
                  p_handle->p_sock->remote_port,
                  p_handle->p_sock->err_code,
                  p_handle->p_sock->err_string);
        return (e_code);
    }

    strcpy (p_err_string, "Success");

    return (e_code);
}



/* ***************************************************************************
 *       Filename:  eigw.c
 *    Description:
 *        Version:  1.0
 *        Created:  07/11/18 11:23:33
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <seigw.h>


/* **************************************************************************
 *	@brief      encode_udr_request_header
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
e_error_code_t
encode_http_request_msg_header (char                      *p_fname,
                                pst_eigw_request_header_t p_header,
                                int                       body_length)
{
    e_error_code_t  e_code = E_SUCCESS;
    unsigned short  frame  = 0xFEFE;
    char            value [256];

    /*  http method field           */
    memcpy (p_header->cFrame, &frame, 2);

    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_FRAME, value);
    if (value[0] != 0x00)
    {
        frame = (short) atoi (value);
        memcpy (p_header->cFrame, &frame, 2);
    }

    p_header->usLength  = offsetof (st_eigw_request_t, body) + body_length;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_LENGTH, value);
    if (value[0] != 0x00)
    {
        p_header->usLength = (unsigned short) atoi (value);
    }
    p_header->usLength  = htons (p_header->usLength);


    p_header->unMsgName  = MK_MSGN (MSG_TYPE_REQ,EIGW_MSG_NAME_REST_REQ);
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_NAME, value);
    if (value[0] != 0x00)
    {
        p_header->unMsgName = (unsigned int) atoi (value);
    }

    p_header->ulSeq      = 0;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_SEQ, value);
    if (value[0] != 0x00)
    {
        p_header->ulSeq = (unsigned long) atoi (value);
    }


    p_header->unGwRteVal = 0;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_GWRTVAL, value);
    if (value[0] != 0x00)
    {
        p_header->unGwRteVal = (unsigned int) atoi (value);
    }


    p_header->sRet = 0;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_RET, value);
    if (value[0] != 0x00)
    {
        p_header->sRet = (short) atoi (value);
    }

    p_header->ucVersion  = '1';
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_VERSION, value);
    if (value[0] != 0x00)
    {
        p_header->ucVersion = value[0];
    }

    p_header->ucReserved = '0';
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_EIGW_HEADER_RESERVED, value);
    if (value[0] != 0x00)
    {
        p_header->ucReserved = value[0];
    }

    return (e_code);
}



/* **************************************************************************
 *	@brief      encode_http_request_msg_body
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
e_error_code_t
encode_http_request_msg_body (char                      *p_fname,
                              pst_eigw_request_body_t    p_body,
                              int                       *p_length)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            value [256];
    int             ind_now, ind_next;


    /*  http method field           */
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG (p_fname, CFG_HTTP_METHOD, value);
    strcpy (p_body->method, value);


    /*  network function indicator  */
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG(p_fname, CFG_HTTP_NF, value);

    ind_now = 0;
    strcpy (p_body->data + ind_now, value);
    ind_next = ind_now + strlen (p_body->data + ind_now) + 1;
    p_body->data [ind_next - 1] = '\0';
    p_body->ind1 = ind_now;

    /*  request uri indicator  */
    ind_now = ind_next;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG(p_fname, CFG_HTTP_URI, value);

    strcpy (p_body->data + ind_now, value);
    ind_next = ind_now + strlen (p_body->data + ind_now) + 1;
    p_body->data [ind_next - 1] = '\0';
    p_body->ind2 = ind_now;


    /*  request content type  */
    ind_now = ind_next;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG(p_fname, CFG_HTTP_CONTENT_TYPE, value);

    strcpy (p_body->data + ind_now,value);
    ind_next = ind_now + strlen (p_body->data + ind_now) + 1;
    p_body->data [ind_next - 1] = '\0';
    p_body->ind3 = ind_now;

    /*  request content length  */
    ind_now = ind_next;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG(p_fname, CFG_HTTP_CONTENT_LENGTH, value);
    strcpy (p_body->data + ind_now, value);
    ind_next = ind_now + strlen (p_body->data + ind_now) + 1;
    p_body->data [ind_next - 1] = '\0';
    p_body->ind4 = ind_now;

    /*  request add header      */
    ind_now = ind_next;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG(p_fname, CFG_HTTP_ADD_HEADER, value);
    strcpy (p_body->data + ind_now, value);
    ind_next = ind_now + strlen (p_body->data + ind_now) + 1;
    p_body->data [ind_next - 1] = '\0';
    p_body->ind5 = ind_now;

    ind_now = ind_next;
    memset (value, 0x00, sizeof (value));
    READ_CONFIG_MSG(p_fname, CFG_HTTP_BODY, value);
    strcpy (p_body->data + ind_now, value);
    ind_next = ind_now + strlen (p_body->data + ind_now) + 1;
    p_body->data [ind_next - 1] = '\0';
    p_body->ind6 = ind_now;

    *p_length = offsetof (st_eigw_request_body_t, data) + ind_next;


    return (e_code);
}




/* **************************************************************************
 *	@brief      encode_udr_request_msg
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

e_error_code_t
encode_udr_request_msg (char *p_fname, pst_eigw_request_t p_msg)
{
    int     length = 0;


    if (Is_FileExist (p_fname) != TRUE)
    {
        fprintf (stderr, "fail, not found test suite file\n");
        fflush  (stderr);
        return (E_FAILURE);
    }

    encode_http_request_msg_body   (p_fname, &p_msg->body, &length);
    encode_http_request_msg_header (p_fname, &p_msg->header, length);

    return (E_SUCCESS);
}




/* **************************************************************************
 *	@brief      decode_udr_response_msg
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

e_error_code_t
decode_udr_response_msg (pst_eigw_response_t p_msg)
{
    e_error_code_t      e_code = E_SUCCESS;
    short               *p_ind = &p_msg->body.ind1;

    fprintf (stdout, "METHOD         => %s\n", p_msg->body.method);
    fprintf (stdout, "STATUS         => %s\n", p_msg->body.status_code);
    fprintf (stdout, "NF             => [ind:%d] %s\n",
            *(p_ind + 0), p_msg->body.data + *(p_ind + 0));
    fprintf (stdout, "CONTENT_TYPE   => [ind:%d] %s\n",
            *(p_ind + 1), p_msg->body.data + *(p_ind + 1));
    fprintf (stdout, "CONTENT_LENGTH => [ind:%d] %s\n",
            *(p_ind + 2), p_msg->body.data + *(p_ind + 2));
    fprintf (stdout, "BODY           => [ind:%d] %s\n",
            *(p_ind + 3),
            p_msg->body.data + *(p_ind + 3));


    return (e_code);

}


/* **************************************************************************
 *	@brief      decode_udr_notification_msg
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

e_error_code_t
decode_udr_notification_msg (pst_eigw_request_t p_msg)
{
    e_error_code_t      e_code = E_SUCCESS;
    short               *p_ind = &p_msg->body.ind1;

    fprintf (stdout, "SEQ            => %lu\n", p_msg->header.ulSeq);
    fprintf (stdout, "METHOD         => %s\n", p_msg->body.method);

    fprintf (stdout, "NF             => [ind:%d] %s\n",
            *(p_ind + 0), p_msg->body.data + *(p_ind + 0));
    fprintf (stdout, "URI            => [ind:%d] %s\n",
            *(p_ind + 1), p_msg->body.data + *(p_ind + 1));
    fprintf (stdout, "CONTENT_TYPE   => [ind:%d] %s\n",
            *(p_ind + 2), p_msg->body.data + *(p_ind + 2));
    fprintf (stdout, "CONTENT_LENGTH => [ind:%d] %s\n",
            *(p_ind + 3), p_msg->body.data + *(p_ind + 3));
    fprintf (stdout, "HEADER         => [ind:%d] %s\n",
            *(p_ind + 4), p_msg->body.data + *(p_ind + 4));
    fprintf (stdout, "BODY           => [ind:%d] %s\n",
            *(p_ind + 5),
            p_msg->body.data + *(p_ind + 5));

    return (e_code);
}


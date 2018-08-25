/* ***************************************************************************
 *
 *       Filename:  seigw.h
 *    Description:
 *        Version:  1.0
 *        Created:  07/13/18 09:51:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _SIM_EIGW_H_
#define _SIM_EIGW_H_

#include <cmn/lib/util/cfg/cfg.h>
#include <hcommon.h>
#include <heigw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>


/*  server listen port */
#define SEIGW_DEFAULT_PORT          8082
#define SEIGW_DEFAULT_BACKLOG       128
#define ENV_EIGW_LISTEN_PORT        "SEIGW_LISTEN_PORT"

#define CFG_TEST_DATA_SECTION       "TEST_CASE_INPUT"
#define READ_CONFIG_MSG(fname, field, buf)  \
    ReadConfFile ((fname), CFG_TEST_DATA_SECTION, (field), (buf), (buf))



/* ------------------------------------------------------------------------
 *  MESSAGE HEADER  VALUE DEFINE
 * ------------------------------------------------------------------------ */
#define CFG_EIGW_HEADER_FRAME               "HEADER_FRAME"
#define CFG_EIGW_HEADER_LENGTH              "HEADER_LENGTH"
#define CFG_EIGW_HEADER_NAME                "HEADER_NAME"
#define CFG_EIGW_HEADER_SEQ                 "HEADER_SEQ"
#define CFG_EIGW_HEADER_GWRTVAL             "HEADER_GWRTVAL"
#define CFG_EIGW_HEADER_RET                 "HEADER_RET"
#define CFG_EIGW_HEADER_VERSION             "HEADER_VERSION"
#define CFG_EIGW_HEADER_RESERVED            "HEADER_RESERVED"


/* ------------------------------------------------------------------------
 *  MESSAGE NAME  VALUE DEFINE
 * ------------------------------------------------------------------------ */
#define CFG_HTTP_NF                 "NF"
#define CFG_HTTP_METHOD             "METHOD"
#define CFG_HTTP_URI                "URI"
#define CFG_HTTP_CONTENT_TYPE       "CONTENT_TYPE"
#define CFG_HTTP_CONTENT_LENGTH     "CONTENT_LENGTH"
#define CFG_HTTP_ADD_HEADER         "ADD_HEADER"
#define CFG_HTTP_BODY               "BODY"



/*  ----------------------------------------------------------------------
 *  uv write object
 *  --------------------------------------------------------------------- */
#define SEIGW_MAX_NUM_OF_SEND_BUF 1024
typedef struct _st_write_req_t   *pst_write_req_t;
typedef struct _st_write_req_t
{
    uv_write_t          req;
    uv_buf_t            buf;
} st_write_req_t;


typedef struct  _st_recv_buf_t   *pst_recv_buf_t;
typedef struct  _st_recv_buf_t
{
    int                 ind;
    st_eigw_response_t  rsp;
} st_recv_buf_t;


void           on_close                         (uv_handle_t    *p_handle);
void           recv_message_from_udr_start      (uv_stream_t    *p_client,
                                                 int            num_of_trans);
void           send_message_to_udr_start        (void           *p_client);

e_error_code_t encode_udr_request_msg           (char *,
                                                 pst_eigw_request_t p_msg);
e_error_code_t decode_udr_response_msg          (pst_eigw_response_t p_msg);
e_error_code_t recv_bind_request_from_udr_start (uv_stream_t  *p_client);


#endif

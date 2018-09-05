/* ***************************************************************************
 *
 *       Filename:  heigw.h
 *    Description:  eigw interface
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _HEIGW_H
#define _HEIGW_H 1

#include <mpool.h>
#include <hstream.h>
#include <hsocket.h>
#include <hdomain.h>
#include <cmn/msg.h>


/* ---------------------------------------------------------------------
 * EIGW I/F Message Name
 * --------------------------------------------------------------------- */

#define EIGW_MSG_NAME_BIND                      0xff00
#define EIGW_MSG_NAME_HEARTBEAT                 0x0000
#define EIGW_MSG_NAME_REST_REQ                  0x0001
#define EIGW_MSG_NAME_REST_RSP
#define EIGW_MSG_NAME_REST_NOTIFY               0x0002


/* ---------------------------------------------------------------------
 * eigw pending buffer (NOT USED)
 * #define HTTP_MAX_EIGW_POOL_SIZE  64
 * --------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 * DEFINE CONFIG OPTION
 * ----------------------------------------------------------------------- */
#define EIGW_CONFIG_SECTION_NAME                "EIGW"
#define EIGW_CONFIG_MODULE_ID_NAME              "MODULE_ID"
#define EIGW_CONFIG_HB_INTERVAL_NAME            "HB_INTERVAL"
#define EIGW_CONFIG_STREAM_BUF_SIZE_NAME        "STREAM_BUF_SIZE"
#define EIGW_CONFIG_PENDING_BUF_COUNT_NAME      "PENDING_BUF_COUNT"

#define EIGW_CONFIG_MODULE_ID_DEFAULT           "0"
#define EIGW_CONFIG_HB_INTERVAL_DEFAULT         "3"
#define EIGW_CONFIG_STREAM_BUF_SIZE_DEFAULT     "65536"
#define EIGW_CONFIG_PENDING_BUF_COUNT_DEFAULT   "64"


/*  ----------------------------------------------------------------------
 *  EIGW (unGwRteVal)
 *  ---------------------------------------------------------------------- */
typedef struct _st_eigw_client_id_t *pst_eigw_client_id_t;
typedef struct _st_eigw_client_id_t
{
    char    system_id;
    char    node_type;
    char    node_id;
    char    module_id;
} st_eigw_client_id_t;

/*  ----------------------------------------------------------------------
typedef struct stMsgHeader{
	char        cFrame[2];  Message frame, must be set to 0xFEFE
	u_short  	usLength;   Total message length (Header length + Body length)
	u_int 		unMsgName;  Message name
	u_long		ulSeq;	    Message sequence
	u_int		unGwRteVal; G/W route value
	short		sRet;		Result
	u_char		ucVersion;	Version
	u_char		ucReserved;	Reserved
}_MSG_HEADER;
 -------------------------------------------------------------------------- */
#define EIGW_HEADER_FRAME_VALUE    0xFEFE

typedef _MSG_HEADER                 st_eigw_request_header_t;
typedef st_eigw_request_header_t   *pst_eigw_request_header_t;

typedef st_eigw_request_header_t    st_eigw_response_header_t;
typedef st_eigw_response_header_t  *pst_eigw_response_header_t;

typedef struct _st_eigw_request_body_t  *pst_eigw_request_body_t;
typedef struct _st_eigw_request_body_t
{
    /*  body fixed value            */
    char    method [8];

    /*  body data   indicator       */
    short   ind1; /* ind_network_function     */
    short   ind2; /* ind_uri                  */
    short   ind3; /* ind_content_type;        */
    short   ind4; /* ind_content_length;      */
    short   ind5; /* ind_http_header;         */
    short   ind6; /* ind_http_body;           */

    char    data [64*1024];
} st_eigw_request_body_t;

typedef struct _st_eigw_response_body_t  *pst_eigw_response_body_t;
typedef struct _st_eigw_response_body_t
{
    /*  body fixed value            */
    char    method      [8];
    char    status_code [4];

    /*  body data   indicator      */
    short   ind1; /* ind_network_function     */
    short   ind2; /* ind_content_type;        */
    short   ind3; /* ind_content_length;      */
    short   ind4; /* ind_http_body;           */

    /*  body data value             */
    char    data [64*1024];
} st_eigw_response_body_t;


typedef struct _st_eigw_request_t   *pst_eigw_request_t;
typedef struct _st_eigw_request_t
{
    st_eigw_request_header_t    header;
    st_eigw_request_body_t      body;
} st_eigw_request_t;

typedef struct _st_eigw_response_t   *pst_eigw_response_t;
typedef struct _st_eigw_response_t
{
    st_eigw_response_header_t    header;
    st_eigw_response_body_t      body;
} st_eigw_response_t;


#define EIGW_MAX_MSG_REQUEST_LENGTH     0xEF
#define EIGW_MAX_MSG_BODY_DATA_LENGTH(x) \
    ((unsigned short)(0xFFFF) - (unsigned short)offsetof(x,  body.data) - 1024)


/* -----------------------------------------------------------------------
 *  EIGW BIND I/F MESSAGE
 * ---------------------------------------------------------------------- */
typedef struct _st_eigw_request_body_bind_t *pst_eigw_request_body_bind_t;
typedef struct _st_eigw_request_body_bind_t
{
    char    system_id;
    char    node_type;
    char    node_id;
    char    module_id;
} st_eigw_request_body_bind_t;

typedef struct _st_eigw_request_bind_t   *pst_eigw_request_bind_t;
typedef struct _st_eigw_request_bind_t
{
    st_eigw_request_header_t    header;
    st_eigw_request_body_bind_t body;
} st_eigw_request_bind_t;

typedef struct _st_eigw_response_bind_t   *pst_eigw_response_bind_t;
typedef struct _st_eigw_response_bind_t
{
    st_eigw_response_header_t    header;
} st_eigw_response_bind_t;


/* -----------------------------------------------------------------------
 *  EIGW HEARTBEAT I/F MESSAGE
 * ---------------------------------------------------------------------- */
typedef struct _st_eigw_request_hb_t   *pst_eigw_request_hb_t;
typedef struct _st_eigw_request_hb_t
{
    st_eigw_request_header_t    header;
} st_eigw_request_hb_t;

typedef struct _st_eigw_response_hb_t   *pst_eigw_response_hb_t;
typedef struct _st_eigw_response_hb_t
{
    st_eigw_response_header_t    header;
} st_eigw_response_hb_t;


/*  ----------------------------------------------------------------------
 *  EIGW SEND SEQUENCE
 *  x: stream buffer,  y: available buffer size
 *  ----------------------------------------------------------------------*/

#define GET_EIGW_SEND_SEQ(handle, value) { \
    pthread_mutex_lock (&handle->send_lock); \
    value = handle->send_seq++; \
    pthread_mutex_unlock (&handle->send_lock);\
}


/* -----------------------------------------------------------------------
 *  EIGW HANDLE
 * ---------------------------------------------------------------------- */
typedef struct _st_eigw_handle_t *pst_eigw_handle_t;
typedef struct _st_eigw_handle_t
{
    /*  EIGW configuration file         */
    char                        cfname   [FNAME_STRING_BUF_LEN];
    char                        csection [FNAME_STRING_BUF_LEN];

    /*  sequenct mutex                  */
    pthread_mutex_t             send_lock;
    unsigned long               send_seq;

    /*  client identifier for EIGW COMM */
	st_eigw_client_id_t         client_id;
    unsigned int                hb_interval;
    unsigned int                last_tick;

    /*  EIGW socket handle              */
    int                         err_no;
    char                        err_string   [256];

    /*  EIGW socket handle              */
    pst_stream_handle_t         p_rbuf;
    pst_tcp_socket_handle_t     p_sock;

    pst_eigw_request_t          p_req;
    pst_eigw_response_t         p_rsp;

    /*  NOT USED                        */
    int                         size_of_pqueue;
    pst_mpool_handle_t          p_pending_queue;


    e_error_code_t              (*pf_connect)         (pst_eigw_handle_t);
    e_error_code_t              (*pf_disconnect)      (pst_eigw_handle_t);
    e_error_code_t              (*pf_bind)            (pst_eigw_handle_t);
    e_error_code_t              (*pf_heartbeat)       (pst_eigw_handle_t);
    e_error_code_t              (*pf_recv)            (pst_eigw_handle_t);
    e_error_code_t              (*pf_send)            (pst_eigw_handle_t,
                                                       pst_eigw_request_t,
                                                       pst_eigw_response_t);

    e_error_code_t              (*pf_encode)          (pst_eigw_request_t,
                                                       pst_eigw_response_t,
                                                       long,
                                                       char *,
                                                       double,
                                                       pst_mchunk_handle_t,
                                                       pst_mchunk_handle_t,
                                                       char *);

    e_error_code_t              (*pf_validate_header) (pst_eigw_handle_t,
                                                       pst_eigw_request_t);

    e_error_code_t              (*pf_validate_body)   (pst_eigw_handle_t,
                                                       pst_eigw_request_t);

    void                        (*pf_show)            (pst_eigw_handle_t,
                                                       char *);
} st_eigw_handle_t;


e_error_code_t
init_eigw_handle (pst_eigw_handle_t *pp_handle,
                  char              *p_fname,
                  char              *p_section);
e_error_code_t
destroy_eigw_handle (pst_eigw_handle_t  p_handle);


#endif

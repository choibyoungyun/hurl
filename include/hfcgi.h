/* ***************************************************************************
 *
 *       Filename:  hfcgi.h
 *    Description:  fast cgi interface
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _HFCGI_H
#define _HFCGI_H 1

#include <hdomain.h>
#include <hsocket.h>
#include <heigw.h>

/* -----------------------------------------------------------------------
 * DEFINE CONFIG OPTION
 * ----------------------------------------------------------------------- */
#define FCGI_CONFIG_SECTION_NAME        "FCGI"
#define FCGI_CONFIG_RBUF_LEN_NAME       "FCGI_RBUF_LENGTH"
#define FCGI_CONFIG_RBUF_LEN_DEFAULT    "65536"

/* -----------------------------------------------------------------------
 *  INTERFACE MESSAGE
 * ---------------------------------------------------------------------- */
typedef struct _st_fcgi_notify_header_t  *pst_fcgi_notify_header_t;
typedef struct _st_fcgi_notify_header_t
{
    unsigned int    length;
    char            pid [8 ];
    char            seq [32];
} st_fcgi_notify_header_t;


#ifndef FCGI_NOTIFY_BODY_LEN
 #define FCGI_NOTIFY_BODY_LEN    (64*1024)
#endif

typedef struct _st_fcgi_notify_body_t  *pst_fcgi_notify_body_t;
typedef struct _st_fcgi_notify_body_t
{
    char    data [FCGI_NOTIFY_BODY_LEN];
} st_fcgi_notify_body_t;


typedef struct _st_fcgi_notify_t  *pst_fcgi_notify_t;
typedef struct _st_fcgi_notify_t
{
    st_fcgi_notify_header_t header;
    st_fcgi_notify_body_t   body;
} st_fcgi_notify_t;



/* -----------------------------------------------------------------------
 *  EIGW HANDLE
 * ---------------------------------------------------------------------- */
typedef struct _st_fcgi_handle_t *pst_fcgi_handle_t;
typedef struct _st_fcgi_handle_t
{
    /*  EIGW configuration file         */
    char                        cfname   [128];
    char                        csection [128];

    /* --------------------------------------
     *  HTTP CGI socket handle
     *  pst_domain_socket_handle_t  p_sock;
     * -------------------------------------- */
    /*
    int                         sndbuf_length;
    char                        *p_sndbuf; */

    pst_stream_handle_t         p_rbuf;
    pst_socket_handle_t         p_sock;


    e_error_code_t              (*pf_set    )   (pst_fcgi_handle_t);
    e_error_code_t              (*pf_connect)   (pst_fcgi_handle_t);
    e_error_code_t              (*pf_disconnect)(pst_fcgi_handle_t);
    e_error_code_t              (*pf_accept)    (pst_fcgi_handle_t);
    e_error_code_t              (*pf_send)      (pst_fcgi_handle_t);
    e_error_code_t              (*pf_recv)      (pst_fcgi_handle_t,
                                                 pst_fcgi_notify_t *);
    void                        (*pf_show)      (pst_fcgi_handle_t, char *);

} st_fcgi_handle_t;


e_error_code_t
init_fcgi_handle    (pst_fcgi_handle_t *pp_handle,
                     char              *p_fname,
                     char              *p_section);
e_error_code_t
destroy_fcgi_handle (pst_fcgi_handle_t  p_handle);


#endif

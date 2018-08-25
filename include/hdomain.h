/* ***************************************************************************
 *
 *       Filename:  hdomain.h
 *    Description:  unix domain socket
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#ifndef _HDOMAIN_H
#define _HDOMAIN_H  1

#include <hcommon.h>
#include <hlog.h>
#include <cmn/lib/ipc/unix_udp/unix_udplib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>


#define DOMAIN_SOCKET_CONFIG_SECTION_NAME   "DOMAIN_SOCKET"
#define DOMAIN_SOCKET_CONFIG_FNAME_NAME     "SOCKET_UNIX_DOMAIN"


typedef struct _st_domain_socket_addr_t    *pst_domain_socket_addr_t;
typedef struct _st_domain_socket_addr_t
{
    char                    fname [128];
} st_domain_socket_addr_t;


typedef struct _st_domain_socket_handle_t  *pst_domain_socket_handle_t;
typedef struct _st_domain_socket_handle_t
{
    char                    cfname   [128];
    char                    csection [128];

    int                     sfd;
    st_domain_socket_addr_t addr;

    int                     err_no;
    char                    err_string   [256];

    char                    *p_rcvbuf;
    char                    *p_sndbuf;

    e_error_code_t      (*pf_set)    (pst_domain_socket_handle_t);
    e_error_code_t      (*pf_connect)(pst_domain_socket_handle_t);
    e_error_code_t      (*pf_accept) (pst_domain_socket_handle_t);
    e_error_code_t      (*pf_recv)   (pst_domain_socket_handle_t, int, int *);
    e_error_code_t      (*pf_send)   (pst_domain_socket_handle_t, int);
    e_error_code_t      (*pf_close  )(pst_domain_socket_handle_t);
} st_domain_handle_t;


/* ------------------------------------------------------------------------
 *   handler  API
 * ------------------------------------------------------------------------ */
e_error_code_t
init_domain_socket      (pst_domain_socket_handle_t  *pp_handle,
                         char                        *p_cfname,
                         char                        *p_section);

e_error_code_t
destroy_domain_socket   (pst_domain_socket_handle_t  *pp_handle);

#endif

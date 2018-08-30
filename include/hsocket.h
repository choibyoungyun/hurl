/* ***************************************************************************
 *
 *       Filename:  hsocket.h
 *    Description:  socket
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#ifndef _HSOCKET_H
#define _HSOCKET_H  1

#include <hcommon.h>
#include <hlog.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>


typedef enum _e_socket_role_t
{
    SOCKET_SERVER_ROLE  = 0,
    SOCKET_CLIENT_ROLE  = 1
} e_socket_role_t;


typedef enum _e_socket_mode_t
{
    SOCKET_BLOCK_MODE    = 0,
    SOCKET_NONBLOCK_MODE = 1
} e_socket_mode_t;


#define SOCKET_CONFIG_SECTION_NAME         "SOCKET"


/* --------------------------------------------------------------
 *  SOCKET ROLE  (SERVER or CLIENT)
 * -------------------------------------------------------------- */
#define SOCKET_CONFIG_ROLE_NAME             "SOCKET_ROLE"
#define SOCKET_CONFIG_ROLE_DEFAULT          "SERVER"

/* --------------------------------------------------------------
 *  SOCKET OPTION
 * -------------------------------------------------------------- */
#define SOCKET_CONFIG_MODE_NAME             "SOCKET_MODE"
#define SOCKET_CONFIG_NODELAY_NAME          "SOCKET_NODELAY"
#define SOCKET_CONFIG_LINGER_NAME           "SOCKET_LINGER"

#define SOCKET_CONFIG_MODE_DEFAULT          "NONBLOCK"
#define SOCKET_CONFIG_NODELAY_DEFAULT       "0"
#define SOCKET_CONFIG_LINGER_DEFAULT        "0"

/* --------------------------------------------------------------
 *  IP ADDRESs
 * -------------------------------------------------------------- */
#define SOCKET_CONFIG_LOCAL_IP_NAME         "SOCKET_LOCAL_IP"
#define SOCKET_CONFIG_LOCAL_PORT_NAME       "SOCKET_LOCAL_PORT"
#define SOCKET_CONFIG_BACKLOG_NAME          "SOCKET_BACKLOG"
#define SOCKET_CONFIG_REMOTE_IP_NAME        "SOCKET_REMOTE_IP"
#define SOCKET_CONFIG_REMOTE_PORT_NAME      "SOCKET_REMOTE_PORT"

/* --------------------------------------------------------------
 * TIMER  OPTION
 * -------------------------------------------------------------- */
#define SOCKET_CONFIG_ATIMEOUT_NAME         "SOCKET_ATIMEOUT"
#define SOCKET_CONFIG_RTIMEOUT_NAME         "SOCKET_RTIMEOUT"
#define SOCKET_CONFIG_STIMEOUT_NAME         "SOCKET_STIMEOUT"
#define SOCKET_CONFIG_RESET_TIMEOUT_NAME    "SOCKET_RESET_TIMEOUT"

#define SOCKET_CONFIG_ATIMEOUT_DEFAULT      "1000"
#define SOCKET_CONFIG_RTIMEOUT_DEFAULT      "1000"
#define SOCKET_CONFIG_STIMEOUT_DEFAULT      "1000"
#define SOCKET_CONFIG_RESET_TIMEOUT_DEFAULT "0"  /* ulimtied */




#define SOCKET_CONFIG_LOCAL_IP_DEFAULT      "0.0.0.0"
#define SOCKET_CONFIG_BACKLOG_DEFAULT       "5"


typedef struct _st_socket_addr_t    *pst_socket_addr_t;
typedef struct _st_socket_addr_t
{
    int     domain;
    int     addr_len;

    union {
        struct  sockaddr_in  a4;
        struct  sockaddr_in6 a6;
    } addr;
} st_socket_addr_t;


typedef struct _st_socket_handle_t  *pst_socket_handle_t;
typedef struct _st_socket_handle_t
{
    char                cfname   [128];
    char                csection [128];

    /* ----------------------------------------------
     *  ROLE (SERVER | CLIENT)
     * --------------------------------------------- */
    e_socket_role_t     role;

    /* ----------------------------------------------
     *  socket  option
     * --------------------------------------------- */
    e_socket_mode_t     mode;
    int                 accept_timeout;
    int                 recv_timeout;
    int                 send_timeout;
    int                 reset_timeout;

    e_bool_t            nodelay;
    e_bool_t            linger;

    int                 sfd;
    int                 err_no;
    char                err_string   [256];

    /*  local  server information   */
    st_socket_addr_t    local;
    char                local_ip     [64];
    char                local_port   [16];
    int                 backlog;

    /*   remote  peer information   */
    st_socket_addr_t    remote;
    char                remote_ip     [64];
    char                remote_port   [16];

    char                *p_rcvbuf;
    char                *p_sndbuf;

    e_error_code_t      (*pf_connect)(pst_socket_handle_t);
    e_error_code_t      (*pf_accept) (pst_socket_handle_t,
                                      pst_socket_handle_t *);
    e_error_code_t      (*pf_recv)   (pst_socket_handle_t, int, int *);
    e_error_code_t      (*pf_send)   (pst_socket_handle_t, int);
    e_error_code_t      (*pf_close  )(pst_socket_handle_t);
    void                (*pf_show   )(pst_socket_handle_t, char *);
} st_socket_handle_t;


/* ------------------------------------------------------------------------
 *   redefinition tcp socket handle
 * ------------------------------------------------------------------------ */
typedef st_socket_handle_t  st_tcp_socket_handle_t;
typedef pst_socket_handle_t pst_tcp_socket_handle_t;


/* ------------------------------------------------------------------------
 *   handler  API
 * ------------------------------------------------------------------------ */
e_error_code_t
init_socket_handle      (pst_socket_handle_t  *pp_handle,
                         char                 *p_cfname,
                         char                 *p_section);

e_error_code_t
destroy_socket_handle   (pst_socket_handle_t  *pp_handle);

#endif

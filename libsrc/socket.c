/* ***************************************************************************
 *
 *       Filename:  socket.c
 *    Description:  socket handle
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hsocket.h>


#ifndef SET_SOCKET_TIMEOUT
 #define SET_SOCKET_TIMEOUT(x,y) { \
     x.tv_sec  = y / 1000; \
     x.tv_usec = (y % 1000) * 1000; \
 }
#endif




/* **************************************************************************
 *  @brief          show_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     p_handle     - socket handle
 *  @param [IN]     p_mname      - socket module name
 *  @retval         integer
 * **************************************************************************/
static
void
show_handle (pst_socket_handle_t   p_handle,
             char                  *p_mname)
{
    char    *p_name = (char *)"SOCKET";

    if (p_mname) p_name = p_mname;

    if (p_handle->role  == SOCKET_SERVER_ROLE)
    {
        Log (DEBUG_CRITICAL,
                "succ, loading %-8s property [ROLE:SERVER, IP:%s, PORT:%s]\n",
                p_name,
                p_handle->local_ip,
                p_handle->local_port);
    }
    else
    {
        Log (DEBUG_CRITICAL,
                "succ, loading %-8s property [ROLE:CLIENT, IP:%s, PORT:%s]\n",
                p_name,
                p_handle->remote_ip,
                p_handle->remote_port);
    }


    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [atimeout:%d, rtimeout:%d, stimeout:%d]\n",
            p_name,
            p_handle->accept_timeout,
            p_handle->recv_timeout,
            p_handle->send_timeout);

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [mode:%s, nodelay:%s, linger:%s]\n",
            p_name,
            p_handle->mode    ? "NONBLOCK" : "BLOCK",
            p_handle->nodelay ? "ON"       : "OFF",
            p_handle->linger  ? "ON"       : "OFF");

    return;
}




/* **************************************************************************
 *  @brief          set_timeout_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
set_timeout_handle (pst_socket_handle_t p_handle,
                    char                *p_arg)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];
    char            *p_section = NULL;


    p_section = p_arg;
    if (p_section == NULL)
    {
        p_section = (char *)SOCKET_CONFIG_SECTION_NAME;
    }

    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_ATIMEOUT_NAME,
                        SOCKET_CONFIG_ATIMEOUT_DEFAULT, buf);
    p_handle->accept_timeout = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_RTIMEOUT_NAME,
                        SOCKET_CONFIG_RTIMEOUT_DEFAULT, buf);
    p_handle->recv_timeout = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_STIMEOUT_NAME,
                        SOCKET_CONFIG_STIMEOUT_DEFAULT, buf);
    p_handle->send_timeout = atoi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_RESET_TIMEOUT_NAME,
                        SOCKET_CONFIG_RESET_TIMEOUT_DEFAULT, buf);
    p_handle->reset_timeout = atoi (buf);


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          set_option_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
set_option_handle (pst_socket_handle_t p_handle,
                   char                *p_arg)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];
    char            *p_section = NULL;


    p_section = p_arg;
    if (p_section == NULL)
    {
        p_section = (char *)SOCKET_CONFIG_SECTION_NAME;
    }

    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_MODE_NAME,
                        SOCKET_CONFIG_MODE_DEFAULT, buf);
    p_handle->mode = SOCKET_NONBLOCK_MODE;
    if (strcmp (buf, SOCKET_CONFIG_MODE_DEFAULT) != 0)
    {
        p_handle->mode = SOCKET_BLOCK_MODE;
    }

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_NODELAY_NAME,
                        SOCKET_CONFIG_NODELAY_DEFAULT, buf);
    p_handle->nodelay = BOOL_FALSE;
    if (buf[0] == '1')
        p_handle->nodelay = BOOL_TRUE;


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_LINGER_NAME,
                        SOCKET_CONFIG_LINGER_DEFAULT, buf);
    p_handle->linger = BOOL_FALSE;
    if (buf[0] == '1')
    {
        p_handle->linger = BOOL_TRUE;
    }


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          set_ipaddr_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
set_ipaddr_handle (pst_socket_handle_t p_handle,
                   char                *p_arg)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];
    char            *p_section = NULL;


    p_section = p_arg;
    if (p_section == NULL)
    {
        p_section = (char *)SOCKET_CONFIG_SECTION_NAME;
    }

    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_MODE_NAME,
                        SOCKET_CONFIG_MODE_DEFAULT, buf);
    p_handle->mode = SOCKET_NONBLOCK_MODE;
    if (strcmp (buf, SOCKET_CONFIG_MODE_DEFAULT) != 0)
    {
        p_handle->mode = SOCKET_BLOCK_MODE;
    }


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_LOCAL_IP_NAME,
                        SOCKET_CONFIG_LOCAL_IP_DEFAULT, buf);
    strcpy (p_handle->local_ip, buf);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_LOCAL_PORT_NAME,
                        NULL, buf);
    strcpy (p_handle->local_port, buf);

    p_handle->local.addr.a4.sin_family      = AF_INET;
    p_handle->local.addr.a4.sin_addr.s_addr = inet_addr (p_handle->local_ip);
    p_handle->local.addr.a4.sin_port        = atoi (p_handle->local_port);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_BACKLOG_NAME,
                        SOCKET_CONFIG_BACKLOG_DEFAULT, buf);
    p_handle->backlog = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_REMOTE_IP_NAME,
                        NULL, buf);
    strcpy (p_handle->remote_ip, buf);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_REMOTE_PORT_NAME,
                        NULL, buf);
    strcpy (p_handle->remote_port, buf);

    p_handle->remote.addr.a4.sin_family      = AF_INET;
    p_handle->remote.addr.a4.sin_addr.s_addr = inet_addr (p_handle->remote_ip);
    p_handle->remote.addr.a4.sin_port        = atoi (p_handle->remote_port);


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          set_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
set_handle (pst_socket_handle_t p_handle,
            char                *p_arg)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];
    char            *p_section = NULL;

    p_section = p_arg;
    if (p_section == NULL)
    {
        p_section = (char *)SOCKET_CONFIG_SECTION_NAME;
    }

    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_section,
                        SOCKET_CONFIG_ROLE_NAME,
                        SOCKET_CONFIG_ROLE_DEFAULT, buf);
    p_handle->role = SOCKET_SERVER_ROLE;
    if (strcmp (buf, SOCKET_CONFIG_ROLE_DEFAULT) != 0)
    {
        p_handle->role = SOCKET_CLIENT_ROLE;
    }


    try_exception ((e_code = set_option_handle  (p_handle, p_arg))
                   != E_SUCCESS,
                   exception_set_option);

    try_exception ((e_code = set_timeout_handle (p_handle, p_arg))
                   != E_SUCCESS,
                   exception_set_option);

    try_exception ((e_code = set_ipaddr_handle  (p_handle, p_arg))
                   != E_SUCCESS,
                   exception_set_option);


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
        Log (DEBUG_CRITICAL,
                "fail, not found file:%s]\n", p_handle->cfname);
    }
    try_catch (exception_set_option)
    {
        e_code = E_FILE_NOTFOUND;
        Log (DEBUG_CRITICAL,
                "fail, set socket option\n");
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          close_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     p_handle  - socket handle
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
close_handle (pst_socket_handle_t   p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    struct  linger  solinger = {1, 0};


    if (p_handle->linger == BOOL_TRUE)
    {
        (void)setsockopt (p_handle->sfd,
                          SOL_SOCKET,
                          SO_LINGER,
                          (char *)&solinger,
                          sizeof (struct linger));
    }

    close (p_handle->sfd);
    p_handle->sfd = -1;

    return (e_code);
}




/* **************************************************************************
 *  @brief          recv_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     sfd          - socket fd
 *  @param [IN]     p_buf        - recv buf
 *  @param [IN]     length       - recv length
 *  @param [IN]     micro_second - recv time out micor second
 *  @retval         integer
 * **************************************************************************/

static
e_error_code_t
recv_handle (pst_socket_handle_t    p_sock,
             int                    total_length,
             int                    *p_recv_length)
{
    e_error_code_t  e_code        = E_SUCCESS;
    int             ret           = 0;
    int             flag          = 0;
    int             recv_length   = 0;
    int             remain_length = 0;
    char            *p_now        = NULL;
    fd_set          read_fd_set;
    int             sfd;
    struct  timeval timeout;

    sfd  = p_sock->sfd;
    FD_ZERO (&read_fd_set);
    FD_SET (sfd, &read_fd_set);


    SET_SOCKET_TIMEOUT (timeout, p_sock->recv_timeout);
    p_now  = p_sock->p_rcvbuf;
    remain_length = total_length;

    flag = fcntl (sfd, F_GETFL, 0);
    fcntl (sfd, F_SETFL, flag | O_NONBLOCK);
    while (1)
    {
        try_exception ((ret = select (sfd + 1,
                                      &read_fd_set, NULL, NULL, &timeout)) <= 0,
                       exception_socket_select);
        try_exception (FD_ISSET (sfd, &read_fd_set) == 0,
                       exception_socket_select);

        try_exception ((recv_length = recv (sfd, p_now, remain_length, 0))
                       == 0,
                       exception_socket_recv);
        if (recv_length < 0)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))
            {
                continue;
            }
            try_exception (1, exception_socket_recv);
        }

        if ((remain_length = remain_length - recv_length) <= 0)
        {
            (*p_recv_length) += recv_length;
            e_code = E_SUCCESS;
            HTTP_LOG ("socket recv data [%d]\n", recv_length);
            break;
        }
        p_now =  p_now + recv_length;
        (*p_recv_length) += recv_length;
    }


    try_catch (exception_socket_select)
    {
        e_code = E_FAILURE;

        if (ret == 0)
        {
            e_code = E_TIMEOUT;
        }
        else
        {
            fcntl (sfd, F_SETFL, flag);

            p_sock->err_no = errno;
            strcpy (p_sock->err_string, strerror(errno));
            (*p_sock->pf_close)(p_sock);
            e_code      = E_SOCK_DISCONNECT;
        }
    }
    try_catch (exception_socket_recv)
    {
        /*  restore socket  option */
        fcntl (sfd, F_SETFL, flag);

        p_sock->err_no = errno;
        strcpy (p_sock->err_string, strerror(errno));
        (*p_sock->pf_close)(p_sock);
        e_code      = E_SOCK_DISCONNECT;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          send_socket
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     sfd          - socket fd
 *  @param [IN]     p_buf        - recv buf
 *  @param [IN]     length       - recv length
 *  @param [IN]     micro_second - recv time out micor second
 *  @retval         integer
 * **************************************************************************/

static
e_error_code_t
send_handle (pst_socket_handle_t    p_sock,
             int                    total_length)
{
    e_error_code_t  e_code        = E_SUCCESS;
    int             flag          = 0;
    int             send_length   = 0;
    int             remain_length = 0;
    char            *p_now        = NULL;
    int             sfd;

    sfd  = p_sock->sfd;
    flag = fcntl (sfd, F_GETFL, 0);
    fcntl (sfd, F_SETFL, flag | O_NONBLOCK);

    p_now = p_sock->p_sndbuf;
    remain_length = total_length;
    while (1)
    {
        try_exception ((send_length = send (sfd, p_now, remain_length, 0))
                       == 0,
                       exception_socket_send);

        if (send_length < 0)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))
            {
                fprintf (stdout,
                    "fail, send socket (%d, %s)\n", errno, strerror(errno));

                continue;
            }
            try_exception (1, exception_socket_send);
        }

        if ((remain_length = remain_length - send_length) <= 0)
        {
            e_code = E_SUCCESS;
            break;
        }
        p_now =  p_now + send_length;
    }


    try_catch (exception_socket_send)
    {
        fcntl (sfd, F_SETFL, flag);
        e_code = E_FAILURE;
        if ((errno == EPIPE) || (errno == ECONNRESET) || (send_length == 0))
        {
            p_sock->err_no = errno;
            strcpy (p_sock->err_string, strerror(errno));

            (*p_sock->pf_close)(p_sock);
            e_code = E_SOCK_DISCONNECT;
        }
        else
        {
            Log (DEBUG_ERROR,
                    "fail, send socket (%d, %s)\n", errno, strerror(errno));
        }
    }
    try_finally;

    /*  restore socket  option */
    fcntl (sfd, F_SETFL, flag);

    return (e_code);
}




/* **************************************************************************
 *  @brief          connect_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/

static
e_error_code_t
connect_handle (pst_socket_handle_t    p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    socklen_t       address_len;
    int             opt      = 1;


    address_len  = sizeof (struct sockaddr_in);
    p_handle->remote.domain                  = AF_INET;
    p_handle->remote.addr.a4.sin_family      = AF_INET;
    inet_pton (p_handle->remote.addr.a4.sin_family,
               p_handle->remote_ip, &(p_handle->remote.addr.a4.sin_addr));
    p_handle->remote.addr.a4.sin_port = htons(atoi(p_handle->remote_port));

    try_exception ((p_handle->sfd = socket (p_handle->remote.domain,
                                            SOCK_STREAM, 0)) < 0,
                   exception_create_socket);


    if (p_handle->nodelay == BOOL_TRUE)
    {
        try_exception (setsockopt (p_handle->sfd,
                                   IPPROTO_TCP,
                                   TCP_NODELAY,
                                   &opt,
                                   sizeof (opt)) < 0,
                       exception_option_socket);
    }

    try_exception (connect (p_handle->sfd,
                            (const struct sockaddr *)&(p_handle->remote.addr),
                            address_len)
                    != 0,
                   exception_connect_socket);


    try_catch (exception_create_socket)
    {
        p_handle->err_no = errno;
        strcpy (p_handle->err_string, strerror(errno));

        e_code = E_FAILURE;
    }
    try_catch (exception_option_socket)
    {
        p_handle->err_no = errno;
        strcpy (p_handle->err_string, strerror(errno));

        (*p_handle->pf_close)(p_handle);
        e_code = E_FAILURE;
    }
    try_catch (exception_connect_socket)
    {
        p_handle->err_no = errno;
        strcpy (p_handle->err_string, strerror(errno));

        (*p_handle->pf_close)(p_handle);
        e_code = E_FAILURE;
    }
    try_finally;


    return (e_code);
}




/* **************************************************************************
 *  @brief          listen_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     p_handle - socket handle
 *  @retval         integer
 * **************************************************************************/

static
e_error_code_t
listen_handle (pst_socket_handle_t   p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             option = 1;
    socklen_t       address_len;


    p_handle->local.domain = AF_INET6;
    address_len            = sizeof (struct sockaddr_in6);
    if (strchr (p_handle->local_ip, ':') == NULL)
    {
        p_handle->local.domain = AF_INET;
        address_len            = sizeof (struct sockaddr_in);
    }

    p_handle->local.addr.a4.sin_family      = AF_INET;
    inet_pton (p_handle->local.addr.a4.sin_family,
               p_handle->local_ip, &p_handle->local.addr.a4.sin_addr);
    p_handle->local.addr.a4.sin_port = htons (atoi(p_handle->local_port));

    try_exception ((p_handle->sfd = socket (p_handle->local.domain,
                                            SOCK_STREAM, 0)) < 0,
                    exception_create_socket);

    try_exception (setsockopt (p_handle->sfd,
                               SOL_SOCKET,
                               SO_REUSEADDR,
                               (char *)&option, sizeof(option)) < 0,
                    exception_listen_socket);

    try_exception (bind (p_handle->sfd,
                          (struct sockaddr *)&p_handle->local.addr,
                          (socklen_t) address_len) < 0,
                    exception_listen_socket);

    try_exception (listen (p_handle->sfd, p_handle->backlog) < 0,
                    exception_listen_socket);

    try_catch (exception_create_socket)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_listen_socket)
    {
        p_handle->err_no = errno;
        strcpy (p_handle->err_string, strerror(errno));

        (*p_handle->pf_close)(p_handle);
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          accept_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     argc
 *  @param [IN]     argv
 *  @retval         integer
 * **************************************************************************/

static
e_error_code_t
accept_handle (pst_socket_handle_t    p_local,
               pst_socket_handle_t    *pp_remote)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_socket_handle_t p_remote = NULL;
    fd_set          active_fd_set, read_fd_set;
    struct  timeval timeout;
    int             ret;
    int             addr_len;
    int             sfd = -1;


    if (p_local->sfd < 0)
    {
        try_exception (listen_handle (p_local)  != E_SUCCESS,
                       exception_listen_socket);
    }

    FD_ZERO (&active_fd_set);
    FD_SET (p_local->sfd, &active_fd_set);
    read_fd_set = active_fd_set;

    /*  set local bind port timeout */
    SET_SOCKET_TIMEOUT (timeout,p_local->accept_timeout);
    try_exception ((ret = select (p_local->sfd + 1,
                                  &read_fd_set, NULL, NULL, &timeout)) <= 0,
                   exception_listen_socket);

    if (FD_ISSET (p_local->sfd, &read_fd_set))
    {
        if (p_local->local.domain == AF_INET)
        {
            addr_len = sizeof (struct sockaddr_in);
        }
        else
        {
            addr_len = sizeof (struct sockaddr_in6);
        }

        sfd = accept (p_local->sfd,
                        (struct sockaddr *)&p_local->remote.addr,
                        (socklen_t *) &addr_len);
        try_exception (sfd < 0, exception_accept_call);
    }

    try_exception ((p_remote = (pst_socket_handle_t)
                                malloc (sizeof (st_socket_handle_t))) == NULL,
                   exception_alloc_handle);
    memcpy (p_remote, p_local, sizeof (*p_remote));
    p_remote->sfd  = sfd;
    p_remote->role = SOCKET_SERVER_ROLE;
    if (addr_len == sizeof (struct sockaddr_in))
    {
        p_remote->remote.domain = AF_INET;
        inet_ntop (p_remote->remote.domain,
                   &p_remote->remote.addr.a4.sin_addr.s_addr,
                   p_remote->remote_ip, sizeof (p_remote->remote_ip));
    }
    else
    {
        p_remote->remote.domain = AF_INET6;
        inet_ntop (p_remote->remote.domain,
                   &p_remote->remote.addr.a6.sin6_addr.s6_addr,
                   p_remote->remote_ip, sizeof (p_remote->remote_ip));
    }

    (*pp_remote) = p_remote;


    try_catch (exception_listen_socket)
    {
        if (ret == 0)
        {
            e_code = E_TIMEOUT;
        }
        else if (ret < 0)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                e_code = E_TIMEOUT;
            }
            else
            {
                e_code = E_FAILURE;
            }
        }
    }
    try_catch (exception_accept_call)
    {
        p_local->err_no = errno;
        strcpy (p_local->err_string, strerror(errno));
        e_code = E_FAILURE;
    }
    try_catch (exception_alloc_handle)
    {
        e_code = E_ALLOC_HANDLE;
    }
    try_finally;


    return (e_code);
}



/* **************************************************************************
 *  @brief          init_socket
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN/OUT] pp_handle - socket handle
 *  @param [IN]     p_cfname  - config file name
 *  @param [IN]     p_section - config section name
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_socket_handle (pst_socket_handle_t    *pp_handle,
                    char                   *p_cfname,
                    char                   *p_section)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_socket_handle_t p_handle = NULL;

    try_exception ((p_handle =
                        (pst_socket_handle_t) MALLOC (sizeof (*p_handle)))
                   == NULL,
                   exception_init_socket);
    memset (p_handle, 0x00, sizeof (*p_handle));

    strcpy (p_handle->cfname, p_cfname);
    if (p_section)
    {
        strcpy (p_handle->csection, p_section);
    }
    try_exception ((e_code = set_handle (p_handle, p_section)) != E_SUCCESS,
                   exception_init_socket);


    p_handle->sfd = -1;

    /*  define member function */
    p_handle->pf_connect = connect_handle;
    p_handle->pf_recv    = recv_handle;
    p_handle->pf_send    = send_handle;
    p_handle->pf_close   = close_handle;
    p_handle->pf_accept  = accept_handle;
    p_handle->pf_show    = show_handle;

    (*pp_handle) = p_handle;

    try_catch (exception_init_socket)
    {
        FREE (p_handle);
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          destroy_socket_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     argc
 *  @param [IN]     argv
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
destroy_socket_handle (pst_socket_handle_t *pp_handle)
{
    pst_socket_handle_t p_handle;

    p_handle = *pp_handle;

    if  (p_handle)
    {

        (*p_handle->pf_close)(p_handle);
        free (p_handle);

        *pp_handle = NULL;
    }

    return (E_SUCCESS);
}
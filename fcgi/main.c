
/* ***************************************************************************
 *       Filename:  main.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/06/18 16:11:45
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *                  - required following setting in httpd.conf file
 *                    SetEnv   PKG_ROOT   path
 * ***************************************************************************/

#include <hcommon.h>
#include <hworker.h>
#include <hfcgi.h>

#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>


#define HTTP_HEADER_STRING     "Content-Type: text/plain\r\n"
#define HTTP_HEADER_DELIMITER  "\r\n"

extern e_error_code_t
process_put_method(pst_fcgi_handle_t    p_handle,
                   char                 *p_err_string);

/* **************************************************************************
 *	@brief      init_process
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @retval     void
 * **************************************************************************/

e_error_code_t
init_process (pst_process_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;


    e_code = init_fcgi_handle (&p_handle->p_fcgi,
                               p_handle->cfname,
                               (char *) FCGI_CONFIG_SECTION_NAME);

    if (e_code != E_SUCCESS)
        return (E_FAILURE);

    return (E_SUCCESS);
}




/* **************************************************************************
 *	@brief			set_process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN/OUT] PROCESS HANDLE
 *  @param [IN] 	argv
 *  @retval     	E_SUCCESS/E_FAILURE
 * **************************************************************************/
e_error_code_t
set_process (pst_process_handle_t p_handle,
             char                 **argv)
{
    e_error_code_t  e_code  = E_SUCCESS;
    char            *p_base = NULL;
    char            *p_home = NULL;


    try_exception ((p_base = basename (argv[0])) == NULL,
                    exception_system_call);

    sprintf (p_handle->pname, "%s", p_base);
    if ((p_base = strchr(p_handle->pname,'.')) != NULL)
    {
        *p_base= 0x00;
    }

    if (p_handle->cfname[0] == 0x00)
    {
        try_exception ((p_home = getenv (PKG_BASE_VARIABLE)) == NULL,
                        exception_system_call);
        sprintf (p_handle->cfname,
                 "%s/config/%s.INI", p_home, p_handle->pname);
    }

    try_catch (exception_system_call)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief			get_option
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]		argc
 *  @param [IN] 	argv
 *  @param [IN/OUT]	PROCESS HANDLE
 *  @retval     	E_SUCCESS/E_FAILURE
 * **************************************************************************/
e_error_code_t
get_option (int argc, char **argv, pst_process_handle_t p_handle)
{
    e_error_code_t	e_code;
    int             opt;


    /* initialize	local	variable    */
    e_code = E_SUCCESS;

    while ((opt = getopt(argc, argv, "f:")) != -1)
    {
        switch (opt)
        {
            case 'f':
                strncpy (p_handle->cfname,
                         optarg, sizeof (p_handle->cfname) - 1);
                break;
            case 'h':
            default:
                break;
        }
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      main
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] process handle
 *  @retval     void
 * **************************************************************************/

int
main (int argc, char *argv[])
{
    int                 e_code = E_SUCCESS;
    st_process_handle_t handle;
    char                err_string [512]={0,};

    /* initialize local variable    */
    UNUSED (argc);
    UNUSED (argv);

    memset (&handle, 0x00, sizeof (handle));
    try_exception ((e_code = get_option (argc, argv, &handle)) != E_SUCCESS,
                   exception_main_process);


    while (FCGI_Accept() >= 0)
    {
        memset (err_string, 0x00, sizeof (err_string));

        if (strcmp (getenv ("REQUEST_METHOD"), "PUT") != 0)
        {
            printf (HTTP_HEADER_STRING);
            printf (HTTP_HEADER_DELIMITER);
            printf ("unsupport mehtod\n");
        }
        else
        {
            if (getenv ("CONTENT_LENGTH") == NULL)
            {
                printf (HTTP_HEADER_STRING);
                printf (HTTP_HEADER_DELIMITER);
                printf ("required Content-Length Header\n");
                continue;
            }


            if (handle.cfname [0] == 0x00)
            {
                (void) set_process (&handle, argv);
            }

            if (handle.p_fcgi == NULL)
            {
                (void) init_fcgi_handle (&handle.p_fcgi,
                                          handle.cfname,
                                        (char *) FCGI_CONFIG_SECTION_NAME);
            }

            if (handle.p_fcgi->p_sock == NULL)
            {
                (void) init_socket_handle (&handle.p_fcgi->p_sock,
                                           handle.cfname,
                                        (char *) FCGI_CONFIG_SECTION_NAME);
            }

            e_code = process_put_method (handle.p_fcgi, err_string);
            printf (HTTP_HEADER_STRING);
            printf (HTTP_HEADER_DELIMITER);
            printf (err_string);
        }
    }


    try_catch (exception_main_process)
    {

    }
    try_finally;

    return (e_code);
}

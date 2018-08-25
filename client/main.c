/* ***************************************************************************
 *
 *       Filename:  main.c
 *    Description:  main
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hworker.h>


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
static
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
        sprintf (p_handle->cfname, "%s/config/%s.INI", p_home, p_handle->pname);
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
static
e_error_code_t
get_option (int argc, char **argv, pst_process_handle_t p_handle)
{
    e_error_code_t	e_code;
    int             opt;


    /* initialize	local	variable    */
    e_code = E_SUCCESS;

    while ((opt = getopt(argc, argv, "i:f:")) != -1)
    {
        switch (opt)
        {
            case 'i':
                p_handle->pindex = StrToi(optarg);
                break;
            case 'f':
                strncpy (p_handle->cfname,
                         optarg, sizeof (p_handle->cfname) - 1);
                break;
            case 'h':
            default:
                break;
        }
    }

    /*  validate multi process index option (1 or 2) */
    /*
    if ((p_handle->pindex != 1) && (p_handle->pindex != 2))
    {
        Log (DEBUG_CRITICAL,
                "fail, invalid process index (%d)\n",
                p_handle->pindex);
        e_code = E_FAILURE;
    }
    */

    return (e_code);
}





/* **************************************************************************
 *  @brief          main function
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     argc
 *  @param [IN]     argv
 *  @retval         integer
 * **************************************************************************/

int
main (int argc, char *argv[])
{
    int                 e_code = E_SUCCESS;
    st_process_handle_t handle;


    /* initialize local variable    */
    memset (&handle, 0x00, sizeof (handle));


    try_exception ((e_code = get_option (argc, argv, &handle)) != E_SUCCESS,
                   exception_main_process);
    (void) set_process (&handle, argv);
    (void) set_stack_size (0);

    try_exception ((e_code = init_process (&handle)) != E_SUCCESS,
                   exception_main_process);

    e_code = do_worker (&handle);


    try_catch (exception_main_process)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    (void) destroy_process (&handle);

    return (e_code);
}

/* ***************************************************************************
 *       Filename:  main.c
 *    Description:
 *        Version:  1.0
 *        Created:  07/13/18 09:47:14
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <seigw.h>

extern e_error_code_t   do_job (int, char *, char *);


/* **************************************************************************
 *	@brief      init process
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

static
e_error_code_t
init_process (int  *p_port)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            *p_env = NULL;

    if ((p_env = getenv (ENV_EIGW_LISTEN_PORT)) == NULL)
    {
        *p_port = SEIGW_DEFAULT_PORT;
    }
    else
    {
        *p_port = atoi (p_env);
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      display usage
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

static
void
usage (char *p_name)
{
    fprintf (stderr,
            "Usage : %s -i [test_case]  [-p num_of_transmissions]\n",
            p_name);
    fflush (stderr);

    return;
}




/* **************************************************************************
 *	@brief      get option
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/
static
int
get_option (int         argc,
            char        **p_argv,
            char        *p_in_fname,
            char        *p_num)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             opt;


    while ((opt = getopt (argc, p_argv, "i:p:")) != -1)
    {
        switch (opt)
        {
            case 'i' :
                strcpy (p_in_fname, optarg);
                break;
            case 'p' :
                strcpy (p_num, optarg);
                break;
            case 'h':
                usage (basename (p_argv[0]));
            default :
                break;
        }
    }


    if (p_in_fname[0] == 0x00)
    {
        usage (basename (p_argv[0]));
        e_code = E_FAILURE;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      main
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     none
 * **************************************************************************/

int
main (int argc, char *p_argv[])
{
    int         listen_port = 0;
    char        in_fname    [128] = {0,};
    char        num_of_trans [16] = {0,};

    if (get_option (argc, p_argv, in_fname, num_of_trans)
            != E_SUCCESS)
    {
        return (-1);
    }

    if (init_process (&listen_port) != E_SUCCESS)
        return (-1);

    (void) do_job (listen_port, in_fname, num_of_trans);

    return (0);
}

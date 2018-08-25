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

extern e_error_code_t   do_job (int);


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
            "Usage : %s [-p port]\n",
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
 *  @retval     E_SUCCESS/else
 * **************************************************************************/
static
int
get_option (int         argc,
            char        **p_argv,
            int         *p_port)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             opt;
    int             flag   = 0;


    while ((opt = getopt (argc, p_argv, "p:")) != -1)
    {
        switch (opt)
        {
            case 'p' :
                *p_port = atoi(optarg);
                flag    = 1;
                break;
            case 'h':
                usage (basename (p_argv[0]));
            default :
                break;
        }
    }


    if (flag == 0)
    {
        usage (p_argv[0]);
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


    if (get_option (argc, p_argv, &listen_port) != E_SUCCESS)
    {
        return (-1);
    }

    (void) do_job (listen_port);

    return (0);
}

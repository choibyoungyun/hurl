/* ***************************************************************************
 *
 *       Filename:  signal.c
 *    Description:  signal handle
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hsignal.h>
#ifdef _HAF_DEBUG
    #include <hworker.h>
#endif

static pst_signal_handle_t  p_handle;


/* **************************************************************************
 *  @brief        process_signal
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       E_SUCCESS/E_FAILURE
 * **************************************************************************/
static
void
process_signal (int sig_num)
{
    switch (sig_num)
    {
        case SIGHUP :
            Log (DEBUG_CRITICAL,
                    "succ, received reload config signal\n");
            if (p_handle->pf_reload != NULL)
            {
                (void) (*p_handle->pf_reload)(p_handle->p_rarg);
            }
            break;
        case SIGINT  :
        case SIGTERM :
            Log (DEBUG_CRITICAL,
                    "succ, received term process signal\n");
            if (p_handle->pf_term != NULL)
            {
                (void) (*p_handle->pf_term)(p_handle->p_targ);
            }
            break;
#ifdef _HAF_DEBUG
        case SIGUSR1 :
            ((pst_process_handle_t)(p_handle->p_rarg))->standby_flag = BOOL_TRUE;
            break;
        case SIGUSR2 :
            ((pst_process_handle_t)(p_handle->p_rarg))->standby_flag = BOOL_FALSE;
            break;
#endif
        default :
            break;
    }

    return;
}




/* **************************************************************************
 *  @brief        initialize signal handler (signal loop)
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
init_signal (pst_signal_handle_t *pp_handle,
             pf_reload_t         p_func1,  void   *p_rarg,
             pf_term_t           p_func2,  void   *p_targ)
{
    e_error_code_t      e_code   = E_SUCCESS;


    try_exception ((p_handle = (pst_signal_handle_t)
                                malloc (sizeof (st_signal_handle_t)))
                    == NULL,
                    exception_init_signal);

    memset (p_handle, 0x00, sizeof (*p_handle));
    p_handle->pf_reload = p_func1;
    p_handle->p_rarg    = p_rarg;
    p_handle->pf_term   = p_func2;
    p_handle->p_targ    = p_targ;


    try_exception (Init_SignalLibThread (1, 64, 0) != E_SUCCESS,
                   exception_init_signal);

    try_exception (setSignal (SIGTERM, process_signal) != E_SUCCESS,
                   exception_set_signal);

    try_exception (setSignal (SIGINT, process_signal) != E_SUCCESS,
                   exception_set_signal);

    try_exception (setSignal (SIGHUP, process_signal) != E_SUCCESS,
                   exception_set_signal);

#ifdef _HAF_DEBUG
    try_exception (setSignal (SIGUSR1, process_signal) != E_SUCCESS,
                   exception_set_signal);

    try_exception (setSignal (SIGUSR2, process_signal) != E_SUCCESS,
                   exception_set_signal);
#endif

    (*pp_handle) = p_handle;

    try_catch (exception_init_signal)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_set_signal)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          destroy signal handler
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval         SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
destroy_signal (pst_signal_handle_t *pp_handle)
{
    if (*pp_handle != NULL)
    {
        free (*pp_handle);
    }

    *pp_handle = NULL;
    return (E_SUCCESS);
}

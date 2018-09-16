/* ***************************************************************************
 *
 *       Filename:  init.c
 *    Description:  initialize process
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

extern e_error_code_t  do_job   (pst_process_handle_t   p_handle);
extern e_error_code_t  set_client_id_eigw_handle (pst_eigw_client_id_t);


/* **************************************************************************
 *  @brief          set reload/term process flag
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     process handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
set_reload_process (pst_process_handle_t    p_handle)
{
    p_handle->reload_flag = BOOL_TRUE;

    return (E_SUCCESS);
}

static
e_error_code_t
set_term_process (pst_process_handle_t    p_handle)
{
    p_handle->stop_flag = BOOL_TRUE;

    return (E_SUCCESS);
}


/* **************************************************************************
 *  @brief          reload_process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     process handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
reload_process (pst_process_handle_t    p_handle)
{
    (p_handle->p_trace->pf_config)(p_handle->p_trace);
    (p_handle->p_worker->pf_config)(p_handle->p_worker);

    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          period_process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     PROCESS HANDLE
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
period_process (pst_process_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;


    /*  1. checking reload config option    */
    if (p_handle->reload_flag == BOOL_TRUE)
    {
        if (p_handle->pf_config != NULL)
        {
            (*p_handle->pf_config)(p_handle);
        }
        p_handle->reload_flag = BOOL_FALSE;
    }

    /*  2. checking HAF stand by option     */
    ;

    /*  3. checking HAF stand by option     */
    /* notify_haf ();                       */

    return (e_code);
}




/* **************************************************************************
 *  @brief          active process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN/OUT] PROCESS HANDLE
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
active_process (pst_process_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;

    UNUSED (p_handle);
    return (e_code);
}




/* **************************************************************************
 *  @brief          inactive process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN/OUT] PROCESS HANDLE
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
inactive_process (pst_process_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;

    UNUSED (p_handle);
    return (e_code);
}




/* **************************************************************************
 *  @brief          initialize process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN/OUT] PROCESS HANDLE
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_process (pst_process_handle_t p_handle)
{
    e_error_code_t  e_code      = E_SUCCESS;


    p_handle->stop_flag   = BOOL_FALSE;
    p_handle->reload_flag = BOOL_FALSE;
    p_handle->pf_job      = do_job;
    p_handle->pf_config   = reload_process;
    p_handle->pf_period   = period_process;
    p_handle->pf_active   = active_process;
    p_handle->pf_inactive = inactive_process;


    try_exception (confirm_only_process ((char *) PKG_BASE_VARIABLE,
                                         p_handle->pname)
                   != E_SUCCESS,
                   exception_confirm_only_process);

    try_exception ((e_code = init_logger (&p_handle->p_trace,
                                           p_handle->pname, p_handle->cfname))
                    != E_SUCCESS,
                   exception_init_process);
    if (p_handle->p_trace->pf_show)
    {
        (void)(p_handle->p_trace->pf_show)(p_handle->p_trace,
                                           p_handle->p_trace->csection);
    }


    try_exception ((e_code = init_signal (&p_handle->p_sig,
                                          (pf_reload_t) set_reload_process,
                                          (void *) p_handle,
                                          (pf_term_t) set_term_process,
                                          (void *) p_handle))
                    != E_SUCCESS,
                   exception_init_process);


    /* --------------------------------------------------------------------
     * try_exception ((e_code = init_haf (p_handle)) != E_SUCCESS,
     *            exception_init_process);
     * initialize   OAM interface
     * try_exception ((e_code = init_oam (p_handle, mod_name)) != E_SUCCESS,
     *             exception_init_process);
     * -------------------------------------------------------------------- */

    try_exception ((e_code = init_worker (&p_handle->p_worker,
                                          p_handle->cfname,
                                 (char *) WORKER_CONFIG_SECTION_NAME))
                   != E_SUCCESS,
                   exception_init_process);
    if (p_handle->p_worker->pf_show)
    {
        (void)(p_handle->p_worker->pf_show)(p_handle->p_worker,
                                            p_handle->p_worker->csection);
    }

    try_exception ((e_code = init_eigw_handle (&p_handle->p_eigw,
                                               p_handle->cfname,
                                       (char *)EIGW_CONFIG_SECTION_NAME))
                   != E_SUCCESS,
                   exception_init_process);
    set_client_id_eigw_handle (&p_handle->p_eigw->client_id);
    if (p_handle->p_eigw->pf_show)
    {
        (void)(p_handle->p_eigw->pf_show)(p_handle->p_eigw,
                                          p_handle->p_eigw->csection);
    }

    try_exception ((e_code = init_http_handle (&p_handle->p_http,
                                               p_handle->cfname,
                                        (char *) HTTP_CONFIG_SECTION_NAME,
                                               NULL))
                   != E_SUCCESS,
                   exception_init_process);
    if (p_handle->p_http->pf_show)
    {
        (void)(p_handle->p_http->pf_show)(p_handle->p_http,
                                          p_handle->p_http->csection);
    }


    try_exception ((e_code = init_auth_handle (&p_handle->p_auth,
                                               p_handle->cfname,
                                      (char *) OAUTH_CONFIG_SECTION_NAME))
                   != E_SUCCESS,
                   exception_init_process);
    if (p_handle->p_http->pf_show)
    {
        (void)(p_handle->p_auth->pf_show)(p_handle->p_auth, NULL);
    }
    p_handle->p_http->p_auth = (void *) p_handle->p_auth;


    /* -------------------------------------------------------------------
    try_exception ((e_code = init_job (p_handle)) != E_SUCCESS,
                   exception_init_process);
       ------------------------------------------------------------------ */

    Log (DEBUG_CRITICAL,
            "succ, process startup complete\n");


    try_catch (exception_confirm_only_process)
    {
        Log (DEBUG_CRITICAL,
                "fail, duplication process startup [%s]", p_handle->pname);
        e_code = E_FAILURE;
    }
    try_catch (exception_init_process)
    {
        Log (DEBUG_CRITICAL,
                "fail, process startup [%s]", p_handle->pname);
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief            destroy process resource
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]        void
 *  @retval            void
 * **************************************************************************/

e_error_code_t
destroy_process (pst_process_handle_t    p_handle)
{

    /* -------------------------------------------------------------
     * ADD Transaction Tremination API for Only MSSQL
     * ------------------------------------------------------------ */
    (void) destroy_worker          (&p_handle->p_worker);

    Log (DEBUG_CRITICAL,
            "succ, process termination complete\n");

    (void) destroy_logger          (&p_handle->p_trace);

    return (E_SUCCESS);
}

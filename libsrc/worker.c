/* ***************************************************************************
 *
 *       Filename:  worker.c
 *    Description:  worker
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
 *  @brief          show_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
void
show_worker (pst_worker_handle_t  p_handle,
             char                *p_module)
{
    char    *p_name;

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [interval:%d, delay:%d]\n",
            p_name,
            p_handle->interval,
            p_handle->delay);

    return;
}




/* **************************************************************************
 *  @brief        load worker config []
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       SUCCESS/FAILURE
 * **************************************************************************/

static
e_error_code_t
set_worker (pst_worker_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];


    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        WORKER_CONFIG_INTERVAL_NAME,
                        WORKER_CONFIG_DEFAULT_INTERVAL, buf);
    p_handle->interval = StrToi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        WORKER_CONFIG_DELAY_NAME,
                        WORKER_CONFIG_DEFAULT_DELAY, buf);
    p_handle->delay = StrToi (buf);


    if (p_handle->interval > p_handle->delay)
    {
        p_handle->interval = p_handle->delay;
    }


    try_catch (exception_not_found_file)
    {
        e_code = E_FILE_NOTFOUND;
        Log (DEBUG_CRITICAL,
                "fail, [not found file:%s]\n", p_handle->cfname);
    }
    try_finally;


    return (e_code);
}




/* **************************************************************************
 *  @brief        init_worker
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
init_worker (pst_worker_handle_t *pp_handle,
             char                *p_cfname,
             char                *p_csection)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_worker_handle_t p_handle = NULL;


    try_exception ((p_handle =
                        (pst_worker_handle_t) malloc (sizeof (*p_handle)))
                    == NULL,
                    exception_init_worker);
    memset (p_handle, 0x00, sizeof (*p_handle));

    if (p_cfname != NULL)
    {
        strcpy (p_handle->cfname, p_cfname);
        if (p_csection)
            strcpy (p_handle->csection, p_csection);
        else
            strcpy (p_handle->csection,
                    (const char *)WORKER_CONFIG_SECTION_NAME);
    }
    p_handle->pf_config = set_worker;
    p_handle->pf_show   = show_worker;


    if (p_handle->cfname[0] != 0x00)
    {
        try_exception ((e_code = set_worker (p_handle)) != E_SUCCESS,
                       exception_init_worker);
    }

    (*pp_handle) = p_handle;


    try_catch (exception_init_worker)
    {
        if (p_handle)
        {
            free (p_handle);
        }
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief        destroy_worker
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
destroy_worker (pst_worker_handle_t *pp_handle)
{
    e_error_code_t  e_code = E_SUCCESS;

    if (*pp_handle)
    {
        free (*pp_handle);
        *pp_handle = NULL;
    }

    return (e_code);
}




/* **************************************************************************
 *  @brief          run_process
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     control blocK
 *  @retval         SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
do_worker (pst_process_handle_t    p_handle)
{
    e_error_code_t  e_code       = E_SUCCESS;
    int             delay        = p_handle->p_worker->interval;
    e_bool_t        standby_flag = p_handle->standby_flag;


    while (p_handle->stop_flag != BOOL_TRUE)
    {
        /*  to do periodically ex) reconfing, haf, etc  */
        if (p_handle->pf_period)
        {
            (*p_handle->pf_period)(p_handle);
        }

        if (standby_flag != p_handle->standby_flag)
        {
            if (p_handle->standby_flag == BOOL_TRUE)
            {
                e_code = (*p_handle->pf_inactive)(p_handle);
            }
            else
            {
                e_code = (*p_handle->pf_active)(p_handle);
            }

            if (e_code == E_SUCCESS)
            {
                standby_flag = p_handle->standby_flag;
            }
        }

        if (standby_flag == BOOL_TRUE)
        {
            delay = delay + p_handle->p_worker->interval;
            if (delay > p_handle->p_worker->delay)
            {
                delay = p_handle->p_worker->delay;
            }

            Sleep_m (delay);
            continue;
        }

        try_exception ((e_code = (*p_handle->pf_job)(p_handle)) == E_FAILURE,
                        exception_process_job);

        if (e_code == E_DELAY_JOB)
        {
            delay = delay + p_handle->p_worker->interval;
            if (delay > p_handle->p_worker->delay)
            {
                delay = p_handle->p_worker->delay;
            }
        }
        else if (e_code == E_IMMEDIATE_JOB)
        {
            delay = 0;
        }
        else
        {
            delay = p_handle->p_worker->interval;
        }

        Sleep_m (delay);
    }


    try_catch (exception_process_job)
    {
    }
    try_finally;

    return (e_code);
}

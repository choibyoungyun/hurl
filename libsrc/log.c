/* ***************************************************************************
 *
 *       Filename:  hlog.c
 *    Description:  logger
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <hlog.h>


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
show_logger (pst_log_handle_t  p_handle,
             char              *p_module)
{
    char    *p_name;

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [level:%d, num_of_file:%d, size:%d]\n",
            p_name,
            p_handle->level,
            p_handle->max_num,
            p_handle->size);

    return;
}




/* **************************************************************************
 *  @brief        set_logger
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       E_SUCCESS/E_FAILURE
 * **************************************************************************/

static
e_error_code_t
set_logger (pst_log_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];


    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        LOG_CONFIG_SIZE_NAME,
                        LOG_DEFAULT_SIZE, buf);
    p_handle->size = StrToi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        LOG_CONFIG_LEVEL_NAME,
                        LOG_DEFAULT_LEVEL, buf);
    p_handle->level = StrToi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        LOG_CONFIG_FILECNT_NAME,
                        LOG_DEFAULT_FILECNT, buf);
    p_handle->max_num = StrToi (buf);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        LOG_CONFIG_DIR_NAME,
                        NULL, buf);
    if (buf[0] != 0x00)
    {
        strcpy (p_handle->dname, buf);
    }

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        LOG_CONFIG_PREFIX_NAME,
                        NULL, buf);
    if (buf[0] != 0x00)
    {
        strcpy (p_handle->prefix, buf);
    }


    /* Set_log (u_int level, u_int file_cnt, file size, const char *)  */
    Set_Log (p_handle->level, p_handle->max_num, p_handle->size);


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
 *  @brief        init_logger (wrapper: Init_Log)
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]   log control block
 *  @retval       SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
init_logger (pst_log_handle_t   *pp_handle,
             char               *p_pname,
             char               *p_cfname)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_log_handle_t    p_handle = NULL;
    char                buf [DEFAULT_STRING_BUF_LEN] = "";


    try_exception ((p_handle =
                        (pst_log_handle_t) malloc (sizeof (st_log_handle_t)))
                    == NULL,
                    exception_init_logger);

    strcpy (p_handle->cfname,   p_cfname);
    strcpy (p_handle->csection, (char *) LOG_CONFIG_SECTION_NAME);
    strcpy (p_handle->prefix, p_pname);

    p_handle->pf_config = set_logger;
    p_handle->pf_show   = show_logger;

    memset (buf, 0x00, sizeof (buf));
    try_exception (get_env_variable (PKG_BASE_VARIABLE, buf) != E_SUCCESS,
                   exception_getenv_home);
    snprintf (p_handle->dname,
              sizeof (p_handle->dname) - 1,
              "%s/log/%s", buf, p_pname);


    /* initialize   log handler         */
    memset (buf, 0x00, sizeof (buf));
    snprintf (buf, sizeof(buf) - 1,
                "%s/%s", p_handle->dname, p_handle->prefix);
    try_exception (Init_Log (p_pname, buf) != E_SUCCESS,
                   exception_init_logger);
    try_exception ((e_code = set_logger (p_handle)) != E_SUCCESS,
                   exception_set_log);

    *pp_handle = p_handle;

    try_catch (exception_getenv_home)
    {
        fprintf (stderr,
                "fail, not found env variable [%s]\n", PKG_BASE_VARIABLE);
        fflush (stderr);

        free (p_handle);
        e_code = E_FAILURE;
    }
    try_catch (exception_init_logger)
    {
        fprintf (stderr,
                "fail, Init_Log API CALL [%s, %s]\n", p_pname, buf);
        fflush (stderr);

        free (p_handle);
        e_code = E_FAILURE;
    }
    try_catch (exception_set_log)
    {
        free (p_handle);
        /* bypass  error code (E_FILE_NOTFOUND) */
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          destroy log(trace) handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN/OUT] log handle ref. pointer
 *  @retval         SUCCESS/FAILURE
 * **************************************************************************/

e_error_code_t
destroy_logger (pst_log_handle_t *pp_handle)
{
    if ((*pp_handle) != NULL)
    {
        free (*pp_handle);
    }

    *pp_handle = NULL;

    return (E_SUCCESS);
}

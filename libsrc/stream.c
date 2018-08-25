/* ***************************************************************************
 *       Filename:  stream.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/09/18 11:25:08
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hstream.h>
#include <hsocket.h>

/* **************************************************************************
 *  @brief          show_stream_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
void
show_stream_handle (pst_stream_handle_t  p_handle,
                    char              *p_module)
{
    char    *p_name;
    char    mode_name [64] = {0,};

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    if (p_handle->mode == SINGLE_STREAM_MODE) strcpy (mode_name,   "SINGLE");
    if (p_handle->mode == DOUBLE_STREAM_MODE) strcpy (mode_name,   "DOUBLE");
    if (p_handle->mode == CIRCULAR_STREAM_MODE) strcpy (mode_name, "CIRCULAR");

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [stream buffer        : %d (%s)]\n",
            p_name,
            p_handle->bucket_size, mode_name);

    return;
}




/* **************************************************************************
 *  @brief          set_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
set_stream_handle (pst_stream_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf [DEFAULT_STRING_BUF_LEN];


    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                        STREAM_CONFIG_MODE_NAME,
                        STREAM_CONFIG_MODE_DEFAULT, buf);
    if (strcmp(buf, "DOUBLE") == 0)
    {
        p_handle->mode = DOUBLE_STREAM_MODE;
    }
    else if (strcmp(buf, "CIRCULAR") == 0)
    {
        /*  unspport mode  (default: SINGLE) */
        p_handle->mode = SINGLE_STREAM_MODE;
    }
    else
    {
        p_handle->mode = SINGLE_STREAM_MODE;
    }


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         p_handle->csection,
                         STREAM_CONFIG_BUCKET_SIZE_NAME,
                         STREAM_CONFIG_BUCKET_SIZE_DEFAULT, buf);
    p_handle->bucket_size = atoi (buf);


    try_catch (exception_not_found_file)
    {
        p_handle->mode = SINGLE_STREAM_MODE;
        p_handle->bucket_size = MAX_STREAM_BUCKET_SIZE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          reset_stream_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [IN ] p_handle  - handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
reset_stream_handle (pst_stream_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;

    p_handle->last_location = 0;
    p_handle->curr_location = 0;

    p_handle->active_bucket = 0;

    return (e_code);
}




/* **************************************************************************
 *  @brief          reset_stream_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [IN ] p_handle  - handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
next_stream_handle (pst_stream_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            *p_prev_bucket = p_handle->p_buf[p_handle->active_bucket];
    char            *p_next_bucket = p_prev_bucket;
    int             next_active;


    next_active = p_handle->active_bucket;
    if (p_handle->mode == DOUBLE_STREAM_MODE)
    {
        next_active = p_handle->active_bucket ? 0 : 1;
        p_next_bucket = p_handle->p_buf[next_active];
    }

    if (p_handle->curr_location > p_handle->last_location)
    {
        memcpy (p_next_bucket,
                p_prev_bucket + p_handle->last_location,
                p_handle->curr_location - p_handle->last_location);
    }
    p_handle->curr_location
        = p_handle->curr_location - p_handle->last_location;
    p_handle->last_location = 0;
    p_handle->active_bucket = next_active;

    return (e_code);
}



/* **************************************************************************
 *  @brief          validation stream buffer
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [IN ] p_handle            - handle
 *  @param    [IN ] required_min_size   - data size of data validation
 *  @param    [IN ] pf_validate         - data validation function
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
validate_stream_handle (pst_stream_handle_t     p_handle,
                        int                     required_min_size,
                        pf_validate_function_t  pf_validate,
                        int                     *p_rbytes)
{
    e_error_code_t  e_code = E_FAILURE;
    int             received_bytes;
    char            *p_now;

    p_now          = p_handle->p_buf[p_handle->active_bucket]
                                             + p_handle->last_location;
    received_bytes = p_handle->curr_location - p_handle->last_location;

    if (required_min_size < received_bytes)
    {
        if (pf_validate)
        {
            e_code = (*pf_validate)(p_now, received_bytes, p_rbytes);
        }
        else
        {
            *p_rbytes = received_bytes;
            e_code = E_SUCCESS;
        }
    }

    return (e_code);
}




/* **************************************************************************
 *  @brief          read_stream_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [IN ] p_handle            - handle
 *  @param    [IN ] required_min_size   - data size of data validation
 *  @param    [IN ] pf_validate         - data validation function
 *  @param    [IN ] pf_read             - data reading    function
 *  @param    [IN ] p_read_arg          - read function argument
 *  @param    [OUT] p_rbytes            - read bytes
 *  @param    [IN ] pp_ptr              -
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
read_stream_handle (pst_stream_handle_t     p_handle,
                    int                     required_min_size,
                    pf_validate_function_t  pf_validate,
                    pf_read_function_t      pf_read,
                    void                    *p_read_arg,
                    int                     *p_rbytes,
                    char                    **pp_ptr)
{
    e_error_code_t  e_code = E_SUCCESS;


    e_code = validate_stream_handle (p_handle,
                                     required_min_size,
                                     pf_validate, p_rbytes);
    try_exception (e_code == E_SUCCESS,
                   exception_already_recevied_bytes);


    if (p_handle->bucket_size == p_handle->curr_location)
    {
        (void) next_stream_handle (p_handle);
    }

    e_code = (*pf_read)(p_read_arg,
                        p_handle->p_buf[p_handle->active_bucket]
                                + p_handle->curr_location,
                        p_handle->bucket_size - p_handle->curr_location,
                        p_rbytes);
    p_handle->curr_location += (*p_rbytes);
    try_exception ((e_code != E_SUCCESS) && (e_code != E_TIMEOUT),
                   exception_recv_bytes_function);
#ifdef _DEBUG
    /* --------------------------------------------------------------
     *  debuging log
     * -------------------------------------------------------------- */
    if ((*p_rbytes) > 0)
    {
        Log (DEBUG_LOW,
                "info, recv bytes from stream callback (%d, %d, %d)\n",
                (*p_rbytes),
                p_handle->last_location,
                p_handle->curr_location);
    }
#endif
    *p_rbytes = 0;
    e_code = validate_stream_handle (p_handle,
                                     required_min_size,
                                     pf_validate, p_rbytes);
    try_exception (e_code == E_SUCCESS,
                   exception_already_recevied_bytes);

    e_code = E_TIMEOUT;


    try_catch (exception_already_recevied_bytes)
    {
        Log (DEBUG_LOW,
                "info, recv bytes from stream buffer   (%d)\n",
                (*p_rbytes));

        (*pp_ptr)  = p_handle->p_buf[p_handle->active_bucket]
                                             + p_handle->last_location;
        p_handle->last_location += *p_rbytes;
        e_code = E_SUCCESS;
    }
    try_catch (exception_recv_bytes_function)
    {
        ;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          init_stream_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle  - handle
 *  @param    [IN ] p_fnmae    - config file full path
 *  @param    [IN ] p_section  - config section name
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_stream_handle (pst_stream_handle_t *pp_handle,
                    char                *p_fname,
                    char                *p_section)
{
    e_error_code_t      e_code                      = E_SUCCESS;
    pst_stream_handle_t p_handle = NULL;


    try_exception ((p_handle =
                    (pst_stream_handle_t) MALLOC (sizeof (*p_handle)))
                   == NULL,
                   exception_init_stream);
    memset (p_handle, 0x00, sizeof (*p_handle));

    strcpy (p_handle->cfname, p_fname);
    strcpy (p_handle->csection, p_section);
    p_handle->last_location = 0;
    p_handle->curr_location = 0;
    p_handle->active_bucket = 0;
    p_handle->p_buf[0]      = NULL;
    p_handle->p_buf[1]      = NULL;

    p_handle->pf_read       = read_stream_handle;
    p_handle->pf_show       = show_stream_handle;
    p_handle->pf_reset      = reset_stream_handle;

    (void) set_stream_handle(p_handle);

    try_exception ((p_handle->p_buf[0]
                    = (char *)MALLOC (p_handle->bucket_size)) == NULL,
                   exception_init_stream);

    if (p_handle->mode == DOUBLE_STREAM_MODE)
    {
        try_exception ((p_handle->p_buf[1]
                        = (char *)MALLOC (p_handle->bucket_size)) == NULL,
                       exception_init_stream);
    }
    (*pp_handle) = p_handle;


    try_catch (exception_init_stream)
    {
        Log (DEBUG_CRITICAL,
                "fail, alloc EIGW handle (stream buffer)\n");
        if (p_handle)
        {
            if (p_handle->p_buf[0])
                free (p_handle->p_buf[0]);

            if (p_handle->p_buf[0])
                free (p_handle->p_buf[0]);

            free (p_handle);
        }
        e_code = E_ALLOC_HANDLE;
    }
    try_finally;

    return (e_code);
}



/* **************************************************************************
 *  @brief          destroy_stream_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle  - handle
 *  @param    [IN ] p_fnmae    - config file full path
 *  @param    [IN ] p_section  - config section name
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
destroy_stream_handle (pst_stream_handle_t *pp_handle)
{

    if  (pp_handle && *pp_handle)
    {
        if ((*pp_handle)->p_buf[0])
        {
            FREE ((*pp_handle)->p_buf[0]);
        }

        if ((*pp_handle)->p_buf[1])
        {
            FREE ((*pp_handle)->p_buf[1]);
        }

        FREE (*pp_handle);
        *pp_handle = NULL;
    }

    return (E_SUCCESS);
}

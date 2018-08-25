/* ***************************************************************************
 *
 *       Filename:  util.c
 *    Description:  utility
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hutil.h>


/* **************************************************************************
 *  @brief        make_datetime_to_string
 *  @brief        - CONVERT TIME TICK TO STRING (FORMAT:YYYYMMDDHHMMSS)
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       E_SUCCESS/E_FAILURE
 * **************************************************************************/

e_error_code_t
make_tick_to_string  (time_t    tick,
                      char      *p_string,
                      int        length)
{
    e_error_code_t  e_code = E_SUCCESS;
    time_t          timep;
    struct  tm      tm;

    timep = tick;
    localtime_r (&timep, &tm);
    if (strftime (p_string,  (size_t) length, "%Y%m%d%H%M%S", &tm) <= 0)
    {
        e_code = E_FAILURE;
    }

    return (e_code);
}




/* **************************************************************************
 *  @brief        make_string_to_tick
 *  @brief        - CONVERT STRING (FORMAT:YYYYMMDDHHMMSS) TO TICK
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       E_SUCCESS/E_FAILURE
 * **************************************************************************/

e_error_code_t
make_string_to_tick  (char      *p_format,
                      time_t    *p_tick)
{
    e_error_code_t  e_code = E_SUCCESS;
    struct  tm      tm;
    char            buf [4]= "";


    memset (&tm, 0x00, sizeof (tm));
    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format, 4);
    tm.tm_year = atoi (buf) - 1900;

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 4, 2);
    tm.tm_mon = atoi (buf) - 1;

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 6, 2);
    tm.tm_mday = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 8, 2);
    tm.tm_hour = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 10, 2);
    tm.tm_min = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 12, 2);
    tm.tm_sec = atoi (buf);

    *p_tick = mktime (&tm);

    return (e_code);
}



/* **************************************************************************
 *  @brief        get_env_variable
 *  @brief
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]
 *  @retval       E_SUCCESS/E_FAILURE
 * **************************************************************************/

e_error_code_t
get_env_variable (const char *p_name,
                  char       *p_buf)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            *p_env = NULL;

    if ((p_env = getenv (p_name)) == NULL)
    {
        return (E_FAILURE);
    }

    strcpy (p_buf, p_env);

    return (e_code);
}




/* **************************************************************************
 *	@brief      set_stack
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] process handle
 *  @retval     void
 * **************************************************************************/

e_error_code_t
set_stack_size (int size)
{
    e_error_code_t  e_code = E_SUCCESS;
    struct rlimit   rlim;


    if (getrlimit (RLIMIT_STACK, &rlim) != 0)
    {
        fprintf (stderr,
                 "fail, getrlimit() syscall [%d:%s]\n",
                 errno, strerror(errno));
        return (E_FAILURE);
    }

    if (size == 0)
    {
        rlim.rlim_cur = rlim.rlim_max;
    }
    else
    {
        rlim.rlim_cur = size;
    }

    if (setrlimit (RLIMIT_STACK, &rlim) != 0)
    {
        fprintf (stderr,
                 "fail, setrlimit() syscall [%d:%s]\n",
                 errno, strerror(errno));
        return (E_FAILURE);
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      open_file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] process handle
 *  @retval     void
 * **************************************************************************/

FILE *
open_file (const char *p_fname, const char *p_mode)
{
    FILE    *p_fp  = NULL;

    while (1)
    {
        p_fp = fopen (p_fname, p_mode);
        if (p_fp == NULL)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
        }
        break;
    }

    return (p_fp);
}




/* **************************************************************************
 *	@brief      close_file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] process handle
 *  @retval     void
 * **************************************************************************/

e_error_code_t
close_file (FILE *p_fp)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             status = 0;

    while (1)
    {
        status = fclose (p_fp);
        if (status != 0)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
            e_code = E_FAILURE;
        }
        break;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      write_file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] process handle
 *  @retval     void
 * **************************************************************************/

e_error_code_t
write_file (const void *p_buf,
            int         size,
            FILE       *p_fp)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             status = 0;

    while (1)
    {
        status = fwrite (p_buf, size, 1, p_fp);
        if (status != 1)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
            e_code = E_FAILURE;
        }
        else
        {
            fflush (p_fp);
            e_code = E_SUCCESS;
        }
        break;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      read_file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] p_buf - buffer
 *  @param [IN] size  - buffer size
 *  @param [IN] p_fp  - FILE pointer
 *  @retval     E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
read_file (void     *p_buf,
           int      size,
           FILE     *p_fp)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             status = 0;

    while (1)
    {
        status = fread (p_buf, size, 1, p_fp);
        if (status <= 0)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }

            if ((status != 0) || ((status == 0)  && (ferror (p_fp) != 0)))
            {
                e_code = E_FAILURE;
            }
        }
        break;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      rename_file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] p_old - old file name
 *  @param [IN] p_new - new file name
 *  @retval     void
 * **************************************************************************/

e_error_code_t
rename_file (const char *p_old,
             const char *p_new)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             status;

    while (1)
    {
        status = rename (p_old, p_new);
        if (status < 0)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
            e_code = E_FAILURE;
        }
        break;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      stat_file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] p_old - old file name
 *  @param [IN] p_new - new file name
 *  @retval     void
 * **************************************************************************/

e_error_code_t
stat_file (const char  *p_fname,
           struct stat *p_st)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             status = 0;

    while (1)
    {
        status = stat (p_fname, p_st);
        if (status < 0)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
            e_code = E_FAILURE;
        }
        break;
    }

    return (e_code);
}


e_error_code_t
is_exist_file (const char  *p_fname)
{
    struct  stat    st;

    return (stat_file (p_fname, &st));
}




/* **************************************************************************
 *	@brief      make_dir
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] p_old - old file name
 *  @param [IN] p_new - new file name
 *  @retval     void
 * **************************************************************************/

e_error_code_t
make_dir (const char *p_dname)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             status;

    while (1)
    {
        status = mkdir (p_dname, S_IRWXU | S_IRWXG);
        if ((status < 0) && (errno != EEXIST))
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
            else
            {
                e_code = E_FAILURE;
            }
        }
        else
        {
            e_code = E_SUCCESS;
        }
        break;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      get_fname_from_dir
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]  p_dname     - directory name
 *  @param [IN]  p_substring - file name sub-string
 *  @param [OUT] p_substring - file name sub-string
 *  @retval      FOUND (E_SUCCESS)/else
 * **************************************************************************/

e_error_code_t
get_fname_from_dir (const char *p_dname,
                    const char *p_prefix,
                    const char *p_suffix,
                    char       *p_fname)
{
    e_error_code_t  e_code = E_SUCCESS;
    DIR             *p_dir = NULL;
    struct dirent   *p_dirs;
    struct dirent   entry;
    struct stat     st;
    char            fname [256];
    int             status = 0;


    try_exception ((p_dir = opendir (p_dname)) == NULL,
                   exception_open_dir);

    while (1)
    {
        status = readdir_r (p_dir, &entry, &p_dirs);
        if ((status != 0) || (p_dirs == NULL))
        {
            break;
        }

        if ((strcmp (entry.d_name, ".") == 0)
                || (strcmp (entry.d_name, "..") == 0))
        {
            continue;
        }

        if ((p_prefix != NULL)
                && (strstr (entry.d_name, p_prefix) == NULL))
        {
            continue;
        }

        if ((p_suffix != NULL)
                && (strstr (entry.d_name, p_suffix) == NULL))
        {
            continue;
        }

        memset (fname, 0x00, sizeof (fname));
        sprintf (fname, "%s/%s", p_dname, entry.d_name);
        try_exception (stat_file (fname, &st) != E_SUCCESS,
                      exception_stat_file);
        if (S_ISDIR(st.st_mode))
        {
            continue;
        }

        strcpy (p_fname, fname);
        try_exception (1, exception_found_file);
    }

    e_code = E_FILE_NOTFOUND;


    try_catch (exception_open_dir)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_stat_file)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_found_file)
    {
        e_code = E_SUCCESS;
    }
    try_finally;

    if (p_dir)
    {
        closedir (p_dir);
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      init_pthread
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]  p_func - thread function
 *  @param [IN]  p_arg  - thread argument
 *  @retval      FOUND (E_SUCCESS)/else
 * **************************************************************************/

e_error_code_t
init_pthread (void *(*p_func)(void *),
              void *p_arg)
{
    e_error_code_t  e_code = E_SUCCESS;
    pthread_t       tid;
    pthread_attr_t  attr;


    try_exception (pthread_attr_init (&attr) != 0, exception_pthread_attr_init);
    try_exception (pthread_attr_setdetachstate (&attr,
                                                PTHREAD_CREATE_DETACHED)
                   != 0,
                   exception_pthread_setdetachstate);

    try_exception(pthread_create (&tid, &attr,
                                  (void *(*)(void *))p_func,
                                  (void *)p_arg)
                  != 0,
                  exception_pthread_create);

    pthread_attr_destroy (&attr);

    try_catch (exception_pthread_attr_init)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_pthread_setdetachstate)
    {
        e_code = E_FAILURE;
    }
    try_catch (exception_pthread_create)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      confirm_only_process
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]  p_pname - process name
 *  @retval      FOUND (E_SUCCESS)/else
 * **************************************************************************/

e_error_code_t
confirm_only_process (char  *p_env,
                      char  *pname)
{
    e_error_code_t  e_code     = E_SUCCESS;
    char            hdir  [256] = {0,};
    char            lfile [256] = {0,};

    try_exception ((p_env == NULL) || (pname == NULL),
                   exception_invalid_arg);

    try_exception ((e_code = get_env_variable (p_env, hdir)) != E_SUCCESS,
                   exception_get_env);

    snprintf (lfile, sizeof (lfile) - 1, "%s/key/%s", hdir, pname);

    if (Lock_Process (lfile) != SUCC)
    {
        e_code = E_FAILURE;
    }

    try_catch (exception_invalid_arg)
    {
        e_code = E_INVALID_ARG;
    }
    try_catch (exception_get_env)
    {
        e_code = E_ENV_NOTFOUND;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *	@brief      is_mount
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN] p_old - old file name
 *  @param [IN] p_new - new file name
 *  @retval     void
 * **************************************************************************/

#define SYSTEM_CMD_DF   "df --output=source"

e_error_code_t
is_mount_directory (char *p_dname)
{
    e_error_code_t  e_code               = E_SUCCESS;
    char            cmd           [128]  = {0,};
    char            result_string [1024] = {0,};
    FILE            *p_fp                = NULL;

    snprintf (cmd, sizeof (cmd) - 1,
              "%s /%s 2> /dev/null", SYSTEM_CMD_DF, p_dname);
    try_exception ((p_fp = popen (cmd, "r")) == NULL,
                   exception_file_open);

    e_code = read_file ((void *)result_string,
                        sizeof (result_string),
                        p_fp);
    try_exception (e_code != E_SUCCESS, exception_file_read);

    if (result_string [0] == 0x00)
    {
        e_code = E_FAILURE;
    }

    try_catch (exception_file_open)
    {
        e_code = E_FILE_OPEN;
    }
    try_catch (exception_file_read)
    {
        e_code = E_FILE_READ;
    }
    try_finally;

    if (p_fp != NULL)
    {
        close_file (p_fp);
    }

    return (e_code);
}


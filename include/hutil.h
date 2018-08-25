/* ***************************************************************************
 *
 *       Filename:  hutil.h
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


#ifndef _HUTIL_H
#define _HUTIL_H  1

#include <hcommon.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>


/* ------------------------------------------------------------------------
 *   time API
 * ------------------------------------------------------------------------ */
e_error_code_t
make_tick_to_string (time_t tick, char *p_string, int length);
e_error_code_t
make_string_to_tick  (char  *p_format, time_t   *p_tick);


e_error_code_t
get_env_variable  (const char  *p_variable, char   *p_buf);
e_error_code_t set_stack_size   (int size);
FILE *         open_file        (const char *p_fname, const char *p_mode);
e_error_code_t close_file       (FILE *p_fp);
e_error_code_t rename_file      (const char *p_ofname, const char *p_nfname);
e_error_code_t write_file       (const void *p_buf, int size, FILE *p_fp);
e_error_code_t read_file        (void *p_buf, int size, FILE *p_fp);
e_error_code_t stat_file        (const char *p_fname, struct stat *p_sm);
e_error_code_t is_exist_file    (const char *p_fname);
e_error_code_t make_dir         (const char *p_dname);
e_error_code_t
get_fname_from_dir              (const char *p_dname,
                                 const char *p_prefix,
                                 const char *p_suffix,
                                 char       *p_fname);
e_error_code_t
init_pthread                    (void *(*p_func)(void *), void *p_arg);

e_error_code_t
confirm_only_process            (char  *p_env, char  *pname);

e_error_code_t
is_mount_directory              (char  *p_dname);

#endif

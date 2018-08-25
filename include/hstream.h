/* ***************************************************************************
 *
 *       Filename:  hstream.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/09/18 11:21:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _HSTREAM_H
#define _HSTREAM_H 1

#include <hcommon.h>
#include <hlog.h>

/* -----------------------------------------------------------------------
 *  APPLICATION STREAM BUFFER
 * ---------------------------------------------------------------------- */

#ifndef MAX_STREAM_BUFFER_SIZE
 #define MAX_STREAM_BUCKET_SIZE  (1024 * 64)
#endif

/* -----------------------------------------------------------------------
 * DEFINE CONFIG OPTION
 * ----------------------------------------------------------------------- */
#define STREAM_CONFIG_SECTION_NAME              "STREAM"
#define STREAM_CONFIG_MODE_NAME                 "STREAM_MODE"
#define STREAM_CONFIG_BUCKET_SIZE_NAME          "STREAM_BUCKET_SIZE"


#define STREAM_CONFIG_MODE_DEFAULT              "SINGLE"
#define STREAM_CONFIG_BUCKET_SIZE_DEFAULT       "65536"


typedef e_error_code_t (*pf_validate_function_t)(char *, int, int *);
typedef e_error_code_t (*pf_read_function_t)    (void *, char *, int, int *);

typedef enum _e_stream_mode_t
{
    SINGLE_STREAM_MODE    = 0,
    DOUBLE_STREAM_MODE,
    CIRCULAR_STREAM_MODE
} e_stream_mode_t;


typedef struct _st_stream_handle_t  *pst_stream_handle_t;
typedef struct _st_stream_handle_t
{
    /*  STREAM configuration file         */
    char            cfname   [FNAME_STRING_BUF_LEN];
    char            csection [FNAME_STRING_BUF_LEN];

    e_stream_mode_t mode;

    int             bucket_size;

    int             last_location;      /*  last processed location */
    int             curr_location;      /*  current recv   location */

    int             active_bucket;
    char            *p_buf [2];

    e_error_code_t  (*pf_read)  (pst_stream_handle_t,
                                 int            ,
                                 pf_validate_function_t,
                                 pf_read_function_t,
                                 void           *,
                                 int            *,
                                 char           **);
    e_error_code_t  (*pf_reset) (pst_stream_handle_t);
    void            (*pf_show)  (pst_stream_handle_t, char *);
} st_stream_handle_t;


e_error_code_t
init_stream_handle    (pst_stream_handle_t *pp_handle,
                       char                *p_fname,
                       char                *p_section);
e_error_code_t
destroy_stream_handle (pst_stream_handle_t *pp_handle);

#endif

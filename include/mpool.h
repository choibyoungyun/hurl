/* ***************************************************************************
 *
 *       Filename:  mpool.h
 *    Description:  http memeory alloc (FIFO)
 *        Version:  1.0
 *        Created:  07/10/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _MPOOL_H
#define _MPOOL_H

#include <hcommon.h>
#include <pthread.h>


#ifndef MEMORY_ALIGNED
 #define MEMORY_ALIGNED(offset,align) \
     ((offset) + (((align) - ((offset) % (align))) % (align)))
#endif

/*  ----------------------------------------------------------------------
 *  MACRO: MCHUNK ATTRIBUTE
 *  ----------------------------------------------------------------------*/
#define MCHUNK_MEM(x)       ((x)->p_mem)
#define MCHUNK_NOW_SIZE(x)  ((x)->now_size)
#define MCHUNK_MAX_SIZE(x)  ((x)->max_size)

/* ------------------------------------------------------------------------
 *  expandable memory chunk
 * ------------------------------------------------------------------------ */
typedef struct _st_mchunk_handle_t    *pst_mchunk_handle_t;
typedef struct _st_mchunk_handle_t
{
    int                 block_size;     /*  allocation unit     */
    int                 max_size;       /*  max size            */
    int                 now_size;       /*  size currently used */
    char                *p_mem;

    e_error_code_t      (*pf_add)  (pst_mchunk_handle_t, char *, int);
    e_error_code_t      (*pf_reset)(pst_mchunk_handle_t);
} st_mchunk_handle_t;

e_error_code_t
init_mchunk_handle (pst_mchunk_handle_t  *pp_handle, int block_size);
e_error_code_t
destroy_mchunk_handle (pst_mchunk_handle_t  *pp_handle);


/* ------------------------------------------------------------------------
 *  fixed size  memory block
 * ------------------------------------------------------------------------ */
typedef struct _st_mpool_mblock_t    *pst_mpool_mblock_t;
typedef struct _st_mpool_mblock_t
{
    pst_mpool_mblock_t  p_bank;     /*  bank start pointer      */

    pst_mpool_mblock_t  p_next;

    int                 index;      /*  block index             */
    e_bool_t            state;      /*  FREE or ALLOC           */
    void                *p_base;    /*  user memeory pointer    */

    void                *p_mem;
} st_mpool_mblock_t;


typedef struct _st_mpool_handle_t    *pst_mpool_handle_t;
typedef struct _st_mpool_handle_t
{
    int                 num_of_block;
    int                 size_of_block;

#ifdef _DEBUG
    int                 num_of_used;
    int                 last_used_block;
    int                 last_free_block;
#endif

    pst_mpool_mblock_t  p_head;
    pst_mpool_mblock_t  p_tail;

    pst_mpool_mblock_t  p_bank;

    e_error_code_t      (*pf_alloc)(pst_mpool_handle_t, void **);
    e_error_code_t      (*pf_free) (pst_mpool_handle_t, void *);
} st_mpool_handle_t;



e_error_code_t
init_mpool_handle    (pst_mpool_handle_t    *pp_handle,
                      int                 num_of_block,
                      int                 size_of_block,
                      e_error_code_t      (*pf_func)(void *, void *),
                      void                *p_arg);
e_error_code_t
destroy_mpool_handle (pst_mpool_handle_t  *pp_handle);

#endif

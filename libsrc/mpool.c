
/* ***************************************************************************
 *
 *       Filename:  mpool.c
 *    Description:  memory pool
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <mpool.h>


/* **************************************************************************
 *  @brief          alloc_mpool_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle       - handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
alloc_mpool_handle (pst_mpool_handle_t  p_handle,
                    void                **pp_mem)
{
    if (p_handle->p_head != NULL)
    {
        (*pp_mem) = (void *)p_handle->p_head->p_base;

        p_handle->p_head = p_handle->p_head->p_next;

        if (p_handle->p_head == NULL)
            p_handle->p_tail = NULL;

        HTTP_LOG ("alloc(U) memory pool [%p: used (%d), total (%d), addr[%p]\n",
                    p_handle,
                    p_handle->num_of_used,
                    p_handle->num_of_block,
                    *pp_mem);
#ifdef _DEBUG
        p_handle->num_of_used++;
#endif
        return (E_SUCCESS);
    }

    return (E_BUSY);
}




/* **************************************************************************
 *  @brief          free_mpool_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle       - handle
 *  @param    [IN ] num_of_block    - number of memory block
 *  @param    [IN ] size_of_block   - block size
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

static
e_error_code_t
free_mpool_handle (pst_mpool_handle_t    p_handle,
                   void                *p_mem)
{
    pst_mpool_mblock_t   p_now;

    p_now = (pst_mpool_mblock_t)((char *)p_mem
                                - offsetof (st_mpool_mblock_t, p_mem));

    if (p_handle->p_tail != NULL)
    {
        p_handle->p_tail->p_next = p_now;
    }
    else
    {
        p_handle->p_head = p_now;
    }
    p_now->p_next    = NULL;
    p_handle->p_tail = p_now;

#ifdef _DEBUG
    p_handle->num_of_used--;
#endif

    HTTP_LOG ("free (F) memory pool [%p: used (%d), total (%d), addr[%p]\n",
               p_handle,
               p_handle->num_of_used,
               p_handle->num_of_block,
               p_now->p_base);


    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          init_mpool_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle       - handle
 *  @param    [IN ] num_of_block    - number of memory block
 *  @param    [IN ] size_of_block   - block size
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_mpool_handle (pst_mpool_handle_t    *pp_handle,
                   int                 num_of_block,
                   int                 size_of_block,
                   e_error_code_t      (*pf_func)(void *, void *),
                   void                *p_arg)
{
    e_error_code_t      e_code  = E_SUCCESS;
    pst_mpool_handle_t  p_handle= NULL;
    pst_mpool_mblock_t  p_tmp   = NULL;
    int                 i       = 0;


    p_handle = (pst_mpool_handle_t) malloc (sizeof (*p_handle));
    if (p_handle == NULL)
    {
        return (E_ALLOC_HANDLE);
    }

    p_handle->num_of_block  = num_of_block;
    p_handle->size_of_block = offsetof (st_mpool_mblock_t, p_mem)
                                + size_of_block;
    p_handle->size_of_block = MEMORY_ALIGNED(p_handle->size_of_block,64);

#ifdef _DEBUG
    p_handle->num_of_used     = 0;
    p_handle->last_used_block = p_handle->num_of_block - 1;
    p_handle->last_free_block = -1;
#endif

    p_handle->p_bank     = (pst_mpool_mblock_t)
                           calloc (p_handle->size_of_block,
                                   p_handle->num_of_block);
    if (p_handle->p_bank == NULL)
    {
        free (p_handle);
        return (E_ALLOC_HANDLE);
    }

    p_handle->p_head   = p_handle->p_bank;
    for (i=0; i < p_handle->num_of_block; i++)
    {
        p_tmp = (pst_mpool_mblock_t)((char *)p_handle->p_bank + (p_handle->size_of_block * i));
        p_tmp->p_bank = p_handle->p_bank;
        p_tmp->index  = i;
        p_tmp->state  = BOOL_FALSE;
        p_tmp->p_base = &p_tmp->p_mem;

        if (pf_func)
        {
            if ((*pf_func)(p_tmp->p_base, p_arg) != E_SUCCESS)
            {
                free (p_handle->p_bank);
                free (p_handle);

                return (E_FAILURE);
            }
        }

        if (i == (p_handle->num_of_block - 1))
        {
            /*  circula pointer */
            p_tmp->p_next    = p_handle->p_bank;
            p_tmp->p_next    = NULL;
            p_handle->p_tail = p_tmp;
        }
        else
        {
            p_tmp->p_next = (pst_mpool_mblock_t)
                            ((char *)p_handle->p_bank + (p_handle->size_of_block * (i+1)));
        }
    }


    p_handle->pf_alloc = alloc_mpool_handle;
    p_handle->pf_free  = free_mpool_handle;

    (*pp_handle) = p_handle;

    return (e_code);
}




/* **************************************************************************
 *  @brief          destroy_http_mpool_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle       - handle
 *  @param    [IN ] num_of_block    - number of memory block
 *  @param    [IN ] size_of_block   - block size
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
destroy_http_mpool_handle (pst_mpool_handle_t  *pp_handle)
{
    free ((*pp_handle)->p_bank);
    free ((*pp_handle));

    return (E_SUCCESS);
}

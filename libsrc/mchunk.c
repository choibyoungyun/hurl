/* ***************************************************************************
 *
 *       Filename:  mchunk.c
 *    Description:  memory chunk
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
 *	@brief      add_mchunk_handle
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

static
e_error_code_t
add_mchunk_handle (pst_mchunk_handle_t p_mchunk,
                   char               *p_ptr,
                   int                add_size)
{
    int     new_length = 0;


    if (p_mchunk->max_size < (p_mchunk->now_size + add_size + 1))
    {
        new_length = MEMORY_ALIGNED (p_mchunk->now_size + add_size + 1,
                                    p_mchunk->block_size);

        p_mchunk->p_mem = (char *) REALLOC (p_mchunk->p_mem, new_length);
        if (p_mchunk->p_mem == NULL)
        {
            return (E_ALLOC_HANDLE);
        }
        p_mchunk->max_size = new_length;

        HTTP_LOG ("info, alloc http response chunk [%d]\n",
                  p_mchunk->max_size);
    }

    memcpy (p_mchunk->p_mem + p_mchunk->now_size,
            p_ptr,
            add_size);
    p_mchunk->now_size       += add_size;
    p_mchunk->p_mem[p_mchunk->now_size] = 0x00;

    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          init_mchunk_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle       - handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
init_mchunk_handle (pst_mchunk_handle_t  *pp_handle,
                    int                   block_size)
{
    pst_mchunk_handle_t   p_handle = NULL;


    p_handle = (pst_mchunk_handle_t) MALLOC (sizeof(*p_handle));
    if (p_handle == NULL)
    {
        return (E_ALLOC_HANDLE);
    }

    p_handle->block_size = block_size;
    if (p_handle->block_size == 0)
    {
        p_handle->block_size = getpagesize ();
    }
    p_handle->now_size   = 0;
    p_handle->max_size   = p_handle->block_size;

    p_handle->p_mem      = (char *)MALLOC (p_handle->block_size);
    if (p_handle->p_mem == NULL)
    {
        FREE (p_handle);
        return (E_ALLOC_HANDLE);
    }
    p_handle->pf_add = add_mchunk_handle;

    (*pp_handle) = p_handle;

    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          destroy_mchunk_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param    [OUT] pp_handle       - handle
 *  @retval         E_SUCCESS/else
 * **************************************************************************/

e_error_code_t
destroy_mchunk_handle (pst_mchunk_handle_t  *pp_handle)
{
    FREE ((*pp_handle)->p_mem);
    FREE ((*pp_handle));

    return (E_SUCCESS);
}


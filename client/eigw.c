/* ***************************************************************************
 *       Filename:  eigw.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/07/18 17:26:38
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <heigw.h>


/* **************************************************************************
 *  @brief          set_client_id_eigw_handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static int HALibGetAreaID      () {return 1;}
static int HALibGetNodeTypeNo  () {return 2;}
static int HALibGetNodeGroupID () {return 3;}

e_error_code_t
set_client_id_eigw_handle (pst_eigw_client_id_t p_client_id)
{
    p_client_id->system_id = (char) HALibGetAreaID();
    p_client_id->node_type = (char) HALibGetNodeTypeNo();
    p_client_id->node_id   = (char) HALibGetNodeGroupID ();

    return (E_SUCCESS);
}

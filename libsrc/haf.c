/* ------------------------------------------------------------------------
 *  fname       : haf.c
 *  version     :
 *  date        :
 *  author      :
 *  description :
 * ------------------------------------------------------------------------ */


#include <oam/haf/lib/haf_appLib.h>
#include <oam/haf/lib/haf_proc_util.h>
#include <hworker.h>


#define HAF_CONFIG_SECTION_NAME         WORKER_CONFIG_SECTION_NAME
#define HAF_CONFIG_INTERVAL_NAME        "HAF_INTERVAL"
#define HAF_KEEPALIVE_DEFAULT_INTERVAL  5

/*  static variable for active/standby setting  */
static pst_process_handle_t p_handle;
static int            keepalive_interval = HAF_KEEPALIVE_DEFAULT_INTERVAL;



static
const char*
Str_HafState (const unsigned int unType)
{
    switch (unType)
    {
        case HAF_HOST_OFF_SYSTEM :
            return "SYSTEM";
        case HAF_HOST_INIT_SYSTEM :
            return "INIT_SYSTEM";
        case HAF_HOST_STANDBY_SYSTEM :
            return "STANDBY_SYSTEM";
        case HAF_HOST_ACTIVE_SYSTEM :
            return "ACTIVE_SYSTEM";
        default :
            return "UNKNOWN_HAF_TYPE";
    }
}




/* **************************************************************************
 *  @brief          OnHafActiveStat
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         NONE
 * **************************************************************************/

static
void
OnHafActiveStat (HAF_HOST_STATUS_RPT    *pstHostStatus)
{
    char    szTitle [DEF_BUF12] = {0,};

    snprintf (szTitle, sizeof (szTitle), "[HAF][ACT  <-  STD] ");

    Log (LOG_2,
            "succ,[OnHafActiveStat] Local[%d:%s] Remote[%d:%s] \n",
             pstHostStatus->nLocalAdmnStatus,
             getHostAdmnState(pstHostStatus->nLocalAdmnStatus),
             pstHostStatus->nRemotAdmnStatus,
             getHostAdmnState(pstHostStatus->nRemotAdmnStatus));

    p_handle->standby_flag = BOOL_FALSE;

    return;
}




/* **************************************************************************
 *  @brief          OnHafStandbyStat
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         NONE
 * **************************************************************************/
static
void
OnHafStandbyStat (HAF_HOST_STATUS_RPT    *pstHostStatus)
{
    char    szTitle [DEF_BUF12] = {0,};

    snprintf (szTitle, sizeof (szTitle), "[HAF][ACT  <-  STD] ");

    Log (LOG_2,
            "succ,[OnHafStandbyStat] Local[%d:%s] Remote[%d:%s] \n",
             pstHostStatus->nLocalAdmnStatus,
             getHostAdmnState(pstHostStatus->nLocalAdmnStatus),
             pstHostStatus->nRemotAdmnStatus,
             getHostAdmnState(pstHostStatus->nRemotAdmnStatus));

    p_handle->standby_flag = BOOL_TRUE;

    return;
}




/* **************************************************************************
 *  @brief          OnHafHostStatus
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         NONE
 * **************************************************************************/
static
void
OnHafHostStatus (HAF_HOST_STATUS_RPT    *pstHostStatus)
{
    Log (LOG_2,
            "succ,[OnHafHostStatus] Local[%d:%s] Remote[%d:%s] \n",
             pstHostStatus->nLocalAdmnStatus,
             getHostAdmnState(pstHostStatus->nLocalAdmnStatus),
             pstHostStatus->nRemotAdmnStatus,
             getHostAdmnState(pstHostStatus->nRemotAdmnStatus));

    if (pstHostStatus->nLocalRedundancyStatus != HAF_HOST_ACTIVE_SYSTEM)
    {
        p_handle->standby_flag = BOOL_TRUE;
    }

    return;
}



/* **************************************************************************
 *  @brief          OnHafHangThrID
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     argc
 *  @retval         integer
 * **************************************************************************/
static
void
OnHafHangThrID (unsigned int tid)
{
    Log (DEBUG_FORCE,
            "fail, OnHangThrID  Thread-Id(%d) IS HANG UP \n", tid);

    abort ();

    return;
}




/* **************************************************************************
 *  @brief          notify keepalive to HAF
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         NONE
 * **************************************************************************/

void
notify_haf ()
{
    static int i = 0;

    HAThrAlive (keepalive_interval);

    if ( (i % keepalive_interval) == 0)
    {
        Log (DEBUG_LOW,
                "succ, notify keepalive state to HAF [%d]\n",
                keepalive_interval);
        i = 1;
    }
    i++;

    return;
}




/* **************************************************************************
 *  @brief          set notify INTERVAL
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         NONE
 * **************************************************************************/

static
e_error_code_t
set_haf (pst_process_handle_t  p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf     [DEFAULT_STRING_BUF_LEN];


    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                         HAF_CONFIG_SECTION_NAME,
                         HAF_CONFIG_INTERVAL_NAME,
                         NULL, buf);
    if (buf[0] != 0x00)
    {
        keepalive_interval = atoi (buf);
    }

    Log (DEBUG_CRITICAL,
            "succ, loading HAF... property [interval:%d]\n",
            keepalive_interval);

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
 *  @brief          init haf
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     argc
 *  @param [IN]     argv
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
init_haf (pst_process_handle_t  p_process)
{
    e_error_code_t  e_code = E_SUCCESS;
    int             ret    = 0;
    HAF_HOST_STATUS host_status;

    p_handle = p_process;
    set_haf (p_process);

    try_exception ((ret = HALibInit((char*)"1.0.0", OnHafHangThrID)) < 0,
                   exception_init_ha);

    if (HALibRegisterHostStatusCallback (OnHafHostStatus) != SUCC)
    {
        Log (DEBUG_CRITICAL,
                "fail, HALibRegisterHostStatusCallback \n");
    }

    if (HALibRegisterActiveStatCallback (OnHafActiveStat) != SUCC)
    {
        Log (DEBUG_CRITICAL,
                "fail, HALibRegisterActiveStatCallback() \n");
    }

    if (HALibRegisterStandbyStatCallback (OnHafStandbyStat) != SUCC)
    {
        Log (DEBUG_CRITICAL,
                "fail, HALibRegisterStandbyStatCallback() \n");
    }


    Memset (&host_status, 0x00, sizeof(HAF_HOST_STATUS));
    try_exception (HALibGetHostStatus (&host_status) < 0,
                   exception_get_host_status)

    Log  (DEBUG_CRITICAL,
              "succ, HAF_HOST_STATUS RedundancyState -> [%s_(%d)] \n",
              Str_HafState(host_status.nLocalRedundancyStatus),
              host_status.nLocalRedundancyStatus);

    if (host_status.nLocalRedundancyStatus != HAF_HOST_ACTIVE_SYSTEM)
    {
        p_process->standby_flag = BOOL_TRUE;
    }


    try_catch (exception_init_ha)
    {
        Log (LOG_1,"fail, HALibInit n =%d \n", ret);
        e_code = E_FAILURE;
    }
    try_catch (exception_get_host_status)
    {
        Log (LOG_2,
                "fail, Get HAF_HOST_STATUS RedundancyState -> [%s_(%d)] \n",
                Str_HafState(host_status.nLocalRedundancyStatus),
                host_status.nLocalRedundancyStatus);
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}

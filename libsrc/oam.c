/* ------------------------------------------------------------------------
 *  fname       : oam.c
 *  version     :
 *  date        :
 *  author      :
 *  description :
 * ------------------------------------------------------------------------ */

#include <app/charging/coam.h>



static
pst_process_handle_t    p_handle;


/* **************************************************************************
 *  @brief          alarm_oam
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     code
 *  @param [IN]     p_loc   :  "EIS", "IPCC", "INS"
 *  @param [IN]     p_msg   :  alarm message
 *  @param [IN]     on_off  :  alarm ON/OFF
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
alarm_oam (int  code, char *p_loc, char  *p_msg, short on_off)
{
    int             nRet     = FAIL;
    e_error_code_t  e_code   = E_SUCCESS;


    try_exception ((on_off != ON) && (on_off != OFF),
                    exception_invalid_arg_value);

    nRet = OamSend_Alarm (code, on_off, OAM_ALM_MAJ, NULL, p_loc, p_msg);
    if (nRet != SUCC)
    {
        Log (DEBUG_CRITICAL,
                "fail, send alarm to oam (%d, %s, %s, %s)=%d\n",
                code, p_loc, p_msg,
                (on_off == OFF) ? (char *)"OFF": (char *)"ON",
                nRet);
        e_code = E_FAILURE;
    }
    else
    {
        Log (DEBUG_CRITICAL,
                "succ, send alarm to oam (%d, %s, %s, %s)=%d\n",
                code, p_loc, p_msg,
                (on_off == OFF) ? (char *)"OFF": (char *)"ON",
                nRet);
    }


    try_catch (exception_invalid_arg_value)
    {
        Log (DEBUG_CRITICAL,
                "fail, send alarm to oam (%d : unknown ON/OFF)\n",
                code);
        e_code = E_INVALID_ARG;
    }
    try_finally;

    return (e_code);
}




/* **************************************************************************
 *  @brief          fault_oam
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     code
 *  @param [IN]     p_loc   :  "EIS", "IPCC", "INS"
 *  @param [IN]     p_msg   :  alarm message
 *  @param [IN]     on_off  :  alarm ON/OFF
 *  @retval         integer
 * **************************************************************************/
static
e_error_code_t
fault_oam (int  code, char *p_loc, char  *p_msg)
{
    int             nRet            = FAIL;
    e_error_code_t  e_code          = E_SUCCESS;

    nRet = OamSend_Fault (code, p_loc, p_msg);
    Log (LOG_3,
            "Send to OamSend_Fault(%d, %s, %s)=%d\n",
            code,
            p_loc, p_msg, nRet);

    return (e_code);
}




/* **************************************************************************
 *  @brief          send_fault_to_oam (direct, indirec, inquery)
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
send_direct_call_fault (void)
{
    fault_oam (OAM_INVALID_CALL_EIS_FAULT_CODE,
               (char *)OAM_EIS_LOCATION,
               (char *)OAM_INVALID_DIRECT_EIS_FAULT_MSG);

    return (E_SUCCESS);
}

e_error_code_t
send_indirect_call_fault (void)
{
    fault_oam (OAM_INVALID_CALL_EIS_FAULT_CODE,
               (char *)OAM_EIS_LOCATION,
               (char *)OAM_INVALID_INDIRECT_EIS_FAULT_MSG);

    return (E_SUCCESS);
}

e_error_code_t
send_inquery_call_fault (void)
{
    fault_oam (OAM_INVALID_CALL_EIS_FAULT_CODE,
               (char *)OAM_EIS_LOCATION,
               (char *)OAM_INVALID_INQUERY_EIS_FAULT_MSG);

    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          send_cti_database_alarm
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     code
 *  @param [IN]     p_loc   :  "EIS", "IPCC", "INS"
 *  @param [IN]     p_msg   :  alarm message
 *  @param [IN]     on_off  :  alarm ON/OFF
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
send_cti_database_alarm (short on_off)
{
    e_error_code_t e_code          = E_SUCCESS;
    static short   is_oam_notified = -1;
    static time_t  last_tick       = 0;
    char          *p_msg           = (char *)OAM_DISCONNECT_DATABASE_CTI_ON_MSG;


    if (is_oam_notified != on_off)
    {
        if (on_off == OFF)
        {
            p_msg = (char *)OAM_DISCONNECT_DATABASE_CTI_OFF_MSG;
        }

        /*  delay alarm notification    */
        if ((time(NULL) - last_tick) < OAM_ALARM_MIN_INTERVAL)
        {
            sleep (1);
        }
        e_code = alarm_oam (OAM_DISCONNECT_DATABASE_ALL_ALARM_CODE,
                            (char *)OAM_CTI_LOCATION, p_msg, on_off);
        if (e_code == E_SUCCESS)
        {
            is_oam_notified = on_off;
            last_tick = time (NULL);
        }
    }

    return (e_code);
}


e_error_code_t
send_ins_database_alarm (short on_off)
{
    e_error_code_t e_code          = E_SUCCESS;
    static short   is_oam_notified = -1;
    static time_t  last_tick       = 0;
    char          *p_msg           = (char *)OAM_DISCONNECT_DATABASE_INS_ON_MSG;


    if (is_oam_notified != on_off)
    {
        if (on_off == OFF)
        {
            p_msg = (char *)OAM_DISCONNECT_DATABASE_INS_OFF_MSG;
        }


        /*  delay alarm notification    */
        if ((time(NULL) - last_tick) < OAM_ALARM_MIN_INTERVAL)
        {
            sleep (1);
        }
        e_code = alarm_oam (OAM_DISCONNECT_DATABASE_ALL_ALARM_CODE,
                            (char *)OAM_INS_LOCATION, p_msg, on_off);
        if (e_code == E_SUCCESS)
        {
            is_oam_notified = on_off;
            last_tick = time (NULL);
        }
    }

    return (e_code);
}


e_error_code_t
send_eis_database_alarm (short on_off)
{
    e_error_code_t e_code          = E_SUCCESS;
    static short   is_oam_notified = -1;
    static time_t  last_tick       = 0;
    char          *p_msg           = (char *)OAM_DISCONNECT_DATABASE_EIS_ON_MSG;


    if (is_oam_notified != on_off)
    {
        if (on_off == OFF)
        {
            p_msg = (char *)OAM_DISCONNECT_DATABASE_EIS_OFF_MSG;
        }


        /*  delay alarm notification    */
        if ((time(NULL) - last_tick) < OAM_ALARM_MIN_INTERVAL)
        {
            sleep (1);
        }
        e_code = alarm_oam (OAM_DISCONNECT_DATABASE_ALL_ALARM_CODE,
                            (char *)OAM_EIS_LOCATION, p_msg, on_off);
        if (e_code == E_SUCCESS)
        {
            last_tick = time (NULL);
            is_oam_notified = on_off;
        }
    }

    return (e_code);
}




/* **************************************************************************
 *  @brief          send_tcdr_send_alarm
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     code
 *  @param [IN]     p_loc   :  "EIS", "IPCC", "INS"
 *  @param [IN]     p_msg   :  alarm message
 *  @param [IN]     on_off  :  alarm ON/OFF
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
send_tcdr_alarm (int code, short on_off)
{
    char    *p_msg = NULL;
    static time_t  last_tick       = 0;

    if (code == OAM_SEND_TCDR_MZN_ALARM_CODE)
    {
        p_msg = (char *)OAM_SEND_TCDR_MZN_ON_MSG;
        if (on_off != ON)
        {
            p_msg = (char *)OAM_SEND_TCDR_MZN_OFF_MSG;
        }
    }
    else if (code == OAM_RESEND_TCDR_MZN_ALARM_CODE)
    {
        p_msg = (char *)OAM_RESEND_TCDR_MZN_ON_MSG;
        if (on_off != ON)
        {
            p_msg = (char *)OAM_RESEND_TCDR_MZN_OFF_MSG;
        }
    }
    else
    {
        Log (LOG_3,
                "fail, unknown alarm code [%d]", code);
        return (E_FAILURE);
    }


    /*  delay alarm notification    */
    if ((time(NULL) - last_tick) < OAM_ALARM_MIN_INTERVAL)
    {
        sleep (1);
    }
    (void) alarm_oam (code, (char *)OAM_MZN_LOCATION, p_msg, on_off);
    last_tick = time (NULL);


    return (E_SUCCESS);
}




/* **************************************************************************
 *  @brief          send_cti_fetch_tcd_alarm
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     on_off  :  alarm ON/OFF
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
send_cti_fetch_tcd_alarm (short on_off)
{
    e_error_code_t e_code          = E_SUCCESS;
    static time_t  last_tick       = 0;
    char          *p_msg           = (char *)OAM_FETCH_TCD_CTI_ON_MSG;

    if (on_off == OFF)
    {
        p_msg = (char *)OAM_FETCH_TCD_CTI_OFF_MSG;
    }


    /*  delay alarm notification    */
    if ((time(NULL) - last_tick) <= OAM_ALARM_MIN_INTERVAL)
    {
        sleep (1);
    }
    e_code = alarm_oam (OAM_FETCH_TCD_CTI_ALARM_CODE,
                        (char *)OAM_CTI_LOCATION, p_msg, on_off);
    last_tick = time (NULL);


    return (e_code);
}



e_error_code_t
send_cti_fetch_cdr_alarm (short on_off)
{
    e_error_code_t e_code          = E_SUCCESS;
    static time_t  last_tick       = 0;
    char          *p_msg           = (char *)OAM_FETCH_CDR_CTI_ON_MSG;

    if (on_off == OFF)
    {
        p_msg = (char *)OAM_FETCH_CDR_CTI_OFF_MSG;
    }


    /*  delay alarm notification    */
    if ((time(NULL) - last_tick) <= OAM_ALARM_MIN_INTERVAL)
    {
        sleep (1);
    }
    e_code = alarm_oam (OAM_FETCH_CDR_CTI_ALARM_CODE,
                        (char *)OAM_CTI_LOCATION, p_msg, on_off);
    last_tick = time (NULL);


    return (e_code);
}




/* **************************************************************************
 *  @brief          init oam
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param [IN]     argc
 *  @param [IN]     argv
 *  @retval         integer
 * **************************************************************************/

e_error_code_t
init_oam (pst_process_handle_t  p_process, int mod_name)
{
    e_error_code_t  e_code   = E_SUCCESS;

    Log (DEBUG_LOW,"HALibGetNodeName: %s\n", HALibGetNodeName ());
    try_exception (OamInitCli (mod_name, NULL, HALibGetNodeName())
                   == FAIL,
                   exception_init_oam_client);

    p_handle = p_process;

    try_catch (exception_init_oam_client)
    {
        e_code = E_FAILURE;
    }
    try_finally;

    return (e_code);
}


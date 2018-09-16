/* ------------------------------------------------------------------------
 *  fname       : coam.h
 *  version     :
 *  date        :
 *  author      :
 *  description :
 * ------------------------------------------------------------------------ */

#ifndef _CHARING_COAM_H_
#define _CHARING_COAM_H_

#include <cmn/def.h>
#include <cmn/msg.h>
#include <oam/haf/lib/haf_appLib.h>
#include <oam/haf/lib/haf_proc_util.h>
#include <oam/lib/oam/liboam.h>
#include <oam/oam_const.h>
#include <app/charging/cworker.h>


/* -------------------------------------------------------------------------
 * OAM LOCATION DEFINITION
 * ------------------------------------------------------------------------- */
#define OAM_CTI_LOCATION                        "CTI"
#define OAM_MZN_LOCATION                        "MZN"
#define OAM_INS_LOCATION                        "INS"
#define OAM_EIS_LOCATION                        "EIS"

/* -------------------------------------------------------------------------
 * OAM ALARM MINIMUM INTERVAL
 * ------------------------------------------------------------------------- */
#define OAM_ALARM_MIN_INTERVAL                   2

/* -------------------------------------------------------------------------
 * OAM ALARM COMMON WRAPPER FUNCTION
 * ------------------------------------------------------------------------- */
e_error_code_t
alarm_oam (int  code, char *p_loc, char  *p_msg, short on_off);


/* -------------------------------------------------------------------------
 * OAM DATABASE ALARM DEFINITION
 * ------------------------------------------------------------------------- */
#define OAM_DISCONNECT_DATABASE_ALL_ALARM_CODE  4006
#define OAM_DISCONNECT_DATABASE_CTI_ON_MSG      "FAIL, DISCONNECT CTI DATABASE"
#define OAM_DISCONNECT_DATABASE_INS_ON_MSG      "FAIL, DISCONNECT INS DATABASE"
#define OAM_DISCONNECT_DATABASE_EIS_ON_MSG      "FAIL, DISCONNECT EIS DATABASE"
#define OAM_DISCONNECT_DATABASE_CTI_OFF_MSG     "SUCC, CONNECT CTI DATABASE"
#define OAM_DISCONNECT_DATABASE_INS_OFF_MSG     "SUCC, CONNECT INS DATABASE"
#define OAM_DISCONNECT_DATABASE_EIS_OFF_MSG     "SUCC, CONNECT EIS DATABASE"
e_error_code_t send_cti_database_alarm (short on_off);
e_error_code_t send_ins_database_alarm (short on_off);
e_error_code_t send_eis_database_alarm (short on_off);


/* -------------------------------------------------------------------------
 * OAM SEND TCDR ALARM  DEFINITION
 * ------------------------------------------------------------------------- */
#define OAM_SEND_TCDR_MZN_ALARM_CODE            7102
#define OAM_RESEND_TCDR_MZN_ALARM_CODE          7103
#define OAM_SEND_TCDR_MZN_ON_MSG                "FAIL, SEND   TCDR"
#define OAM_SEND_TCDR_MZN_OFF_MSG               "SUCC, SEND   TCDR"
#define OAM_RESEND_TCDR_MZN_ON_MSG              "FAIL, RESEND TCDR"
#define OAM_RESEND_TCDR_MZN_OFF_MSG             "SUCC, RESEND TCDR"
e_error_code_t send_tcdr_alarm (int code, short on_off);

/* -------------------------------------------------------------------------
 * OAM CALL FAULT DEFINITION
 * ------------------------------------------------------------------------- */
#define OAM_INVALID_CALL_EIS_FAULT_CODE         7104
#define OAM_INVALID_DIRECT_EIS_FAULT_MSG        "FAULT DIRECT CALL"
#define OAM_INVALID_INDIRECT_EIS_FAULT_MSG      "FAULT INDIRECT CALL"
#define OAM_INVALID_INQUERY_EIS_FAULT_MSG       "FAULT INQURY CALL"
e_error_code_t send_direct_call_fault  ();
e_error_code_t send_indirect_call_fault();
e_error_code_t send_inquery_call_fault ();


/* -------------------------------------------------------------------------
 * OAM FETCH TCD(Termination Call Detail) ALARM  DEFINITION
 * ------------------------------------------------------------------------- */
#define OAM_FETCH_TCD_CTI_ALARM_CODE            7105
#define OAM_FETCH_TCD_CTI_ON_MSG                "FAIL, NO TCD ENTRY IN CTI"
#define OAM_FETCH_TCD_CTI_OFF_MSG               "SUCC, FETCH TCD FROM CTI"
e_error_code_t send_cti_fetch_tcd_alarm (short on_off);


/* -------------------------------------------------------------------------
 * OAM FETCH CDR(Call Detailed Record) ALARM  DEFINITION
 * ------------------------------------------------------------------------- */
#define OAM_FETCH_CDR_CTI_ALARM_CODE            7106
#define OAM_FETCH_CDR_CTI_ON_MSG                "FAIL, NO CDR ENTRY IN CTI"
#define OAM_FETCH_CDR_CTI_OFF_MSG               "SUCC, FETCH CDR FROM CTI"
e_error_code_t send_cti_fetch_cdr_alarm (short on_off);


/* -------------------------------------------------------------------------
 * OAM PRIMITIVE FUNCTION DEFINE
 * ------------------------------------------------------------------------- */
e_error_code_t init_oam (pst_process_handle_t   p_handle, int mod_name);

#endif

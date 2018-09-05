/* ***************************************************************************
 *
 *       Filename:  hauth.h
 *
 *    Description:  HTTP OAuth Handle
 *
 *        Version:  1.0
 *        Created:  2018년 09월 03일 14시 40분 36초
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#ifndef _HAUTH_H
#define _HAUTH_H 1

#include <hcommon.h>
#include <hcurl.h>
#include <hlog.h>
#include <cmn/lib/util/json/json.h>


#ifndef OAUTH_MAX_DOMAIN_LEN
 #define OAUTH_MAX_DOMAIN_LEN           128
#endif

#ifndef OAUTH_MAX_URI_LEN
 #define OAUTH_MAX_URI_LEN              512
#endif

#ifndef OAUTH_MAX_HEADER_LEN
 #define OAUTH_MAX_HEADER_LEN           512
#endif

#ifndef OAUTH_MAX_CLIENT_ID
 #define OAUTH_MAX_CLIENT_ID_LEN        128
#endif


#ifndef OAUTH_MAX_CLIENT_SECRET_LEN
 #define OAUTH_MAX_CLIENT_SECRET_LEN    512
#endif

#ifndef OAUTH_MAX_USERNAME_LEN
 #define OAUTH_MAX_USERNAME_LEN         128
#endif

#ifndef OAUTH_MAX_PASSWD_LEN
 #define OAUTH_MAX_PASSWD_LEN           128
#endif

#ifndef OAUTH_MAX_TOKEN_LEN
 #define OAUTH_MAX_TOKEN_LEN            512
#endif

#ifndef OAUTH_MAX_SCOPE_LEN
 #define OAUTH_MAX_SCOPE_LEN            64
#endif

#ifndef OAUTH_MAX_EXPIRES_LEN
 #define OAUTH_MAX_EXPIRES_LEN          16
#endif

#ifndef OAUTH_MAX_HEADER_LEN
 #define OAUTH_MAX_HEADER_LEN           (OAUTH_AUTH_MAX_TOKEN_LEN + 128)
#endif


/* ************************************************************************
 * DEFINE OAUTH OAUTH AUTHORIXATION CONFIG FILE FIELD
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_SECTION_NAME       "OAUTH"


/* ************************************************************************
 * DEFINE OAUTH AUTHORIXATION CONFIG FILE FIELD
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_USED_FIELD                 "AUTH_USED"
#define OAUTH_CONFIG_TOKEN_FNAME_FIELD          "AUTH_TOKEN_FNAME"
#define OAUTH_CONFIG_VERIFY_TICK_FIELD          "AUTH_VERIFY_TICK"
#define OAUTH_CONFIG_DOMAIN_FIELD               "AUTH_DOMAIN"
#define OAUTH_CONFIG_URI_FIELD                  "AUTH_URI"
#define OAUTH_CONFIG_PROTOCOL_VERSION_FIELD     "AUTH_PROTOCOL_VERSION"
#define OAUTH_CONFIG_GRANT_FIELD                "AUTH_GRANT_TYPE"
#define OAUTH_CONFIG_CLIENT_ID_FIELD            "AUTH_CLIENT_ID"
#define OAUTH_CONFIG_CLIENT_SECRET_FIELD        "AUTH_CLIENT_SECRET"
#define OAUTH_CONFIG_USERNAME_FIELD             "AUTH_USERNAME"
#define OAUTH_CONFIG_PASSWD_FIELD               "AUTH_PASSWD"
#define OAUTH_CONFIG_HEADER_CONTENT_TYPE_FIELD  "AUTH_CONTENT_TYPE"
#define OAUTH_CONFIG_HEADER_ACCEPT_TYPE_FIELD   "AUTH_ACCEPT_TYPE"


/* -----------------------------------------------------------------------
 * DEFINE OAUTH VERSION
 * CURL_HTTP_VERSION_NONE               = 0
 * CURL_HTTP_VERSION_1_0                = 1
 * CURL_HTTP_VERSION_1_1                = 2
 * CURL_HTTP_VERSION_2_0                = 3
 * CURL_HTTP_VERSION_2TLS               = 4
 * CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE  = 5
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_PROTOCOL_VERSION_DEFAULT   "2"

/* -----------------------------------------------------------------------
 * DEFINE OAUTH AUTHORIXATION GRANT TYPE
 * 1  : E_AUTH_GRANT_CREDENTIALS
 * 2  : E_AUTH_GRANT_PASSWD
 * 3  : E_AUTH_GRANT_REFRESH
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_GRANT_DEFAULT              "1"

/* -----------------------------------------------------------------------
 * DEFINE OAUTH USED FLAG
 * 0  : NOT USED (FALSE)
 * 1  : USED     (TRUE)
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_USED_DEFAULT               "0"

/* -----------------------------------------------------------------------
 * DEFINE OAUTH TOKEN REPOSITORY FILENAME
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_TOKEN_FNAME_DEFAULT        "key/auth_token.dat"


/* -----------------------------------------------------------------------
 * DEFINE OAUTH TOKEN VERIFICATION TICK
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_VERIFY_TICK_DEFAULT       "5"

/* -----------------------------------------------------------------------
 * DEFINE OAUTH REQUEST HEADER
 * ----------------------------------------------------------------------- */
#define OAUTH_CONFIG_HEADER_ACCEPT_TYPE_DEFAULT     "application/json"
#define OAUTH_CONFIG_HEADER_CONTENT_TYPE_DEFAULT    "application/json"

/* -----------------------------------------------------------------------
 * DEFINE OAUTH JSON ATTRIBUTE
 * ----------------------------------------------------------------------- */
#define OAUTH_JSON_ACCESS_TOKEN                 "access_token"
#define OAUTH_JSON_REFRESH_TOKEN                "refresh_token"
#define OAUTH_JSON_TOKEN_TYPE                   "token_type"
#define OAUTH_JSON_TOKEN_SCOPE                  "scope"
#define OAUTH_JSON_TOKEN_EXPIRES_IN             "expires_in"


typedef enum _e_http_auth_grant_t
{
    ENUM_OAUTH_GRANT_CREDENTIALS = 1,
    ENUM_OAUTH_GRANT_PASSWD,
    ENUM_OAUTH_GRANT_REFRESH
} e_http_auth_grant_t;


typedef struct _st_http_auth_token_t *pst_http_auth_token_t;
typedef struct _st_http_auth_token_t
{
    char                        access_token  [OAUTH_MAX_TOKEN_LEN];
    char                        scope         [OAUTH_MAX_SCOPE_LEN];
    char                        token_type    [OAUTH_MAX_TOKEN_LEN];
    char                        refresh_token [OAUTH_MAX_TOKEN_LEN];

    int                         expires_in;
    int                         expires_tick;
} st_http_auth_token_t;


typedef struct _st_auth_handle_t *pst_auth_handle_t;
typedef struct _st_auth_handle_t
{
    char                        cfname        [FNAME_STRING_BUF_LEN];
    char                        csection      [FNAME_STRING_BUF_LEN];

    e_bool_t                    is_used;
    char                        domain        [OAUTH_MAX_DOMAIN_LEN];

    /* authorization token save file name && token */
    st_http_auth_token_t        token;
    char                        tfname        [FNAME_STRING_BUF_LEN];
    int                         verification_tick;
    char                        auth_header   [OAUTH_MAX_HEADER_LEN];


    int                         hversion;
    char                        uri           [OAUTH_MAX_URI_LEN];
    char                        content_type  [OAUTH_MAX_HEADER_LEN];
    char                        accept_type   [OAUTH_MAX_HEADER_LEN];

    e_http_auth_grant_t         grant_type;
    char                        client_id     [OAUTH_MAX_CLIENT_SECRET_LEN];
    char                        client_secret [OAUTH_MAX_CLIENT_SECRET_LEN];
    char                        username      [OAUTH_MAX_USERNAME_LEN];
    char                        passwd        [OAUTH_MAX_PASSWD_LEN];


    e_error_code_t              (*pf_set)    (pst_auth_handle_t);
    e_error_code_t              (*pf_load)   (pst_auth_handle_t);
    e_error_code_t              (*pf_save)   (pst_auth_handle_t, char *);
    e_error_code_t              (*pf_send)   (pst_auth_handle_t,
                                              pst_http_handle_t);
    e_error_code_t              (*pf_recv)   (pst_http_request_t);
    e_error_code_t              (*pf_verify) (pst_auth_handle_t);
    e_error_code_t              (*pf_clean)  (pst_auth_handle_t);
    void                        (*pf_show)   (pst_auth_handle_t, char *);
} st_auth_handle_t;


e_error_code_t
init_auth_handle    (pst_auth_handle_t *pp_handle,
                     char              *p_fname,
                     char              *p_section);

e_error_code_t
destroy_auth_handle (pst_auth_handle_t *pp_handle);

#endif

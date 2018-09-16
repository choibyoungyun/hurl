/* ***************************************************************************
 *       Filename:  auth.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/07/18 17:26:38
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/

#include <hauth.h>


/* **************************************************************************
 *  @brief          extrace access token from JSON
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
extract_token_from_json (pst_http_auth_token_t  p_token,
                         char                   *p_json)
{
    e_error_code_t  e_code = E_SUCCESS;
    JsonDocument    Auth;

    if (p_json && *p_json != 0x00)
    {
        Auth.Parse (p_json);

        strcpy (p_token->access_token,
                Auth[OAUTH_JSON_ACCESS_TOKEN].GetString());
        strcpy (p_token->refresh_token,
                Auth[OAUTH_JSON_REFRESH_TOKEN].GetString());
        strcpy (p_token->scope,
                Auth[OAUTH_JSON_TOKEN_SCOPE].GetString());
        strcpy (p_token->token_type,
                Auth[OAUTH_JSON_TOKEN_TYPE].GetString());
        p_token->expires_in =
                Auth[OAUTH_JSON_TOKEN_EXPIRES_IN].GetInt();
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      set_auth_postfield
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
set_auth_postfield (pst_auth_handle_t   p_handle)
{
    e_error_code_t      e_code = E_SUCCESS;
    e_http_auth_grant_t type = p_handle->parameter.grant_type;

    if (type == ENUM_OAUTH_GRANT_PASSWD)
    {
        if (p_handle->token.refresh_token[0] != 0x00)
        {
            type = ENUM_OAUTH_GRANT_REFRESH;
        }
    }

    switch (type)
    {
        case ENUM_OAUTH_GRANT_CREDENTIALS :
            if (p_handle->parameter.post_type == ENUM_OAUTH_POST_HEADER)
            {
                sprintf (p_handle->parameter.post_field,
                    "grant_type=client_credentials&client_id=%s&client_secret=%s",
                    p_handle->parameter.client_id,
                    p_handle->parameter.client_secret);
            }
            else
            {
                sprintf (p_handle->parameter.post_field,
                            "{ "
                            "  \"clientId\"     : \"%s\","
                            "  \"clientSecret\" : \"%s\","
                            "  \"grantType\"    : \"client_credentials\""
                            "}",
                            p_handle->parameter.client_id,
                            p_handle->parameter.client_secret);

            }
            break;
        case ENUM_OAUTH_GRANT_PASSWD:
            sprintf (p_handle->parameter.post_field,
                "grant_type=password&client_id=%s&client_secret=%s&username=%s&password=%s",
                p_handle->parameter.client_id,
                p_handle->parameter.client_secret,
                p_handle->parameter.username,
                p_handle->parameter.passwd);
            break;
        case ENUM_OAUTH_GRANT_REFRESH:
            sprintf (p_handle->parameter.post_field,
                "grant_type=refresh_token&client_id=%s&client_secret=%s&refresh_token=%s",
                p_handle->parameter.client_id,
                p_handle->parameter.client_secret,
                p_handle->token.refresh_token);
            break;
        default:
            e_code = E_FAILURE;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      clean auth token
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
clean_auth_handle (pst_auth_handle_t  p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;

    memset (p_handle->auth_header, 0x00, sizeof (p_handle->auth_header));

    return (e_code);
}




/* **************************************************************************
 *	@brief      is available authorization token
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
verify_auth_handle (pst_auth_handle_t    p_handle,
                    char                *p_domain)
{
    e_error_code_t  e_code = E_SUCCESS;


    if (p_handle->is_used != BOOL_TRUE)
    {
        return (E_SUCCESS);
    }

    /*  1. verify auth token        */
    if (((p_handle->token.expires_tick - time(NULL))
                < p_handle->verification_tick)
            || (p_handle->auth_header[0] == 0x00))
    {
        e_code = E_FAILURE;
    }

    /*  2. verify destination domain */
    if ((p_handle->domain[0] != 0x00) && p_domain)
    {
        if (strcmp (p_domain, p_handle->domain) != 0) e_code = E_FAILURE;
    }

    return (e_code);
}




/* **************************************************************************
 *	@brief      save authorization token
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
save_auth_handle (pst_auth_handle_t  p_handle,
                  char              *p_json)
{
    e_error_code_t  e_code = E_SUCCESS;
    FILE            *p_fp = NULL;


    if ((p_fp = open_file (p_handle->tfname, "w")) == NULL)
    {
        return (E_FILE_OPEN);
    }

    if (write_file (p_json, strlen (p_json), p_fp) != E_SUCCESS)
    {
        return (E_FILE_WRITE);
    }
    close_file (p_fp);

    (void) extract_token_from_json (&p_handle->token,  p_json);

    memset (p_handle->auth_header, 0x00, sizeof (p_handle->auth_header));
    snprintf (p_handle->auth_header, sizeof (p_handle->auth_header) - 1,
             "%s: %s %s",
              (char *)OAUTH_REQUEST_AUTH_HEADER_FIELD,
              p_handle->token.token_type,
              p_handle->token.access_token);

    memset (p_handle->cache_header, 0x00, sizeof (p_handle->cache_header));
    snprintf (p_handle->cache_header, sizeof (p_handle->cache_header) - 1,
             "%s: %s",
              (char *)OAUTH_REQUEST_CACHE_HEADER_FIELD,
              (char *)OAUTH_REQUEST_CACHE_HEADER_VALUE);

    return (e_code);
}




/* **************************************************************************
 *	@brief      load authorization token from file
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
load_auth_handle (pst_auth_handle_t  p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    FILE            *p_fp = NULL;
    char            json [8192]={0,};


    if ((p_fp = open_file (p_handle->tfname, "r")) == NULL)
    {
        Log (DEBUG_ERROR,
                "fail, open authorization info. from %s [%d:%s]\n",
                p_handle->tfname,
                errno,
                strerror (errno));
        return (E_FILE_OPEN);
    }

    if (read_file (json, sizeof (json) - 1, p_fp) != E_SUCCESS)
    {
        Log (DEBUG_ERROR,
                "fail, read authorization info. from %s [%d:%s]\n",
                p_handle->tfname,
                errno,
                strerror (errno));
        return (E_FILE_READ);
    }
    close_file (p_fp);


    (void) extract_token_from_json (&p_handle->token,  json);
    if (p_handle->token.access_token[0] == 0x00)
    {
        return (E_FAILURE);
    }

    memset (p_handle->auth_header, 0x00, sizeof (p_handle->auth_header));
    snprintf (p_handle->auth_header, sizeof (p_handle->auth_header) - 1,
             "%s: %s %s",
              (char *)OAUTH_REQUEST_AUTH_HEADER_FIELD,
              p_handle->token.token_type,
              p_handle->token.access_token);

    memset (p_handle->cache_header, 0x00, sizeof (p_handle->cache_header));
    snprintf (p_handle->cache_header, sizeof (p_handle->cache_header) - 1,
             "%s: %s",
              (char *)OAUTH_REQUEST_CACHE_HEADER_FIELD,
              (char *)OAUTH_REQUEST_CACHE_HEADER_VALUE);

    return (e_code);
}




/* **************************************************************************
 *	@brief      setopt_http_request
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     E_SUCCESS/fail
 * **************************************************************************/
static
e_error_code_t
setopt_auth_request (pst_auth_handle_t    p_auth,
                     pst_http_request_t   p_req)
{
    e_error_code_t      e_code    = E_SUCCESS;
    CURL                *p_handle = p_req->p_context;
    char                post_field [HTTP_MAX_HEADER_VALUE_LEN] = {0,};
    char                uri [HTTP_MAX_HEADER_LEN] = {0,};


    /* --------------------------------------------------------------------
     *  COMMON option
     * -------------------------------------------------------------------  */
    p_req->err_code = curl_easy_setopt (p_handle,
                                        CURLOPT_HTTP_VERSION,
                                        p_auth->hversion);
    try_exception (p_req->err_code != CURLE_OK,
                        exception_setopt_http_request);

    p_req->err_code = curl_easy_setopt (p_handle,
                                        CURLOPT_CONNECTTIMEOUT_MS,
                                        1000);
    try_exception (p_req->err_code != CURLE_OK,
                        exception_setopt_http_request);

    p_req->err_code = curl_easy_setopt (p_handle,
                                        CURLOPT_TIMEOUT_MS,
                                        1000);
    try_exception (p_req->err_code != CURLE_OK,
                        exception_setopt_http_request);

    /* --------------------------------------------------------------------
     *  URI option
     * -------------------------------------------------------------------  */
    (void) set_auth_postfield (p_auth);
    if (p_auth->parameter.post_type == ENUM_OAUTH_POST_HEADER)
    {
        sprintf (uri, "%s?%s",p_req->uri, post_field);
        p_req->err_code = curl_easy_setopt (p_handle,
                                            CURLOPT_URL,
                                            uri);
    }
    else
    {
        p_req->err_code = curl_easy_setopt (p_handle,
                                            CURLOPT_URL,
                                            p_req->uri);

        /* ---------------------------------------------------------------
         * NOTICE: POST_FIELD
         * The data pointed to is NOT copied by the library: as a consequence,
         * it must be preserved by the calling application
         * until the associated transfer finishes
         * --------------------------------------------------------------- */
        p_req->err_code = curl_easy_setopt (p_handle,
                                            CURLOPT_POSTFIELDS,
                                            p_auth->parameter.post_field);
        try_exception (p_req->err_code != CURLE_OK,
                                exception_setopt_http_request);
    }
    try_exception (p_req->err_code != CURLE_OK,
                        exception_setopt_http_request);

    /* --------------------------------------------------------------------
     *  METHOD option
     * -------------------------------------------------------------------  */
    try_exception (curl_easy_setopt (p_handle,
                                     CURLOPT_CUSTOMREQUEST,
                                     p_req->method)
                   != CURLE_OK,
                   exception_setopt_http_request);
    try_exception (p_req->err_code != CURLE_OK,
                    exception_setopt_http_request);


    /* --------------------------------------------------------------------
     *  HEADER option
     * -------------------------------------------------------------------  */
    if (p_auth->parameter.post_type == ENUM_OAUTH_POST_BODY)
    {
        p_req->p_header
            = curl_slist_append (p_req->p_header,
                                 p_auth->parameter.content_type);
    }
    p_req->p_header
        = curl_slist_append (p_req->p_header,
                             p_auth->accept_type);

    p_req->err_code = curl_easy_setopt (p_handle,
                                        CURLOPT_HTTPHEADER,
                                        p_req->p_header);
    try_exception (p_req->err_code != CURLE_OK,
                        exception_setopt_http_request);


    p_req->err_code = curl_easy_setopt (p_handle,
                                        CURLOPT_USERAGENT,
               (HTTP_HANDLE_FROM_REQUEST(p_req))->header.agent_name);
    try_exception (p_req->err_code != CURLE_OK,
                            exception_setopt_http_request);


    try_catch (exception_setopt_http_request)
    {
        HTTP_REQUEST_INTERNAL_ERROR (p_req);
        Log (DEBUG_ERROR,
                "fail, set http option (%d, %s)\n",
                p_req->err_code,
                p_req->err_string);

        e_code = E_PROTOCOL_HTTP_SETOPT;
    }
    try_finally;


    return (e_code);
}




/* *************************************************************************
 *  @brief       recv authorizaiton response from CURL (callback)
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param
 *  @retval     E_SUCCESS/else
 * ************************************************************************/

static
e_error_code_t
recv_auth_handle (pst_http_request_t p_http)
{
    e_error_code_t      e_code   = E_SUCCESS;
    pst_auth_handle_t   p_handle = NULL;


    Log (DEBUG_INFO, "info, [HEADER ] %s\n", MCHUNK_MEM (p_http->rsp.p_header));
    Log (DEBUG_INFO, "info, [BODY   ] %s\n", MCHUNK_MEM (p_http->rsp.p_body));

    if (p_http->rsp.status_code != HTTP_RESULT_OK)
    {
        if (p_http->err_string [0] != 0x00)
        {
            Log (DEBUG_ERROR,
                    "fail, authorizaiton token request (%s)\n",
                    p_http->err_string);
        }
        Log (DEBUG_ERROR,
                "fail, authorizaiton token request (%d, %s)\n",
                p_http->rsp.status_code,
                p_http->rsp.p_done_url);

        e_code = E_FAILURE;
    }
    else
    {
        p_handle = (pst_auth_handle_t) p_http->p_user1;
        e_code = save_auth_handle (p_handle,
                                   p_http->rsp.p_body->p_mem);
        p_handle->token.expires_tick = time (NULL) + p_handle->token.expires_in;

        Log (DEBUG_CRITICAL,
                "succ, recv authorization token [access token  : %s]\n",
                p_handle->token.access_token);
        Log (DEBUG_CRITICAL,
                "succ, recv authorization token [refresh_token : %s]\n",
                p_handle->token.refresh_token);
        Log (DEBUG_CRITICAL,
                "succ, recv authorization token [token_type    : %s]\n",
                p_handle->token.token_type);
        Log (DEBUG_CRITICAL,
                "succ, recv authorization token [scope         : %s]\n",
                p_handle->token.scope);
        Log (DEBUG_CRITICAL,
                "succ, recv authorization token [expires_in    : %d]\n",
                p_handle->token.expires_in);
    }

    return (e_code);
}




/* *************************************************************************
 *  @brief      send_request_to_http_server
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @param
 *  @retval     E_SUCCESS/else
 * ************************************************************************/
static
e_error_code_t
send_auth_handle (pst_auth_handle_t p_handle,
                  pst_http_handle_t p_http)
{
    e_error_code_t      e_code = E_SUCCESS;
    pst_http_request_t  p_uri  = NULL;


    do
    {
        e_code = init_http_request (p_http,
                                    &p_uri,
                                    NULL,
                                    recv_auth_handle);
    } while (e_code == E_BUSY);


    p_uri->p_user1 = p_handle;

    strcpy (p_uri->uri, p_handle->uri);
    strcpy (p_uri->method, "POST");

    /* set http option                  */
    if ((e_code = setopt_auth_request (p_handle, p_uri)) != E_SUCCESS)
    {
        return (e_code);
    }

    /*  send http request               */
    e_code = (p_http->pf_perform)(p_http, p_uri);
    if (e_code != E_SUCCESS)
    {
        Log (DEBUG_ERROR,
                "fail, perform http handle (%d, %s)\n",
                p_http->err_code,
                p_http->err_string);
        return (e_code);
    }


    while (p_http->still_running)
    {
        e_code = (p_http->pf_perform)(p_http, NULL);
    }

    return (e_code);
}




/* **************************************************************************
 *  @brief          display auth handle propertiy
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
void
show_auth_handle (pst_auth_handle_t p_handle,
                  char              *p_module)
{
    char    *p_name;

    p_name = p_handle->csection;
    if (p_module)
        p_name = p_module;

    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth used            : %s]\n",
            p_name,
            p_handle->is_used == BOOL_TRUE ? "TRUE" : "FALSE");
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth dest domain     : %s]\n",
            p_name,
            p_handle->domain[0] == 0x00 ? "ALL" : p_handle->domain);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth URI             : %s]\n",
            p_name,
            p_handle->uri);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth accept type     : %s]\n",
            p_name,
            p_handle->accept_type);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth post  type      : %s]\n",
            p_name,
            p_handle->parameter.post_type == ENUM_OAUTH_POST_HEADER ?
            "HEADER" : "BODY");
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth content type    : %s]\n",
            p_name,
            p_handle->parameter.content_type);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth grant type      : %d]\n",
            p_name,
            p_handle->parameter.grant_type);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth client id       : %s]\n",
            p_name,
            p_handle->parameter.client_id);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth client secret   : %s]\n",
            p_name,
            p_handle->parameter.client_secret);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth user name       : %s]\n",
            p_name,
            p_handle->parameter.username);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [auth user passwd     : %s]\n",
            p_name,
            p_handle->parameter.passwd);
    Log (DEBUG_CRITICAL,
            "succ, loading %-8s property [token file name      : %s]\n",
            p_name,
            p_handle->tfname);

    return;
}



/* **************************************************************************
 *  @brief          set http authorizaion parameter
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
set_parameter_auth_handle (pst_auth_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf  [DEFAULT_STRING_BUF_LEN] = {0,};


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_POST_FIELD,
                        OAUTH_CONFIG_POST_DEFAULT, buf);
    p_handle->parameter.post_type = (e_http_auth_post_t) atoi (buf);


    /* --------------------------------------------------------------------
     * GRAND TYPE
     * -------------------------------------------------------------------- */
    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_GRANT_FIELD,
                        OAUTH_CONFIG_GRANT_DEFAULT, buf);
    p_handle->parameter.grant_type = (e_http_auth_grant_t) atoi (buf);

    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_CLIENT_ID_FIELD,
                        NULL,
                        p_handle->parameter.client_id);

    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_CLIENT_SECRET_FIELD,
                        NULL,
                        p_handle->parameter.client_secret);

    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_USERNAME_FIELD,
                        NULL,
                        p_handle->parameter.username);

    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_PASSWD_FIELD,
                        NULL,
                        p_handle->parameter.passwd);

    /* --------------------------------------------------------------------
     * default header option (content_type)
     * -------------------------------------------------------------------- */
    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_HEADER_CONTENT_TYPE_FIELD,
                        OAUTH_CONFIG_HEADER_CONTENT_TYPE_DEFAULT,
                        buf);
    sprintf (p_handle->parameter.content_type, "Content-Type: %s",buf);


    return (e_code);
}




/* **************************************************************************
 *  @brief          set http authorizaion handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
static
e_error_code_t
set_auth_handle (pst_auth_handle_t p_handle)
{
    e_error_code_t  e_code = E_SUCCESS;
    char            buf  [DEFAULT_STRING_BUF_LEN] = {0,};
    char            base [DEFAULT_STRING_BUF_LEN] = {0,};


    try_exception (Is_FileExist (p_handle->cfname) != TRUE,
                   exception_not_found_file);


    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_USED_FIELD,
                        OAUTH_CONFIG_USED_DEFAULT, buf);
    if (atoi (buf) != 0)
    {
        p_handle->is_used = BOOL_TRUE;
    }

    /* --------------------------------------------------------------------
     * the name of the file that stores the token.
     * -------------------------------------------------------------------- */
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_TOKEN_FNAME_FIELD,
                        OAUTH_CONFIG_TOKEN_FNAME_DEFAULT,
                        p_handle->tfname);
    if (p_handle->tfname[0] != '/')
    {
        (void) get_env_variable (PKG_BASE_VARIABLE,base);
        strcpy (buf, p_handle->tfname);

        memset (p_handle->tfname, 0x00, sizeof (p_handle->tfname));
        snprintf (p_handle->tfname, sizeof (p_handle->tfname) - 1,
                "%s/%s", base, buf);
    }

    /* --------------------------------------------------------------------
     * verification tick
     * -------------------------------------------------------------------- */
    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_VERIFY_TICK_FIELD,
                        OAUTH_CONFIG_VERIFY_TICK_DEFAULT,
                        buf);
    p_handle->verification_tick = atoi (buf);

    /* --------------------------------------------------------------------
     * destination URI
     * -------------------------------------------------------------------- */
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_URI_FIELD,
                        NULL,
                        p_handle->uri);

    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_PROTOCOL_VERSION_FIELD,
                        OAUTH_CONFIG_PROTOCOL_VERSION_DEFAULT,
                        buf);
    p_handle->hversion = atoi (buf);

    /* --------------------------------------------------------------------
     * required domain
     * -------------------------------------------------------------------- */
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_DOMAIN_FIELD,
                        NULL, p_handle->domain);

    /* --------------------------------------------------------------------
     * default header option (accept_type)
     * -------------------------------------------------------------------- */
    memset (buf, 0x00, sizeof (buf));
    (void) ReadConfFile (p_handle->cfname,
                        p_handle->csection,
                        OAUTH_CONFIG_HEADER_ACCEPT_TYPE_FIELD,
                        OAUTH_CONFIG_HEADER_ACCEPT_TYPE_DEFAULT,
                        buf);
    sprintf (p_handle->accept_type, "Accept: %s",buf);


    (void) set_parameter_auth_handle (p_handle);

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
 *  @brief          init http authorizaion handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
e_error_code_t
init_auth_handle  (pst_auth_handle_t *pp_handle,
                   char              *p_fname,
                   char              *p_section)
{
    e_error_code_t    e_code = E_SUCCESS;
    pst_auth_handle_t p_handle = NULL;


    /*  1. alloc http handle                */
    if ((p_handle
                = (pst_auth_handle_t) MALLOC (sizeof (*p_handle)))
        == NULL)
    {
        Log (DEBUG_CRITICAL,
                "fail, alloc http handle \n");
        return (E_ALLOC_HANDLE);
    }

    /*  2. initialize local data & method   */
    memset (p_handle, 0x00, sizeof (*p_handle));
    strcpy (p_handle->cfname,   p_fname);
    strcpy (p_handle->csection, p_section);

    p_handle->pf_set    = set_auth_handle;
    p_handle->pf_send   = send_auth_handle;
    p_handle->pf_recv   = recv_auth_handle;
    p_handle->pf_show   = show_auth_handle;
    p_handle->pf_load   = load_auth_handle;
    p_handle->pf_save   = save_auth_handle;
    p_handle->pf_verify = verify_auth_handle;
    p_handle->pf_clean  = clean_auth_handle;

    /*  3. set handle from config file      */
    if ((e_code = (p_handle->pf_set)(p_handle)) != E_SUCCESS)
    {
        FREE (p_handle);
        return (e_code);
    }

    (*pp_handle) = p_handle;

    return (e_code);
}




/* **************************************************************************
 *  @brief          deatroy http authorizaion handle
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @retval         E_SUCCESS/else
 * **************************************************************************/
e_error_code_t
destroy_auth_handle_t  (pst_auth_handle_t *pp_handle)
{
    e_error_code_t e_code = E_SUCCESS;

    if (pp_handle && *pp_handle)
    {
        memset (*pp_handle, 0x00, sizeof (st_auth_handle_t));
        FREE (*pp_handle);
    }

    return (e_code);
}


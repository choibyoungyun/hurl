/* ****************************************************************************
 *       Filename:  test_simple.c
 *    Description:
 *        Version:  1.0
 *        Created:  06/27/18 15:14:14
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * **************************************************************************/

#include <hurl.h>



/* **************************************************************************
 *	@brief      setopt_http_request
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/
static
e_http_error_code_t
setopt_http_request (pst_http_request_t   p_req)
{
    e_http_error_code_t e_code    = E_SUCCESS;
    CURL                *p_handle = p_req->p_context;


    /*
    curl_easy_setopt (p_handle, CURLOPT_URL,
                "https://192.168.0.230:8081/restapi/env_fcgi");
    */
    curl_easy_setopt (p_handle, CURLOPT_URL,
                "https://localhost:8081/restapi/env_fcgi");

    /* please be verbose */
    /*
    curl_easy_setopt (p_handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt (p_handle, CURLOPT_DEBUGFUNCTION, NULL);
    */

    /* we use a self-signed test server, skip verification during debugging */
    curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt (p_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    return (e_code);
}



/* **************************************************************************
 *	@brief      setopt_http_request
 *	@version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

static
e_http_error_code_t
process_http_response (long rsp_code,
                       char *p_header,
                       char *p_body)
{
    UNUSED (rsp_code);
    UNUSED (p_header);
    UNUSED (p_body);
    HTTP_LOG("==> %s\n", p_header);
    /*
    HTTP_LOG("==> %ld\n", rsp_code);
    HTTP_LOG("==> %s\n", p_body);
    */

    return (E_SUCCESS);
}



int
main (int argc, char *argv[])
{
    e_http_error_code_t e_code;
    pst_http_handle_t   p_handle;
    pst_http_request_t  p_request;
    int                 i=0;
    int                 num = 1024;


    if (init_http_handle (&p_handle, HTTP_SOCKET_MODE, NULL) != E_SUCCESS)
    {
        return (-1);
    }

    if (argc > 1)
    {
        num = atoi (argv[1]);
    }

    for (i=0; i < num;i++)
    {
        p_request = NULL;
        e_code = init_http_request (p_handle,
                                    &p_request,
                                    setopt_http_request,
                                    process_http_response);
        if (e_code != E_SUCCESS)
        {
            if (e_code == E_BUSY)
            {
                (void) perform_http_handle (p_handle, p_request);
                i--;
                continue;
            }
            else
            {
                HTTP_LOG ("fail, init_http_request \n");
                break;
            }
        }

        (void) perform_http_handle (p_handle, p_request);
    }

    while (p_handle->still_running)
    {
        (void) perform_http_handle (p_handle, NULL);
    }

    (void) destory_http_handle (p_handle);

    return (0);
}

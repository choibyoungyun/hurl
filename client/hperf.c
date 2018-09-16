/* ***************************************************************************
 *
 *       Filename:  hperf.c
 *    Description:  http performance
 *        Version:  1.0
 *        Created:  08/01/18 14:05:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Y CHOI (LIVE), NONE
 *   Organization:
 *
 * ***************************************************************************/


#include <hcommon.h>
#include <hcurl.h>

static  int recv_count;


curl_socket_t
opensocket_callback (void *p_clientp,
                     curlsocktype purpose,
                     struct curl_sockaddr *p_addr)
{
    curl_socket_t   sockfd;
    sockfd = *(curl_socket_t *)p_clientp;

    UNUSED (purpose);
    UNUSED (p_addr);

    HTTP_LOG("-->connect %d\n", sockfd);

    return (sockfd);
}


/* **************************************************************************
 *  @brief      setopt_http_request
 *  @version
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


    /*
    curl_easy_setopt (p_handle, CURLOPT_OPENSOCKETFUNCTION,
                opensocket_callback);
    curl_easy_setopt (p_handle, CURLOPT_OPENSOCKETDATA,
                &fd);
                */


    return (e_code);
}



/* **************************************************************************
 *  @brief      process_http_response
 *  @version
 *  @ingroup
 *  @date
 *  @author
 *  @param
 *  @retval     int
 * **************************************************************************/

static
e_http_error_code_t
process_http_response (pst_http_request_t p_http)
{
    recv_count++;
    UNUSED (p_http);

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


    if (init_http_handle (&p_handle,
                          NULL,
                          NULL,
                          NULL) != E_SUCCESS)
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
                                    NULL,
                                    process_http_response);
        if (e_code != E_SUCCESS)
        {
            if (e_code == E_BUSY)
            {
                (void) (p_handle->pf_perform)(p_handle, p_request);
                i--;
                continue;
            }
            else
            {
                HTTP_LOG ("fail, init_http_request \n");
                break;
            }
        }

        setopt_http_request (p_request);
        (void) (p_handle->pf_perform)(p_handle, p_request);
    }

    while (p_handle->still_running)
    {
        (void) (p_handle->pf_perform)(p_handle, p_request);
    }

    (void) destory_http_handle (p_handle);

    fprintf (stdout, "info, recv response count : %d\n", recv_count);
    fflush  (stdout);

    return (0);
}

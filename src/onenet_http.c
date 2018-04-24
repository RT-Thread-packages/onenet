/*
 * File      : onenet_http.c
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-24     chenyong     first version
 */
#include <stdlib.h>
#include <string.h>
 
#include <cJSON.h>
#include <webclient.h>
 
#include <onenet.h>

#define ONENET_SEND_DATA_LEN           1024
#define ONENET_HEAD_DATA_LEN           256
#define ONENET_CON_URI_LEN             256
 
 static rt_err_t onenet_upload_data(char *send_buffer)
{
    struct webclient_session* session = NULL;
    char *header = RT_NULL, *header_ptr;
    char *buffer = send_buffer;
    char *URI = RT_NULL;
    rt_err_t result = RT_EOK;

    session = (struct webclient_session *) ONENET_CALLOC(1, sizeof(struct webclient_session));
    if (!session)
    {
        log_e("OneNet Send data failed! No memory for session structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    URI = ONENET_CALLOC(1, ONENET_CON_URI_LEN);
    if (URI == NULL)
    {
        log_e("OneNet Send data failed! No memory for URI buffer!");
        result = -RT_ENOMEM;
        goto __exit;
    }

#ifdef ONENET_USING_MQTT
    extern struct rt_onenet_info onenet_info;

    rt_snprintf(URI, ONENET_CON_URI_LEN, "http://api.heclouds.com/devices/%s/datapoints?type=3", onenet_info.device_id);
#else
    rt_snprintf(URI, ONENET_CON_URI_LEN, "http://api.heclouds.com/devices/%s/datapoints?type=3", ONENET_INFO_DEVID);
#endif

    /* connect OneNET cloud */
    result = webclient_connect(session, URI);
    if (result < 0)
    {
        log_e("OneNet Send data failed! Webclient connect URI(%s) failed!", URI);
        goto __exit;
    }

    header = (char*) ONENET_CALLOC(1, ONENET_HEAD_DATA_LEN);
    if (header == NULL)
    {
        log_e("OneNet Send data failed! No memory for header buffer!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    header_ptr = header;

    /* build header for upload */
#ifdef ONENET_USING_MQTT
    header_ptr += rt_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "api-key: %s\r\n", onenet_info.api_key);
#else
    header_ptr += rt_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "api-key: %s\r\n", ONENET_INFO_APIKEY);
#endif
    header_ptr += rt_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Length: %d\r\n", strlen(buffer));
    header_ptr += rt_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Type: application/octet-stream\r\n");

    /* send header data */
    result = webclient_send_header(session, WEBCLIENT_POST, header, header_ptr - header);
    if (result < 0)
    {
        log_e("OneNet Send data failed! Send header buffer failed return %d!", result);
        goto __exit;
    }

    /* send body data */
    webclient_write(session, (unsigned char *) buffer, strlen(buffer));
    log_d("buffer : %.*s", strlen(buffer), buffer);

    if (webclient_handle_response(session))
    {
        if (session->response != 200)
        {
            log_e("OneNet Send data failed! Handle response(%d) error!", session->response);
            result = -RT_ERROR;
            goto __exit;
        }
    }

__exit:
    if (session)
    {
        webclient_close(session);
    }
    if (URI)
    {
        ONENET_FREE(URI);
    }
    if (header)
    {
        ONENET_FREE(header);
    }
    return result;
}


static rt_err_t onenet_get_string_data(const char *name, char *str, char *out_buff)
{
    rt_err_t result = RT_EOK;
    cJSON *root = RT_NULL;
    char * msg_str = RT_NULL;

    root = cJSON_CreateObject();
    if (!root)
    {
        log_e("MQTT online push failed! cJSON create object error return NULL!");
        return -RT_ENOMEM;
    }

    cJSON_AddStringToObject(root, name, str);

    /* render a cJSON structure to buffer */
    msg_str = cJSON_PrintUnformatted(root);
    if (!msg_str)
    {
        log_e("Device online push failed! cJSON print unformatted error return NULL!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    strncpy(out_buff, msg_str, strlen(msg_str));

__exit:
    if (root)
    {
        cJSON_Delete(root);
    }
    if (msg_str)
    {
        ONENET_FREE(msg_str);
    }

    return result;
}

static rt_err_t onenet_get_digit_data(const char *name, int digit, char *out_buff)
{
    rt_err_t result = RT_EOK;
    cJSON *root = RT_NULL;
    char * msg_str = RT_NULL;

    root = cJSON_CreateObject();
    if (!root)
    {
        log_e("MQTT online push failed! cJSON create object error return NULL!");
        return -RT_ENOMEM;
    }

    cJSON_AddNumberToObject(root, name, digit);

    /* render a cJSON structure to buffer */
    msg_str = cJSON_PrintUnformatted(root);
    if (!msg_str)
    {
        log_e("Device online push failed! cJSON print unformatted error return NULL!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    strncpy(out_buff, msg_str, strlen(msg_str));

__exit:
    if (root)
    {
        cJSON_Delete(root);
    }
    if (msg_str)
    {
        ONENET_FREE(msg_str);
    }

    return result;
}

rt_err_t onenet_http_upload_digit(const char *name, int digit)
{
    char *send_buffer = RT_NULL;
    rt_err_t result = RT_EOK;

    assert(name);

    send_buffer = ONENET_CALLOC(1, ONENET_SEND_DATA_LEN);
    if (!send_buffer)
    {
        log_e("ONENET HTTP upload digit failed! No memory for send buffer!");
        return -RT_ENOMEM;
    }

    /* get JSON format data */
    result = onenet_get_digit_data(name, digit, send_buffer);
    if (result < 0)
    {
        goto __exit;
    }

    /* send data to cloud by HTTP */
    result = onenet_upload_data(send_buffer);
    if (result < 0)
    {
        goto __exit;
    }

__exit:
    if (send_buffer)
    {
        ONENET_FREE(send_buffer);
    }

    return result;
}

rt_err_t onenet_http_upload_string(const char *name, char *str)
{
    char *send_buffer = RT_NULL;
    rt_err_t result = RT_EOK;

    assert(name);
    assert(str);

    send_buffer = RT_ONENET_CALLOC(1, ONENET_SEND_DATA_LEN);
    if (!send_buffer)
    {
        log_e("ONENET HTTP upload digit failed! No memory for send buffer!");
        return -RT_ENOMEM;
    }

    /* get JSON format data */
    result = onenet_get_string_data(name, str, send_buffer);
    if (result < 0)
    {
        goto __exit;
    }

    /* send data to cloud by HTTP */
    result = onenet_upload_data(send_buffer);
    if (result < 0)
    {
        goto __exit;
    }

__exit:
    if (send_buffer)
    {
        ONENET_FREE(send_buffer);
    }

    return result;
}

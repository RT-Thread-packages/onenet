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
#include "cJSON_util.h"
#include <webclient.h>

#include <onenet.h>

#define ONENET_SEND_DATA_LEN           1024
#define ONENET_HEAD_DATA_LEN           256
#define ONENET_CON_URI_LEN             256
#define ONENET_RECV_RESP_LEN           256

static rt_err_t onenet_upload_data(char *send_buffer)
{
    struct webclient_session *session = RT_NULL;
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
    if (URI == RT_NULL)
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

    header = (char *) ONENET_CALLOC(1, ONENET_HEAD_DATA_LEN);
    if (header == RT_NULL)
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


static rt_err_t onenet_get_string_data(const char *ds_name, const char *str, char **out_buff)
{
    rt_err_t result = RT_EOK;
    cJSON *root = RT_NULL;

    root = cJSON_CreateObject();
    if (!root)
    {
        log_e("onenet publish string data failed! cJSON create object error return NULL!");
        return -RT_ENOMEM;
    }

    cJSON_AddStringToObject(root, ds_name, str);

    /* render a cJSON structure to buffer */
    *out_buff = cJSON_PrintUnformatted(root);
    if (!(*out_buff))
    {
        log_e("onenet publish string data failed! cJSON print unformatted error return NULL!");
        result = -RT_ENOMEM;
        goto __exit;
    }

__exit:
    if (root)
    {
        cJSON_Delete(root);
    }

    return result;
}

static rt_err_t onenet_get_digit_data(const char *ds_name, const double digit, char **out_buff)
{
    rt_err_t result = RT_EOK;
    cJSON *root = RT_NULL;

    root = cJSON_CreateObject();
    if (!root)
    {
        log_e("onenet publish digit data failed! cJSON create object error return NULL!");
        return -RT_ENOMEM;
    }

    cJSON_AddNumberToObject(root, ds_name, digit);

    /* render a cJSON structure to buffer */
    *out_buff = cJSON_PrintUnformatted(root);
    if (!(*out_buff))
    {
        log_e("onenet publish digit data failed! cJSON print unformatted error return NULL!");
        result = -RT_ENOMEM;
        goto __exit;
    }

__exit:
    if (root)
    {
        cJSON_Delete(root);
    }

    return result;
}

/**
 * upload digit data to OneNET cloud.
 *
 * @param   ds_name     datastream name
 * @param   digit       digit data
 *
 * @return  0 : upload data success
 *         -5 : no memory
 */
rt_err_t onenet_http_upload_digit(const char *ds_name, const double digit)
{
    char *send_buffer = RT_NULL;
    rt_err_t result = RT_EOK;

    assert(ds_name);

    /* get JSON format data */
    result = onenet_get_digit_data(ds_name, digit, &send_buffer);
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
        cJSON_free(send_buffer);
    }

    return result;
}

/**
 * upload string data to OneNET cloud.
 *
 * @param   ds_name     datastream name
 * @param   str         string data
 *
 * @return  0 : upload data success
 *         -5 : no memory
 */
rt_err_t onenet_http_upload_string(const char *ds_name, const char *str)
{
    char *send_buffer = RT_NULL;
    rt_err_t result = RT_EOK;

    assert(ds_name);
    assert(str);

    /* get JSON format data */
    result = onenet_get_string_data(ds_name, str, &send_buffer);
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
        cJSON_free(send_buffer);
    }

    return result;
}

static rt_err_t response_register_handlers(const unsigned char *rec_buf, const size_t length)
{
    cJSON *root = RT_NULL;
    cJSON *item = RT_NULL;
    cJSON *itemid = RT_NULL;
    cJSON *itemapikey = RT_NULL;

    log_d("response is %.*s\n", length, rec_buf);

    root = cJSON_Parse((char *)rec_buf);
    if (!root)
    {
        log_e("onenet register device failed! cJSON Parse data error return NULL!");
        return -RT_ENOMEM;
    }

    item = cJSON_GetObjectItem(root, "errno");
    if (item->valueint == 0)
    {
        itemid = cJSON_GetObjectItem(root->child->next, "device_id");
        itemapikey = cJSON_GetObjectItem(root->child->next, "key");

        onenet_port_save_device_info(itemid->valuestring, itemapikey->valuestring);
    }
    else
    {
        log_e("onenet register device failed! errno is %d", item->valueint);
        return -RT_ERROR;
    }

    return RT_EOK;

}

/* upload register device data to Onenet cloud */
static rt_err_t onenet_upload_register_device(char *send_buffer)
{
    struct webclient_session *session = RT_NULL;
    char *header = RT_NULL, *header_ptr;
    char *buffer = send_buffer;
    char *URI = RT_NULL;
    size_t length;
    char *rec_buf;
    rt_err_t result = RT_EOK;

    session = (struct webclient_session *) ONENET_CALLOC(1, sizeof(struct webclient_session));
    if (!session)
    {
        log_e("OneNet register device failed! No memory for session structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    URI = (char *) ONENET_CALLOC(1, ONENET_CON_URI_LEN);
    if (URI == RT_NULL)
    {
        log_e("OneNet register device failed! No memory for URI buffer!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    rec_buf = (char *) ONENET_CALLOC(1, ONENET_RECV_RESP_LEN);
    if (rec_buf == RT_NULL)
    {
        log_e("OneNet register device failed! No memory for response data buffer!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    rt_snprintf(URI, ONENET_CON_URI_LEN, "http://api.heclouds.com/register_de?register_code=");
    strcat(URI, ONENET_REGISTRATION_CODE);

    /* connect OneNET cloud */
    result = webclient_connect(session, URI);
    if (result < 0)
    {
        log_e("OneNet register device failed! Webclient connect URI(%s) failed!", URI);
        goto __exit;
    }

    header = (char *) ONENET_CALLOC(1, ONENET_HEAD_DATA_LEN);
    if (header == RT_NULL)
    {
        log_e("OneNet register device failed! No memory for header buffer!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    header_ptr = header;

    extern struct rt_onenet_info onenet_info;

    /* build header for upload */
#ifdef ONENET_USING_MQTT
    header_ptr += rt_snprintf(header_ptr,
                              WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
                              "api-key: %s\r\n", ONENET_MASTER_APIKEY);
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
        log_e("OneNet register device failed! Send header buffer failed return %d!", result);
        goto __exit;
    }

    /* send body data */
    webclient_write(session, (unsigned char *) buffer, strlen(buffer));
    log_d("buffer : %.*s", strlen(buffer), buffer);

    if (webclient_handle_response(session))
    {
        if (session->response != 200)
        {
            log_e("OneNet register device failed! Handle response(%d) error!", session->response);
            result = -RT_ERROR;
            goto __exit;
        }
        else
        {
            length = webclient_read(session, rec_buf, ONENET_RECV_RESP_LEN);
            response_register_handlers(rec_buf, length);
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
    if (rec_buf)
    {
        ONENET_FREE(rec_buf);
    }
    return result;
}

static rt_err_t onenet_get_register_device_data(const char *ds_name, const char *auth_info, char *out_buff)
{
    rt_err_t result = RT_EOK;
    cJSON *root = RT_NULL;
    char *msg_str = RT_NULL;

    root = cJSON_CreateObject();
    if (!root)
    {
        log_e("MQTT register device failed! cJSON create object error return NULL!");
        return -RT_ENOMEM;
    }

    cJSON_AddStringToObject(root, "sn", auth_info);
    cJSON_AddStringToObject(root, "title", ds_name);

    /* render a cJSON structure to buffer */
    msg_str = cJSON_PrintUnformatted(root);
    if (!msg_str)
    {
        log_e("Device register device failed! cJSON print unformatted error return NULL!");
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

/**
 * Register device to OneNET cloud.
 *
 * @param   name            device name
 * @param   auth_info       authentication information
 *
 * @return  0 : register device success
 *         -5 : no memory
 */
rt_err_t onenet_http_register_device(const char *name, const char *auth_info)
{
    char *send_buffer = RT_NULL;
    rt_err_t result = RT_EOK;

    assert(name);
    assert(auth_info);

    send_buffer = (char *) ONENET_CALLOC(1, ONENET_SEND_DATA_LEN);
    if (!send_buffer)
    {
        log_e("ONENET register device failed! No memory for send buffer!");
        return -RT_ENOMEM;
    }

    /* get JSON format data */
    result = onenet_get_register_device_data(name, auth_info, send_buffer);
    if (result < 0)
    {
        goto __exit;
    }

    /* send data to cloud by HTTP */
    result = onenet_upload_register_device(send_buffer);
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

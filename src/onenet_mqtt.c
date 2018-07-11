/*
 * File      : onenet_mqtt.c
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

#include <paho_mqtt.h>

#include <onenet.h>

static rt_bool_t init_ok = RT_FALSE;
static MQTTClient mq_client;
struct rt_onenet_info onenet_info;

struct onenet_device
{
    struct rt_onenet_info *onenet_info;

    void(*cmd_rsp_cb)(uint8_t *recv_data, rt_size_t recv_size, uint8_t **resp_data, rt_size_t *resp_size);

} onenet_mqtt;

static void mqtt_callback(MQTTClient *c, MessageData *msg_data)
{
    rt_size_t res_len = 0;
    uint8_t *response_buf = RT_NULL;

    if(onenet_mqtt.cmd_rsp_cb != RT_NULL)
    {
        onenet_mqtt.cmd_rsp_cb((uint8_t *)msg_data->message->payload, msg_data->message->payloadlen, &response_buf, &res_len);
    }
    
}

static void mqtt_connect_callback(MQTTClient *c)
{
    log_d("Enter mqtt_connect_callback!");
}

static void mqtt_online_callback(MQTTClient *c)
{
    log_d("Enter mqtt_online_callback!");
}

static void mqtt_offline_callback(MQTTClient *c)
{
    log_d("Enter mqtt_offline_callback!");
}

static rt_err_t onenet_mqtt_entry(void)
{
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;

    mq_client.uri = onenet_info.server_uri;
    memcpy(&(mq_client.condata), &condata, sizeof(condata));
    mq_client.condata.clientID.cstring = onenet_info.device_id;
    mq_client.condata.keepAliveInterval = 30;
    mq_client.condata.cleansession = 1;
    mq_client.condata.username.cstring = onenet_info.pro_id;
    mq_client.condata.password.cstring = onenet_info.auth_info;

    mq_client.buf_size = mq_client.readbuf_size = 1024 * 2;
    mq_client.buf = ONENET_CALLOC(1, mq_client.buf_size);
    mq_client.readbuf = ONENET_CALLOC(1, mq_client.readbuf_size);
    if (!(mq_client.buf && mq_client.readbuf))
    {
        log_e("No memory for MQTT client buffer!");
        return -RT_ENOMEM;
    }

    /* registered callback */
    mq_client.connect_callback = mqtt_connect_callback;
    mq_client.online_callback = mqtt_online_callback;
    mq_client.offline_callback = mqtt_offline_callback;

    /* set subscribe table. */
    mq_client.messageHandlers[0].topicFilter = ONENET_MQTT_SUBTOPIC;
    mq_client.messageHandlers[0].callback = mqtt_callback;

    mq_client.defaultMessageHandler = mqtt_callback;

    paho_mqtt_start(&mq_client);

    return RT_EOK;
}

static rt_err_t onenet_get_info(void)
{
    char dev_id[ONENET_INFO_DEVICE_LEN] = { 0 };
    char api_key[ONENET_INFO_DEVICE_LEN] = { 0 };
    char auth_info[ONENET_INFO_DEVICE_LEN] = { 0 };

    if (onenet_port_get_device_info(dev_id, api_key, auth_info))
    {
        log_e("onenet get device id fail,dev_id is %s,api_key is %s,auth_info is %s\n", dev_id, api_key, auth_info);
        return -RT_ERROR;
    }

    if (strlen(api_key) < 15)
    {
        strncpy(api_key, ONENET_MASTER_APIKEY, strlen(ONENET_MASTER_APIKEY));
    }

    strncpy(onenet_info.device_id, dev_id, strlen(dev_id));
    strncpy(onenet_info.api_key, api_key, strlen(api_key));
    strncpy(onenet_info.pro_id, ONENET_INFO_PROID, strlen(ONENET_INFO_PROID));
    strncpy(onenet_info.auth_info, auth_info, strlen(auth_info));
    strncpy(onenet_info.server_uri, ONENET_SERVER_URL, strlen(ONENET_SERVER_URL));

    return RT_EOK;
}

/**
 * onenet mqtt client init.
 *
 * @param   NULL
 *
 * @return  0 : init success
 *         -1 : get device info fail
*          -2 : onenet mqtt client init fail
 */
int onenet_mqtt_init(void)
{
    int result = 0;

    if (init_ok)
    {
        log_d("onenet mqtt already init!");
        return 0;
    }

    if (onenet_get_info() < 0)
    {
        result = -1;
        goto __exit;
    }

    onenet_mqtt.onenet_info = &onenet_info;
    onenet_mqtt.cmd_rsp_cb = RT_NULL;

    if (onenet_mqtt_entry() < 0)
    {
        result = -2;
        goto __exit;
    }

__exit:
    if (!result)
    {
        log_i("RT-Thread OneNET package(V%s) initialize success.", ONENET_SW_VERSION);
        init_ok = RT_TRUE;
    }
    else
    {
        log_e("RT-Thread OneNET package(V%s) initialize failed(%d).", ONENET_SW_VERSION, result);
    }

    return result;
}

rt_err_t onenet_mqtt_publish(const char *topic, const uint8_t *msg, int len)
{
    MQTTMessage message;
    message.qos = QOS1;
    message.retained = 0;
    message.payload = (void *) msg;
    message.payloadlen = len;

    if (MQTTPublish(&mq_client, topic, &message) < 0)
    {
        return -1;
    }

    return 0;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>

int onenet_publish(int argc, char **argv)
{
    if (argc != 3)
    {
        log_e("onenet_publish [topic] [message]    -OneNET mqtt pulish message to this topic.\n");
        return 0;
    }

    onenet_mqtt_publish(argv[1], (uint8_t *)argv[2], strlen(argv[2]));

    return 0;
}
MSH_CMD_EXPORT(onenet_mqtt_init, OneNET cloud mqtt initializate);
MSH_CMD_EXPORT_ALIAS(onenet_publish, onenet_mqtt_publish, OneNET cloud send data to subscribe topic);
#endif


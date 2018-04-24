/*
 * File      : onenet.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-23    chenyong     the first version
 */
#ifndef _ONENET_H_
#define _ONENET_H_

#include <rtthread.h>

#define ONENET_DEBUG                   1

#define ONENET_SW_VERSION              "0.1.0"

#ifndef ONENET_MALLOC
#define ONENET_MALLOC                  rt_malloc
#endif

#ifndef ONENET_CALLOC
#define ONENET_CALLOC                  rt_calloc
#endif

#ifndef ONENET_FREE
#define ONENET_FREE                    rt_free
#endif

#if ONENET_DEBUG
#ifdef assert
#undef assert
#endif
#define assert(EXPR)                                                           \
if (!(EXPR))                                                                   \
{                                                                              \
    rt_kprintf("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);        \
    while (1);                                                                 \
}

/* error level log */
#ifdef  log_e
#undef  log_e
#endif
#define log_e(...)                     rt_kprintf("\033[31;22m[E/ONENET] (%s:%d) ", __FUNCTION__, __LINE__);rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

/* info level log */
#ifdef  log_i
#undef  log_i
#endif
#define log_i(...)                     rt_kprintf("\033[36;22m[I/ONENET] ");                                rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)                     rt_kprintf("[D/ONENET] (%s:%d) ", __FUNCTION__, __LINE__);           rt_kprintf(__VA_ARGS__);rt_kprintf("\n")

#else

#ifdef assert
#undef assert
#endif
#define assert(EXPR)                   ((void)0);

/* error level log */
#ifdef  log_e
#undef  log_e
#endif
#define log_e(...)

/* info level log */
#ifdef  log_i
#undef  log_i
#endif
#define log_i(...)

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)
#endif /* ONENET_DEBUG */

#ifndef ONENET_MQTT_SUBTOPIC
#define ONENET_MQTT_SUBTOPIC           "/topic_test"
#endif

#if !defined(ONENET_INFO_DEVID) || !defined(ONENET_INFO_APIKEY) || !defined(ONENET_INFO_PROID) || !defined(ONENET_INFO_AUTH)
#define ONENET_INFO_DEVID              "29573339"
#define ONENET_INFO_APIKEY             "a2gVVf1hggZfuATkNogulHK1V=s="
#define ONENET_INFO_PROID              "131494"
#define ONENET_INFO_AUTH               "EF4016D6658466CA3E3606"
#endif 

#define ONENET_SERVER_URL              "tcp://183.230.40.39:6002"
#define ONENET_INFO_DEVID_LEN          16
#define ONENET_INFO_APIKEY_LEN         32
#define ONENET_INFO_PROID_LEN          16
#define ONENET_INFO_AUTH_LEN           64
#define ONENET_INFO_URL_LEN            32

struct rt_onenet_info
{
    char device_id[ONENET_INFO_DEVID_LEN];
    char api_key[ONENET_INFO_APIKEY_LEN];

    char pro_id[ONENET_INFO_PROID_LEN];
    char auth_info[ONENET_INFO_AUTH_LEN];

    char server_uri[ONENET_INFO_URL_LEN];

};
typedef struct rt_onenet_info *rt_onenet_info_t;

/* OneNET MQTT initialize. */
int onenet_init(void);

/* Publish MQTT data to subscribe topic. */
int onenet_mqtt_publish(const char *topic, const char *msg_str);

/* Device send data to OneNET cloud. */
rt_err_t onenet_send_digit(const char *name, int digit);
rt_err_t onenet_send_string(const char *name, char *str);

/* ========================== User port function ============================ */

/* Get MQTT data from OneNET cloud and process data. */
int onenet_port_data_process(char *recv_data, rt_size_t size);

#endif /* _ONENET_H_ */

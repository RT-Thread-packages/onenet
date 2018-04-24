/*
 * File      : onenet_sample.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-23    chenyong     the first version
 */
#include <stdlib.h>

#include <onenet.h>

static void onenet_send_data_entry(void *parameter)
{
    int value = 0;

    while (1)
    {
        value = rand() % 100;

        if (onenet_send_digit("temperature", value) < 0)
        {
            break;
        }

        rt_thread_delay(5 * 1000);
    }
}

int onenet_send_data_cycle(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("onenet_send",
            onenet_send_data_entry,
            RT_NULL,
            2 * 1024,
            RT_THREAD_PRIORITY_MAX / 3 - 1,
            5);
    if (tid)
    {
        rt_thread_startup(tid);
    }

    return 0;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT(onenet_send_data_cycle, send data to OneNET cloud cycle);
#endif

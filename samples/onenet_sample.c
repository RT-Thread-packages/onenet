/*
 * File      : onenet_sample.c
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

#include <onenet.h>

#ifdef FINSH_USING_MSH
#include <finsh.h>

static void onenet_upload_entry(void *parameter)
{
    int value = 0;

    while (1)
    {
        value = rand() % 100;

        if (onenet_http_upload_digit("temperature", value) < 0)
        {
            log_e("upload has an error, stop uploading");
            break;
        }

        rt_thread_delay(rt_tick_from_millisecond(5 * 1000));
    }
}

int onenet_upload_cycle(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("onenet_send",
            onenet_upload_entry,
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
MSH_CMD_EXPORT(onenet_upload_cycle, send data to OneNET cloud cycle);

#endif /* FINSH_USING_MSH */

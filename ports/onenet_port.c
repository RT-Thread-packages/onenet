/*
 * File      : onenet_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-20    chenyong     the first version
 */
#include <stdlib.h>

#include <onenet.h>

int onenet_port_data_process(char *recv_data, rt_size_t size)
{
	log_d("Recv data : %.*s", size, recv_data);
	
	return 0;
}

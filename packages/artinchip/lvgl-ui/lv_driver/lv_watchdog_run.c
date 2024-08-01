/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-13     RiceChen     the first version
 */
#include <rtconfig.h>
#ifdef KERNEL_RTTHREAD
#include <rtdevice.h>
#include <rtthread.h>
#include "lv_uart_run.h"
#include "lv_port_indev.h"
#include "aic_osal.h"
#include <rtthread.h>
#include <aic_core.h>
#include <aic_core.h>
#include <aic_hal.h>

#define WDT_DEVICE_NAME  "wdt"
static rt_device_t wdg_dev;
static int g_watchdog_exit=0;
static void idle_hook(void)
{
	// aicos_msleep(4);   
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
//	rt_kprintf("feed the dog!");
   
}


static void watchdog_thread_entry(void *parameter)
{
  
    while (!g_watchdog_exit)
    {
		 rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
    	//aicos_msleep(1);    
    }

	g_watchdog_exit=1;
}
int watchdog_run(void)
{
    rt_err_t ret = RT_EOK;
 
	rt_uint32_t timeout=1;
	
	wdg_dev =  rt_device_find(WDT_DEVICE_NAME);
    rt_device_init(wdg_dev);
 	ret=rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
	if(ret!=RT_EOK){
		rt_kprintf("set %s timeout failed!",WDT_DEVICE_NAME);
		return RT_ERROR;
	}

    ret= rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
	if(ret!=RT_EOK){
		rt_kprintf("start %s  failed!",WDT_DEVICE_NAME);
		return RT_ERROR;
	}
	#if 0
   rt_thread_idle_sethook(idle_hook);
	#else
	 rt_thread_t thread = rt_thread_create("watchdog", watchdog_thread_entry, RT_NULL, 1024*4, 21, 10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
	#endif
    return ret;
}

#endif

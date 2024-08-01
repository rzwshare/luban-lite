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
//#include "../components/drivers/include/drivers/touch.h"

//#include "cluster.h"

#include <rtthread.h>
#include <aic_core.h>
#include <aic_core.h>
#include <aic_hal.h>
static int g_touchkey_exit=0;

#define TOUCH_PWM  "PE.12"
unsigned int g, p;
int g_keydown_triggered =0;
int g_keyup_triggered =0;
extern bool  isLCD;

//bool isLCD=false;
void initLcd(void)
{
    unsigned int g, p;
    long pin;
	int value=0;
    pin = hal_gpio_name2pin(AIC_PANEL_ENABLE_GPIO);

    g = GPIO_GROUP(pin);
    p = GPIO_GROUP_PIN(pin);

	 hal_gpio_direction_output(g, p);
  //  hal_gpio_direction_input(g, p);
	hal_gpio_get_value(g, p, &value);
	if(value)
		isLCD=true;
	else
		isLCD=false;
	printf("isLCD:%d\r\n",isLCD);
}

bool getLCD(void)
{
	return isLCD;
}
void backlight_enable(void)
{
#ifdef AIC_PANEL_ENABLE_GPIO
    unsigned int g, p;
    long pin;

    pin = hal_gpio_name2pin(AIC_PANEL_ENABLE_GPIO);

    g = GPIO_GROUP(pin);
    p = GPIO_GROUP_PIN(pin);

    hal_gpio_direction_output(g, p);
    //hal_gpio_direction_input(g, p);
#ifndef AIC_PANEL_ENABLE_GPIO_LOW
    hal_gpio_set_output(g, p);
#else
    hal_gpio_clr_output(g, p);
#endif
isLCD=true;
#endif /* AIC_PANEL_ENABLE_GPIO */

}

void backlight_disable()
{
#ifdef AIC_PANEL_ENABLE_GPIO
    unsigned int g, p;
    long pin;

    pin = hal_gpio_name2pin(AIC_PANEL_ENABLE_GPIO);

    g = GPIO_GROUP(pin);
    p = GPIO_GROUP_PIN(pin);

#ifndef AIC_PANEL_ENABLE_GPIO_LOW
    hal_gpio_clr_output(g, p);
#else
    hal_gpio_set_output(g, p);
#endif
isLCD=false;
#endif /* AIC_PANEL_ENABLE_GPIO */

}


static void touchkey_thread_entry(void *parameter)
{
   int value=0;
   uint32_t downtick=0, lasttick=0,readtick=0,writetick=0;	
	
    while (!g_touchkey_exit)
    {
        hal_gpio_get_value(g, p, &value);
		#if 1
		if(!value)
		{
		
			downtick = rt_tick_get();
			if ((downtick - lasttick) >=0 && (downtick - lasttick)<500)
			{
				 lasttick  = downtick;
				 g_keydown_triggered=1;
			}
			else{
				 g_keydown_triggered=0;
				 lasttick  = downtick;
				}
			g_keyup_triggered=0;
		}
		else{
			if(g_keydown_triggered)
				g_keyup_triggered=1;
			
			  g_keydown_triggered=0;
			 lasttick  = downtick;
			 
			}
		
	//	printf("g_keyup_triggered:%d,isLCD:%d\r\n",g_keyup_triggered,isLCD);
		if(g_keyup_triggered)//(g_keydown_triggered)
		{
			//printf("g_keyup_triggered:%d,isLCD:%d\r\n",g_keyup_triggered,isLCD);
		
			g_keyup_triggered=0;
			if(isLCD)
				backlight_disable();
			else
				backlight_enable();
		}
		#else
		if(!value)
		{
			aicos_msleep(10);   
			 hal_gpio_get_value(g, p, &value);
			if(!value)
			{
				if(isLCD)
					backlight_disable();
				else
					backlight_enable();
			}
		}
		
		/*	while(1)
		{	
			hal_gpio_get_value(g, p, &value);
			if(value)
				break;
		}*/
		#endif
        aicos_msleep(10);    
    }

	g_touchkey_exit=1;
}
void touchkey_pin(void)
{
	
    long pin;
	int value=0;
    pin = hal_gpio_name2pin(TOUCH_PWM);

    g = GPIO_GROUP(pin);
    p = GPIO_GROUP_PIN(pin);
	hal_gpio_direction_input(g, p);
}
int touchkey_run(void)
{
    rt_err_t ret = RT_EOK;
 	touchkey_pin();
	//initLcd();
    rt_thread_t thread = rt_thread_create("touch_key", touchkey_thread_entry, RT_NULL, 1024*4, 16, 10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}

#endif

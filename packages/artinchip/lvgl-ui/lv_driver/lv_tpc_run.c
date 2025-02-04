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
#include "lv_tpc_run.h"
#include "lv_port_indev.h"
#include "aic_osal.h"
#include "../components/drivers/include/drivers/touch.h"

#define THREAD_PRIORITY   25
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5

static rt_thread_t  touch_thread = RT_NULL;
static rt_sem_t     touch_sem = RT_NULL;
static rt_device_t  dev = RT_NULL;
static struct rt_touch_data *read_data;
static struct rt_touch_info info;

#ifdef AIC_TOUCH_PANEL_GT911
static void touch_panel_gt911_read_info(void)
{
    void *id;
    id = rt_malloc(sizeof(rt_uint8_t) * 8);
    rt_device_control(dev, RT_TOUCH_CTRL_GET_ID, id);
    rt_uint8_t * read_id = (rt_uint8_t *)id;
    rt_kprintf("id = GT%d%d%d \n", read_id[0] - '0', read_id[1] - '0', read_id[2] - '0');

    /* if possible you can set your x y coordinate */
    //rt_device_control(dev, RT_TOUCH_CTRL_SET_X_RANGE, &x);
    //rt_device_control(dev, RT_TOUCH_CTRL_SET_Y_RANGE, &y);
    rt_device_control(dev, RT_TOUCH_CTRL_GET_INFO, id);
    rt_kprintf("range_x = %d \n", (*(struct rt_touch_info*)id).range_x);
    rt_kprintf("range_y = %d \n", (*(struct rt_touch_info*)id).range_y);
    rt_kprintf("point_num = %d \n", (*(struct rt_touch_info*)id).point_num);
    rt_free(id);
}
#endif

static void touch_entry(void *parameter) /* touch panel control entry */
{
    rt_device_control(dev, RT_TOUCH_CTRL_GET_INFO, &info);
#ifdef AIC_USING_RTP /* by default, only one point is supported */
    info.point_num = 1;
    rt_device_control(dev, RT_TOUCH_CTRL_PDEB_VALID_CHECK, RT_NULL);
#endif

    read_data = (struct rt_touch_data *)rt_malloc(sizeof(struct rt_touch_data) * info.point_num);

    while (1)
    {
        rt_sem_take(touch_sem, RT_WAITING_FOREVER);
        int num = rt_device_read(dev, 0, read_data, info.point_num);
        if (num == info.point_num)
        {
            for (rt_uint8_t i = 0; i < info.point_num; i++)
            {
                if (read_data[i].event == RT_TOUCH_EVENT_DOWN
                        || read_data[i].event == RT_TOUCH_EVENT_UP
                        || read_data[i].event == RT_TOUCH_EVENT_MOVE)
                {
                    rt_uint16_t  u16X, u16Y;
#ifdef AIC_USING_RTP
                    extern void lv_convert_adc_to_coord(struct rt_touch_data *data);
                    lv_convert_adc_to_coord(&read_data[i]);
#endif
                    u16X = read_data[i].x_coordinate;
                    u16Y = read_data[i].y_coordinate;

                    //rt_kprintf("[%d] %d %d\n", read_data[i].event, u16X, u16Y);
                    aic_touch_inputevent_cb(u16X, u16Y, read_data[i].event);
                }
            }
        }
        //aicos_msleep(10);
        rt_device_control(dev, RT_TOUCH_CTRL_ENABLE_INT, RT_NULL);
    }
}

static rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
#ifdef AIC_PM_POWER_TOUCH_WAKEUP
    rt_uint8_t sleep_mode;
#endif
    rt_sem_release(touch_sem);
    rt_device_control(dev, RT_TOUCH_CTRL_DISABLE_INT, RT_NULL);
#ifdef AIC_PM_POWER_TOUCH_WAKEUP
    sleep_mode = rt_pm_get_sleep_mode();
    if (sleep_mode != PM_SLEEP_MODE_NONE && !wakeup_triggered)
    {
        rt_pm_module_request(PM_POWER_ID, PM_SLEEP_MODE_NONE);
        wakeup_triggered = 1;
    }
    /* touch timer restart */
    rt_timer_start(touch_timer);
#endif
    return 0;
}

int tpc_run(const char *name, rt_uint16_t x, rt_uint16_t y)
{
    dev = rt_device_find(name);
    if (dev == RT_NULL)
    {
        rt_kprintf("can't find device:%s\n", name);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("open device failed!");
        return -1;
    }

#ifdef AIC_USING_RTP
    extern void lv_rtp_calibrate(rt_device_t rtp_dev, int fb_width, int fb_height);
    lv_rtp_calibrate(dev, x, y);
#endif

#ifdef AIC_TOUCH_PANEL_GT911
    touch_panel_gt911_read_info();
#endif
    rt_device_set_rx_indicate(dev, rx_callback);
    touch_sem = rt_sem_create("touch_sem", 0, RT_IPC_FLAG_FIFO);
    if (touch_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }

    touch_thread = rt_thread_create("touch",
                                     touch_entry,
                                     RT_NULL,
                                     THREAD_STACK_SIZE,
                                     THREAD_PRIORITY,
                                     THREAD_TIMESLICE);

    if (touch_thread != RT_NULL)
        rt_thread_startup(touch_thread);

    return 0;
}
#endif

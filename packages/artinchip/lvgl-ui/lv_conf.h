/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-18     Meco Man      First version
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <rtconfig.h>

#define  LV_SUPPORT_SET_IMAGE_STRIDE 1
// using LV_AIC_COLOR_SCREEN_TRANSP instead of LV_COLOR_SCREEN_TRANSP
#define LV_AIC_COLOR_SCREEN_TRANSP 1

#define LV_USE_LOG 0
#if LV_USE_LOG

    /*How important log should be added:
    *LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
    *LV_LOG_LEVEL_INFO        Log important events
    *LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
    *LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
    *LV_LOG_LEVEL_USER        Only logs added by the user
    *LV_LOG_LEVEL_NONE        Do not log anything*/
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /*1: Print the log with 'printf';
    *0: User need to register a callback with `lv_log_register_print_cb()`*/
    #define LV_LOG_PRINTF 1

    /*Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs*/
    #define LV_LOG_TRACE_MEM        1
    #define LV_LOG_TRACE_TIMER      1
    #define LV_LOG_TRACE_INDEV      1
    #define LV_LOG_TRACE_DISP_REFR  1
    #define LV_LOG_TRACE_EVENT      1
    #define LV_LOG_TRACE_OBJ_CREATE 1
    #define LV_LOG_TRACE_LAYOUT     1
    #define LV_LOG_TRACE_ANIM       1

#endif  /*LV_USE_LOG*/

#ifndef LV_COLOR_DEPTH
#define LV_COLOR_DEPTH          32
#endif

#define LV_HOR_RES_MAX          BSP_LCD_WIDTH
#define LV_VER_RES_MAX          BSP_LCD_HEIGHT
#define LV_IMG_CACHE_DEF_SIZE 1
#define LV_USE_MEM_MONITOR 0
#define LV_INDEV_DEF_READ_PERIOD 10

#if defined(KERNEL_BAREMETAL)
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC   malloc
#define LV_MEM_CUSTOM_FREE    free
#define LV_MEM_CUSTOM_REALLOC realloc
#endif

#if defined(KERNEL_BAREMETAL) || defined(KERNEL_FREERTOS)
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE <aic_common.h>
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (aic_get_time_ms())    /*Expression evaluating to current system time in ms*/
#define LV_DISP_DEF_REFR_PERIOD 10
#endif

#ifdef AIC_LVGL_MUSIC_DEMO
#define LV_USE_PERF_MONITOR 1
#endif

#define LV_USE_FS_POSIX 1
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER 'L'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_POSIX_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_POSIX_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

#ifdef LPKG_USING_LV_MUSIC_DEMO
/* music player demo */
#define LV_USE_DEMO_RTT_MUSIC       1
#define LV_DEMO_RTT_MUSIC_AUTO_PLAY 1
#define LV_FONT_MONTSERRAT_12       1
#define LV_FONT_MONTSERRAT_16       1
// #define LV_COLOR_SCREEN_TRANSP    1
#endif /* LPKG_USING_LV_MUSIC_DEMO */

//#define LV_LOG_LEVEL LV_LOG_LEVEL_TRACE
#define LV_FONT_MONTSERRAT_20       1
//#define LV_COLOR_SCREEN_TRANSP    1



#define LV_FONT_MONTSERRAT_24       1
#define LV_FONT_MONTSERRAT_28       1
#define LV_USE_FREETYPE 0
#if LV_USE_FREETYPE
    // Memory used by FreeType to cache characters [bytes]
    #define LV_FREETYPE_CACHE_SIZE (16 * 1024)
    #if LV_FREETYPE_CACHE_SIZE >= 0
        // 1: bitmap cache use the sbit cache, 0:bitmap cache use the image cache.
        // sbit cache:it is much more memory efficient for small bitmaps(font size < 256)
        // if font size >= 256, must be configured as image cache */
        #define LV_FREETYPE_SBIT_CACHE 0
        // Maximum number of opened FT_Face/FT_Size objects managed by this cache instance.
        // (0:use system defaults)
        #define LV_FREETYPE_CACHE_FT_FACES 0
        #define LV_FREETYPE_CACHE_FT_SIZES 0
    #endif
#endif

#define LV_USE_GIF 1

#endif // LV_CONF_H

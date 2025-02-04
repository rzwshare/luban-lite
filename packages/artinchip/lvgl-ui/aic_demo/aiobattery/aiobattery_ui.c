/*
 * Copyright (C) 2022-2023 ArtinChip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "lvgl.h"
#include "lv_aio_uart_run.h"
#include "aic_ui.h"
#include "lv_port_disp.h"
#include "mpp_fb.h"

#ifdef LPKG_USING_CPU_USAGE
#include "cpu_usage.h"
#endif
#include "aiobattery_ui.h"


extern AioEventDisplay aio_eventDisplay;

#if 1

static lv_obj_t * ui_BGImage;
//lv_obj_t * ui_FootGroup;
///lv_obj_t * ui_ErrorGroup;
static lv_obj_t * ui_errorImage;
static lv_obj_t * ui_errorLabel;
//lv_obj_t * ui_CautionGroup;
static lv_obj_t * ui_cautionImage;
static lv_obj_t * ui_cautionLabel;
//lv_obj_t * ui_BluetoothGroup;
static lv_obj_t * ui_bluetoothImage;
static lv_obj_t * ui_bluetoothLabel;

//lv_obj_t * ui_WiFiGroup;
static lv_obj_t * ui_wifiImage;
static lv_obj_t * ui_wifiLabel;
//lv_obj_t * ui_CenterGroup;
static lv_obj_t * ui_LineImage;
static lv_obj_t * ui_LineImage2;


//lv_obj_t * ui_EventLabel;
//lv_obj_t * ui_EventValueLabel;



void ui_Screen1_screen_init(void);
static  lv_obj_t * ui_Screen1;
///lv_obj_t * ui_BGImage;
static lv_obj_t * ui_ver;
static lv_obj_t * ui_centerPanel;
static lv_obj_t * ui_panellistPanel;
static lv_obj_t * ui_panelLabel;
static lv_obj_t * ui_panelBar;
static lv_obj_t * ui_panelbarLabel;
//static lv_obj_t * ui_panelbarLabel2;

static lv_obj_t * ui_gridlistPanel;
static lv_obj_t * ui_gridBar;
static lv_obj_t * ui_gridLabel;
static lv_obj_t * ui_gridbarLabel;
//static lv_obj_t * ui_gridbarLabel2;

static lv_obj_t * ui_autolistPanel;
static lv_obj_t * ui_autoLabel;
static lv_obj_t * ui_autoBar;
static lv_obj_t * ui_autobarLabel;
//static lv_obj_t * ui_autobarLabel2;

static lv_obj_t * ui_homelistPanel;
static lv_obj_t * ui_homeLabel;
static lv_obj_t * ui_homeBar;
static lv_obj_t * ui_homebarLabel;
//static lv_obj_t * ui_homebarLabel2;

static lv_obj_t * ui_eventlistPanel;
static lv_obj_t * ui_eventValueLabel;
static lv_obj_t * ui_eventLabel;
static lv_obj_t * ui_FootGroup;
static lv_obj_t * ui_ErrorGroup;
static lv_obj_t * ui_WiFiGroup;
static lv_obj_t * ui_CautionGroup;
static lv_obj_t * ui_BluetoothGroup;

#if 0


lv_obj_t * ui_ErrorImage;
lv_obj_t * ui_ErrorLabel;
lv_obj_t * ui_CautionGroup;
lv_obj_t * ui_CautionImage;
lv_obj_t * ui_CautionLabel;
lv_obj_t * ui_BluetoothGroup;
lv_obj_t * ui_BluetoothImage;
lv_obj_t * ui_BluetoothLabel;

lv_obj_t * ui_WiFiImage;
lv_obj_t * ui_WiFiLabel;
#endif
static lv_obj_t * ui_HeadGroup;
static lv_obj_t * ui_panel;
static lv_obj_t * ui_grid;
static lv_obj_t * ui_auto;
static lv_obj_t * ui_BatteryName;
static lv_obj_t * ui_BATTERY;
static lv_obj_t * ui_BatteryText;
static lv_obj_t * ui_home;
static lv_obj_t * ui_panelpoint;
static lv_obj_t * ui_panelName;
static lv_obj_t * ui_gridpoint;
static lv_obj_t * ui_gridName;
static lv_obj_t * ui_autoName;
static lv_obj_t * ui_homeName;
static lv_obj_t * ui_autopoint;
static lv_obj_t * ui_homepoint;
static lv_obj_t * ui_line1Image;
static lv_obj_t * ui_line2Image;
//static lv_obj_t * ui_ponitImage;
#endif


static int aio_fault_state_flag=-1;
static int aio_alarm_state_flag =-1;
static int aio_bt_state=-1;
static int aio_wifi_state=-1;
//static bool wifi_connecting=false;
static bool error_state=false;

static int grid_state_flag=-1;
static int panel_state=-1;
static int grid_states=-1;
static int auto_state=-1;
static int home_state=-1;
static int panelpoint_state=-1;

static int autopoint_state=-1;
static int homepoint_state=-1;

static int grid_state=-1;
static int consumption=-1;
static int self_sufficiency=-1;
static int total_electricity_consumption=-1;
static int old_total_electricity_consumption=-1;

static int soc_5ko_smart_system=-1;
//static lv_style_t aio_style_main_default;		
//static lv_style_t style_main_default;		

static lv_timer_t *wifi_timer=NULL;
static lv_timer_t *error_timer=NULL;
static int dataisvalid=-1;

//#define AIO_TEST 
#ifdef AIO_TEST
#define version    "AIO_VER__TEST0.3"
#else
//#define version    "AIO_VER_0.8"
#define version    "AIO_VER_1.3_test"
#endif
static void  bar_event_cb (lv_event_t * e)
{
	lv_obj_t * obj = lv_event_get_target(e);
	lv_obj_t *label=lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	char data_str[128];	
	if(code == LV_EVENT_VALUE_CHANGED||code==LV_EVENT_PRESSING)
	{
		int num = lv_event_get_param(e);
		
		if(obj==ui_panelBar)
		{	
			
			lv_bar_set_range(obj,0,aio_eventDisplay.rated_power_generation_value_of_photovoltaic_panels);
			lv_bar_set_value(obj,num, LV_ANIM_OFF);
			//num=(num/aio_eventDisplay.rated_power_generation_value_of_photovoltaic_panels)*1000;
			if(num>0){
				#if 0
				lv_label_set_text_fmt(label, "%d.",aio_eventDisplay.photovoltaic_panel_power_generation/100);
				lv_label_set_text_fmt(ui_panelbarLabel2, "%d kW",aio_eventDisplay.photovoltaic_panel_power_generation%100);
				#else
				ui_snprintf(data_str, "%d.%d kW", aio_eventDisplay.photovoltaic_panel_power_generation/100, aio_eventDisplay.photovoltaic_panel_power_generation%100);
				lv_label_set_text(label, data_str);
				#endif
			}
			else
			{
				#if 0
				lv_label_set_text(label, "0.");
				lv_label_set_text(ui_panelbarLabel2, "0 kW");
				#else
				ui_snprintf(data_str, "%d.%d kW", 0, 0);
				lv_label_set_text(label, data_str);
				#endif
			
			}
			
		}
		else if(obj==ui_gridBar)
		{

			lv_bar_set_range(obj,0,aio_eventDisplay.rated_power_of_the_grid_to_take_or_sell_electricity);
			lv_bar_set_value(obj,num, LV_ANIM_OFF);

			//num=(num/aio_eventDisplay.rated_power_of_the_grid_to_take_or_sell_electricity)*1000;
			if(grid_state==1)//red
			{
				
				lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_opa(ui_gridBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_border_color(ui_gridBar, lv_color_hex(0x241517), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_border_opa(ui_gridBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
			// lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0x143F27), LV_PART_INDICATOR | LV_STATE_DEFAULT);
				
				lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0x1F1012), LV_PART_INDICATOR | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_opa(ui_gridBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_grad_color(ui_gridBar, lv_color_hex(0xA10422), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			
			
			
			   lv_obj_set_style_text_color(ui_gridbarLabel, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_text_opa(ui_gridbarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_text_font(ui_gridbarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
			
			  
			  // lv_obj_set_style_text_color(ui_gridbarLabel2, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
			 //  lv_obj_set_style_text_opa(ui_gridbarLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
			 //  lv_obj_set_style_text_font(ui_gridbarLabel2, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
				if(num>0){
					#if 0
			  		 lv_label_set_text_fmt(label, "%d.",aio_eventDisplay.draw_power_from_the_grid/100);
			  		 lv_label_set_text_fmt(ui_gridbarLabel2, "%d kW",aio_eventDisplay.draw_power_from_the_grid%100);
					#else
					ui_snprintf(data_str, "%d.%d kW", aio_eventDisplay.draw_power_from_the_grid/100, aio_eventDisplay.draw_power_from_the_grid%100);
					lv_label_set_text(label, data_str);
					#endif
				}
				else{
					#if 0
					 lv_label_set_text(label, "0.");
			  		 lv_label_set_text(ui_gridbarLabel2, "0 kW");
					#else
					ui_snprintf(data_str, "%d.%d kW", 0,0);
					lv_label_set_text(label, data_str);
					#endif
					}

			}
			else// if(grid_state==2)//green
			{
				lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_opa(ui_gridBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_border_color(ui_gridBar, lv_color_hex(0x0E1812), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_border_opa(ui_gridBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
			// lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0x143F27), LV_PART_INDICATOR | LV_STATE_DEFAULT);
				
				lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0x010300), LV_PART_INDICATOR | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_opa(ui_gridBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_grad_color(ui_gridBar, lv_color_hex(0x6BB212), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			
			
			    lv_obj_set_style_text_color(ui_gridbarLabel, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_text_opa(ui_gridbarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_set_style_text_font(ui_gridbarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
			
			  
			  // lv_obj_set_style_text_color(ui_gridbarLabel2, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
			  // lv_obj_set_style_text_opa(ui_gridbarLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
			  // lv_obj_set_style_text_font(ui_gridbarLabel2, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
			
				if(num>0){
					#if 0
			  		 lv_label_set_text_fmt(label, "%d.",aio_eventDisplay.sell_electricity_to_the_grid/100);
			  		 lv_label_set_text_fmt(ui_gridbarLabel2, "%d kW",aio_eventDisplay.sell_electricity_to_the_grid%100);
					#else
					ui_snprintf(data_str, "%d.%d kW", aio_eventDisplay.sell_electricity_to_the_grid/100, aio_eventDisplay.sell_electricity_to_the_grid%100);
					lv_label_set_text(label, data_str);
					#endif
				}
				else{
					#if 0
					 lv_label_set_text(label, "0.");
			  		 lv_label_set_text(ui_gridbarLabel2, "0 kW");
					#else
					ui_snprintf(data_str, "%d.%d kW", 0, 0);
					lv_label_set_text(label, data_str);
					#endif
					}
			}
			
			
		}
		else if(obj==ui_autoBar)//Consumption
		{	
			lv_bar_set_range(obj,0,aio_eventDisplay.household_loads_are_rated_for_daily_electricity_consumption);
			lv_bar_set_value(obj,num, LV_ANIM_OFF);

			if(num>=aio_eventDisplay.household_loads_are_rated_for_daily_electricity_consumption)
			{

				lv_obj_set_style_bg_color(obj, lv_color_hex(0x1F1012), LV_PART_INDICATOR | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
				lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xA10422), LV_PART_INDICATOR | LV_STATE_DEFAULT);

				lv_obj_set_style_text_color(ui_autobarLabel, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
				
			  
			 //  lv_obj_set_style_text_color(ui_autobarLabel2, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
			 
			}
			else
			{
				 lv_obj_set_style_bg_color(obj, lv_color_hex(0x151818), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			    lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
			    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x55E3C9), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			  //  lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);
 				
			  lv_obj_set_style_text_color(ui_autobarLabel, lv_color_hex(0x55E3C9), LV_PART_MAIN | LV_STATE_DEFAULT);
			 // lv_obj_set_style_text_color(ui_autobarLabel2, lv_color_hex(0x55E3C9), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
			if(num>0){
				#if 0
				lv_label_set_text_fmt(label, "%d.",num/10);
			
				lv_label_set_text_fmt(ui_autobarLabel2, "%d kWh",num%10);
				#else
				ui_snprintf(data_str, "%d.%d kWh",num/10, num%10);
				lv_label_set_text(label, data_str);
				#endif
			}
			else{
				#if 0
					 lv_label_set_text(label, "0.");
			  		 lv_label_set_text(ui_autobarLabel2, "0 kWh");
				#else
				ui_snprintf(data_str, "%d.%d kWh",0, 0);
				lv_label_set_text(label, data_str);
				#endif
				}
			
		}
		else if(obj==ui_homeBar)//Self sufficiency
		{	
		
			//int all_num=(aio_eventDisplay.amount_of_charging_the_tram+aio_eventDisplay.household_load_electricity_consumption_per_day);
			
			
			if(total_electricity_consumption>0){


				if(num>=total_electricity_consumption )
				{
					
					lv_bar_set_range(obj,0,total_electricity_consumption);
					lv_bar_set_value( obj,total_electricity_consumption, LV_ANIM_OFF);
					#if 0
					lv_label_set_text_fmt(label, "%d.",100);		
					lv_label_set_text_fmt(ui_homebarLabel2, "%d%%",0);
					#else
					ui_snprintf(data_str, "%d.%d%%",100, 0);
					lv_label_set_text(label, data_str);
					#endif
				}
				else{
					lv_bar_set_range(obj,0,total_electricity_consumption);
			
					lv_bar_set_value( obj,num, LV_ANIM_OFF);
					int real_num=(num*1000/total_electricity_consumption);
					
				//	printf("############real_num:%d,num:%d,all_num:%d,dd:%f\r\n",real_num,num,all_num,(num/all_num));
					#if 0
					lv_label_set_text_fmt(label, "%d.",real_num/10);		
					lv_label_set_text_fmt(ui_homebarLabel2, "%d%%",real_num%10);
					#else
					ui_snprintf(data_str, "%d.%d%%",real_num/10, real_num%10);
					lv_label_set_text(label, data_str);
					#endif
				}
			}
			else{
				lv_bar_set_range(obj,0,total_electricity_consumption);
				lv_bar_set_value( obj,0, LV_ANIM_OFF);
				#if 0
				lv_label_set_text(label, "0.");		
				lv_label_set_text(ui_homebarLabel2, "0%");
				#else
					ui_snprintf(data_str, "%d.%d%%",0, 0);
					lv_label_set_text(label, data_str);
				#endif
				
				}
		}
		
	}
	
}

	
static void show_event_event(lv_event_t * e)
{
	lv_obj_t * point = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_VALUE_CHANGED)
	{
		 if(point==ui_eventValueLabel){
	      if(aio_eventDisplay.system_alarm_nums>0&&aio_eventDisplay.system_fault_nums >0)
			lv_label_set_text_fmt(point, "%s %s",aio_eventDisplay.system_alarms,aio_eventDisplay.system_faults);
		  else if(aio_eventDisplay.system_alarm_nums==0&&aio_eventDisplay.system_fault_nums >0)
			lv_label_set_text_fmt(point, "%s",aio_eventDisplay.system_faults);
		 else if(aio_eventDisplay.system_alarm_nums>0&&aio_eventDisplay.system_fault_nums ==0)
			lv_label_set_text_fmt(point, "%s",aio_eventDisplay.system_alarms);
		 else
		 	lv_label_set_text(point, "");
			//lv_label_set_text_fmt(speed_num, "%s",lv_event_get_param(e));
		}
	}
}
	
static void battery_text_event(lv_event_t * e)
{
	lv_obj_t * point = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_VALUE_CHANGED)
	{
		int num = lv_event_get_param(e);
		
		lv_label_set_text_fmt(point, "%d%%",num);
		
	}
}

static void panel_point_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED)
	{
		int num = lv_event_get_param(e);
		char data_str[128];
		ui_snprintf(data_str, "%spanel/panel%d.png", LVGL_DIR,num);
		//printf("data_str:%s\r\n",data_str);
		lv_img_set_src(point, data_str);
		//lv_img_set_angle(point, 0);
		
	}
}

static void grid_point_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED)
	{
		int num = lv_event_get_param(e);
		char data_str[128];
		if(grid_state==1){
			ui_snprintf(data_str, "%sgrid/gridred%d.png", LVGL_DIR,num);
			//printf("data_str:%s\r\n",data_str);
			lv_img_set_src(point, data_str);
		}else if(grid_state==2){
		    ui_snprintf(data_str, "%sgrid/gridgreen%d.png", LVGL_DIR,num);
			//printf("data_str:%s\r\n",data_str);
			lv_img_set_src(point, data_str);
		}
	}
}

static void auto_point_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED)
	{
		int num = lv_event_get_param(e);
		char data_str[128];
		ui_snprintf(data_str, "%sauto/auto%d.png", LVGL_DIR,num);
		//printf("data_str:%s\r\n",data_str);
		lv_img_set_src(point, data_str);
		//lv_img_set_angle(point, 0);
		
	}
}

static void home_point_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED)
	{
		int num = lv_event_get_param(e);
		char data_str[128];
		ui_snprintf(data_str, "%shome/home%d.png", LVGL_DIR,num);
		//printf("data_str:%s\r\n",data_str);
		lv_img_set_src(point, data_str);
		//lv_img_set_angle(point, 0);
		
	}
}

static void aio_footimg_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
		int num = lv_event_get_param(e);
#if 1
		if(point==ui_errorImage)
		{
		#if 1
			if(num==0)
			{
				//lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
				lv_img_set_src(point,LVGL_PATH(ERROR.png));
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(ERROR1.png));
				//lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
				{
					lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
					//	lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
					lv_img_set_src(point,LVGL_PATH(ERROR.png));
				}
			#endif
		}
		else if(point==ui_cautionImage)
		{
		#if 1
			//printf("ui_CautionImage num:%d\r\n",num);
			if(num==0)
			{
				lv_img_set_src(point,LVGL_PATH(CAUTION.png));
				//lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(CAUTION1.png));
				//lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_img_set_src(point,LVGL_PATH(CAUTION.png));
				//lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
				//lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			#endif
				
		}
		else if(point==ui_bluetoothImage)
		{
		#if 1
			if(num==0)
			{
				lv_img_set_src(point,LVGL_PATH(BLUETOOTH.png));
				//lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(BLUETOOTH1.png));
				//lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_img_set_src(point,LVGL_PATH(BLUETOOTH.png));
				//lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
				//lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			#endif
		}
		else if(point==ui_wifiImage)
		{
		#if 1
			if(num==0)
			{
				lv_img_set_src(point,LVGL_PATH(WiFi.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else if(num==1)//配网
			{
				lv_img_set_src(point,LVGL_PATH(WiFi.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}//2
			else if(num==2)
			{
				lv_img_set_src(point,LVGL_PATH(WiFi1.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_img_set_src(point,LVGL_PATH(WiFi.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
				//lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			#endif
		}
	#endif
    }
}
static void aio_footlabel_event(lv_event_t * e)
{
    lv_obj_t * speed_num = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
				
    if(code == LV_EVENT_VALUE_CHANGED) {
#if 1
		int num = lv_event_get_param(e);//

		if(speed_num==ui_errorLabel)
		{
			if(num==0)
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
			else
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
		}
		else if(speed_num==ui_cautionLabel)
		{
			if(num==0)
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
			else
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xFFf200), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
		}
		else if(speed_num==ui_bluetoothLabel)
		{
			if(num==0)
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
			else
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0x8aee49), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
		}
		else if(speed_num==ui_wifiLabel)
		{
			if(num==2)
			{
			
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0x8aee49), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_clear_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_clear_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
			
			}
		}
		
	#endif
    }
}
static void obj_set_clear_hidden_flag(lv_obj_t *obj)
{
	if(lv_obj_has_flag(obj,LV_OBJ_FLAG_HIDDEN))
		lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
			
}
static void error_call_back(lv_timer_t *errortmr)
{
	static uint8_t  err_count=-1;
	static bool error_state=false;

	(void)errortmr;
#if 1	
	err_count++;
#ifndef AIO_TEST
	if(!aio_eventDisplay.dataisvalid){
		err_count=-1;
	}
#endif
	if(err_count >7)
	{
		err_count=0;
		
		if(aio_eventDisplay.system_fault_state==1){
			aio_fault_state_flag=1;
			error_state=!error_state;	
			if(error_state)
			{
				lv_img_set_src(ui_errorImage,LVGL_PATH(ERROR.png));
				lv_obj_set_style_text_color(ui_errorLabel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
						
			}
			else 
			{
				lv_img_set_src(ui_errorImage,LVGL_PATH(ERROR1.png));
				lv_obj_set_style_text_color(ui_errorLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
							
			}
		}
		else
			{
				if(aio_eventDisplay.system_fault_state!=aio_fault_state_flag){
				 	aio_fault_state_flag=aio_eventDisplay.system_fault_state;
				 	printf("erro time aio_eventDisplay.system_fault_state:%d\r\n",aio_eventDisplay.system_fault_state);
				
					lv_event_send(ui_errorImage, LV_EVENT_VALUE_CHANGED,aio_fault_state_flag);
					lv_event_send(ui_errorLabel, LV_EVENT_VALUE_CHANGED,aio_fault_state_flag);
				}
			}
	}

#endif
}

static void wifi_call_back(lv_timer_t *wifitmr)
{
	static uint8_t  wifi_count=-1;

	(void)wifitmr;
#if 1	
	wifi_count++;
#ifndef AIO_TEST
	if(!aio_eventDisplay.dataisvalid){
		wifi_count=-1;
		}
#endif	
	if(wifi_count >7 ){//7
		wifi_count=0;
		if(aio_eventDisplay.wifi_state==1){
			aio_wifi_state=1;

			obj_set_clear_hidden_flag(ui_wifiImage);
			obj_set_clear_hidden_flag(ui_wifiLabel);
		
		}
		else {
			if(aio_eventDisplay.wifi_state!=aio_wifi_state){
				aio_wifi_state=aio_eventDisplay.wifi_state;
			    printf("witi time aio_eventDisplay.wifi_state:%d\r\n",aio_eventDisplay.wifi_state);
			
			    lv_event_send(ui_wifiImage, LV_EVENT_VALUE_CHANGED,aio_wifi_state);
			    lv_event_send(ui_wifiLabel, LV_EVENT_VALUE_CHANGED,aio_wifi_state);

			}	
		}
	}
#endif	
}

static bool isupdate=false;
static void update_xcb(lv_timer_t *tmr)
{
	 char data_str[128];
  //  static int hour = 2;
    static int min = 0;

    (void)tmr;
    min++;
    if (min >= 30) {//60
      //  hour++;
        min = 0;
	  isupdate=true;
    }

}


static void backlight_xcb(lv_timer_t *tmr)
{
	
    static int min = 0;
	static uint8_t  auto_close_lcd_id =-1;
    (void)tmr;

  //  if(ui_ponitImage!=NULL)
//	obj_set_clear_hidden_flag(ui_ponitImage);	
	
	if(getLCD())
		auto_close_lcd_id++;
	else
		auto_close_lcd_id=0;
	if(auto_close_lcd_id >= 60)
		backlight_disable();
}
#ifdef AIO_TEST

static void timer_xcb(lv_timer_t *tmr)
{

	//static uint8_t  id = -1;
	static uint8_t  panelid = -1;
	static uint8_t  gridid = -1;
	static uint8_t  autoid = -1;
	static uint8_t  homeid = -1;
	static uint8_t  hide_ver_id=-1;
	
	(void)tmr;
	hide_ver_id++;
	if(hide_ver_id >5)   //ver 1s 
		lv_obj_add_flag(ui_ver, LV_OBJ_FLAG_HIDDEN);
	
	if(isupdate)
	{
		isupdate=false;
		lv_obj_clean(ui_HeadGroup);	
		lv_obj_clean(ui_BATTERY);	
		lv_obj_clean(ui_FootGroup);	
		lv_obj_clean(ui_WiFiGroup);	
		lv_obj_clean(ui_ErrorGroup);
		lv_obj_clean(ui_BluetoothGroup);	
		lv_obj_clean(ui_CautionGroup);	
		printf("30 min  lv_obj_clean\r\n");
		memset(&aio_eventDisplay, 0, sizeof(AioEventDisplay));
		aio_footgroup_init();
	        aio_headgroup_init();
		aio_eventDisplay.panel_state=0;
		
			aio_eventDisplay.grid_state=0;
			aio_eventDisplay.auto_state=0;
			aio_eventDisplay.home_state=0;

			aio_eventDisplay.photovoltaic_panel_power_generation =0;
			aio_eventDisplay.photovoltaic_panel_power_generation_power_return_value=0;
			aio_eventDisplay.draw_power_from_the_grid =0;
			aio_eventDisplay.grid_power_take_or_sell_power_return_variance=0;

			aio_eventDisplay.sell_electricity_to_the_grid =0;
			//aio_eventDisplay.grid_power_take_or_sell_power_return_variance=
			aio_eventDisplay.tram_charging_power =0;
			aio_eventDisplay.tram_charging_power_return_difference=0;
			aio_eventDisplay.household_load_electricity_consumption_per_day=0;

			aio_eventDisplay.electricity_power_for_household_loads =0;
			aio_eventDisplay.power_return_value_of_household_load_power=0;

			aio_eventDisplay.photovoltaic_panels_generate_electricity =0;
			aio_eventDisplay.soc_5ko_smart_system=0;
			aio_eventDisplay.system_alarm_state=0;
			aio_eventDisplay.bt_state=0;
			aio_eventDisplay.wifi_state=0;
			aio_eventDisplay.system_fault_state=0;
			aio_eventDisplay.rated_power_generation_value_of_photovoltaic_panels=0;
			aio_eventDisplay.rated_power_of_the_grid_to_take_or_sell_electricity=0;
			aio_eventDisplay.household_loads_are_rated_for_daily_electricity_consumption=0;
			

	}
	else
		{
			aio_eventDisplay.panel_state=1;
		
			aio_eventDisplay.grid_state=1;
			aio_eventDisplay.auto_state=1;
			aio_eventDisplay.home_state=1;

			aio_eventDisplay.photovoltaic_panel_power_generation =10;
			aio_eventDisplay.photovoltaic_panel_power_generation_power_return_value=0;
			aio_eventDisplay.draw_power_from_the_grid =20;
			aio_eventDisplay.grid_power_take_or_sell_power_return_variance=0;

			aio_eventDisplay.sell_electricity_to_the_grid =30;
			//aio_eventDisplay.grid_power_take_or_sell_power_return_variance=
			aio_eventDisplay.tram_charging_power =40;
			aio_eventDisplay.tram_charging_power_return_difference=0;
			aio_eventDisplay.household_load_electricity_consumption_per_day=100;

			aio_eventDisplay.electricity_power_for_household_loads =50;
			aio_eventDisplay.power_return_value_of_household_load_power=0;

			aio_eventDisplay.photovoltaic_panels_generate_electricity =60;

			aio_eventDisplay.rated_power_generation_value_of_photovoltaic_panels=60;
			aio_eventDisplay.rated_power_of_the_grid_to_take_or_sell_electricity=60;
			aio_eventDisplay.household_loads_are_rated_for_daily_electricity_consumption=60;
			
			aio_eventDisplay.soc_5ko_smart_system=78;
			aio_eventDisplay.system_alarm_state=1;
			aio_eventDisplay.bt_state=1;
			aio_eventDisplay.wifi_state=2;
			aio_eventDisplay.system_fault_state=1;
		}
	#if 1
	if(panel_state!=aio_eventDisplay.panel_state){
		panel_state=aio_eventDisplay.panel_state;
		if(aio_eventDisplay.panel_state){
			lv_obj_clear_flag(ui_panel, LV_OBJ_FLAG_HIDDEN);
			//lv_obj_clear_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_panel, LV_OBJ_FLAG_HIDDEN);
			//lv_obj_add_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
		}
	}

	if(grid_states!=aio_eventDisplay.grid_state){
		grid_states=aio_eventDisplay.grid_state;
		if(aio_eventDisplay.grid_state){
			lv_obj_clear_flag(ui_grid, LV_OBJ_FLAG_HIDDEN);
			//lv_obj_clear_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_grid, LV_OBJ_FLAG_HIDDEN);
			//lv_obj_add_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
		}
	}

	if(auto_state!=aio_eventDisplay.auto_state){
		auto_state=aio_eventDisplay.auto_state;
		if(aio_eventDisplay.auto_state){
			lv_obj_clear_flag(ui_auto, LV_OBJ_FLAG_HIDDEN);
		//	lv_obj_clear_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_auto, LV_OBJ_FLAG_HIDDEN);
		//	lv_obj_add_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
			}
		
	}

	if(home_state!=aio_eventDisplay.home_state){
		home_state=aio_eventDisplay.home_state;
		if(aio_eventDisplay.home_state){
			lv_obj_clear_flag(ui_home, LV_OBJ_FLAG_HIDDEN);
		//	lv_obj_clear_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_home, LV_OBJ_FLAG_HIDDEN);
		//	lv_obj_add_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
			}
	}
	#endif
#if 1
	if(aio_eventDisplay.panel_state && (aio_eventDisplay.photovoltaic_panel_power_generation > aio_eventDisplay.photovoltaic_panel_power_generation_power_return_value))
	{
		panelpoint_state=-1;
		lv_obj_clear_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
		panelid++;
		if(panelid>21)
			panelid=0;
		lv_event_send(ui_panelpoint, LV_EVENT_VALUE_CHANGED,panelid);
		lv_event_send(ui_panelBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.photovoltaic_panel_power_generation);
	}
	else {
		
			panelid=-1;
			if(panelpoint_state==-1){
				panelpoint_state=1;
				lv_obj_add_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
				lv_event_send(ui_panelpoint, LV_EVENT_VALUE_CHANGED,0);
				lv_event_send(ui_panelBar, LV_EVENT_VALUE_CHANGED,0);
			}
		}
	//ȡ�� red
	if(aio_eventDisplay.grid_state &&  (aio_eventDisplay.draw_power_from_the_grid >aio_eventDisplay.grid_power_take_or_sell_power_return_variance))
	{
		grid_state_flag=grid_state=1;
		lv_obj_clear_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
		gridid++;
		if(gridid>21)
			gridid=0;
		lv_event_send(ui_gridpoint, LV_EVENT_VALUE_CHANGED,gridid);
		
		lv_event_send(ui_gridBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.draw_power_from_the_grid);
	
 	}//�ŵ� green
	else if( aio_eventDisplay.grid_state && (aio_eventDisplay.sell_electricity_to_the_grid >aio_eventDisplay.grid_power_take_or_sell_power_return_variance))
	{
		grid_state_flag=grid_state=2;
		lv_obj_clear_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
		gridid++;
		if(gridid>21)
			gridid=0;
		lv_event_send(ui_gridpoint, LV_EVENT_VALUE_CHANGED,gridid);
		lv_event_send(ui_gridBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.sell_electricity_to_the_grid);
	}
	else{
		    gridid=-1;
			grid_state=0;
			if(grid_state_flag!=grid_state){
				grid_state_flag=grid_state;			
				lv_event_send(ui_gridpoint, LV_EVENT_VALUE_CHANGED,0);
				lv_obj_add_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
				lv_event_send(ui_gridBar, LV_EVENT_VALUE_CHANGED,0);
			}
		}
	
	if( aio_eventDisplay.auto_state && (aio_eventDisplay.tram_charging_power >aio_eventDisplay.tram_charging_power_return_difference))
	{
		autopoint_state=-1;
		lv_obj_clear_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
		autoid++;
		if(autoid>21)
			autoid=0;;
		lv_event_send(ui_autopoint, LV_EVENT_VALUE_CHANGED,autoid);
	}
	else
	{
	    autoid=-1;
		if(autopoint_state==-1){
			autopoint_state=1;
		lv_obj_add_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
		lv_event_send(ui_autopoint, LV_EVENT_VALUE_CHANGED,0);
		//lv_event_send(ui_autoBar, LV_EVENT_VALUE_CHANGED,0);
		}
	}

	
	if(consumption!=aio_eventDisplay.household_load_electricity_consumption_per_day){
		consumption=aio_eventDisplay.household_load_electricity_consumption_per_day;
		lv_event_send(ui_autoBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.household_load_electricity_consumption_per_day);
	}

	if( aio_eventDisplay.home_state && (aio_eventDisplay.electricity_power_for_household_loads > aio_eventDisplay.power_return_value_of_household_load_power))
	{
		homepoint_state=-1;
		homeid++;
		if(homeid>21)
			homeid=0;
		lv_obj_clear_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
		lv_event_send(ui_homepoint, LV_EVENT_VALUE_CHANGED,homeid);
	}
	else
	{
		homeid=-1;
		if(homepoint_state==-1){
			homepoint_state=1;
		lv_obj_add_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
		lv_event_send(ui_homepoint, LV_EVENT_VALUE_CHANGED,0);
		//lv_event_send(ui_homeBar, LV_EVENT_VALUE_CHANGED,0);
		}
	}

	 total_electricity_consumption=(aio_eventDisplay.amount_of_charging_the_tram+aio_eventDisplay.household_load_electricity_consumption_per_day);
	
	
	if(self_sufficiency!=aio_eventDisplay.photovoltaic_panels_generate_electricity 
		|| old_total_electricity_consumption!=total_electricity_consumption){
		self_sufficiency=aio_eventDisplay.photovoltaic_panels_generate_electricity;
		old_total_electricity_consumption=total_electricity_consumption;
		lv_event_send(ui_homeBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.photovoltaic_panels_generate_electricity);

	}

	if(soc_5ko_smart_system!=aio_eventDisplay.soc_5ko_smart_system){
		soc_5ko_smart_system=aio_eventDisplay.soc_5ko_smart_system;
		lv_event_send(ui_BatteryText, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.soc_5ko_smart_system);
	}
	#endif
	
	
#if 1
	if(aio_alarm_state_flag != aio_eventDisplay.system_alarm_state){
		aio_alarm_state_flag=aio_eventDisplay.system_alarm_state;
		printf("aio_eventDisplay.system_alarm_state:%d\r\n",aio_eventDisplay.system_alarm_state);
		lv_event_send(ui_cautionImage, LV_EVENT_VALUE_CHANGED,aio_alarm_state_flag);
		lv_event_send(ui_cautionLabel, LV_EVENT_VALUE_CHANGED,aio_alarm_state_flag);
	}

#if 0
	if(aio_eventDisplay.system_fault_state!=aio_fault_state_flag){
		aio_fault_state_flag=aio_eventDisplay.system_fault_state;
		printf("aio_eventDisplay.system_fault_state:%d\r\n",aio_eventDisplay.system_fault_state);
		
		lv_event_send(ui_errorImage, LV_EVENT_VALUE_CHANGED,aio_fault_state_flag);
		lv_event_send(ui_errorLabel, LV_EVENT_VALUE_CHANGED,aio_fault_state_flag);
	}
	
#endif
	if(aio_eventDisplay.bt_state!=aio_bt_state){
		aio_bt_state=aio_eventDisplay.bt_state;
		printf("aio_eventDisplay.bt_state:%d\r\n",aio_eventDisplay.bt_state);
		
		lv_event_send(ui_bluetoothImage, LV_EVENT_VALUE_CHANGED,aio_bt_state);
		lv_event_send(ui_bluetoothLabel, LV_EVENT_VALUE_CHANGED,aio_bt_state);
	}
	
	#if 0
	if(aio_eventDisplay.wifi_state!=aio_wifi_state){
				aio_wifi_state=aio_eventDisplay.wifi_state;
			printf("aio_eventDisplay.wifi_state:%d\r\n",aio_eventDisplay.wifi_state);
			
			lv_event_send(ui_wifiImage, LV_EVENT_VALUE_CHANGED,aio_wifi_state);
			lv_event_send(ui_wifiLabel, LV_EVENT_VALUE_CHANGED,aio_wifi_state);

		}	
//	#else
	if(aio_eventDisplay.wifi_state==1)
	{
		if(aio_eventDisplay.wifi_state!=aio_wifi_state)
				aio_wifi_state=aio_eventDisplay.wifi_state;
		wifi_count++;
			
		if(wifi_count >10){
			wifi_count=0;
			wifi_connecting=!wifi_connecting;
		printf("aio_eventDisplay.wifi_state:%d\r\n",aio_eventDisplay.wifi_state);
		
			lv_event_send(ui_wifiImage, LV_EVENT_VALUE_CHANGED,wifi_connecting?1:0);
			
			lv_event_send(ui_wifiLabel, LV_EVENT_VALUE_CHANGED,wifi_connecting?1:0);
		}
	}
	else{
		wifi_count=0;
			if(aio_eventDisplay.wifi_state!=aio_wifi_state){
				aio_wifi_state=aio_eventDisplay.wifi_state;
			printf("aio_eventDisplay.wifi_state:%d\r\n",aio_eventDisplay.wifi_state);
			
			lv_event_send(ui_wifiImage, LV_EVENT_VALUE_CHANGED,aio_wifi_state);
			lv_event_send(ui_wifiLabel, LV_EVENT_VALUE_CHANGED,aio_wifi_state);
		}	
	}
	#endif
#endif

	lv_event_send(ui_eventValueLabel, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.system_faults);

		 
}
#else

static void timer_xcb(lv_timer_t *tmr)
{
	char data_str[128];
#ifdef KERNEL_RTTHREAD
	float value;
#endif
	static uint8_t  panelid = -1;
	static uint8_t  gridid = -1;
	static uint8_t  autoid = -1;
	static uint8_t  homeid = -1;
	static uint8_t  hide_ver_id=-1;
	
	(void)tmr;
	hide_ver_id++;
	if(hide_ver_id >5)   //ver 1s 
		lv_obj_add_flag(ui_ver, LV_OBJ_FLAG_HIDDEN);

		
	if(!aio_eventDisplay.dataisvalid)
	{
		if(dataisvalid!=aio_eventDisplay.dataisvalid){
			dataisvalid=aio_eventDisplay.dataisvalid;
			lv_obj_clean(ui_HeadGroup);	
			lv_obj_clean(ui_BATTERY);	
			lv_obj_clean(ui_FootGroup);	
			lv_obj_clean(ui_WiFiGroup);	
			lv_obj_clean(ui_ErrorGroup);
			lv_obj_clean(ui_BluetoothGroup);	
			lv_obj_clean(ui_CautionGroup);	
			
			memset(&aio_eventDisplay, 0, sizeof(AioEventDisplay));
			aio_footgroup_init();
			aio_headgroup_init();

			if(aio_eventDisplay.system_fault_state!=aio_fault_state_flag){
				aio_fault_state_flag=aio_eventDisplay.system_fault_state;
				printf("aio_eventDisplay.system_fault_state:%d\r\n",aio_eventDisplay.system_fault_state);

				lv_event_send(ui_errorImage, LV_EVENT_VALUE_CHANGED,aio_fault_state_flag);
				lv_event_send(ui_errorLabel, LV_EVENT_VALUE_CHANGED,aio_fault_state_flag);
		   }


			if(aio_eventDisplay.wifi_state!=aio_wifi_state){
				aio_wifi_state=aio_eventDisplay.wifi_state;
				printf("aio_eventDisplay.wifi_state:%d\r\n",aio_eventDisplay.wifi_state);

				lv_event_send(ui_wifiImage, LV_EVENT_VALUE_CHANGED,aio_wifi_state);
				lv_event_send(ui_wifiLabel, LV_EVENT_VALUE_CHANGED,aio_wifi_state);

			}	

#if 1
			aio_fault_state_flag=-1;
			aio_alarm_state_flag =-1;
			aio_bt_state=-1;
			aio_wifi_state=-1;
			error_state=false;

			grid_state_flag=-1;
			panel_state=-1;
			grid_states=-1;
			auto_state=-1;
			home_state=-1;
			panelpoint_state=-1;

			autopoint_state=-1;
			homepoint_state=-1;

			grid_state=-1;
			consumption=-1;
			self_sufficiency=-1;
			soc_5ko_smart_system=-1;
			old_total_electricity_consumption=-1;
#endif
		}	


	}else{
	dataisvalid=-1;
	}

#if 1
	if(panel_state!=aio_eventDisplay.panel_state){
		panel_state=aio_eventDisplay.panel_state;
		if(aio_eventDisplay.panel_state){
			lv_obj_clear_flag(ui_panel, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_panel, LV_OBJ_FLAG_HIDDEN);
		}
	}

	if(grid_states!=aio_eventDisplay.grid_state){
		grid_states=aio_eventDisplay.grid_state;
		if(aio_eventDisplay.grid_state){
			lv_obj_clear_flag(ui_grid, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_grid, LV_OBJ_FLAG_HIDDEN);
		}
	}

	if(auto_state!=aio_eventDisplay.auto_state){
		auto_state=aio_eventDisplay.auto_state;
		if(aio_eventDisplay.auto_state){
			lv_obj_clear_flag(ui_auto, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_auto, LV_OBJ_FLAG_HIDDEN);
		}

	}

	if(home_state!=aio_eventDisplay.home_state){
		home_state=aio_eventDisplay.home_state;
		if(aio_eventDisplay.home_state){
			lv_obj_clear_flag(ui_home, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(ui_home, LV_OBJ_FLAG_HIDDEN);
		}
	}
#endif
#if 1
	if(aio_eventDisplay.panel_state && (aio_eventDisplay.photovoltaic_panel_power_generation > aio_eventDisplay.photovoltaic_panel_power_generation_power_return_value))
	{
		panelpoint_state=-1;
		lv_obj_clear_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
		panelid++;
		if(panelid>21)
			panelid=0;
		lv_event_send(ui_panelpoint, LV_EVENT_VALUE_CHANGED,panelid);
		lv_event_send(ui_panelBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.photovoltaic_panel_power_generation);
	}
	else 
	{

		panelid=-1;
		if(panelpoint_state==-1){
			panelpoint_state=1;
			lv_obj_add_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
			lv_event_send(ui_panelpoint, LV_EVENT_VALUE_CHANGED,0);
			lv_event_send(ui_panelBar, LV_EVENT_VALUE_CHANGED,0);
		}
	}
	//取电 red
	if(aio_eventDisplay.grid_state &&  (aio_eventDisplay.draw_power_from_the_grid >aio_eventDisplay.grid_power_take_or_sell_power_return_variance))
	{
		grid_state_flag=grid_state=1;
		lv_obj_clear_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
		gridid++;
		if(gridid>21)
			gridid=0;
		lv_event_send(ui_gridpoint, LV_EVENT_VALUE_CHANGED,gridid);

		lv_event_send(ui_gridBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.draw_power_from_the_grid);

	}//放电 green
	else if( aio_eventDisplay.grid_state && (aio_eventDisplay.sell_electricity_to_the_grid >aio_eventDisplay.grid_power_take_or_sell_power_return_variance))
	{
		grid_state_flag=grid_state=2;
		lv_obj_clear_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
		gridid++;
		if(gridid>21)
			gridid=0;
		lv_event_send(ui_gridpoint, LV_EVENT_VALUE_CHANGED,gridid);
		lv_event_send(ui_gridBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.sell_electricity_to_the_grid);
	}
	else{
		gridid=-1;
		grid_state=0;
		if(grid_state_flag!=grid_state){
			grid_state_flag=grid_state;			
			lv_event_send(ui_gridpoint, LV_EVENT_VALUE_CHANGED,0);
			lv_obj_add_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
			lv_event_send(ui_gridBar, LV_EVENT_VALUE_CHANGED,0);
		}
	}

	if( aio_eventDisplay.auto_state && (aio_eventDisplay.tram_charging_power >aio_eventDisplay.tram_charging_power_return_difference))
	{
		autopoint_state=-1;
		lv_obj_clear_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
		autoid++;
		if(autoid>21)
			autoid=0;
		lv_event_send(ui_autopoint, LV_EVENT_VALUE_CHANGED,autoid);
	}
	else
	{
		autoid=-1;
		if(autopoint_state==-1){
			autopoint_state=1;
			lv_obj_add_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
			lv_event_send(ui_autopoint, LV_EVENT_VALUE_CHANGED,0);
		}
	}


	if(consumption!=aio_eventDisplay.household_load_electricity_consumption_per_day){
		consumption=aio_eventDisplay.household_load_electricity_consumption_per_day;
		lv_event_send(ui_autoBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.household_load_electricity_consumption_per_day);
	}

	if( aio_eventDisplay.home_state && (aio_eventDisplay.electricity_power_for_household_loads > aio_eventDisplay.power_return_value_of_household_load_power))
	{
		homepoint_state=-1;
		homeid++;
		if(homeid>21)
			homeid=0;
		lv_obj_clear_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
		lv_event_send(ui_homepoint, LV_EVENT_VALUE_CHANGED,homeid);
	}
	else
	{
		homeid=-1;
		if(homepoint_state==-1){
			homepoint_state=1;
			lv_obj_add_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
			lv_event_send(ui_homepoint, LV_EVENT_VALUE_CHANGED,0);
		}
	}

	total_electricity_consumption=(aio_eventDisplay.amount_of_charging_the_tram+aio_eventDisplay.household_load_electricity_consumption_per_day);


	if(self_sufficiency!=aio_eventDisplay.photovoltaic_panels_generate_electricity 
	|| old_total_electricity_consumption!=total_electricity_consumption){
		self_sufficiency=aio_eventDisplay.photovoltaic_panels_generate_electricity;
		old_total_electricity_consumption=total_electricity_consumption;
		lv_event_send(ui_homeBar, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.photovoltaic_panels_generate_electricity);

	}

	if(soc_5ko_smart_system!=aio_eventDisplay.soc_5ko_smart_system){
		soc_5ko_smart_system=aio_eventDisplay.soc_5ko_smart_system;
		lv_event_send(ui_BatteryText, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.soc_5ko_smart_system);
	}
#endif
#if 1
	if(aio_eventDisplay.system_alarm_state !=aio_alarm_state_flag ){
		aio_alarm_state_flag=aio_eventDisplay.system_alarm_state;
		printf("aio_eventDisplay.system_alarm_state:%d\r\n",aio_eventDisplay.system_alarm_state);
		lv_event_send(ui_cautionImage, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.system_alarm_state);
		lv_event_send(ui_cautionLabel, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.system_alarm_state);
	}

	if(aio_eventDisplay.bt_state!=aio_bt_state){
		aio_bt_state=aio_eventDisplay.bt_state;
		printf("aio_eventDisplay.bt_state:%d\r\n",aio_eventDisplay.bt_state);

		lv_event_send(ui_bluetoothImage, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.bt_state);
		lv_event_send(ui_bluetoothLabel, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.bt_state);
	}

#endif

	lv_event_send(ui_eventValueLabel, LV_EVENT_VALUE_CHANGED,aio_eventDisplay.system_faults);
#if 0
#ifdef KERNEL_RTTHREAD
	/* cpu usage */
#ifdef LPKG_USING_CPU_USAGE
#include "cpu_usage.h"
	value = cpu_load_average();
#else
	value = 0;
#endif

	// ui_snprintf(data_str, "%d%% CPU", (int)value);
	// lv_label_set_text(bg_cpu, data_str);

	printf( "aio cpu: %d%% CPU\r\n", (int)value);
#endif /* KERNEL_RTTHREAD */


    /* mem usage */
#ifdef RT_USING_MEMHEAP
    extern long get_mem_used(void);
    //value = ((float)(get_mem_used())) / (1024.0 * 1024.0);
	 value = ((float)(get_mem_used())) / (1024.0 );
#else
    value = 0;
#endif

#ifdef AIC_PRINT_FLOAT_CUSTOM
    int mem_i;
    int mem_frac;

    mem_i = (int)value;
    mem_frac = (value - mem_i) * 100;
  //  ui_snprintf(data_str, "%d.%02dMB\n", mem_i, mem_frac);
	ui_snprintf(data_str, "%d.%02dKB\n", mem_i, mem_frac);
#else
   // ui_snprintf(data_str, "%.2fMB\n", value);
    ui_snprintf(data_str, "%.2fKB\n", value);
	printf( "aio MEM: %s \r\n", data_str);

#endif
#endif
}
#endif
void aiobattery_ui_init()
{
	printf("aiobattery_ui_init\r\n");	
	ui_Screen1_screen_init();
	aio_footgroup_init();
	aio_headgroup_init();

	lv_timer_t* timer= lv_timer_create(timer_xcb, 200, 0);	 
	wifi_timer=lv_timer_create(wifi_call_back,150,0);
	error_timer=lv_timer_create(error_call_back,150,0);//150ms	
	lv_timer_create(backlight_xcb,1000,0);// time 1s

	lv_timer_create(update_xcb, 1000*60, 0);	

}

void aio_footgroup_init()
{
	ui_FootGroup = lv_obj_create(ui_BGImage);
	lv_obj_set_width(ui_FootGroup, 720);
	lv_obj_set_height(ui_FootGroup, 134);
	lv_obj_set_x(ui_FootGroup, 0);
	lv_obj_set_y(ui_FootGroup, 565);
	lv_obj_set_align(ui_FootGroup, LV_ALIGN_CENTER);
	//lv_obj_set_align(ui_FootGroup, LV_ALIGN_BOTTOM_MID);

	lv_obj_clear_flag(ui_FootGroup, LV_OBJ_FLAG_SCROLLABLE);	  /// Flags
	lv_obj_set_style_bg_color(ui_FootGroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_FootGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_FootGroup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_FootGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_ErrorGroup = lv_obj_create(ui_FootGroup);
	lv_obj_set_width(ui_ErrorGroup, 130);
	lv_obj_set_height(ui_ErrorGroup, 98);
	lv_obj_set_x(ui_ErrorGroup, -256);
	lv_obj_set_y(ui_ErrorGroup, -8);//18
	lv_obj_set_align(ui_ErrorGroup, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_ErrorGroup, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_bg_color(ui_ErrorGroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_ErrorGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_ErrorGroup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_ErrorGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_errorImage = lv_img_create(ui_ErrorGroup);
	lv_img_set_src(ui_errorImage,LVGL_PATH(error.png));
	lv_obj_set_width(ui_errorImage, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_errorImage, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_errorImage, 2);
	lv_obj_set_y(ui_errorImage, -18);
	lv_obj_set_align(ui_errorImage, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_errorImage, LV_OBJ_FLAG_ADV_HITTEST);	 /// Flags
	lv_obj_clear_flag(ui_errorImage, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags

	ui_errorLabel = lv_label_create(ui_ErrorGroup);
	lv_obj_set_width(ui_errorLabel, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_errorLabel, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_errorLabel, 1);
	lv_obj_set_y(ui_errorLabel, 32);
	lv_obj_set_align(ui_errorLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_errorLabel, "ERROR");
	lv_obj_set_style_text_color(ui_errorLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_errorLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_errorLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);


	lv_obj_add_event_cb(ui_errorImage, aio_footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_errorLabel, aio_footlabel_event, LV_EVENT_ALL, NULL);
	
	ui_CautionGroup = lv_obj_create(ui_FootGroup);
	lv_obj_set_width(ui_CautionGroup, 145);
	lv_obj_set_height(ui_CautionGroup, 98);
	lv_obj_set_x(ui_CautionGroup, -101);
	lv_obj_set_y(ui_CautionGroup, -8);//18
	lv_obj_set_align(ui_CautionGroup, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_CautionGroup, LV_OBJ_FLAG_SCROLLABLE); 	 /// Flags
	lv_obj_set_style_bg_color(ui_CautionGroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_CautionGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_CautionGroup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_CautionGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_cautionImage = lv_img_create(ui_CautionGroup);
	lv_img_set_src(ui_cautionImage, LVGL_PATH(CAUTION.png));
	lv_obj_set_width(ui_cautionImage, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_cautionImage, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_x(ui_cautionImage, -2);
	lv_obj_set_y(ui_cautionImage, -18);
	lv_obj_set_align(ui_cautionImage, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_cautionImage, LV_OBJ_FLAG_ADV_HITTEST);	   /// Flags
	lv_obj_clear_flag(ui_cautionImage, LV_OBJ_FLAG_SCROLLABLE); 	 /// Flags

	ui_cautionLabel = lv_label_create(ui_CautionGroup);
	lv_obj_set_width(ui_cautionLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_cautionLabel, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_x(ui_cautionLabel, 3);
	lv_obj_set_y(ui_cautionLabel, 32);
	lv_obj_set_align(ui_cautionLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_cautionLabel, "CAUTION");
	lv_obj_set_style_text_color(ui_cautionLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_cautionLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_cautionLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_cautionImage, aio_footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_cautionLabel, aio_footlabel_event, LV_EVENT_ALL, NULL);
		
	ui_BluetoothGroup = lv_obj_create(ui_FootGroup);
	lv_obj_set_width(ui_BluetoothGroup, 190);
	lv_obj_set_height(ui_BluetoothGroup, 98);
	lv_obj_set_x(ui_BluetoothGroup, 81);
	lv_obj_set_y(ui_BluetoothGroup, -8);//18
	lv_obj_set_align(ui_BluetoothGroup, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_BluetoothGroup, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_bg_color(ui_BluetoothGroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BluetoothGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BluetoothGroup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BluetoothGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_bluetoothImage = lv_img_create(ui_BluetoothGroup);
	lv_img_set_src(ui_bluetoothImage,LVGL_PATH(BLUETOOTH.png));
	lv_obj_set_width(ui_bluetoothImage, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_bluetoothImage, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_bluetoothImage, -7);
	lv_obj_set_y(ui_bluetoothImage, -18);
	lv_obj_set_align(ui_bluetoothImage, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_bluetoothImage, LV_OBJ_FLAG_ADV_HITTEST);	 /// Flags
	lv_obj_clear_flag(ui_bluetoothImage, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags

	ui_bluetoothLabel = lv_label_create(ui_BluetoothGroup);
	lv_obj_set_width(ui_bluetoothLabel, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_bluetoothLabel, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_bluetoothLabel, 0);
	lv_obj_set_y(ui_bluetoothLabel, 36);
	lv_obj_set_align(ui_bluetoothLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_bluetoothLabel, "BLUETOOTH");
	lv_obj_set_style_text_color(ui_bluetoothLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_bluetoothLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_bluetoothLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_bluetoothImage, aio_footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_bluetoothLabel, aio_footlabel_event, LV_EVENT_ALL, NULL);
		   

	
	ui_WiFiGroup = lv_obj_create(ui_FootGroup);
	lv_obj_set_width(ui_WiFiGroup, 131);
	lv_obj_set_height(ui_WiFiGroup, 98);
	lv_obj_set_x(ui_WiFiGroup, 261);
	lv_obj_set_y(ui_WiFiGroup, -8);//18
	lv_obj_set_align(ui_WiFiGroup, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_WiFiGroup, LV_OBJ_FLAG_SCROLLABLE);	  /// Flags
	lv_obj_set_style_bg_color(ui_WiFiGroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_WiFiGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_WiFiGroup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_WiFiGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_wifiImage = lv_img_create(ui_WiFiGroup);
	lv_img_set_src(ui_wifiImage, LVGL_PATH(WiFi.png));
	lv_obj_set_width(ui_wifiImage, LV_SIZE_CONTENT);   /// -59
	lv_obj_set_height(ui_wifiImage, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_wifiImage, -13);
	lv_obj_set_y(ui_wifiImage, -20);
	lv_obj_set_align(ui_wifiImage, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_wifiImage, LV_OBJ_FLAG_ADV_HITTEST); 	/// Flags
	lv_obj_clear_flag(ui_wifiImage, LV_OBJ_FLAG_SCROLLABLE);	  /// Flags

	ui_wifiLabel = lv_label_create(ui_WiFiGroup);
	lv_obj_set_width(ui_wifiLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_wifiLabel, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_wifiLabel, -12);
	lv_obj_set_y(ui_wifiLabel, 34);
	lv_obj_set_align(ui_wifiLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_wifiLabel, "WiFi");
	lv_obj_set_style_text_color(ui_wifiLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_wifiLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_wifiLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_wifiImage, aio_footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_wifiLabel, aio_footlabel_event, LV_EVENT_ALL, NULL);

}

void aio_headgroup_init()
{
	ui_HeadGroup = lv_obj_create(ui_BGImage);
	lv_obj_set_width(ui_HeadGroup, 720);
	lv_obj_set_height(ui_HeadGroup, 781);
	lv_obj_set_align(ui_HeadGroup, LV_ALIGN_TOP_MID);
	lv_obj_clear_flag(ui_HeadGroup, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_bg_color(ui_HeadGroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_HeadGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_HeadGroup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_HeadGroup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_panel = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_panel, LVGL_PATH(panel.png));
	lv_obj_set_width(ui_panel, 150);
	lv_obj_set_height(ui_panel, 151);
	lv_obj_set_x(ui_panel, 105);
	lv_obj_set_y(ui_panel, 29);
	lv_obj_add_flag(ui_panel, LV_OBJ_FLAG_ADV_HITTEST);	 /// Flags
	lv_obj_clear_flag(ui_panel, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags

	ui_grid = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_grid, LVGL_PATH(grid.png));
	lv_obj_set_width(ui_grid, 150);
	lv_obj_set_height(ui_grid, 150);
	lv_obj_set_x(ui_grid, -132);
	lv_obj_set_y(ui_grid, 28);
	lv_obj_set_align(ui_grid, LV_ALIGN_TOP_RIGHT);
	lv_obj_add_flag(ui_grid, LV_OBJ_FLAG_ADV_HITTEST); 	/// Flags
	lv_obj_clear_flag(ui_grid, LV_OBJ_FLAG_SCROLLABLE);	  /// Flags

	ui_auto = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_auto, LVGL_PATH(auto.png));
	lv_obj_set_width(ui_auto, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_auto, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_auto, -158);
	lv_obj_set_y(ui_auto, 313);
	lv_obj_set_align(ui_auto, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_auto, LV_OBJ_FLAG_ADV_HITTEST); 	/// Flags
	lv_obj_clear_flag(ui_auto, LV_OBJ_FLAG_SCROLLABLE);	  /// Flags

	ui_BatteryName = lv_label_create(ui_HeadGroup);
	lv_obj_set_width(ui_BatteryName, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_BatteryName, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_x(ui_BatteryName, -7);
	lv_obj_set_y(ui_BatteryName, 129);
	lv_obj_set_align(ui_BatteryName, LV_ALIGN_CENTER);
	lv_label_set_text(ui_BatteryName, "BATTERY");
	lv_obj_set_style_text_color(ui_BatteryName, lv_color_hex(0xFBFAFA), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_BatteryName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_BatteryName, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_BATTERY = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_BATTERY, LVGL_PATH(BATTERY.png));
	lv_obj_set_width(ui_BATTERY, 161);
	lv_obj_set_height(ui_BATTERY, 161);
	lv_obj_set_x(ui_BATTERY, -14);
	lv_obj_set_y(ui_BATTERY, 23);
	lv_obj_set_align(ui_BATTERY, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_BATTERY, LV_OBJ_FLAG_ADV_HITTEST);	   /// Flags
	lv_obj_clear_flag(ui_BATTERY, LV_OBJ_FLAG_SCROLLABLE); 	 /// Flags

	ui_BatteryText = lv_label_create(ui_BATTERY);
	lv_obj_set_width(ui_BatteryText, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_BatteryText, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_x(ui_BatteryText, 1);
	lv_obj_set_y(ui_BatteryText, 34);
	lv_obj_set_align(ui_BatteryText, LV_ALIGN_CENTER);
	lv_label_set_text(ui_BatteryText, "0%");
	lv_obj_set_style_text_color(ui_BatteryText, lv_color_hex(0xFBFAFA), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_BatteryText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_BatteryText, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_BatteryText, battery_text_event, LV_EVENT_ALL, NULL);


	ui_home = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_home, LVGL_PATH(home.png));
	lv_obj_set_width(ui_home, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_home, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_home, 132);
	lv_obj_set_y(ui_home, 313);
	lv_obj_set_align(ui_home, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_home, LV_OBJ_FLAG_ADV_HITTEST); 	/// Flags
	lv_obj_clear_flag(ui_home, LV_OBJ_FLAG_SCROLLABLE);	  /// Flags

	ui_panelpoint = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_panelpoint, LVGL_PATH(panel/panel21.png));
	lv_obj_set_width(ui_panelpoint, 74);
	lv_obj_set_height(ui_panelpoint, 185);
	lv_obj_set_x(ui_panelpoint, 176);
	lv_obj_set_y(ui_panelpoint, 179);
	lv_obj_add_flag(ui_panelpoint, LV_OBJ_FLAG_ADV_HITTEST);	  /// Flags
	lv_obj_clear_flag(ui_panelpoint, LV_OBJ_FLAG_SCROLLABLE);		/// Flags

	lv_obj_add_flag(ui_panelpoint, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_event_cb(ui_panelpoint, panel_point_event, LV_EVENT_ALL, NULL);


	ui_panelName = lv_label_create(ui_HeadGroup);
	lv_obj_set_width(ui_panelName, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_panelName, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_panelName, -222);
	lv_obj_set_y(ui_panelName, -152);
	lv_obj_set_align(ui_panelName, LV_ALIGN_CENTER);
	lv_label_set_text(ui_panelName, "Panel\n");
	lv_obj_set_style_text_color(ui_panelName, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_panelName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui_panelName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_panelName, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_gridpoint = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_gridpoint, LVGL_PATH(grid/gridgreen21.png));
	lv_obj_set_width(ui_gridpoint, 74);
	lv_obj_set_height(ui_gridpoint, 185);
	lv_obj_set_x(ui_gridpoint, 98);
	lv_obj_set_y(ui_gridpoint, -97);
	lv_obj_set_align(ui_gridpoint, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_gridpoint, LV_OBJ_FLAG_ADV_HITTEST);	 /// Flags
	lv_obj_clear_flag(ui_gridpoint, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_border_color(ui_gridpoint, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_gridpoint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_flag(ui_gridpoint, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_event_cb(ui_gridpoint, grid_point_event, LV_EVENT_ALL, NULL);

	ui_gridName = lv_label_create(ui_HeadGroup);
	lv_obj_set_width(ui_gridName, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_gridName, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_gridName, 211);
	lv_obj_set_y(ui_gridName, -150);
	lv_obj_set_align(ui_gridName, LV_ALIGN_CENTER);
	lv_label_set_text(ui_gridName, "Grid\n");
	lv_obj_set_style_text_color(ui_gridName, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_gridName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui_gridName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_gridName, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_autoName = lv_label_create(ui_HeadGroup);
	lv_obj_set_width(ui_autoName, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_autoName, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_autoName, -230);
	lv_obj_set_y(ui_autoName, 215);
	lv_obj_set_align(ui_autoName, LV_ALIGN_CENTER);
	lv_label_set_text(ui_autoName, "Auto");
	lv_obj_set_style_text_color(ui_autoName, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_autoName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui_autoName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_autoName, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_homeName = lv_label_create(ui_HeadGroup);
	lv_obj_set_width(ui_homeName, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_homeName, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_homeName, 221);
	lv_obj_set_y(ui_homeName, 217);
	lv_obj_set_align(ui_homeName, LV_ALIGN_CENTER);
	lv_label_set_text(ui_homeName, "Home");
	lv_obj_set_style_text_color(ui_homeName, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_homeName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui_homeName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_homeName, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_autopoint = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_autopoint, LVGL_PATH(auto/auto21.png));
	lv_obj_set_width(ui_autopoint, 74);
	lv_obj_set_height(ui_autopoint, 185);
	lv_obj_set_x(ui_autopoint, -125);
	lv_obj_set_y(ui_autopoint, 141);
	lv_obj_set_align(ui_autopoint, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_autopoint, LV_OBJ_FLAG_ADV_HITTEST);	 /// Flags
	lv_obj_clear_flag(ui_autopoint, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_add_flag(ui_autopoint, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_event_cb(ui_autopoint, auto_point_event, LV_EVENT_ALL, NULL);

	ui_homepoint = lv_img_create(ui_HeadGroup);
	lv_img_set_src(ui_homepoint, LVGL_PATH(home/home21.png));
	lv_obj_set_width(ui_homepoint, 74);
	lv_obj_set_height(ui_homepoint, 185);
	lv_obj_set_x(ui_homepoint, 100);
	lv_obj_set_y(ui_homepoint, 141);
	lv_obj_set_align(ui_homepoint, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_homepoint, LV_OBJ_FLAG_ADV_HITTEST);	 /// Flags
	lv_obj_clear_flag(ui_homepoint, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_add_flag(ui_homepoint, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_event_cb(ui_homepoint, home_point_event, LV_EVENT_ALL, NULL);

}

 void ui_Screen1_screen_init(void)
{
#if 0
	ui_Screen1 = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE); 	 /// Flags


	ui_BGImage = lv_img_create(ui_Screen1);
	lv_img_set_src(ui_BGImage, &ui_img_bg_png);
	lv_obj_set_width(ui_BGImage, 720);
	lv_obj_set_height(ui_BGImage, 1280);
	lv_obj_set_align(ui_BGImage, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_BGImage, LV_OBJ_FLAG_ADV_HITTEST);	  /// Flags
	lv_obj_clear_flag(ui_BGImage, LV_OBJ_FLAG_SCROLLABLE);		/// Flags  
#else
	ui_BGImage = lv_img_create(lv_scr_act());
	lv_img_set_src(ui_BGImage,LVGL_PATH(bg.png));

	lv_obj_set_pos(ui_BGImage, 0, 0);
	lv_obj_set_size(ui_BGImage, 720,1280);

#endif

	ui_centerPanel = lv_obj_create(ui_BGImage);
	lv_obj_set_width(ui_centerPanel, 720);
	lv_obj_set_height(ui_centerPanel, 321);
	lv_obj_set_x(ui_centerPanel, 0);
	lv_obj_set_y(ui_centerPanel, 319);//353
	lv_obj_set_align(ui_centerPanel, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_centerPanel, LV_OBJ_FLAG_SCROLLABLE); 	 /// Flags
	lv_obj_set_style_bg_color(ui_centerPanel, lv_color_hex(0x282826), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_centerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_centerPanel, lv_color_hex(0x282826), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_centerPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_ver = lv_label_create(ui_BGImage);
	lv_obj_set_width(ui_ver, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_ver, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_ver, 0);
	lv_obj_set_y(ui_ver,20);
	lv_obj_set_align(ui_ver, LV_ALIGN_TOP_LEFT);
	lv_label_set_text(ui_ver, version);
	lv_obj_set_style_text_color(ui_ver, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_ver, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_ver, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);


	ui_panellistPanel = lv_obj_create(ui_centerPanel);
	lv_obj_set_width(ui_panellistPanel, 720);
	lv_obj_set_height(ui_panellistPanel, 50);
	lv_obj_set_x(ui_panellistPanel, 0);
	lv_obj_set_y(ui_panellistPanel, -20);
	lv_obj_set_align(ui_panellistPanel, LV_ALIGN_TOP_MID);
	lv_obj_clear_flag(ui_panellistPanel, LV_OBJ_FLAG_SCROLLABLE);		/// Flags
	lv_obj_set_style_bg_color(ui_panellistPanel, lv_color_hex(0x282826), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_panellistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_panellistPanel, lv_color_hex(0x282826), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_panellistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_panelLabel = lv_label_create(ui_panellistPanel);
	lv_obj_set_width(ui_panelLabel, 190);
	lv_obj_set_height(ui_panelLabel, 36);
	lv_obj_set_x(ui_panelLabel, 39);
	lv_obj_set_y(ui_panelLabel, -15);
	lv_label_set_text(ui_panelLabel, "Panel Power");
	lv_obj_set_style_text_color(ui_panelLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_panelLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_panelLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_panelBar = lv_bar_create(ui_panellistPanel);
	lv_bar_set_value(ui_panelBar, 0, LV_ANIM_OFF);
	lv_bar_set_start_value(ui_panelBar, 0, LV_ANIM_OFF);
	lv_obj_set_width(ui_panelBar, 230);//290
	lv_obj_set_height(ui_panelBar, 18);
	lv_obj_set_x(ui_panelBar, 23);//55
	lv_obj_set_y(ui_panelBar, 1);
	lv_obj_set_align(ui_panelBar, LV_ALIGN_CENTER);
	lv_obj_set_style_bg_color(ui_panelBar, lv_color_hex(0x282826), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_panelBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_panelBar, lv_color_hex(0x282826), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_panelBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui_panelBar, lv_color_hex(0x111513), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_panelBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui_panelBar, lv_color_hex(0x0BBB25), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui_panelBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

#if 0
	ui_panelbarLabel = lv_label_create(ui_panellistPanel);
	lv_obj_set_width(ui_panelbarLabel, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_panelbarLabel, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_panelbarLabel, -85);
	lv_obj_set_y(ui_panelbarLabel, -4);
	lv_obj_set_align(ui_panelbarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_panelbarLabel, "0.");
	lv_obj_set_style_text_color(ui_panelbarLabel, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_panelbarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_panelbarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_panelbarLabel2 = lv_label_create(ui_panellistPanel);
	lv_obj_set_width(ui_panelbarLabel2, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_panelbarLabel2, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_panelbarLabel2, 598);
	lv_obj_set_y(ui_panelbarLabel2, -4);
	lv_obj_set_align(ui_panelbarLabel2, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_panelbarLabel2, "0kW");
	lv_obj_set_style_text_color(ui_panelbarLabel2, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_panelbarLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_panelbarLabel2, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_panelBar, bar_event_cb, LV_EVENT_ALL, ui_panelbarLabel);
#else
	ui_panelbarLabel = lv_label_create(ui_panellistPanel);
	lv_obj_set_width(ui_panelbarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_panelbarLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_panelbarLabel, -40);//8
	lv_obj_set_y(ui_panelbarLabel, 1);
	lv_obj_set_align(ui_panelbarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_panelbarLabel, "0.0kW");
	lv_obj_set_style_text_color(ui_panelbarLabel, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_panelbarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_panelbarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_panelBar, bar_event_cb, LV_EVENT_ALL, ui_panelbarLabel);

#endif
	ui_gridlistPanel = lv_obj_create(ui_centerPanel);
	lv_obj_set_width(ui_gridlistPanel, 720);
	lv_obj_set_height(ui_gridlistPanel, 50);
	lv_obj_set_x(ui_gridlistPanel, 0);
	lv_obj_set_y(ui_gridlistPanel, -63);
	lv_obj_set_align(ui_gridlistPanel, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_gridlistPanel, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_bg_color(ui_gridlistPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_gridlistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_gridlistPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_gridlistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_gridBar = lv_bar_create(ui_gridlistPanel);
	lv_bar_set_value(ui_gridBar, 0, LV_ANIM_OFF);
	lv_bar_set_start_value(ui_gridBar, 0, LV_ANIM_OFF);
	lv_obj_set_width(ui_gridBar, 230);//290
	lv_obj_set_height(ui_gridBar, 18);
	lv_obj_set_x(ui_gridBar, 23);
	lv_obj_set_y(ui_gridBar, -4);
	lv_obj_set_align(ui_gridBar, LV_ALIGN_CENTER);
	lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_gridBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);//
	lv_obj_set_style_border_color(ui_gridBar, lv_color_hex(0x241517), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_gridBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	// lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0x143F27), LV_PART_INDICATOR | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui_gridBar, lv_color_hex(0x131111), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_gridBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui_gridBar, lv_color_hex(0xA10422), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui_gridBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

	ui_gridLabel = lv_label_create(ui_gridlistPanel);
	lv_obj_set_width(ui_gridLabel, 190);
	lv_obj_set_height(ui_gridLabel, 36);
	lv_obj_set_x(ui_gridLabel, 39);
	lv_obj_set_y(ui_gridLabel, -15);
	lv_label_set_text(ui_gridLabel, "Grid Power");
	lv_obj_set_style_text_color(ui_gridLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_gridLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_gridLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
#if 0
	ui_gridbarLabel = lv_label_create(ui_gridlistPanel);
	lv_obj_set_width(ui_gridbarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_gridbarLabel, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_gridbarLabel, -85);
	lv_obj_set_y(ui_gridbarLabel, -4);
	lv_obj_set_align(ui_gridbarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_gridbarLabel, "0.");
	lv_obj_set_style_text_color(ui_gridbarLabel, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_gridbarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_gridbarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_gridbarLabel2 = lv_label_create(ui_gridlistPanel);
	lv_obj_set_width(ui_gridbarLabel2, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_gridbarLabel2, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_gridbarLabel2, 598);
	lv_obj_set_y(ui_gridbarLabel2, -4);
	lv_obj_set_align(ui_gridbarLabel2, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_gridbarLabel2, "0kW");
	lv_obj_set_style_text_color(ui_gridbarLabel2, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_gridbarLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_gridbarLabel2, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_gridBar, bar_event_cb, LV_EVENT_ALL, ui_gridbarLabel);
#else
	ui_gridbarLabel = lv_label_create(ui_gridlistPanel);
	lv_obj_set_width(ui_gridbarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_gridbarLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_gridbarLabel, -40);
	lv_obj_set_y(ui_gridbarLabel, -2);
	lv_obj_set_align(ui_gridbarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_gridbarLabel, "0.0kW");
	lv_obj_set_style_text_color(ui_gridbarLabel, lv_color_hex(0xBE062A), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_gridbarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_gridbarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_gridBar, bar_event_cb, LV_EVENT_ALL, ui_gridbarLabel);

#endif
	ui_autolistPanel = lv_obj_create(ui_centerPanel);
	lv_obj_set_width(ui_autolistPanel, 720);
	lv_obj_set_height(ui_autolistPanel, 50);
	lv_obj_set_x(ui_autolistPanel, 0);
	lv_obj_set_y(ui_autolistPanel, 3);
	lv_obj_set_align(ui_autolistPanel, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_autolistPanel, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_bg_color(ui_autolistPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_autolistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_autolistPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_autolistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_autoLabel = lv_label_create(ui_autolistPanel);
	lv_obj_set_width(ui_autoLabel, 210);
	lv_obj_set_height(ui_autoLabel, 36);
	lv_obj_set_x(ui_autoLabel, 39);
	lv_obj_set_y(ui_autoLabel, -15);
	lv_label_set_text(ui_autoLabel, "Consumption");
	lv_obj_set_style_text_color(ui_autoLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_autoLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_autoLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_autoBar = lv_bar_create(ui_autolistPanel);
	lv_bar_set_value(ui_autoBar, 0, LV_ANIM_OFF);
	lv_bar_set_start_value(ui_autoBar, 0, LV_ANIM_OFF);
	lv_obj_set_width(ui_autoBar, 230);
	lv_obj_set_height(ui_autoBar, 18);
	lv_obj_set_x(ui_autoBar, 23);
	lv_obj_set_y(ui_autoBar, -4);
	lv_obj_set_align(ui_autoBar, LV_ALIGN_CENTER);
	lv_obj_set_style_bg_color(ui_autoBar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_autoBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);//
	lv_obj_set_style_border_color(ui_autoBar, lv_color_hex(0x0E1812), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_autoBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui_autoBar, lv_color_hex(0x151818), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_autoBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui_autoBar, lv_color_hex(0x55E3C9), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui_autoBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);
#if 0
	ui_autobarLabel = lv_label_create(ui_autolistPanel);
	lv_obj_set_width(ui_autobarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_autobarLabel, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_autobarLabel, -85);
	lv_obj_set_y(ui_autobarLabel, -4);
	lv_obj_set_align(ui_autobarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_autobarLabel, "0.");
	lv_obj_set_style_text_color(ui_autobarLabel, lv_color_hex(0x55E3C9), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_autobarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_autobarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_autobarLabel2 = lv_label_create(ui_autolistPanel);
	lv_obj_set_width(ui_autobarLabel2, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_autobarLabel2, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_autobarLabel2, 598);
	lv_obj_set_y(ui_autobarLabel2, -4);
	lv_obj_set_align(ui_autobarLabel2, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_autobarLabel2, "0kWh");
	lv_obj_set_style_text_color(ui_autobarLabel2, lv_color_hex(0x55E3C9), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_autobarLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_autobarLabel2, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_autoBar, bar_event_cb, LV_EVENT_ALL, ui_autobarLabel);
#else
	ui_autobarLabel = lv_label_create(ui_autolistPanel);
	lv_obj_set_width(ui_autobarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_autobarLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_autobarLabel, -40);
	lv_obj_set_y(ui_autobarLabel, -4);
	lv_obj_set_align(ui_autobarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_autobarLabel, "0.0kWh");
	lv_obj_set_style_text_color(ui_autobarLabel, lv_color_hex(0x55E3C9), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_autobarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_autobarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_autoBar, bar_event_cb, LV_EVENT_ALL, ui_autobarLabel);

#endif
	ui_homelistPanel = lv_obj_create(ui_centerPanel);
	lv_obj_set_width(ui_homelistPanel, 720);
	lv_obj_set_height(ui_homelistPanel, 60);
	lv_obj_set_x(ui_homelistPanel, 0);
	lv_obj_set_y(ui_homelistPanel, 68);
	lv_obj_set_align(ui_homelistPanel, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_homelistPanel, LV_OBJ_FLAG_SCROLLABLE);	   /// Flags
	lv_obj_set_style_bg_color(ui_homelistPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_homelistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_homelistPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_homelistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_homeLabel = lv_label_create(ui_homelistPanel);
	lv_obj_set_width(ui_homeLabel, 190);
	lv_obj_set_height(ui_homeLabel, 60);
	lv_obj_set_x(ui_homeLabel, 39);
	lv_obj_set_y(ui_homeLabel, -20);
	lv_label_set_text(ui_homeLabel, "Self sufficiency");
	lv_obj_set_style_text_color(ui_homeLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_homeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_homeLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_homeBar = lv_bar_create(ui_homelistPanel);
	lv_bar_set_value(ui_homeBar, 0, LV_ANIM_OFF);
	lv_bar_set_start_value(ui_homeBar, 0, LV_ANIM_OFF);
	lv_obj_set_width(ui_homeBar, 230);//300
	lv_obj_set_height(ui_homeBar, 18);
	lv_obj_set_x(ui_homeBar, 23);
	lv_obj_set_y(ui_homeBar, -4);
	lv_obj_set_align(ui_homeBar, LV_ALIGN_CENTER);
	lv_obj_set_style_bg_color(ui_homeBar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_homeBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);//
	lv_obj_set_style_border_color(ui_homeBar, lv_color_hex(0x0E1812), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_homeBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	//  lv_obj_set_style_bg_color(ui_homeBar, lv_color_hex(0x143F27), LV_PART_INDICATOR | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui_homeBar, lv_color_hex(0x151718), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_homeBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui_homeBar, lv_color_hex(0x306B7E), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui_homeBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

#if 0
	ui_homebarLabel = lv_label_create(ui_homelistPanel);
	lv_obj_set_width(ui_homebarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_homebarLabel, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_x(ui_homebarLabel, -85);
	lv_obj_set_y(ui_homebarLabel, -4);
	lv_obj_set_align(ui_homebarLabel, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(ui_homebarLabel, "0.");
	lv_obj_set_style_text_color(ui_homebarLabel, lv_color_hex(0x306B7E), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_homebarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_homebarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_homebarLabel2 = lv_label_create(ui_homelistPanel);
	lv_obj_set_width(ui_homebarLabel2, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_homebarLabel2, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_homebarLabel2, 598);
	lv_obj_set_y(ui_homebarLabel2, -4);
	lv_obj_set_align(ui_homebarLabel2, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_homebarLabel2, "0");

	lv_obj_set_style_text_color(ui_homebarLabel2, lv_color_hex(0x306B7E), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_homebarLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_homebarLabel2, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_homeBar, bar_event_cb, LV_EVENT_ALL, ui_homebarLabel);
#else
	ui_homebarLabel = lv_label_create(ui_homelistPanel);
	lv_obj_set_width(ui_homebarLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_homebarLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_homebarLabel, -40);//598
	lv_obj_set_y(ui_homebarLabel, -1);//-4
	lv_obj_set_align(ui_homebarLabel, LV_ALIGN_RIGHT_MID);//LV_ALIGN_LEFT_MID
	lv_label_set_text(ui_homebarLabel, "0.0");

	lv_obj_set_style_text_color(ui_homebarLabel, lv_color_hex(0x306B7E), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_homebarLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_homebarLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_homeBar, bar_event_cb, LV_EVENT_ALL, ui_homebarLabel);

#endif
	ui_eventlistPanel = lv_obj_create(ui_centerPanel);
	lv_obj_set_width(ui_eventlistPanel, 720);
	lv_obj_set_height(ui_eventlistPanel, 50);
	lv_obj_set_x(ui_eventlistPanel, 0);
	lv_obj_set_y(ui_eventlistPanel, 131);
	lv_obj_set_align(ui_eventlistPanel, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_eventlistPanel, LV_OBJ_FLAG_SCROLLABLE);		/// Flags
	lv_obj_set_style_bg_color(ui_eventlistPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_eventlistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_eventlistPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_eventlistPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_eventValueLabel = lv_label_create(ui_eventlistPanel);
#if 0
	lv_obj_set_width(ui_eventValueLabel, LV_SIZE_CONTENT);   /// 1
	lv_obj_set_height(ui_eventValueLabel, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_x(ui_eventValueLabel, 269);
	lv_obj_set_y(ui_eventValueLabel, -4);
	lv_obj_set_align(ui_eventValueLabel, LV_ALIGN_CENTER);
	lv_label_set_text(ui_eventValueLabel, "124AB,124AB");
#else
	lv_obj_set_width(ui_eventValueLabel, 512);
	lv_obj_set_height(ui_eventValueLabel, 32);
	lv_obj_set_x(ui_eventValueLabel, -25);
	lv_obj_set_y(ui_eventValueLabel, -4);
	lv_obj_set_align(ui_eventValueLabel, LV_ALIGN_RIGHT_MID);
	lv_obj_add_flag(ui_eventValueLabel,LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
	lv_label_set_long_mode(ui_eventValueLabel, LV_LABEL_LONG_SCROLL_CIRCULAR/*LV_LABEL_LONG_WRAP*/);
	//lv_obj_set_style_text_align(ui_eventValueLabel, LV_TEXT_ALIGN_RIGHT, 0);
	//lv_obj_set_style_anim_speed(ui_eventValueLabel, 20, 0);
	lv_label_set_text(ui_eventValueLabel, "   ");
	//lv_label_set_text(ui_eventValueLabel, "1235,23AB");
#endif
	lv_obj_set_style_text_color(ui_eventValueLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_eventValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui_eventValueLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_eventValueLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_eventLabel = lv_label_create(ui_eventlistPanel);
	lv_obj_set_width(ui_eventLabel, LV_SIZE_CONTENT);	 /// 1
	lv_obj_set_height(ui_eventLabel, LV_SIZE_CONTENT);    /// 1
	lv_obj_set_x(ui_eventLabel, 39);
	lv_obj_set_y(ui_eventLabel, -15);
	lv_label_set_text(ui_eventLabel, "Event");
	lv_obj_set_style_text_color(ui_eventLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_eventLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_eventLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_eventValueLabel, show_event_event, LV_EVENT_ALL, NULL);



	ui_line1Image = lv_img_create(ui_BGImage);
	lv_img_set_src(ui_line1Image, LVGL_PATH(line1.png));
	lv_obj_set_width(ui_line1Image, 600);
	lv_obj_set_height(ui_line1Image, 4);
	lv_obj_set_x(ui_line1Image, 0);
	lv_obj_set_y(ui_line1Image, 154);//185
	lv_obj_set_align(ui_line1Image, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_line1Image, LV_OBJ_FLAG_ADV_HITTEST);	  /// Flags
	lv_obj_clear_flag(ui_line1Image, LV_OBJ_FLAG_SCROLLABLE);		/// Flags

	ui_line2Image = lv_img_create(ui_BGImage);
	lv_img_set_src(ui_line2Image, LVGL_PATH(line2.png));
	lv_obj_set_width(ui_line2Image, 600);
	lv_obj_set_height(ui_line2Image, 3);
	lv_obj_set_x(ui_line2Image, 0);
	lv_obj_set_y(ui_line2Image, 485);//518
	lv_obj_set_align(ui_line2Image, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_line2Image, LV_OBJ_FLAG_ADV_HITTEST);	  /// Flags
	lv_obj_clear_flag(ui_line2Image, LV_OBJ_FLAG_SCROLLABLE);		/// Flags

	/*ui_ponitImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_ponitImage, LVGL_PATH(point.png));
    lv_obj_set_width(ui_ponitImage, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_ponitImage, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_ponitImage, 0);
    lv_obj_set_y(ui_ponitImage, 131);
    lv_obj_set_align(ui_ponitImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_ponitImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_ponitImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags*/
}


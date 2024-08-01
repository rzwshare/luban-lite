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
#include "lv_uart_run.h"
#include "aic_ui.h"
#include "lv_port_disp.h"
#include "mpp_fb.h"

#ifdef LPKG_USING_CPU_USAGE
#include "cpu_usage.h"
#endif
#include "sdbattery_ui.h"
#if 1
typedef struct {
	int cluster_battery_id;
    int cluster_battery_num;
	int cluster_battery1_soc;
	int cluster_battery2_soc;
	int cluster_battery3_soc;
	int cluster_battery4_soc;
   // int cluster_x;
	//int cluster_y;
	
} cluster_t;

typedef	struct  {
	    lv_obj_t  *battery_img;
		lv_obj_t  *battery_label;
		lv_obj_t  *battery_label1;
	    int battery_id;
 }battery_id_info_list_t;

static battery_id_info_list_t battery_id_info_list[12] ;
static battery_id_info_list_t old_battery_id_info_list[12] ;
#define CLUSTERNUM    3
cluster_t cluster[CLUSTERNUM];
cluster_t cluster_show[CLUSTERNUM];
void cluster_init(void);
#endif

static lv_obj_t *clusterimg[3][4];
static lv_obj_t *clusterlabel[3][4];
static lv_obj_t *weco;
static lv_obj_t *foot;
static lv_obj_t *clustername;

static lv_obj_t *clusterbar[3];
static lv_obj_t *cluster_soc_name[3];
static lv_obj_t *cluster_soc_value[3];
static int clusternum=0;
static int battery_num=-1;
extern EventDisplay eventDisplay;

#if 1

static void ui_Screen1_screen_init(void);

static lv_obj_t * ui_BGImage;
static lv_obj_t * ui_ErrorImage;
static lv_obj_t * ui_ErrorLabel;
static lv_obj_t * ui_CautionImage;
static lv_obj_t * ui_CautionLabel;
static lv_obj_t * ui_BluetoothImage;
static lv_obj_t * ui_BluetoothLabel;
static lv_obj_t * ui_ver;
static lv_obj_t * ui_ponitImage;
lv_obj_t * ui_WiFiImage;
lv_obj_t * ui_WiFiLabel;
lv_obj_t * ui_LineImage;
lv_obj_t * ui_LineImage2;
lv_obj_t * ui_ChargedLabel;
static lv_obj_t * ui_ChargedBar;
static lv_obj_t * ui_ChargedValueLabel;
lv_obj_t * ui_DisChargedLabel;
static lv_obj_t * ui_DisChargedBar;
static lv_obj_t * ui_DisChargedValueLabel;
static lv_obj_t * ui_VoltageLabel;
static lv_obj_t * ui_VoltageBar;
static lv_obj_t * ui_VoltageValueLabel;
lv_obj_t * ui_CurrentLabel;
static lv_obj_t * ui_CurrentBar;
lv_obj_t * ui_CurrentValueLabel;
lv_obj_t * ui_EventLabel;
lv_obj_t * ui_EventValueLabel;
lv_obj_t * ui_HeadGroup;

#endif


static int fault_state_flag=-1;
static int alarm_state_flag =-1;
static int bt_state=-1;
static int wifi_state=-1;
static int system_battery_state=-1;
static int system_charging_power=-1;
static int system_discharge_power=-1;
static int charging_animation_range=-1;
static int discharging_animation_range=-1;

static int system_voltage=-1;
static int system_current=-1;
static int cluster1_battery_soc=-1;
static int cluster2_battery_soc=-1;
static int cluster3_battery_soc=-1;
static int cluster1_battery_num=-1;
static int cluster2_battery_num=-1;
static int cluster3_battery_num=-1;
lv_obj_t * ui_Image;
lv_anim_t a;
static int dataisvalid=-1;


//#define SD5KO_TEST 
#ifdef SD5KO_TEST
#define version    "SD5KO_VERTEST_0.1"
#else
#define version    "SD5KO_VER_0.5"
#endif

static void footimg_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
		int num = lv_event_get_param(e);

		if(point==ui_ErrorImage)
		{
			if(num==0)
			{
			#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
			#endif
				lv_img_set_src(point,LVGL_PATH(ERROR.png));
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(ERROR1.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
				{
				#ifndef SD5KO_TEST
						lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
					lv_img_set_src(point,LVGL_PATH(ERROR.png));
				}
		}
		else if(point==ui_CautionImage)
		{
			if(num==0)
			{
				lv_img_set_src(point,LVGL_PATH(CAUTION.png));
				#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(CAUTION1.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_img_set_src(point,LVGL_PATH(CAUTION.png));
				#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
			}
				
		}
		else if(point==ui_BluetoothImage)
		{
			if(num==0)
			{
				lv_img_set_src(point,LVGL_PATH(BLUETOOTH.png));
				#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(BLUETOOTH1.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_img_set_src(point,LVGL_PATH(BLUETOOTH.png));
				#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
			}
		}
		else if(point==ui_WiFiImage)
		{
			if(num==0)
			{
				lv_img_set_src(point,LVGL_PATH(WiFi.png));
				#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
			}
			else if(num==1)
			{
				lv_img_set_src(point,LVGL_PATH(WiFi1.png));
				lv_obj_clear_flag(point, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_img_set_src(point,LVGL_PATH(WiFi.png));
				#ifndef SD5KO_TEST
				lv_obj_add_flag(point, LV_OBJ_FLAG_HIDDEN);
				#endif
			}
		}
	
    }
}
static void footlabel_event(lv_event_t * e)
{
    lv_obj_t * speed_num = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
				
    if(code == LV_EVENT_VALUE_CHANGED) {

		int num = lv_event_get_param(e);//

		if(speed_num==ui_ErrorLabel)
		{
			if(num==0)
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
				lv_obj_add_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
#endif
			}
			else
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_clear_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
			}
		}
		else if(speed_num==ui_CautionLabel)
		{
			if(num==0)
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
				lv_obj_add_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
#endif
			}
			else
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xFFf200), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_clear_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
			}
		}
		else if(speed_num==ui_BluetoothLabel||speed_num==ui_WiFiLabel)
		{
			if(num==0)
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
				lv_obj_add_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
#endif
			}
			else
			{
				lv_obj_set_style_text_color(speed_num, lv_color_hex(0x8aee49), LV_PART_MAIN | LV_STATE_DEFAULT);
				lv_obj_clear_flag(speed_num, LV_OBJ_FLAG_HIDDEN);
			}
		}
		
		
    }
}
static void main_speed_num_event(lv_event_t * e)
{
    lv_obj_t * speed_num = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
				
    if(code == LV_EVENT_VALUE_CHANGED) {

		 if(speed_num==ui_EventValueLabel){
	      if(eventDisplay.system_alarm_nums>0&&eventDisplay.system_fault_nums >0)
			lv_label_set_text_fmt(speed_num, "%s %s",eventDisplay.system_alarms,eventDisplay.system_faults);
		  else if(eventDisplay.system_alarm_nums==0&&eventDisplay.system_fault_nums >0)
			lv_label_set_text_fmt(speed_num, "%s",eventDisplay.system_faults);
		 else if(eventDisplay.system_alarm_nums>0&&eventDisplay.system_fault_nums ==0)
			lv_label_set_text_fmt(speed_num, "%s",eventDisplay.system_alarms);
		 else
		 	lv_label_set_text(speed_num, "");
		}
		else{
	        int num = lv_event_get_param(e);
			lv_label_set_text_fmt(speed_num, "%d %%",num);
		}
    }
}
bool findimg(lv_obj_t * point )
{
   for(int i=0;i<battery_num+1;i++)
   	{
   		if(battery_id_info_list[i].battery_img == point)
			return true;
   	}
	 return false;
}
static void main_point_event(lv_event_t * e)
{
    lv_obj_t * point = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED)
	{
		if(findimg(point))
		{
			int num = lv_event_get_param(e);
			char data_str[128];
			ui_snprintf(data_str, "%sbattery/battery_%02d.png", LVGL_DIR,(int)(num*0.38));
			lv_img_set_src(point, data_str);
		}
	}
}
static void set_value(void * bar, int32_t temp)
{
    lv_bar_set_value(bar, temp, LV_ANIM_ON);
}
void get_float(int data,int *integer,int *decimal)
{
	float temp = data*0.1;
	int a=(int)temp;
	int b=(temp-a)*10;

	*integer=a;
	*decimal=b;
	
}
static void obj_set_clear_hidden_flag(lv_obj_t *obj)
{
	if(lv_obj_has_flag(obj,LV_OBJ_FLAG_HIDDEN))
		lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
			
}
static void  bar_event_cb (lv_event_t * e)
{
	lv_obj_t * obj = lv_event_get_target(e);
	lv_obj_t *label=lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	char data_str[128];	
	if(code == LV_EVENT_VALUE_CHANGED||code==LV_EVENT_PRESSING)
	{
		int num = lv_event_get_param(e);
		
		if(obj==ui_ChargedBar)
		{	
#if 0
			lv_anim_t a;
			lv_anim_init(&a);
			lv_anim_set_var(&a, obj);
			lv_anim_set_values(&a,0,num*0.1);
			lv_anim_set_exec_cb(&a, set_value);
			lv_anim_set_time(&a,6000);
			lv_anim_set_repeat_count(&a, 1);
			lv_anim_set_repeat_delay(&a,0);
		    lv_anim_set_values(&a,0,999);
			lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
			if(system_battery_state==2)//0.null,1  discharged  2  charged
			{	
				lv_bar_set_value( obj,lv_bar_get_value(obj)+1, LV_ANIM_ON);
				lv_anim_start(&a);
			}
			else
			{
				 if(a.exec_cb != NULL) lv_anim_del(a.var, a.exec_cb);
				 lv_bar_set_value( obj,0, LV_ANIM_ON);
		   		 lv_bar_set_value( obj,num*0.1, LV_ANIM_ON);				
			}
#else
			lv_bar_set_range(obj,0,eventDisplay.charging_animation_range);
			lv_bar_set_value(obj,num*0.1, LV_ANIM_OFF);

#endif
			
			ui_snprintf(data_str, "%d.%d kWh", num/10, num%10);
			lv_label_set_text(label, data_str);
		}
		else if(obj==ui_DisChargedBar)
		{
#if 0
			lv_anim_t aa;
			lv_anim_init(&aa);
			lv_anim_set_var(&aa, obj);
			lv_anim_set_values(&aa,0,num*0.1);
			lv_anim_set_exec_cb(&aa, set_value);
			lv_anim_set_time(&aa,6000);
			lv_anim_set_repeat_count(&aa, 1);
			lv_anim_set_repeat_delay(&aa,0);

		    lv_anim_set_values(&aa,0,999);
			lv_anim_set_repeat_count(&aa, LV_ANIM_REPEAT_INFINITE);
			if(system_battery_state==1)//0.null,1  discharged  2  charged
			{	
				
				lv_bar_set_value( obj,lv_bar_get_value(obj)+1, LV_ANIM_ON);
				lv_anim_start(&aa);
			}
			else
			{
				 if(aa.exec_cb != NULL) lv_anim_del(aa.var, aa.exec_cb);
		
				 lv_bar_set_value( obj,0, LV_ANIM_ON);
		   		 lv_bar_set_value( obj,num*0.1, LV_ANIM_ON);
				
			}
#else
			lv_bar_set_range(obj,0,eventDisplay.discharging_animation_range);
			lv_bar_set_value(obj,num*0.1, LV_ANIM_OFF);

#endif		

			ui_snprintf(data_str, "%d.%d kWh", num/10, num%10);
			lv_label_set_text(label, data_str);
		}
		else if(obj==ui_VoltageBar||obj==ui_CurrentBar)
		{	
#if 0
		    lv_bar_set_value( obj,num*0.1, LV_ANIM_ON);
#else
			lv_bar_set_value( obj,num*0.1, LV_ANIM_OFF);

#endif

			if(obj==ui_VoltageBar){
				ui_snprintf(data_str, "%d.%d V", num/10, num%10);
			}else if(obj==ui_CurrentBar)
			{
			   ui_snprintf(data_str, "%d.%d A", num/10, num%10);
			}
			
			lv_label_set_text(label, data_str);
		}
		else if(obj==clusterbar[0]||obj==clusterbar[1]||obj==clusterbar[2])
		{	
		#if 0
		    lv_bar_set_value( obj,num*0.1, LV_ANIM_ON);
		#else
		    lv_bar_set_value( obj,num*0.1, LV_ANIM_OFF);
		#endif
		
			ui_snprintf(data_str, "%d.%d%%", num/10, num%10);
			lv_label_set_text(label, data_str);
			
		}
		
	}
	
}
void update_show_data(void)
{
	int num=-1;
    for(int i =0;i<clusternum;i++)
    {
		for(int j =0;j<cluster_show[i].cluster_battery_num;j++)
		{			
			switch(j)
			{
				case 0:
					num++;
					battery_id_info_list[num].battery_id=cluster_show[i].cluster_battery1_soc;
					break;
				case 1:
					num++;
					battery_id_info_list[num].battery_id=cluster_show[i].cluster_battery2_soc;
					break;
				case 2:
					num++;
					battery_id_info_list[num].battery_id=cluster_show[i].cluster_battery3_soc;
					break;
				case 3:
					num++;
					battery_id_info_list[num].battery_id=cluster_show[i].cluster_battery4_soc;
					break;
			}
		
   	 	}
	    
    }
}
void cluster_update_data(int id)
{
	//static uint8_t num[12]={0};
	 int count = (battery_num+1);
	for(int i=0;i<count;i++)
	{
		if(battery_id_info_list[i].battery_id!=old_battery_id_info_list[i].battery_id)
		{
			old_battery_id_info_list[i].battery_id=battery_id_info_list[i].battery_id;

			char data_str[128];
			ui_snprintf(data_str, "%sbattery/battery_%02d.png", LVGL_DIR,(int)(battery_id_info_list[i].battery_id/10*0.38));
			lv_img_set_src(battery_id_info_list[i].battery_img, data_str);
		//	printf("battery_id_info_list[%d].battery_id:%d,battery_num:%d\r\n",i,battery_id_info_list[i].battery_id,count);

	
			ui_snprintf(data_str, "%d.%d%%", battery_id_info_list[i].battery_id/10, battery_id_info_list[i].battery_id%10);
			lv_label_set_text(battery_id_info_list[i].battery_label, data_str);
		
		}
	}


	for(int j =0;j<clusternum;j++)
     {	
     	if(cluster_show[j].cluster_battery_id==0){
			if(eventDisplay.cluster1_battery_soc!=cluster1_battery_soc)
			{
			    cluster1_battery_soc=eventDisplay.cluster1_battery_soc;
				lv_event_send(clusterbar[j], LV_EVENT_VALUE_CHANGED,cluster1_battery_soc);
				
			}
     	}else if(cluster_show[j].cluster_battery_id==1){
			if(eventDisplay.cluster2_battery_soc!=cluster2_battery_soc)
			{
				cluster2_battery_soc=eventDisplay.cluster2_battery_soc;
				lv_event_send(clusterbar[j], LV_EVENT_VALUE_CHANGED,cluster2_battery_soc);
			}
     	}else if(cluster_show[j].cluster_battery_id==2){
			if(eventDisplay.cluster3_battery_soc!=cluster3_battery_soc)
			{
				cluster3_battery_soc=eventDisplay.cluster3_battery_soc;
				lv_event_send(clusterbar[j], LV_EVENT_VALUE_CHANGED,cluster3_battery_soc);
				
			}
     	}
	  }
}

void updateEventDisplay()
{
	if(eventDisplay.system_battery_state!=system_battery_state)
		system_battery_state=eventDisplay.system_battery_state;

	if(eventDisplay.system_charging_power!=system_charging_power){
			system_charging_power=eventDisplay.system_charging_power;
	}
	
	if(eventDisplay.system_discharge_power!=system_discharge_power){
			system_discharge_power=eventDisplay.system_discharge_power;
	}
	
	if(eventDisplay.system_voltage!=system_voltage){
		system_voltage=eventDisplay.system_voltage;
	}
	if(eventDisplay.system_current!=system_current){
		system_current=eventDisplay.system_current;
	}
		
	 for(int j =0;j<clusternum;j++)
     {	
     	if(cluster_show[j].cluster_battery_id==0){
			if(eventDisplay.cluster1_battery_soc!=cluster1_battery_soc){
				cluster1_battery_soc=eventDisplay.cluster1_battery_soc;
			}
     	}else if(cluster_show[j].cluster_battery_id==1){
			if(eventDisplay.cluster2_battery_soc!=cluster2_battery_soc){
				cluster2_battery_soc=eventDisplay.cluster2_battery_soc;
			}
     	}else if(cluster_show[j].cluster_battery_id==2){
			if(eventDisplay.cluster3_battery_soc!=cluster3_battery_soc){
				cluster3_battery_soc=eventDisplay.cluster3_battery_soc;
			}
     	}
	  }

	if(eventDisplay.cluster1_battery_num!=cluster1_battery_num)
     {
     	cluster1_battery_num=eventDisplay.cluster1_battery_num;		
     }

	if(eventDisplay.cluster2_battery_num!=cluster2_battery_num)
     {
     	cluster2_battery_num=eventDisplay.cluster2_battery_num;		
     }

	 if(eventDisplay.cluster3_battery_num!=cluster3_battery_num)
     {
     	cluster2_battery_num=eventDisplay.cluster2_battery_num;		
     }

}
static void backlight_xcb(lv_timer_t *tmr)
{
	
    static int min = 0;
	static uint8_t  auto_close_lcd_id =-1;
    (void)tmr;
#ifndef SD5KO_TEST
    if(ui_ponitImage!=NULL)
		obj_set_clear_hidden_flag(ui_ponitImage);	
#endif	
	if(getLCD())
		auto_close_lcd_id++;
	else
		auto_close_lcd_id=0;
	if(auto_close_lcd_id >= 60)
		backlight_disable();
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
#ifdef SD5KO_TEST
static void timer_xcb(lv_timer_t *tmr)
{
	char data_str[128];
	static uint8_t  id = -1;
	static uint8_t num[12]={0};
	//id= id >100 ? -1: id;	 	
	static uint8_t  hide_ver_id=-1;
	
	(void)tmr;
	
	id++;    

	hide_ver_id++;
	if(hide_ver_id >100)   //ver 1s 
		lv_obj_add_flag(ui_ver, LV_OBJ_FLAG_HIDDEN);

	if(isupdate)
	{
		isupdate=false;
		cluster_clear();
		memset(&eventDisplay, 0, sizeof(EventDisplay));
		//eventDisplay=NULL;
		cluster_update();
		cluster_init_ui();
	}
	else
	{
		eventDisplay.cluster1_battery_num=4;//cluster1_battery_num  0
		eventDisplay.cluster1_battery1_soc=450;//cluster1_battery1_soc 1 
		eventDisplay.cluster1_battery2_soc=470;//cluster1_battery2_soc 2
		eventDisplay.cluster1_battery3_soc=430;//cluster1_battery3_soc 3
		eventDisplay.cluster1_battery4_soc=480;//cluster1_battery4_soc 4


		eventDisplay.system_battery_state=2;//15

		eventDisplay.cluster1_battery_soc=457;//cluster1_battery_soc 16 

		eventDisplay.system_soc=457;//system_soc 19
		eventDisplay.system_voltage=4500;//system_voltage 20
		eventDisplay.system_current=0;//system_current 21


		eventDisplay.system_charging_power=50;//system_charging_power 22 
		eventDisplay.system_discharge_power=45;//system_discharge_power 23 

		eventDisplay.parallel_system_total_charging_energy_high;//24 parallel_system_total_charging_energy_high 
		eventDisplay.parallel_system_total_charging_energy_low;//25 parallel_system_total_charging_energy_low

		eventDisplay.parallel_system_total_discharge_energy_high;// 26 parallel_system_total_discharge_energy_high
		eventDisplay.parallel_system_total_discharge_energy_low;//27 parallel_system_total_discharge_energy_low

		eventDisplay.parallel_system_total_charging_time_high;//28 parallel_system_total_charging_time_high
		eventDisplay.parallel_system_total_charging_time_low;//29 parallel_system_total_charging_time_low

		eventDisplay.parallel_system_total_discharge_time_high;//30 parallel_system_total_discharge_time_high
		eventDisplay.parallel_system_total_discharge_time_low;//31 parallel_system_total_discharge_time_low

		eventDisplay.parallel_system_standby_time_high;//32 parallel_system_standby_time_high
		eventDisplay.parallel_system_standby_time_low;//33 parallel_system_standby_time_low

		eventDisplay.bt_state=0;//34 bt_state

		eventDisplay.wifi_state=0;//wifi_state
		eventDisplay.system_fault_state=0;//system_fault_state
		eventDisplay.system_alarm_state=0;//system_alarm_state


		eventDisplay.charging_animation_range=50;//35 Charging animation range gear
		eventDisplay.discharging_animation_range=45;//36 disCharging animation range gear

		eventDisplay.system_alarm_nums=0;
		eventDisplay.system_fault_nums=0;


		eventDisplay.parallel_system_total_charging_energy=890;
		eventDisplay.parallel_system_total_discharge_energy=678;
		eventDisplay.parallel_system_total_charging_time=12;
		eventDisplay.parallel_system_total_discharge_time=25;
		eventDisplay.parallel_system_standby_time=16;
		}
	

	if(eventDisplay.parallel_system_total_charging_energy!=system_charging_power){
			system_charging_power=eventDisplay.parallel_system_total_charging_energy;
		//	printf("system_charging_power:%d\r\n",system_charging_power);
		lv_event_send(ui_ChargedBar, LV_EVENT_VALUE_CHANGED,system_charging_power);
	}
	
	if(eventDisplay.parallel_system_total_discharge_energy!=system_discharge_power){
			system_discharge_power=eventDisplay.parallel_system_total_discharge_energy;
			//	printf("system_discharge_power:%d\r\n",system_discharge_power);
		lv_event_send(ui_DisChargedBar, LV_EVENT_VALUE_CHANGED,system_discharge_power);
	}
#if 1
	if(eventDisplay.charging_animation_range!=charging_animation_range){
			charging_animation_range=eventDisplay.charging_animation_range;
			printf("charging_animation_range:%d\r\n",charging_animation_range);
		lv_event_send(ui_ChargedBar, LV_EVENT_VALUE_CHANGED,system_charging_power);
	}
	
	if(eventDisplay.discharging_animation_range!=discharging_animation_range){
			discharging_animation_range=eventDisplay.discharging_animation_range;
			printf("discharging_animation_range:%d\r\n",discharging_animation_range);
		lv_event_send(ui_DisChargedBar, LV_EVENT_VALUE_CHANGED,system_discharge_power);
	}

#endif
	if(eventDisplay.system_battery_state!=system_battery_state){
		system_battery_state=eventDisplay.system_battery_state;
		lv_event_send(ui_DisChargedBar, LV_EVENT_VALUE_CHANGED,system_discharge_power);
		lv_event_send(ui_ChargedBar, LV_EVENT_VALUE_CHANGED,system_charging_power);
	}
	if(eventDisplay.system_voltage!=system_voltage){
		system_voltage=eventDisplay.system_voltage;
		lv_event_send(ui_VoltageBar, LV_EVENT_VALUE_CHANGED,system_voltage);
	}
	if(eventDisplay.system_current!=system_current){
		system_current=eventDisplay.system_current;
		lv_event_send(ui_CurrentBar, LV_EVENT_VALUE_CHANGED,system_current);
	}
#if 1
	if(alarm_state_flag != eventDisplay.system_alarm_state){
		alarm_state_flag=eventDisplay.system_alarm_state;
		printf("eventDisplay.system_alarm_state:%d\r\n",eventDisplay.system_alarm_state);
		lv_event_send(ui_CautionImage, LV_EVENT_VALUE_CHANGED,alarm_state_flag);
		lv_event_send(ui_CautionLabel, LV_EVENT_VALUE_CHANGED,alarm_state_flag);
	}
	if(eventDisplay.system_fault_state!=fault_state_flag){
		fault_state_flag=eventDisplay.system_fault_state;
		printf("eventDisplay.system_fault_state:%d\r\n",eventDisplay.system_fault_state);
		
		lv_event_send(ui_ErrorImage, LV_EVENT_VALUE_CHANGED,fault_state_flag);
		lv_event_send(ui_ErrorLabel, LV_EVENT_VALUE_CHANGED,fault_state_flag);
	}

	if(eventDisplay.bt_state!=bt_state){
		bt_state=eventDisplay.bt_state;
		printf("eventDisplay.bt_state:%d\r\n",eventDisplay.bt_state);
		
		lv_event_send(ui_BluetoothImage, LV_EVENT_VALUE_CHANGED,bt_state);
		lv_event_send(ui_BluetoothLabel, LV_EVENT_VALUE_CHANGED,bt_state);
	}
	if(eventDisplay.wifi_state!=wifi_state){
		wifi_state=eventDisplay.wifi_state;
		printf("eventDisplay.wifi_state:%d\r\n",eventDisplay.wifi_state);
		
		lv_event_send(ui_WiFiImage, LV_EVENT_VALUE_CHANGED,wifi_state);
		lv_event_send(ui_WiFiLabel, LV_EVENT_VALUE_CHANGED,wifi_state);
	}	
#endif

	
	 cluster_update_data(id);
	

	if(eventDisplay.cluster1_battery_num==cluster1_battery_num
		&&eventDisplay.cluster2_battery_num==cluster2_battery_num
		&&eventDisplay.cluster3_battery_num==cluster3_battery_num)
     {
     		cluster_update();
			update_show_data();
			//cluster_update_data(id);	
	
     }
	else{
			cluster1_battery_num=eventDisplay.cluster1_battery_num;
			cluster2_battery_num=eventDisplay.cluster2_battery_num;
			cluster3_battery_num=eventDisplay.cluster3_battery_num;

			cluster_clear();
			cluster_update();
			cluster_init_ui();
			//cluster_update_data(id);	
		}

	// if(eventDisplay.system_fault_nums>0||eventDisplay.system_alarm_nums>0)
		lv_event_send(ui_EventValueLabel, LV_EVENT_VALUE_CHANGED,eventDisplay.system_faults);		 
}
#else
static void timer_xcb(lv_timer_t *tmr)
{
	char data_str[128];
	static uint8_t  id = -1;
	static uint8_t num[12]={0};
	//id= id >100 ? -1: id;	 	
	static uint8_t  hide_ver_id=-1;
	
	(void)tmr;
	
	id++;    

	hide_ver_id++;
	if(hide_ver_id >100)   //ver 1s 
		lv_obj_add_flag(ui_ver, LV_OBJ_FLAG_HIDDEN);
	
	if(!eventDisplay.dataisvalid)
	{
		if( dataisvalid !=eventDisplay.dataisvalid){
			dataisvalid=eventDisplay.dataisvalid;
			cluster_clear();
			memset(&eventDisplay, 0, sizeof(EventDisplay));
			//eventDisplay=NULL;
			cluster_update();
			cluster_init_ui();
		}
	}
	else
		 dataisvalid=-1;

	if(eventDisplay.parallel_system_total_charging_energy!=system_charging_power){
			system_charging_power=eventDisplay.parallel_system_total_charging_energy;
		//	printf("system_charging_power:%d\r\n",system_charging_power);
		lv_event_send(ui_ChargedBar, LV_EVENT_VALUE_CHANGED,system_charging_power);
	}
	
	if(eventDisplay.parallel_system_total_discharge_energy!=system_discharge_power){
			system_discharge_power=eventDisplay.parallel_system_total_discharge_energy;
			//	printf("system_discharge_power:%d\r\n",system_discharge_power);
		lv_event_send(ui_DisChargedBar, LV_EVENT_VALUE_CHANGED,system_discharge_power);
	}
#if 1
	if(eventDisplay.charging_animation_range!=charging_animation_range){
			charging_animation_range=eventDisplay.charging_animation_range;
			printf("charging_animation_range:%d\r\n",charging_animation_range);
		lv_event_send(ui_ChargedBar, LV_EVENT_VALUE_CHANGED,system_charging_power);
	}
	
	if(eventDisplay.discharging_animation_range!=discharging_animation_range){
			discharging_animation_range=eventDisplay.discharging_animation_range;
			printf("discharging_animation_range:%d\r\n",discharging_animation_range);
		lv_event_send(ui_DisChargedBar, LV_EVENT_VALUE_CHANGED,system_discharge_power);
	}

#endif
	if(eventDisplay.system_battery_state!=system_battery_state){
		system_battery_state=eventDisplay.system_battery_state;
		lv_event_send(ui_DisChargedBar, LV_EVENT_VALUE_CHANGED,system_discharge_power);
		lv_event_send(ui_ChargedBar, LV_EVENT_VALUE_CHANGED,system_charging_power);
	}
	if(eventDisplay.system_voltage!=system_voltage){
		system_voltage=eventDisplay.system_voltage;
		lv_event_send(ui_VoltageBar, LV_EVENT_VALUE_CHANGED,system_voltage);
	}
	if(eventDisplay.system_current!=system_current){
		system_current=eventDisplay.system_current;
		lv_event_send(ui_CurrentBar, LV_EVENT_VALUE_CHANGED,system_current);
	}
#if 1
	if(alarm_state_flag != eventDisplay.system_alarm_state){
		alarm_state_flag=eventDisplay.system_alarm_state;
		printf("eventDisplay.system_alarm_state:%d\r\n",eventDisplay.system_alarm_state);
		lv_event_send(ui_CautionImage, LV_EVENT_VALUE_CHANGED,alarm_state_flag);
		lv_event_send(ui_CautionLabel, LV_EVENT_VALUE_CHANGED,alarm_state_flag);
	}
	if(eventDisplay.system_fault_state!=fault_state_flag){
		fault_state_flag=eventDisplay.system_fault_state;
		printf("eventDisplay.system_fault_state:%d\r\n",eventDisplay.system_fault_state);
		
		lv_event_send(ui_ErrorImage, LV_EVENT_VALUE_CHANGED,fault_state_flag);
		lv_event_send(ui_ErrorLabel, LV_EVENT_VALUE_CHANGED,fault_state_flag);
	}

	if(eventDisplay.bt_state!=bt_state){
		bt_state=eventDisplay.bt_state;
		printf("eventDisplay.bt_state:%d\r\n",eventDisplay.bt_state);
		
		lv_event_send(ui_BluetoothImage, LV_EVENT_VALUE_CHANGED,bt_state);
		lv_event_send(ui_BluetoothLabel, LV_EVENT_VALUE_CHANGED,bt_state);
	}
	if(eventDisplay.wifi_state!=wifi_state){
		wifi_state=eventDisplay.wifi_state;
		printf("eventDisplay.wifi_state:%d\r\n",eventDisplay.wifi_state);
		
		lv_event_send(ui_WiFiImage, LV_EVENT_VALUE_CHANGED,wifi_state);
		lv_event_send(ui_WiFiLabel, LV_EVENT_VALUE_CHANGED,wifi_state);
	}	
#endif

	
	 cluster_update_data(id);
	

	if(eventDisplay.cluster1_battery_num==cluster1_battery_num
		&&eventDisplay.cluster2_battery_num==cluster2_battery_num
		&&eventDisplay.cluster3_battery_num==cluster3_battery_num)
     {
     		cluster_update();
			update_show_data();
			//cluster_update_data(id);	
	
     }
	else{
			cluster1_battery_num=eventDisplay.cluster1_battery_num;
			cluster2_battery_num=eventDisplay.cluster2_battery_num;
			cluster3_battery_num=eventDisplay.cluster3_battery_num;

			cluster_clear();
			cluster_update();
			cluster_init_ui();
			//cluster_update_data(id);	
		}

	// if(eventDisplay.system_fault_nums>0||eventDisplay.system_alarm_nums>0)
		lv_event_send(ui_EventValueLabel, LV_EVENT_VALUE_CHANGED,eventDisplay.system_faults);


#if 0
	printf("eventDisplay.system_battery_state:%d\r\n",eventDisplay.system_battery_state);
	printf("eventDisplay.system_charging_power:%d\r\n",eventDisplay.system_charging_power);

	printf("eventDisplay.system_discharge_power:%d\r\n",eventDisplay.system_discharge_power);
	printf("eventDisplay.system_voltage:%d\r\n",eventDisplay.system_voltage);
	printf("eventDisplay.system_current:%d\r\n",eventDisplay.system_current);
	printf("eventDisplay.cluster1_battery_soc:%d\r\n",eventDisplay.cluster1_battery_soc);
	printf("eventDisplay.cluster2_battery_soc:%d\r\n",eventDisplay.cluster2_battery_soc);
	printf("eventDisplay.cluster3_battery_soc:%d\r\n",eventDisplay.cluster3_battery_soc);
#endif	
		 
}
#endif
#if 1
void cluster_initdata(void)
{
    int m=0;
	
	for(int i=0;i<CLUSTERNUM;i++)
	{
		cluster[i].cluster_battery_id=i;
		cluster[i].cluster_battery_num=0;//   2*i;
		if(i==0){
			cluster[i].cluster_battery1_soc=0;//50;
			cluster[i].cluster_battery2_soc=0;//75;
			cluster[i].cluster_battery3_soc=0;//75;
			cluster[i].cluster_battery4_soc=0;//100;
		}
		else if(i==1){
			cluster[i].cluster_battery1_soc=0;//100;
			cluster[i].cluster_battery2_soc=0;//75;
			cluster[i].cluster_battery3_soc=0;//50;
			cluster[i].cluster_battery4_soc=0;//100;
		}
		else {
			cluster[i].cluster_battery1_soc=0;//50;
			cluster[i].cluster_battery2_soc=0;//75;
			cluster[i].cluster_battery3_soc=0;//100;
			cluster[i].cluster_battery4_soc=0;//50;
		}

		if(cluster[i].cluster_battery_num>0)
		{
			cluster_show[m].cluster_battery_id=cluster[i].cluster_battery_id;
			cluster_show[m].cluster_battery_num=cluster[i].cluster_battery_num;
			cluster_show[m].cluster_battery1_soc=cluster[i].cluster_battery1_soc;
			cluster_show[m].cluster_battery2_soc=cluster[i].cluster_battery2_soc;
			cluster_show[m].cluster_battery3_soc=cluster[i].cluster_battery3_soc;
			cluster_show[m].cluster_battery4_soc=cluster[i].cluster_battery4_soc;
			m++;
			clusternum++;
			
		}
		
	}

	

}

void cluster_clear(void)
{
//	printf("cluster_clear\r\n");
	battery_num=-1;
	memset(battery_id_info_list, 0, 12);
	//memset(old_battery_id_info_list, 0, 12);
	for(int i=0;i<12;i++){
		battery_id_info_list[i].battery_id=0;
		old_battery_id_info_list[i].battery_id=-1;
	//	printf("battery_id_info_list[%d].battery_id:%d\r\n",i,battery_id_info_list[i].battery_id);
		
	}	
	memset(cluster, 0, 3);
	memset(cluster_show, 0, 3);
	cluster1_battery_soc=-1;
	cluster2_battery_soc=-1;
	cluster3_battery_soc=-1;
	fault_state_flag=-1;
	alarm_state_flag =-1;
	bt_state=-1;
	wifi_state=-1;
	lv_obj_clean(ui_HeadGroup);		
}
void cluster_soc_init(int num)
{
	#if 1
	int x= 71,y=496;//546
	
	cluster_soc_name[num]= lv_label_create(ui_HeadGroup);
    lv_obj_set_size(cluster_soc_name[num],140,32);   /// 1
    lv_obj_set_style_text_align(cluster_soc_name[num], LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_set_style_text_color(cluster_soc_name[num], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cluster_soc_name[num], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(cluster_soc_name[num], LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cluster_soc_name[num], &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

	if(cluster_show[num].cluster_battery_id==0/*num==0*/)
	{
		lv_obj_set_pos(cluster_soc_name[num], x, 60*num+y);
		#if 0
		lv_label_set_text(cluster_soc_name[num], "Main");
		#else
		lv_label_set_text(cluster_soc_name[num], "Master");
		#endif
	}else if(cluster_show[num].cluster_battery_id==1/*num==1*/)
	{
		lv_obj_set_pos(cluster_soc_name[num], x, 60*num+y);	
		#if 0
		lv_label_set_text(cluster_soc_name[num], "expansion1");
		#else
		lv_label_set_text(cluster_soc_name[num], "Cluster1");
		#endif
	}
	else if(cluster_show[num].cluster_battery_id==2/*num==2*/)
	{
		lv_obj_set_pos(cluster_soc_name[num], x, 60*num+y);  
		#if 0
	    lv_label_set_text(cluster_soc_name[num], "expansion2");	  
		#else
		 lv_label_set_text(cluster_soc_name[num], "Cluster2");	 
		#endif
	}

   	clusterbar[num]= lv_bar_create(ui_HeadGroup);	
	lv_bar_set_range(clusterbar[num], 0, 100);
   // lv_bar_set_value(clusterbar[num], 100, LV_ANIM_OFF);
    lv_bar_set_start_value(clusterbar[num], 0, LV_ANIM_OFF);
    lv_obj_set_size(clusterbar[num], 253,18);
    lv_obj_set_pos(clusterbar[num], 233, 60*num+y);
  
  //  lv_obj_set_align(clusterbar[num], LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(clusterbar[num], lv_color_hex(0x646669), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(clusterbar[num], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_anim_time(clusterbar[num], 500, LV_PART_MAIN | LV_STATE_DEFAULT);
	//lv_obj_set_style_radius(clusterbar[num],10,0);
	if(num==0)
	{
		lv_obj_set_style_bg_color(clusterbar[num], lv_color_hex(0x181F1F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(clusterbar[num], 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_grad_color(clusterbar[num], lv_color_hex(0x72BDBE), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_grad_dir(clusterbar[num], LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

	}else if(num==1)
	{
	    lv_obj_set_style_bg_color(clusterbar[num], lv_color_hex(0x181F1F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	    lv_obj_set_style_bg_opa(clusterbar[num], 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	    lv_obj_set_style_bg_grad_color(clusterbar[num], lv_color_hex(0x74C0C1), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	    lv_obj_set_style_bg_grad_dir(clusterbar[num], LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	}
	else if(num==2)
	{
		lv_obj_set_style_bg_color(clusterbar[num], lv_color_hex(0x181F1F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(clusterbar[num], 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_grad_color(clusterbar[num], lv_color_hex(0x74C0C1), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_grad_dir(clusterbar[num], LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	}
		

    cluster_soc_value[num]= lv_label_create(ui_HeadGroup);
    lv_obj_set_size(cluster_soc_value[num], 110,32);    /// 1
    lv_obj_set_pos(cluster_soc_value[num], 529,60*num+y); 
    
	if(num==0)
	{
		lv_label_set_text(cluster_soc_value[num], "0.0%");
		lv_obj_set_style_text_color(cluster_soc_value[num], lv_color_hex(0x32809B), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_opa(cluster_soc_value[num], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_align(cluster_soc_value[num], LV_TEXT_ALIGN_RIGHT, 0);
		lv_obj_set_style_text_font(cluster_soc_value[num], &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
		
	}else if(num==1)
	{
		lv_label_set_text(cluster_soc_value[num], "0.0%");
	    lv_obj_set_style_text_color(cluster_soc_value[num], lv_color_hex(0x74C0C1), LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_set_style_text_opa(cluster_soc_value[num], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_set_style_text_align(cluster_soc_value[num], LV_TEXT_ALIGN_RIGHT, 0);
	    lv_obj_set_style_text_font(cluster_soc_value[num], &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

	}
	else if(num==2)
	{
		lv_label_set_text(cluster_soc_value[num], "0.0%");
		lv_obj_set_style_text_color(cluster_soc_value[num], lv_color_hex(0x74C0C1), LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_set_style_text_opa(cluster_soc_value[num], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	    lv_obj_set_style_text_align(cluster_soc_value[num], LV_TEXT_ALIGN_RIGHT, 0);
	    lv_obj_set_style_text_font(cluster_soc_value[num], &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

	}

	lv_obj_add_event_cb(clusterbar[num], bar_event_cb, LV_EVENT_ALL, cluster_soc_value[num]);
#endif

}
void cluster_init_ui(void)
{
	int cluster_x=0;
	lv_anim_init(&a);
	//printf("clusternum:%d\r\n",clusternum);
    for(int i =0;i<clusternum;i++)
    {
    	if(clusternum==2)
			cluster_x=71*2+39+i*220;
		else if(clusternum==1)
			cluster_x=71*4+7+i*220;//cluster_x=71*4-7+i*220;
		else
			cluster_x=71*(4-clusternum)+i*220;
		cluster_soc_init(i);
		
    	foot= lv_img_create(ui_HeadGroup);
		lv_img_set_src(foot, LVGL_PATH(batteryfoot.png));
	    lv_obj_set_pos(foot,  cluster_x, 453);//503
		lv_obj_set_size(foot, 137,8);
				
		weco=lv_img_create(ui_HeadGroup);
		char data_str[128];
		//	printf("cluster_show[%d].cluster_battery_id:%d\r\n",i,cluster_show[i].cluster_battery_id);
		if(cluster_show[i].cluster_battery_id >0)//if(i>0)
			ui_snprintf(data_str, "%scluster_head%d.png", LVGL_DIR, 1);
		else
			ui_snprintf(data_str, "%scluster_head%d.png", LVGL_DIR, cluster_show[i].cluster_battery_id);
		lv_img_set_src(weco, data_str);
		//lv_img_set_src(weco, LVGL_PATH(WECO.png));
	    lv_obj_set_pos(weco, cluster_x, (450-(77*cluster_show[i].cluster_battery_num))-64-3);
		lv_obj_set_size(weco,138,64);//111,51);
#if 0//by liu qing 2024.03.11

		clustername=lv_label_create(ui_HeadGroup);
		lv_obj_set_style_text_font(clustername, &lv_font_montserrat_24, LV_STATE_DEFAULT);
		
	    lv_obj_set_width(clustername, 138);//LV_SIZE_CONTENT);
	    lv_obj_set_height(clustername, 60);//LV_SIZE_CONTENT);
	    lv_obj_set_pos(clustername, cluster_x, (500-(77*cluster_show[i].cluster_battery_num))-64-3-30);

		if(cluster_show[i].cluster_battery_id==0)
	    	lv_label_set_text(clustername, "MASTER");
		else if(cluster_show[i].cluster_battery_id==1)
	    	lv_label_set_text(clustername, "SLAVE1");
		else if(cluster_show[i].cluster_battery_id==2)
	    	lv_label_set_text(clustername, "SLAVE2");

		lv_obj_set_style_text_align(clustername, LV_TEXT_ALIGN_CENTER,0);
	    lv_obj_set_style_text_color(clustername, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
#endif			
		for(int j =0;j<cluster_show[i].cluster_battery_num;j++)
		{			
    		clusterimg[i][j] = lv_img_create(ui_HeadGroup);
			clusterlabel[i][j]= lv_label_create(ui_HeadGroup);
			battery_num++;
			switch(j)
			{
				case 0:
					{
						barrery_show(clusterimg[i][j],cluster_show[i].cluster_battery1_soc*0.38);
						battery_id_info_list[battery_num].battery_id=cluster_show[i].cluster_battery1_soc;
					}
					break;
				case 1:
					{
						barrery_show(clusterimg[i][j],cluster_show[i].cluster_battery2_soc*0.38);
						battery_id_info_list[battery_num].battery_id=cluster_show[i].cluster_battery2_soc;	
				    }
					break;
				case 2:
					{
						barrery_show(clusterimg[i][j],cluster_show[i].cluster_battery3_soc*0.38);
						battery_id_info_list[battery_num].battery_id=cluster_show[i].cluster_battery3_soc;
				    }
					break;
				case 3:
					{
						barrery_show(clusterimg[i][j],cluster_show[i].cluster_battery4_soc*0.38);
						battery_id_info_list[battery_num].battery_id=cluster_show[i].cluster_battery4_soc;
				    }
					break;
			}	
			
		//	printf("battery_id_info_list[%d].battery_id:%d\r\n",battery_num,battery_id_info_list[battery_num].battery_id);
			battery_id_info_list[battery_num].battery_img= clusterimg[i][j] ;	
			battery_id_info_list[battery_num].battery_label= clusterlabel[i][j] ;
			
			lv_obj_set_pos(clusterimg[i][j], cluster_x, (450-(77*(j+1))));
			lv_obj_set_size(clusterimg[i][j], 138,74);

			lv_obj_set_width(clusterlabel[i][j], 90);
		    lv_obj_set_height(clusterlabel[i][j], 30);
		    lv_obj_set_pos(clusterlabel[i][j], cluster_x+25, (450-(77*(j+1)))+25);//40,30
		
			ui_snprintf(data_str, "%d.%d%%", battery_id_info_list[battery_num].battery_id/10, battery_id_info_list[battery_num].battery_id%10);
			lv_label_set_text(clusterlabel[i][j], data_str);
		
			lv_obj_set_style_text_align(clusterlabel[i][j], LV_TEXT_ALIGN_CENTER, 0);
		    lv_obj_set_style_text_color(clusterlabel[i][j], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

			lv_obj_set_style_text_font(clusterlabel[i][j], &lv_font_montserrat_20, LV_STATE_DEFAULT);

			lv_obj_add_event_cb(clusterlabel[i][j], main_speed_num_event, LV_EVENT_ALL, NULL);
			lv_obj_add_event_cb(clusterimg[i][j], main_point_event, LV_EVENT_ALL, NULL);						
				
   	 	}    
    }
//	cluster_update();
}
void cluster_update(void)
{
    int m=0;
	clusternum=0;
	memset(cluster,0,sizeof(cluster_t));
	memset(cluster_show,0,sizeof(cluster_t));
	for(int i=0;i<CLUSTERNUM;i++)
	{
		cluster[i].cluster_battery_id=i;
		//cluster[i].cluster_battery_num=eventDisplay.cluster1_battery_num;
		if(i==0){			
			cluster[i].cluster_battery_num=eventDisplay.cluster1_battery_num>4?4:eventDisplay.cluster1_battery_num;
			cluster[i].cluster_battery1_soc=eventDisplay.cluster1_battery1_soc;
			cluster[i].cluster_battery2_soc=eventDisplay.cluster1_battery2_soc;
			cluster[i].cluster_battery3_soc=eventDisplay.cluster1_battery3_soc;
			cluster[i].cluster_battery4_soc=eventDisplay.cluster1_battery4_soc;
		}
		else if(i==1){
			cluster[i].cluster_battery_num=eventDisplay.cluster2_battery_num>4?4:eventDisplay.cluster2_battery_num;
			cluster[i].cluster_battery1_soc=eventDisplay.cluster2_battery1_soc;
			cluster[i].cluster_battery2_soc=eventDisplay.cluster2_battery2_soc;
			cluster[i].cluster_battery3_soc=eventDisplay.cluster2_battery3_soc;
			cluster[i].cluster_battery4_soc=eventDisplay.cluster2_battery4_soc;
		}
		else {
			cluster[i].cluster_battery_num=eventDisplay.cluster3_battery_num>4?4:eventDisplay.cluster3_battery_num;
			cluster[i].cluster_battery1_soc=eventDisplay.cluster3_battery1_soc;
			cluster[i].cluster_battery2_soc=eventDisplay.cluster3_battery2_soc;
			cluster[i].cluster_battery3_soc=eventDisplay.cluster3_battery3_soc;
			cluster[i].cluster_battery4_soc=eventDisplay.cluster3_battery4_soc;
		}
		if(cluster[i].cluster_battery_num>0)
		{
			cluster_show[m].cluster_battery_id=cluster[i].cluster_battery_id;
			cluster_show[m].cluster_battery_num=cluster[i].cluster_battery_num;
			cluster_show[m].cluster_battery1_soc=cluster[i].cluster_battery1_soc;
			cluster_show[m].cluster_battery2_soc=cluster[i].cluster_battery2_soc;
			cluster_show[m].cluster_battery3_soc=cluster[i].cluster_battery3_soc;
			cluster_show[m].cluster_battery4_soc=cluster[i].cluster_battery4_soc;
			m++;
			clusternum++;			
		}		
	}
}

#endif

void battery(lv_obj_t *battery_obj,int battery_id)
{
	char data_str[128];
	int start_id=0;
	int end_id=battery_id;
	static int id = -1;
	if(id<=end_id){
		id++;    

		id= id < end_id ? id: end_id;

//		printf("id:%d\r\n",id);

		ui_snprintf(data_str, "%sbattery/battery_%02d.png", LVGL_DIR, id);
//		printf("data_str:%s\r\n",data_str);
		lv_img_set_src(battery_obj, data_str);
		lv_img_set_angle(battery_obj, 0);

	}
}


void barrery_show(lv_obj_t * obj,int id)
{
	char data_str[128];
	ui_snprintf(data_str, "%sbattery/battery_%02d.png", LVGL_DIR,id);
	lv_img_set_src(obj, data_str);
    
}


void sdbattery_ui_init()
{
	printf("sdbattery_ui_init\r\n");	
	ui_Screen1_screen_init();
	cluster_initdata();
	cluster_init_ui();
	lv_timer_t* timer= lv_timer_create(timer_xcb, 10, 0);	
	lv_timer_create(backlight_xcb,1000,0);// time 1s

	lv_timer_create(update_xcb, 1000*60, 0);
}

 void  ui_Screen1_screen_init()
{
    ui_BGImage = lv_img_create(lv_scr_act());
    lv_img_set_src(ui_BGImage,LVGL_PATH(bg.png));

	lv_obj_set_pos(ui_BGImage, 0, 0);
	lv_obj_set_size(ui_BGImage, 720,1280);

 /*	ui_Image = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_Image, LVGL_PATH(battery_23.png));
    lv_obj_set_pos(ui_Image, 0, 0);
	lv_obj_set_size(ui_Image, 138, 74);*/

	ui_ver = lv_label_create(ui_BGImage);
	lv_obj_set_width(ui_ver, LV_SIZE_CONTENT);	/// 1
	lv_obj_set_height(ui_ver, LV_SIZE_CONTENT);	  /// 1
	lv_obj_set_x(ui_ver,0);
	lv_obj_set_y(ui_ver,0);
	lv_obj_set_align(ui_ver, LV_ALIGN_TOP_LEFT);
	lv_label_set_text(ui_ver, version);
	lv_obj_set_style_text_color(ui_ver, lv_color_hex(0x438324), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_ver, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_ver, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_ponitImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_ponitImage, LVGL_PATH(point.png));
    lv_obj_set_width(ui_ponitImage, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_ponitImage, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_ponitImage, 30);
    lv_obj_set_y(ui_ponitImage, 30);
    lv_obj_set_align(ui_ponitImage, LV_ALIGN_TOP_LEFT);
    lv_obj_add_flag(ui_ponitImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_ponitImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

#ifdef SD5KO_TEST
	lv_obj_add_flag(ui_ponitImage, LV_OBJ_FLAG_HIDDEN);
	#endif
#if 1
   
    ui_ErrorImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_ErrorImage, LVGL_PATH(error.png));
    lv_obj_set_pos(ui_ErrorImage, 115-20, 1100);//1130
	lv_obj_set_size(ui_ErrorImage, 57, 47);

   // lv_obj_set_align(ui_ErrorImage, LV_ALIGN_CENTER);
   
    lv_obj_add_flag(ui_ErrorImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_ErrorImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
	
    ui_ErrorLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_ErrorLabel, 93-20, 1163);
	lv_obj_set_size(ui_ErrorLabel, 100, 32);
	
	lv_label_set_long_mode(ui_ErrorLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_ErrorLabel, LV_TEXT_ALIGN_CENTER, 0);
	
   // lv_obj_set_align(ui_ErrorLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_ErrorLabel, "ERROR");
    lv_obj_set_style_text_color(ui_ErrorLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_ErrorLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_ErrorLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
	lv_obj_add_flag(ui_ErrorLabel, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_ErrorImage, LV_OBJ_FLAG_HIDDEN);
#endif	
	lv_obj_add_event_cb(ui_ErrorImage, footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_ErrorLabel, footlabel_event, LV_EVENT_ALL, NULL);

    ui_CautionImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_CautionImage, LVGL_PATH(CAUTION.png));
   	lv_obj_set_pos(ui_CautionImage, 250+10, 1098);
	lv_obj_set_size(ui_CautionImage, 52, 52);

   // lv_obj_set_align(ui_CautionImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_CautionImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_CautionImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_CautionLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_CautionLabel, 211, 1163);
	lv_obj_set_size(ui_CautionLabel, 150, 32);

	lv_label_set_long_mode(ui_CautionLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_CautionLabel, LV_TEXT_ALIGN_CENTER, 0);
   // lv_obj_set_align(ui_CautionLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_CautionLabel, "CAUTION");
    lv_obj_set_style_text_color(ui_CautionLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_CautionLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_CautionLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
	lv_obj_add_flag(ui_CautionImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_CautionLabel, LV_OBJ_FLAG_HIDDEN);
#endif	
	lv_obj_add_event_cb(ui_CautionImage, footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_CautionLabel, footlabel_event, LV_EVENT_ALL, NULL);
   
    ui_BluetoothImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_BluetoothImage,  LVGL_PATH(BLUETOOTH.png));
    lv_obj_set_pos(ui_BluetoothImage, 420+10, 1101);
	lv_obj_set_size(ui_BluetoothImage, 32, 46);
    //lv_obj_set_align(ui_BluetoothImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_BluetoothImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_BluetoothImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_BluetoothLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_BluetoothLabel, 347, 1163);
	lv_obj_set_size(ui_BluetoothLabel, 200, 32);
    lv_label_set_long_mode(ui_BluetoothLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_BluetoothLabel, LV_TEXT_ALIGN_CENTER, 0);
    //lv_obj_set_align(ui_BluetoothLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_BluetoothLabel, "BLUETOOTH");
    lv_obj_set_style_text_color(ui_BluetoothLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BluetoothLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BluetoothLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
	lv_obj_add_flag(ui_BluetoothImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_BluetoothLabel, LV_OBJ_FLAG_HIDDEN);
		
#endif
	lv_obj_add_event_cb(ui_BluetoothImage, footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_BluetoothLabel, footlabel_event, LV_EVENT_ALL, NULL);
  
    ui_WiFiImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_WiFiImage,  LVGL_PATH(wifi.png));
    lv_obj_set_pos(ui_WiFiImage, 562, 1100);
	lv_obj_set_size(ui_WiFiImage, 60, 44);

   // lv_obj_set_align(ui_WiFiImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_WiFiImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_WiFiImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_WiFiLabel = lv_label_create(ui_BGImage);

   lv_obj_set_pos(ui_WiFiLabel, 544, 1163);
	lv_obj_set_size(ui_WiFiLabel, 115, 32);
	//lv_label_set_text(ui_WiFiLabel, "WIFI");
	lv_label_set_long_mode(ui_WiFiLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_WiFiLabel, LV_TEXT_ALIGN_CENTER, 0);
    //lv_obj_set_align(ui_WiFiLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_WiFiLabel, "WiFi");
    lv_obj_set_style_text_color(ui_WiFiLabel, lv_color_hex(0xFEFEFE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_WiFiLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_WiFiLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifndef SD5KO_TEST
	lv_obj_add_flag(ui_WiFiImage, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_WiFiLabel, LV_OBJ_FLAG_HIDDEN);
#endif		

	lv_obj_add_event_cb(ui_WiFiImage, footimg_event, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_WiFiLabel, footlabel_event, LV_EVENT_ALL, NULL);

    ui_LineImage = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_LineImage,  LVGL_PATH(line.png));
    lv_obj_set_pos(ui_LineImage, 71, 707);
	lv_obj_set_size(ui_LineImage, 568, 2);
	
   // lv_obj_set_align(ui_LineImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_LineImage, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_LineImage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_LineImage2 = lv_img_create(ui_BGImage);
    lv_img_set_src(ui_LineImage2,  LVGL_PATH(line.png));
    lv_obj_set_pos(ui_LineImage2, 71, 1046);
	lv_obj_set_size(ui_LineImage2, 568, 2);

   // lv_obj_set_align(ui_LineImage2, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_LineImage2, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_LineImage2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

   
    ui_ChargedLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_ChargedLabel, 71, 748);
	lv_obj_set_size(ui_ChargedLabel, 115, 32);
	lv_label_set_text(ui_ChargedLabel, "Charged");
	lv_label_set_long_mode(ui_ChargedLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_ChargedLabel, LV_TEXT_ALIGN_LEFT, 0);
   // lv_obj_set_align(ui_ChargedLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_ChargedLabel, "Charged");
    lv_obj_set_style_text_color(ui_ChargedLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_ChargedLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_ChargedLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ChargedBar = lv_bar_create(ui_BGImage);

	lv_obj_set_pos(ui_ChargedBar, 233, 748);
	lv_obj_set_size(ui_ChargedBar, 253, 18);
	lv_bar_set_range(ui_ChargedBar,0,999);
  //  lv_bar_set_value(ui_ChargedBar, 999, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_ChargedBar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(ui_ChargedBar, lv_color_hex(0x646669), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ChargedBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_anim_time(ui_ChargedBar, 500, LV_PART_MAIN | LV_STATE_DEFAULT);
#if 0
	lv_obj_set_style_base_dir(ui_ChargedBar, LV_BASE_DIR_RTL, 0);
#endif		
   // lv_obj_set_style_bg_color(ui_ChargedBar, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	 lv_obj_set_style_bg_color(ui_ChargedBar, lv_color_hex(0x181E16), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	

    lv_obj_set_style_bg_opa(ui_ChargedBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_ChargedBar, lv_color_hex(0x63A84B), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_ChargedBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_ChargedValueLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_ChargedValueLabel, 500, 748);
	lv_obj_set_size(ui_ChargedValueLabel, 145, 32);
	lv_label_set_text(ui_ChargedValueLabel, "");
	lv_label_set_long_mode(ui_ChargedValueLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_ChargedValueLabel, LV_TEXT_ALIGN_RIGHT, 0);
	
    //lv_label_set_text(ui_ChargedValueLabel, " ");
    lv_obj_set_style_text_color(ui_ChargedValueLabel, lv_color_hex(0x5BC821), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_ChargedValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_ChargedValueLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_ChargedBar, bar_event_cb, LV_EVENT_ALL, ui_ChargedValueLabel);
   
    ui_DisChargedLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_DisChargedLabel, 71, 807);
	lv_obj_set_size(ui_DisChargedLabel, 152, 32);
	//lv_label_set_text(ui_DisChargedLabel, "DisCharged");
	lv_label_set_long_mode(ui_DisChargedLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_DisChargedLabel, LV_TEXT_ALIGN_LEFT, 0);
    //lv_obj_set_align(ui_DisChargedLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_DisChargedLabel, "DisCharged");
    lv_obj_set_style_text_color(ui_DisChargedLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_DisChargedLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_DisChargedLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_DisChargedBar = lv_bar_create(ui_BGImage);
	lv_bar_set_range(ui_DisChargedBar,0,999);
   // lv_bar_set_value(ui_DisChargedBar, 999, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_DisChargedBar, 0, LV_ANIM_OFF);
  
    lv_obj_set_pos(ui_DisChargedBar, 233, 807);
	lv_obj_set_size(ui_DisChargedBar, 253, 18);

  //  lv_obj_set_align(ui_DisChargedBar, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_DisChargedBar, lv_color_hex(0x646669), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DisChargedBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_anim_time(ui_DisChargedBar, 500, LV_PART_MAIN | LV_STATE_DEFAULT);
	
		lv_obj_set_style_bg_color(ui_DisChargedBar, lv_color_hex(0x201717), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		   

   // lv_obj_set_style_bg_color(ui_DisChargedBar, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DisChargedBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_DisChargedBar, lv_color_hex(0xCB193C), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_DisChargedBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_DisChargedValueLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_DisChargedValueLabel, 500, 807);
	lv_obj_set_size(ui_DisChargedValueLabel, 145, 32);
	//lv_label_set_text(ui_DisChargedValueLabel, "1.0kWh");
	lv_label_set_long_mode(ui_DisChargedValueLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_DisChargedValueLabel, LV_TEXT_ALIGN_RIGHT, 0);
  //lv_obj_set_align(ui_DisChargedValueLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_DisChargedValueLabel, " ");
    lv_obj_set_style_text_color(ui_DisChargedValueLabel, lv_color_hex(0xCB193C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_DisChargedValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   // lv_obj_set_style_text_align(ui_DisChargedValueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_DisChargedValueLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_DisChargedBar, bar_event_cb, LV_EVENT_ALL, ui_DisChargedValueLabel);

  
    ui_VoltageLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_VoltageLabel, 71, 866);
	lv_obj_set_size(ui_VoltageLabel, 115, 32);
	lv_label_set_long_mode(ui_VoltageLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_VoltageLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(ui_VoltageLabel, "Voltage");
    lv_obj_set_style_text_color(ui_VoltageLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_VoltageLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_VoltageLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_VoltageBar = lv_bar_create(ui_BGImage);
	#if 1
	lv_bar_set_range(ui_VoltageBar,0,999);
	#else
	lv_bar_set_range(ui_VoltageBar,0,100);
	#endif
    //lv_bar_set_value(ui_VoltageBar, 100, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_VoltageBar, 0, LV_ANIM_OFF);
    lv_obj_set_pos(ui_VoltageBar, 233, 866);
	lv_obj_set_size(ui_VoltageBar, 253, 18);

    lv_obj_set_style_bg_color(ui_VoltageBar, lv_color_hex(0x646669), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_VoltageBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_anim_time(ui_VoltageBar, 500, LV_PART_MAIN | LV_STATE_DEFAULT);
	

    //lv_obj_set_style_bg_color(ui_VoltageBar, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_VoltageBar, lv_color_hex(0x151918), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    
    lv_obj_set_style_bg_opa(ui_VoltageBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_VoltageBar, lv_color_hex(0x55E3C9), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_VoltageBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_VoltageValueLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_VoltageValueLabel, 500, 866);
	lv_obj_set_size(ui_VoltageValueLabel, 145, 32);
	lv_label_set_long_mode(ui_VoltageValueLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_VoltageValueLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(ui_VoltageValueLabel, " ");
    lv_obj_set_style_text_color(ui_VoltageValueLabel, lv_color_hex(0x52D7BF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_VoltageValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  //lv_obj_set_style_text_align(ui_VoltageValueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_VoltageValueLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_VoltageBar, bar_event_cb, LV_EVENT_ALL, ui_VoltageValueLabel);
   
    ui_CurrentLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_CurrentLabel, 71, 921);
	lv_obj_set_size(ui_CurrentLabel, 115, 32);
	lv_label_set_long_mode(ui_CurrentLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_CurrentLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(ui_CurrentLabel, "Current");
    lv_obj_set_style_text_color(ui_CurrentLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_CurrentLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_CurrentLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_CurrentBar = lv_bar_create(ui_BGImage);
	#if 0
	lv_bar_set_range(ui_CurrentBar,0,999);
	#else
	lv_bar_set_range(ui_CurrentBar,0,60);
	#endif
   // lv_bar_set_value(ui_CurrentBar, 999, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_CurrentBar, 0, LV_ANIM_OFF);
  	lv_obj_set_pos(ui_CurrentBar, 233, 921);
	lv_obj_set_size(ui_CurrentBar, 253, 18);
    lv_obj_set_style_bg_color(ui_CurrentBar, lv_color_hex(0x646669), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_CurrentBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_anim_time(ui_CurrentBar, 500, LV_PART_MAIN | LV_STATE_DEFAULT);

   // lv_obj_set_style_bg_color(ui_CurrentBar, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	 lv_obj_set_style_bg_color(ui_CurrentBar, lv_color_hex(0x1A2326), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	
    lv_obj_set_style_bg_opa(ui_CurrentBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_CurrentBar, lv_color_hex(0x32809B), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_CurrentBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_CurrentValueLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_CurrentValueLabel, 500, 921);
	lv_obj_set_size(ui_CurrentValueLabel, 145, 32);
	
	lv_label_set_long_mode(ui_CurrentValueLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_CurrentValueLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(ui_CurrentValueLabel, " ");
    lv_obj_set_style_text_color(ui_CurrentValueLabel, lv_color_hex(0x32809B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_CurrentValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   // lv_obj_set_style_text_align(ui_CurrentValueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_CurrentValueLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_CurrentBar, bar_event_cb, LV_EVENT_ALL, ui_CurrentValueLabel);

    ui_EventLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_EventLabel, 71, 977);
	lv_obj_set_size(ui_EventLabel, 115, 32);
	lv_label_set_long_mode(ui_EventLabel, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui_EventLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(ui_EventLabel, "Event");
    lv_obj_set_style_text_color(ui_EventLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_EventLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_EventLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_EventValueLabel = lv_label_create(ui_BGImage);
    lv_obj_set_pos(ui_EventValueLabel, 260, 977);
	lv_obj_set_size(ui_EventValueLabel, 390, 32);
	lv_obj_add_flag(ui_EventValueLabel,LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
	lv_label_set_long_mode(ui_EventValueLabel, LV_LABEL_LONG_SCROLL_CIRCULAR/*LV_LABEL_LONG_WRAP*/);
	lv_obj_set_style_text_align(ui_EventValueLabel, LV_TEXT_ALIGN_RIGHT, 0);
	//lv_obj_set_style_anim_speed(ui_EventValueLabel, 20, 0);
    lv_label_set_text(ui_EventValueLabel, "   ");
	//lv_label_set_text(ui_EventValueLabel, "1235,23AB");
    lv_obj_set_style_text_color(ui_EventValueLabel, lv_color_hex(0x32809B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_EventValueLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   //lv_obj_set_style_text_align(ui_EventValueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_EventValueLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
		    
	lv_obj_add_event_cb(ui_EventValueLabel, main_speed_num_event, LV_EVENT_ALL, NULL);

	//Write codes screen_heard_cont
	ui_HeadGroup = lv_obj_create(ui_BGImage);
	lv_obj_set_pos(ui_HeadGroup, 0, 12);
	lv_obj_set_size(ui_HeadGroup, 720, 714);

	//Write style state: LV_STATE_DEFAULT for style_screen_heard_cont_main_main_default
	static lv_style_t style_screen_heard_cont_main_main_default;
	lv_style_reset(&style_screen_heard_cont_main_main_default);
	lv_style_set_radius(&style_screen_heard_cont_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_heard_cont_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_heard_cont_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_heard_cont_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_heard_cont_main_main_default, 0);
	lv_style_set_border_color(&style_screen_heard_cont_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_heard_cont_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_heard_cont_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_heard_cont_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_heard_cont_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_heard_cont_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_heard_cont_main_main_default, 0);
	lv_obj_add_style(ui_HeadGroup, &style_screen_heard_cont_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	
#endif
}


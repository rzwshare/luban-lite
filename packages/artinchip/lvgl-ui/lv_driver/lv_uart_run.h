/*
 * Copyright (c) 2022-2023, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef LV_TPC_RUN
#define LV_TPC_RUN

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#if 1


typedef struct
{
	int cluster1_battery_num;//cluster1_battery_num  0
	int cluster1_battery1_soc;//cluster1_battery1_soc 1 
	int cluster1_battery2_soc;//cluster1_battery2_soc 2
	int cluster1_battery3_soc;//cluster1_battery3_soc 3
	int cluster1_battery4_soc;//cluster1_battery4_soc 4
	
	int cluster2_battery_num;//cluster2_battery_num 5
	int cluster2_battery1_soc;//cluster2_battery1_soc 6
	int cluster2_battery2_soc;//cluster2_battery2_soc 7
	int cluster2_battery3_soc;//cluster2_battery3_soc 8
	int cluster2_battery4_soc;//cluster2_battery4_soc 9

	
	int cluster3_battery_num;//cluster3_battery_num 10 
	int cluster3_battery1_soc;//cluster3_battery1_soc 11
	int cluster3_battery2_soc;//cluster3_battery2_soc 12
	int cluster3_battery3_soc;//cluster3_battery3_soc 13
	int cluster3_battery4_soc;//cluster3_battery4_soc 14

	int system_battery_state;//15
	
	int cluster1_battery_soc;//cluster1_battery_soc 16 
	int cluster2_battery_soc;//cluster2_battery_soc 17
	int cluster3_battery_soc;//cluster3_battery_soc 18 

	int system_soc;//system_soc 19
	int system_voltage;//system_voltage 20
	int system_current;//system_current 21

	
	int system_charging_power;//system_charging_power 22 
	int system_discharge_power;//system_discharge_power 23 

	int parallel_system_total_charging_energy_high;//24 parallel_system_total_charging_energy_high 
	int parallel_system_total_charging_energy_low;//25 parallel_system_total_charging_energy_low

	int parallel_system_total_discharge_energy_high;// 26 parallel_system_total_discharge_energy_high
	int parallel_system_total_discharge_energy_low;//27 parallel_system_total_discharge_energy_low

	int parallel_system_total_charging_time_high;//28 parallel_system_total_charging_time_high
	int parallel_system_total_charging_time_low;//29 parallel_system_total_charging_time_low

    int parallel_system_total_discharge_time_high;//30 parallel_system_total_discharge_time_high
	int parallel_system_total_discharge_time_low;//31 parallel_system_total_discharge_time_low

    int parallel_system_standby_time_high;//32 parallel_system_standby_time_high
	int parallel_system_standby_time_low;//33 parallel_system_standby_time_low

	int bt_state;//34 bt_state
	
	int wifi_state;//wifi_state
	int system_fault_state;//system_fault_state
	int system_alarm_state;//system_alarm_state

	
	int charging_animation_range;//35 Charging animation range gear
	int discharging_animation_range;//36 disCharging animation range gear

	int system_alarm_nums;
	int system_fault_nums;
	
	//char *system_alarms;
	char system_alarms[2048];
	char system_faults[2048];
	//uint8_t *system_faults;
	int parallel_system_total_charging_energy;
	int parallel_system_total_discharge_energy;
	int parallel_system_total_charging_time;
	int	parallel_system_total_discharge_time;
	int parallel_system_standby_time;
	short dataisvalid ;
	
		
} EventDisplay;



#endif

#include <rtconfig.h>

#ifdef KERNEL_RTTHREAD
#include <rtthread.h>
#include <rtdevice.h>

int uart_run(const char *name);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_TPC_RUN */

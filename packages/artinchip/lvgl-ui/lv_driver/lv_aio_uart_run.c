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
#include "lv_aio_uart_run.h"
#include "lv_port_indev.h"
#include "aic_osal.h"

#include <rtthread.h>
#include <aic_core.h>
#include "modbus_rtu.h"
#include "modbus_common.h"
//#include "sdbattery_ui.h"
#define SAMPLE_UART_NAME "uart5"
#define MAX_OUTDATA_SIZE         9//64
#define EXTERNAL_BUFFER_SIZE    2048//128//80

//#define USER_MODBUS   

#ifdef USER_MODBUS
#define DATA_HEAD_LEN   6
#define DATA_START_ID   4
#else
#define DATA_HEAD_LEN   5
#define DATA_START_ID   3
#endif

struct rt_semaphore aio_event_rx_sem;
rt_device_t aio_event_uart_serial;
//char event_str_send[] = "1234567890ArtInChip1234567890\n";
static unsigned char aio_event_str_send[MAX_OUTDATA_SIZE];
static unsigned char aio_str_receive[EXTERNAL_BUFFER_SIZE];
static unsigned char aio_cmdBuf[EXTERNAL_BUFFER_SIZE];
static unsigned char aio_exteralBuf[EXTERNAL_BUFFER_SIZE];
static unsigned int aio_cmdPos = 0;
int g_aio_eventexit = 0;
unsigned int aio_readDataLen=0; 
AioEventDisplay aio_eventDisplay;
static int aio_system_fault_nums=0;
static int aio_system_fault_counts=0;
static int aio_system_alarm_nums=0;
static int aio_system_alarm_counts=0;
static bool isAioFault=false;
static bool isAioAlarm=false;
static bool isAioFaultSend=false;
static bool isAioAlarmSend=false;
static bool isAioDataSend=false;
static bool isAioStartPack=false;
static unsigned int aio_real_data_len=0;
static struct rt_thread thread_uart;
 static rt_uint8_t thread_stack[1024*4];//
rt_err_t aio_eventuart_input(rt_device_t dev, rt_size_t size)
{
    if (size > 0)
        rt_sem_release(&aio_event_rx_sem);

    return RT_EOK;
}

const unsigned char aio_Num2CharTable[] = "0123456789ABCDEF";
void aio_HexArrayToString(uint8_t *hexarray, int length, uint8_t *string)
{
    int i = 0;
    while(i < length)
    {
        *(string++) = aio_Num2CharTable[(hexarray[i] >> 4) & 0x0f];
        *(string++) = aio_Num2CharTable[hexarray[i] & 0x0f];
        i++;
    }
    *string = 0x0;
}

long int aio_HexArrayToDec(uint8_t *hexarray, int len)
{
    uint8_t dst[50];
    aio_HexArrayToString(hexarray, len, dst);
    int t;
    long sum=0;
    for(int i=0;dst[i];i++){
        if(dst[i]<='9')
            t=dst[i]-'0';
        else
            t=dst[i]-'A'+10;
        sum=sum*16+t;
    }
    return sum;
}

static unsigned short int aio_getModbusCRC16(unsigned char *_pBuf, unsigned short int _usLen)
{
    unsigned short int CRCValue = 0xFFFF;          
    unsigned char i,j;

    for(i=0;i<_usLen;++i)
    {
        CRCValue  ^= *(_pBuf+i);                   
        for(j=0;j<8;++j)                           
        {
            if((CRCValue & 0x01) == 0x01)          
            {
                 CRCValue = (CRCValue >> 1)^0xA001; 
            }
			else 
            {
                CRCValue >>= 1;    
            }
        }
    } 


    CRCValue = ((CRCValue & 0x00ff) << 8) | ((CRCValue & 0xff00) >> 8);
    return CRCValue;            
}


int aio_ConvertTo32(int la, int lb, int lc, int ld)
{
	la = 0XFFFFFF | (la << 24);
	lb = 0XFF00FFFF | (lb << 16);
	lc = 0XFFFF00FF | (lc << 8);
	ld = 0XFFFFFF00 | ld;
	return la & lb & lc & ld;
}
void aio_parseFaultData(uint8_t *dataBuf,int datalen)
{

	//modbus_log_hex_print(dataBuf, datalen);
	int m=0;
	for(int j =0;j<datalen;j++)
	{
		char buf[4];
		memcpy(buf, dataBuf+j, 2);
		int data = aio_HexArrayToDec(buf, 2);
		if(data>0){
			j++;
			
			sprintf(aio_eventDisplay.system_faults + m*5, "E%03d,", data); 
		
			m++;
		}
	}
	//printf("eventDisplay.system_faults:%s\r\n",eventDisplay.system_faults);
}
void aio_parseAlarmData(uint8_t *dataBuf,int datalen)
{
	
	int m=0;
	for(int j =0;j<datalen;j++)
	{
		char buf[4];
		memcpy(buf, dataBuf+j, 2);
		int data = aio_HexArrayToDec(buf, 2);
		if(data>0){
			j++;
			
			sprintf(aio_eventDisplay.system_alarms+ m*5, "W%03d,", data); 
		
			m++;
		}
	}
	//printf("eventDisplay.system_alarms: %s\r\n", eventDisplay.system_alarms);
}
#if 1
void aio_parseData(uint8_t *dataBuf,int datalen)
{
//	printf("parseData\r\n");
//	modbus_log_hex_print(exteralBuf, datalen);
int wifi_state=0;
	for(int j =0;j<datalen;j++)
	{
		char buf[4];
		memcpy(buf, dataBuf+j, 2);
		int data = aio_HexArrayToDec(buf, 2);
		
		j++;		
	//	printf("data:%d,i:%d\n",data,j);
		
		
		switch(j){
			case 0:
			case 1://cu1 num	0
				aio_eventDisplay.photovoltaic_panel_power_generation= data;
				break;
			case 2:
			case 3://cu1 num1soc	1
				aio_eventDisplay.rated_power_generation_value_of_photovoltaic_panels = data;
				break;
			case 4:
			case 5://cu1 num2soc	2
				aio_eventDisplay.photovoltaic_panel_power_generation_power_return_value= data;
				break;
			case 6:
			case 7://cu1 num3soc	3
				aio_eventDisplay.photovoltaic_panels_generate_electricity = data;
				break;
			case 8:
			case 9://cu1 num4soc	4
				aio_eventDisplay.draw_power_from_the_grid = data;
				break;
			case 10:
			case 11://cu2 num	5
				aio_eventDisplay.sell_electricity_to_the_grid= data;
				break;
			case 12:
			case 13://cu2 num1soc	6
				aio_eventDisplay.rated_power_of_the_grid_to_take_or_sell_electricity= data;
				break;
			case 14:
			case 15://cu2 num2soc	7
				aio_eventDisplay.grid_power_take_or_sell_power_return_variance= data;
				break;
			case 16:
			case 17://cu2 num3soc	8
				aio_eventDisplay.electricity_is_taken_from_the_grid= data;
				break;
			case 18:
			case 19://cu2 num4soc	9
				aio_eventDisplay.sell_electricity_to_the_grid_on_the_same_day= data;
				break;
			case 20:
			case 21://cu3 num1	10
				aio_eventDisplay.tram_charging_power= data;
				break;
			case 22:
			case 23://cu3 num1soc	11
				aio_eventDisplay.rated_charging_power_value_of_the_tram= data;
				break;
			case 24:
			case 25://cu3 num2soc   12
				aio_eventDisplay.tram_charging_power_return_difference = data;
				break;
			case 26:
			case 27://cu3 num3soc   13
				aio_eventDisplay.amount_of_charging_the_tram = data;
				break;
			case 28:
			case 29://cu3 num4soc    14
				aio_eventDisplay.electricity_power_for_household_loads= data;
				break;
			case 30:
			case 31://system_battery_state  15
				aio_eventDisplay.rated_power_value_of_the_household_load= data;
				break;
			case 32:
			case 33://16
				aio_eventDisplay.power_return_value_of_household_load_power= data;
				break;
			case 34:
			case 35://17
				aio_eventDisplay.household_load_electricity_consumption_per_day= data;
				break;
			case 36:
			case 37://18
				aio_eventDisplay.household_loads_are_rated_for_daily_electricity_consumption= data;
				break;
			case 38:
			case 39://19
				aio_eventDisplay.soc_5ko_smart_system= data;
				break;
			
			case 40:
			case 41://20
				for(int i = 7 ; i >= 0 ; i--)
				{
					switch(i) {
						case 7:
						case 6:
						case 5:
						
							break;
						case 4:
							if(data & (1 << i))
								aio_eventDisplay.system_alarm_state=1;
							else 
								aio_eventDisplay.system_alarm_state=0;
							break;
						case 3:
							if(data & (1 << i))
								aio_eventDisplay.system_fault_state=1;
							else 
								aio_eventDisplay.system_fault_state=0;
							break;
						case 2:
							if(data & (1 << i))
								wifi_state=1;
							else 
								wifi_state=0;
							break;
					    case 1:
							if(data & (1 << i))
								aio_eventDisplay.wifi_state=wifi_state<<1|1;
							else 
								aio_eventDisplay.wifi_state=wifi_state<<1|0;
							break;
						case 0:

							if(data & (1 << i))
								aio_eventDisplay.bt_state=1;
							else 
								aio_eventDisplay.bt_state=0;
							break;
						}
					
				}
				break ;

			case 42:
			case 43: // 21
				for(int i = 7 ; i >= 0 ; i--)
				{
					switch(i) {
						case 7:
						case 6:
						case 5:
						case 4:
							break;
						case 3:
							if(data & (1 << i))
								aio_eventDisplay.home_state=1;
							else 
								aio_eventDisplay.home_state=0;
							break;
						case 2:
							if(data & (1 << i))
								aio_eventDisplay.auto_state=1;
							else 
								aio_eventDisplay.auto_state=0;
							break;
					    case 1:
							if(data & (1 << i))
								aio_eventDisplay.grid_state=1;
							else 
								aio_eventDisplay.grid_state=0;
							break;
						case 0:

							if(data & (1 << i))
								aio_eventDisplay.panel_state=1;
							else 
								aio_eventDisplay.panel_state=0;
							break;
						}
					
				}
				break ;
#ifdef AIC_LVGL_AIOBATTERY	
			case 44:
			case 45: // 22
			data=data>200?200:data;
				aio_eventDisplay.system_alarm_nums= data ;
				aio_system_alarm_nums=aio_eventDisplay.system_alarm_nums;
				if(aio_system_alarm_nums>0)
				{
					if(aio_system_alarm_nums%120==0)
						aio_system_alarm_counts=aio_system_alarm_nums/120;
					else
						aio_system_alarm_counts=aio_system_alarm_nums/120+1;
						
				}
				else{
					aio_system_alarm_counts=0;
					memset(aio_eventDisplay.system_alarms,0,2048);
					}

			//	printf("eventDisplay.system_alarm_nums:%d,system_alarm_counts:%d\r\n",eventDisplay.system_alarm_nums,system_alarm_counts);
				break;
			case 46:
			case 47: // 23
			case 48:
				
				data=data>200?200:data;
				
				aio_eventDisplay.system_fault_nums= data ;
				aio_system_fault_nums=aio_eventDisplay.system_fault_nums;
				if(aio_system_fault_nums>0)
				{
					if(aio_system_fault_nums%120==0)
						aio_system_fault_counts=aio_system_fault_nums/120;
					else
						aio_system_fault_counts=aio_system_fault_nums/120+1;
						
				}
				else
				{
					aio_system_fault_counts=0;
					memset(aio_eventDisplay.system_faults,0,2048);
				}
				//printf("eventDisplay.system_fault_nums:%d,system_fault_nums:%d\r\n",eventDisplay.system_fault_nums,system_fault_nums);
					
				break;
	#else
			case 98:
			case 99: // 49
				data=data>200?200:data;
				aio_eventDisplay.system_alarm_nums= data ;
				aio_system_alarm_nums=aio_eventDisplay.system_alarm_nums;
				if(aio_system_alarm_nums>0)
				{
					if(aio_system_alarm_nums%120==0)
						aio_system_alarm_counts=aio_system_alarm_nums/120;
					else
						aio_system_alarm_counts=aio_system_alarm_nums/120+1;
						
				}
				else
				{
					aio_system_alarm_counts=0;
					memset(aio_eventDisplay.system_alarms,0,2048);
				}

			//	printf("eventDisplay.system_alarm_nums:%d,system_alarm_counts:%d\r\n",eventDisplay.system_alarm_nums,system_alarm_counts);
				break;
			case 100:
			case 101: // 50

				data=data>200?200:data;
				
				aio_eventDisplay.system_fault_nums= data ;
				aio_system_fault_nums=aio_eventDisplay.system_fault_nums;
				if(aio_system_fault_nums>0)
				{
					if(aio_system_fault_nums%120==0)
						aio_system_fault_counts=aio_system_fault_nums/120;
					else
						aio_system_fault_counts=aio_system_fault_nums/120+1;
						
				}
				else
				{
					aio_system_fault_counts=0;
					memset(aio_eventDisplay.system_faults,0,2048);
				}
				//printf("eventDisplay.system_fault_nums:%d,system_fault_nums:%d\r\n",eventDisplay.system_fault_nums,system_fault_nums);
					
				break;
	#endif
			case 49: // 24
			case 50:
			case 51: // 25
			case 52:
			case 53: // 26
			case 54:
			case 55: // 27
			case 56:
			case 57: // 28

			case 58:
			case 59: // 29

			
			case 60:
			case 61: // 30
			
			case 62:
			case 63: // 31
			case 64:
			case 65: // 32
			case 66:
			case 67: // 33
			case 68:
			case 69: // 34

			case 70:
			case 71: // 35
				
			case 72:
			case 73: // 36
			

			
			case 74:
			case 75: // 37
				
			case 76:
			case 77: // 38
				
			case 78:
			case 79: // 39
				
			case 80:
			case 81: // 40
				
			case 82:
			case 83: // 41
				
			case 84:
			case 85: // 42
				
			case 86:
			case 87: // 43
				
			case 88:
			case 89: // 44
				
			case 90:
			case 91: // 45
				
			case 92:
			case 93: // 46
				
			case 94:
			case 95: // 47
				
			case 96:
			case 97: // 48
				break;
					
				
			default:
				break ;
		}		
	}
	memset(aio_exteralBuf, 0, EXTERNAL_BUFFER_SIZE);
}
#endif

void aio_send_fault_or_alarm(int add,int nums)
{
	unsigned int   write_datalen=0;	
	modbus_read_reg_info_st reg_info;	
	//modbus_write_reg_info_st  write_s_reg;	
	//modbus_rtu_reg_data_val_st data_val;	
	memset(aio_event_str_send, 0, MAX_OUTDATA_SIZE);
	memset(&reg_info, 0, sizeof(modbus_read_reg_info_st));	
	//reg_info.slave_addr     = 0x02;	
#ifdef USER_MODBUS
#ifdef AIC_LVGL_SDBATTERY
	reg_info.func_code      = E_FUNC_CODE_READ_COILS;

	reg_info.slave_addr     = 0x02;	
#else
#ifdef AIC_LVGL_AIOBATTERY
	reg_info.func_code      = E_FUNC_CODE_READ_HOLDING_REGISTERS;
	reg_info.slave_addr     = 0xB0;	
#endif
#endif

	
#else

#ifdef AIC_LVGL_AIOBATTERY
		reg_info.func_code		= E_FUNC_CODE_READ_HOLDING_REGISTERS;
		reg_info.slave_addr 	= 0xB0; 
#else
	reg_info.slave_addr     = 0x02;
	reg_info.func_code      = E_FUNC_CODE_READ_HOLDING_REGISTERS;
#endif

#endif
	reg_info.register_addr  = add;					
	reg_info.register_cnt   =nums;					
	aio_readDataLen=nums;	

//	printf("send_fault :add:%d,nums:%d\r\n",add,nums);
	modbus_rtu_pack_read_reg(&reg_info, aio_event_str_send, &write_datalen);
	rt_device_write(aio_event_uart_serial, 0, aio_event_str_send, (sizeof(aio_event_str_send) - 1));
	
	if(add>=501)
		isAioFaultSend=true;
	else
		isAioAlarmSend=true;
	//printf("send Fault and alarm buff:");
	//modbus_log_hex_print(aio_event_str_send, write_datalen);
			
	aicos_msleep(100);
}

void aio_event_uart_serial_thread_entry(void *parameter)
{
	
    int readLen = 0;
    uint32_t tick=0, lasttick=0,readtick=0,writetick=0;		
	
	unsigned int   count = 0,datalen=0,write_datalen=0,timeout=0;	
	bool isToMc = false, bFreshVer = false;	
	bool isCmdCompleted = false; 	
	unsigned short int CRCValue = 0xFFFF; 	
	modbus_read_reg_info_st reg_info;	
	modbus_write_reg_info_st  write_s_reg;	
	modbus_rtu_reg_data_val_st data_val;	
   	int system_fault_count=0;
	int system_alarm_count=0;
    while (!g_aio_eventexit)
    {
	#if 1
		if(isAioFault)
		{
			if(aio_system_fault_nums>0){

				if(aio_system_fault_counts==1)
				{
					aio_send_fault_or_alarm(501+system_fault_count*120,aio_system_fault_nums);
					isAioFault=false;
					system_fault_count=0;
				}
				else{
					if(system_fault_count==aio_system_fault_counts-1){
						aio_send_fault_or_alarm(501+system_fault_count*120,aio_system_fault_nums%120);
						isAioFault=false;
						system_fault_count=0;
					}else
						aio_send_fault_or_alarm(501+system_fault_count*120,120);
					system_fault_count++;
				}
			}	  				
		}
		else if(isAioAlarm)
		{		 
			//	isAlarm=false;				
			if(aio_system_alarm_nums>0)
			{
				if(aio_system_alarm_nums>0)
				{
					if(aio_system_alarm_counts==1)
					{
						aio_send_fault_or_alarm(51+system_alarm_count*120,aio_system_alarm_nums);
						isAioAlarm=false;
						system_alarm_count=0;
					}
					else{
						if(system_alarm_count==aio_system_alarm_counts-1){
							aio_send_fault_or_alarm(51+system_alarm_count*120,aio_system_alarm_nums%120);
							isAioAlarm=false;
							system_alarm_count=0;
						}else
							aio_send_fault_or_alarm(51+system_alarm_count*120,120);
						system_alarm_count++;
					}
				}	  
			}
		 } 			
		 else
		{
			memset(aio_event_str_send, 0, MAX_OUTDATA_SIZE);
			memset(&reg_info, 0, sizeof(modbus_read_reg_info_st));	
			//reg_info.slave_addr     = 0x02;					
			#ifdef USER_MODBUS
#ifdef AIC_LVGL_SDBATTERY
			reg_info.func_code      = E_FUNC_CODE_READ_COILS;

			reg_info.slave_addr     = 0x02;	
#else
#ifdef AIC_LVGL_AIOBATTERY
			reg_info.func_code      = E_FUNC_CODE_READ_HOLDING_REGISTERS;

			reg_info.slave_addr     = 0xB0;	
#endif
#endif
			#else
			
#ifdef AIC_LVGL_AIOBATTERY
				reg_info.func_code		= E_FUNC_CODE_READ_HOLDING_REGISTERS;
				reg_info.slave_addr 	= 0xB0; 
#else
				reg_info.slave_addr 	= 0x02;
				reg_info.func_code		= E_FUNC_CODE_READ_HOLDING_REGISTERS;
#endif
			#endif
			reg_info.register_addr  = 0;	
			
#ifdef AIC_LVGL_AIOBATTERY
				reg_info.register_cnt   =24 ;					
			aio_readDataLen=24;
#else
			reg_info.register_cnt   =51 ;					
			aio_readDataLen=51;
#endif
				
			modbus_rtu_pack_read_reg(&reg_info, aio_event_str_send, &write_datalen);

			writetick = rt_tick_get();
			if ((writetick - lasttick) >= 500)//500
			{
				 lasttick  = writetick;
				 rt_device_write(aio_event_uart_serial, 0, aio_event_str_send, (sizeof(aio_event_str_send) - 1));
				isAioDataSend=true;
			//	printf("sendbuff:");
			//	modbus_log_hex_print(aio_event_str_send, write_datalen);
			}
	        aicos_msleep(100);
		}
	#endif
		 memset(aio_str_receive, 0, EXTERNAL_BUFFER_SIZE);
		 tick = rt_tick_get();
		 readLen = rt_device_read(aio_event_uart_serial, -1, &aio_str_receive, EXTERNAL_BUFFER_SIZE);
		// printf("readlen:%d\r\n",readLen);
		//printf("#inDataBuf#readLen:%d,g_eventexit:%d\r\n",readLen,g_aio_eventexit);
		//modbus_log_hex_print(aio_str_receive, readLen);
		if (readLen>0)
		{		//printf("readbuff:");
				//modbus_log_hex_print(aio_str_receive, readLen);
				int real_len=readLen;
				if ((aio_str_receive[0] == 0xB0) && (aio_str_receive[1] == 0x03)){
					readtick  = tick;
					isAioStartPack=true;
					aio_real_data_len=aio_str_receive[2];
					aio_cmdPos=count=0;
					memset(aio_exteralBuf, 0, EXTERNAL_BUFFER_SIZE);
					memset(aio_cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
					while (real_len--)
					{
						aio_cmdBuf[aio_cmdPos++] = aio_str_receive[count];
						count++;
					}
					//modbus_log_hex_print(aio_cmdBuf, aio_cmdPos);
					if(aio_cmdPos==(aio_real_data_len+DATA_HEAD_LEN))
					{
							//printf("###########inDataBuf############count:%d,aio_readDataLen:%d\r\n",count,aio_readDataLen);
						//	modbus_log_hex_print(aio_cmdBuf, aio_cmdPos);
							#if 1
							memset(&data_val,0,sizeof(modbus_rtu_reg_data_val_st));
							if(modbus_rtu_unpack_read_register(&reg_info,aio_cmdBuf,aio_cmdPos,&data_val)==M_MODBUS_OK)
							{
								//printf("###########modbus_rtu_unpack_read_register M_MODBUS_OK \r\n");
								count=0;
								aio_cmdPos=0;
								isAioStartPack=false;
								aio_real_data_len=0;
								timeout=0;
								isCmdCompleted=true;
								if (!aio_eventDisplay.dataisvalid)
									printf("1 eventDisplay.dataisvalid will change to 1\n") ;
								aio_eventDisplay.dataisvalid = 1 ;
							}
							else
							{
								//printf("###########modbus_rtu_unpack_read_register M_MODBUS_ERR \r\n");
								isCmdCompleted=false;
								count=0;
								aio_cmdPos=0;
								aio_real_data_len=0;
								isAioStartPack=false;
								timeout++;
								if(timeout>2500)
								{
									printf("1 eventDisplay.dataisvalid will change to 0 %d\n", __LINE__) ;
									aio_eventDisplay.dataisvalid = 0 ;
									
									//memset(&aio_eventDisplay, 0, sizeof(AioEventDisplay));
								}
							}
							#endif
							
						}			
				}
				else{
						if(isAioStartPack)
						{
							count=0;
							while (real_len--)
							{
								aio_cmdBuf[aio_cmdPos++] = aio_str_receive[count];
								count++;
							}
						//	modbus_log_hex_print(aio_str_receive, count);
							if(aio_cmdPos==(aio_real_data_len+DATA_HEAD_LEN))
							{
									//printf("###########inDataBuf############count:%d,aio_readDataLen:%d\r\n",count,aio_readDataLen);
								//	modbus_log_hex_print(aio_cmdBuf, aio_cmdPos);
									#if 1
									memset(&data_val,0,sizeof(modbus_rtu_reg_data_val_st));
									if(modbus_rtu_unpack_read_register(&reg_info,aio_cmdBuf,aio_cmdPos,&data_val)==M_MODBUS_OK)
									{
										//printf("###########modbus_rtu_unpack_read_register M_MODBUS_OK \r\n");
										count=0;
										aio_cmdPos=0;
										isAioStartPack=false;
										aio_real_data_len=0;
										timeout=0;
										isCmdCompleted=true;
										if (!aio_eventDisplay.dataisvalid)
											printf("2 eventDisplay.dataisvalid will change to 1\n") ;
										aio_eventDisplay.dataisvalid = 1 ;
									}
									else
									{
										//printf("###########modbus_rtu_unpack_read_register M_MODBUS_ERR \r\n");
										isCmdCompleted=false;
										count=0;
										aio_cmdPos=0;
										aio_real_data_len=0;
										isAioStartPack=false;
										timeout++;
										if(timeout>2500)
										{
											printf("2 eventDisplay.dataisvalid will change to 0 %d\n", __LINE__) ;
											aio_eventDisplay.dataisvalid = 0 ;
											
											//memset(&aio_eventDisplay, 0, sizeof(AioEventDisplay));
										}
									}
									#endif
									
								}
						}
						else
							{
								printf("###########read from UART1############ readLen:%d, readDataLen:%d, %d\r\n", readLen, aio_readDataLen, __LINE__);
			
								for (int ttt = 0 ; ttt < readLen; ttt++) {
									printf("%02X ", aio_str_receive[ttt]) ;
								}
								
								printf("\n") ;
							}
						
					}
				
		}
		
		
		#if 1
		if ((tick - readtick) >= 5000)//2500
		{	           
			  readtick  = tick;
			  count=0;
			  aio_cmdPos=0;
			  timeout=0;
			  isCmdCompleted=false;
			  printf("exteralDisplay.dataisvalid will change to 0 %d\n", __LINE__) ;
			  aio_eventDisplay.dataisvalid = 0 ;
			 	
			 // memset(&aio_eventDisplay, 0, sizeof(AioEventDisplay));
		}
		#endif

		// If read data is completed, start to parse data
		if (isCmdCompleted)
		{
			
			isCmdCompleted=false;
			//printf("###########data_val.read_register_cnt:%d \r\n",data_val.read_register_cnt);
			//modbus_log_hex_print(data_val.reg_data.storage_reg_value,data_val.read_register_cnt);		
			switch(aio_cmdBuf[1])
			{
				case 0x00:	
				#ifdef USER_MODBUS
				case 0x03:
				#else
				case 0x01:
				#endif
				case 0x02:
				break;
				#ifdef USER_MODBUS
				case 0x01:
				#else
				case 0x03://read
				#endif
				{ 
					#if 1
					unsigned char cmdLen;
					#ifndef USER_MODBUS
					cmdLen= aio_cmdBuf[2];
					#else
					char buf[4];
					memcpy(buf, aio_cmdBuf+2, 2);
					int data = aio_HexArrayToDec(buf, 2);
					cmdLen=data;
					//printf("data:%d,cmdLen:%d\r\n",data,cmdLen);
					#endif
					#ifdef AIC_LVGL_AIOBATTERY
					if (cmdLen >= 48 &&isAioDataSend)
					#else
					if (cmdLen >= 102 &&isAioDataSend)
					#endif
					{
						isAioDataSend=false;
						memcpy(aio_exteralBuf, aio_cmdBuf + DATA_START_ID, cmdLen );
						//memcpy(exteralBuf, cmdBuf + 3, cmdLen );
						aio_parseData(aio_exteralBuf, cmdLen);	
						
					    if(aio_system_alarm_nums>0&&aio_system_fault_nums<=0)
					   	{
					     	isAioAlarm=true;
					   		isAioFault=false;
					   	}
  					    else if((aio_system_fault_nums>0&&aio_system_alarm_nums<=0)
					   	||(aio_system_fault_nums>0&&aio_system_alarm_nums>0))
					   	{
					     	isAioFault=true;
					   		isAioAlarm=false;
					   	}
					   
						
					}
					else if(cmdLen==aio_readDataLen*2 &&isAioFaultSend )
					{
						isAioFaultSend=false;
						memcpy(aio_exteralBuf, aio_cmdBuf + DATA_START_ID, cmdLen );
						//memcpy(exteralBuf, cmdBuf + 3, cmdLen );
						aio_parseFaultData(aio_exteralBuf, cmdLen);	
						
						if(!isAioFault&&(aio_system_alarm_nums>0))
							isAioAlarm=true;
						else
							isAioAlarm=false;
					}
					else if(cmdLen==aio_readDataLen*2&&isAioAlarmSend)
					{
						isAioAlarmSend=false;
						memcpy(aio_exteralBuf, aio_cmdBuf + DATA_START_ID, cmdLen );
						aio_parseAlarmData(aio_exteralBuf, cmdLen);	
					}
					#endif
				}
					break;			
				
				default:
					break;
			}
			memset(aio_cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
			isCmdCompleted = false;
		}
		//aicos_msleep(10);
		rt_thread_mdelay(100);
    }

    rt_sem_detach(&aio_event_rx_sem);
   g_aio_eventexit = 1;
    rt_device_close(aio_event_uart_serial);
}

int aio_uart_run(const char *uart_name)
{
    rt_err_t ret = RT_EOK;
    g_aio_eventexit = 0;

	uint32_t tick=0, lasttick=0,readtick=0;		
	unsigned char readLen = 0;	
	unsigned char outDataBuf[MAX_OUTDATA_SIZE]; 	
	unsigned int   count = 0,datalen=0,write_datalen=0,timeout=0;	
	bool isToMc = false, bFreshVer = false;	
	bool isCmdCompleted = false; 	
	unsigned short int CRCValue = 0xFFFF; 	
	modbus_read_reg_info_st reg_info;	
	modbus_write_reg_info_st  write_s_reg;	
	modbus_rtu_reg_data_val_st data_val;	
	 
  	memset(&aio_eventDisplay,0,sizeof(AioEventDisplay));
	printf("aio_uart_run :%s\r\n",uart_name);
    aio_event_uart_serial = rt_device_find(uart_name);
    if (!aio_event_uart_serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    rt_sem_init(&aio_event_rx_sem, "aio_event_rx_sem", 0, RT_IPC_FLAG_FIFO);

    ret = rt_device_open(aio_event_uart_serial, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);

    if (ret != RT_EOK)
    {
        rt_kprintf("open %s failed : %d !\n", uart_name, ret);
        return RT_ERROR;
    }

    rt_device_set_rx_indicate(aio_event_uart_serial, aio_eventuart_input);

#if 0
    // NOTE: thread stack-size at least for 1024*2 Bytes !!!
    rt_thread_t thread = rt_thread_create("aio_event_uart_serial", aio_event_uart_serial_thread_entry, RT_NULL, 1024*4, 15, 10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
#else

 ret= rt_thread_init(&thread_uart, "aio_event_uart_serial",aio_event_uart_serial_thread_entry, RT_NULL, &thread_stack[0], sizeof(thread_stack), 15, 10);

 if (ret != RT_EOK) {
        printf("failed to create aio_event_uart_serial thread");
        return;
    }
    (void)ret;
    rt_thread_startup(&thread_uart);
#endif
    return ret;
}

#endif

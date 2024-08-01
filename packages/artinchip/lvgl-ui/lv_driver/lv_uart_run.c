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
#include "modbus_rtu.h"
#include "modbus_common.h"
//#include "sdbattery_ui.h"
#define SAMPLE_UART_NAME "uart5"
#define MAX_OUTDATA_SIZE         9//64
#define EXTERNAL_BUFFER_SIZE    2048//128//80

#define USER_MODBUS   

#ifdef USER_MODBUS
#define DATA_HEAD_LEN   6
#define DATA_START_ID   4
#else
#define DATA_HEAD_LEN   5
#define DATA_START_ID   3
#endif

struct rt_semaphore event_rx_sem;
rt_device_t event_uart_serial;
//char event_str_send[] = "1234567890ArtInChip1234567890\n";
static unsigned char event_str_send[MAX_OUTDATA_SIZE];
static unsigned char str_receive[EXTERNAL_BUFFER_SIZE];
static unsigned char cmdBuf[EXTERNAL_BUFFER_SIZE];
static unsigned char exteralBuf[EXTERNAL_BUFFER_SIZE];
static unsigned int cmdPos = 0;
int g_eventexit = 0;
unsigned int readDataLen=0; 
EventDisplay eventDisplay;
static int system_fault_nums=0;
static int system_fault_counts=0;
static int system_alarm_nums=0;
static int system_alarm_counts=0;
static bool isFault=false;
static bool isAlarm=false;
static bool isFaultSend=false;
static bool isAlarmSend=false;
static bool isDataSend=false;
static bool isStartPack=false;
static unsigned int real_data_len=0;
rt_err_t eventuart_input(rt_device_t dev, rt_size_t size)
{
    if (size > 0)
        rt_sem_release(&event_rx_sem);

    return RT_EOK;
}

const unsigned char Num2CharTable[] = "0123456789ABCDEF";
void HexArrayToString(uint8_t *hexarray, int length, uint8_t *string)
{
    int i = 0;
    while(i < length)
    {
        *(string++) = Num2CharTable[(hexarray[i] >> 4) & 0x0f];
        *(string++) = Num2CharTable[hexarray[i] & 0x0f];
        i++;
    }
    *string = 0x0;
}

long int HexArrayToDec(uint8_t *hexarray, int len)
{
    uint8_t dst[50];
    HexArrayToString(hexarray, len, dst);
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

static unsigned short int getModbusCRC16(unsigned char *_pBuf, unsigned short int _usLen)
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


int ConvertTo32(int la, int lb, int lc, int ld)
{
	la = 0XFFFFFF | (la << 24);
	lb = 0XFF00FFFF | (lb << 16);
	lc = 0XFFFF00FF | (lc << 8);
	ld = 0XFFFFFF00 | ld;
	return la & lb & lc & ld;
}
void parseFaultData(uint8_t *dataBuf,int datalen)
{

	//modbus_log_hex_print(dataBuf, datalen);
	int m=0;
	for(int j =0;j<datalen;j++)
	{
		char buf[4];
		memcpy(buf, dataBuf+j, 2);
		int data = HexArrayToDec(buf, 2);
		if(data>0){
			j++;
			
			sprintf(eventDisplay.system_faults + m*5, "E%03d,", data); 
		
			m++;
		}
	}
	//printf("eventDisplay.system_faults:%s\r\n",eventDisplay.system_faults);
}
void parseAlarmData(uint8_t *dataBuf,int datalen)
{
	
	int m=0;
	for(int j =0;j<datalen;j++)
	{
		char buf[4];
		memcpy(buf, dataBuf+j, 2);
		int data = HexArrayToDec(buf, 2);
		if(data>0){
			j++;
			
			sprintf(eventDisplay.system_alarms+ m*5, "W%03d,", data); 
		
			m++;
		}
	}
	//printf("eventDisplay.system_alarms: %s\r\n", eventDisplay.system_alarms);
}
#if 1
void parseData(uint8_t *dataBuf,int datalen)
{
//	printf("parseData\r\n");
//	modbus_log_hex_print(exteralBuf, datalen);
	for(int j =0;j<datalen;j++)
	{
		char buf[4];
		memcpy(buf, dataBuf+j, 2);
		int data = HexArrayToDec(buf, 2);
		
		j++;		
	//	printf("data:%d,i:%d\n",data,j);
		switch(j){
			case 0:
			case 1://cu1 num	0
				eventDisplay.cluster1_battery_num= data;
				break;
			case 2:
			case 3://cu1 num1soc	1
				eventDisplay.cluster1_battery1_soc = data;
				break;
			case 4:
			case 5://cu1 num2soc	2
				eventDisplay.cluster1_battery2_soc= data;
				break;
			case 6:
			case 7://cu1 num3soc	3
				eventDisplay.cluster1_battery3_soc = data;
				break;
			case 8:
			case 9://cu1 num4soc	4
				eventDisplay.cluster1_battery4_soc = data;
				break;
			case 10:
			case 11://cu2 num	5
				eventDisplay.cluster2_battery_num= data;
				break;
			case 12:
			case 13://cu2 num1soc	6
				eventDisplay.cluster2_battery1_soc= data;
				break;
			case 14:
			case 15://cu2 num2soc	7
				eventDisplay.cluster2_battery2_soc= data;
				break;
			case 16:
			case 17://cu2 num3soc	8
				eventDisplay.cluster2_battery3_soc= data;
				break;
			case 18:
			case 19://cu2 num4soc	9
				eventDisplay.cluster2_battery4_soc= data;
				break;
			case 20:
			case 21://cu3 num1	10
				eventDisplay.cluster3_battery_num= data;
				break;
			case 22:
			case 23://cu3 num1soc	11
				eventDisplay.cluster3_battery1_soc= data;
				break;
			case 24:
			case 25://cu3 num2soc   12
				eventDisplay.cluster3_battery2_soc = data;
				break;
			case 26:
			case 27://cu3 num3soc   13
				eventDisplay.cluster3_battery3_soc = data;
				break;
			case 28:
			case 29://cu3 num4soc    14
				eventDisplay.cluster3_battery4_soc= data;
				break;
			case 30:
			case 31://system_battery_state  15
				eventDisplay.system_battery_state= data;
				break;
			case 32:
			case 33://16
				eventDisplay.cluster1_battery_soc= data;
				break;
			case 34:
			case 35://17
				eventDisplay.cluster2_battery_soc= data;
				break;
			case 36:
			case 37://18
				eventDisplay.cluster3_battery_soc= data;
				break;
			case 38:
			case 39://19
				eventDisplay.system_soc= data;
				break;
			case 40:
			case 41://20
				eventDisplay.system_voltage= data;
				break;
			case 42:
			case 43://21
				eventDisplay.system_current=data;
				break;
			case 44:
			case 45://22
				eventDisplay.system_charging_power= data;
				break;
			case 46:
			case 47://23
				eventDisplay.system_discharge_power= data;
				break;
			case 48:
			case 49://24
			 	eventDisplay.parallel_system_total_charging_energy_high=data;			   
				break;
			case 50:
			case 51://25
				{ 
					eventDisplay.parallel_system_total_charging_energy_low=data;				
					eventDisplay.parallel_system_total_charging_energy= eventDisplay.parallel_system_total_charging_energy_high << 4|eventDisplay.parallel_system_total_charging_energy_low;
				//	printf("eventDisplay.parallel_system_total_charging_energy:%d\r\n",eventDisplay.parallel_system_total_charging_energy);
			    
				}
				break;
			case 52:
			case 53://26
				eventDisplay.parallel_system_total_discharge_energy_high=data;
				break;
			case 54:
			case 55://27
				{
					eventDisplay.parallel_system_total_discharge_energy_low=data;
					eventDisplay.parallel_system_total_discharge_energy= eventDisplay.parallel_system_total_discharge_energy_high<<4|eventDisplay.parallel_system_total_discharge_energy_low;
					//printf("eventDisplay.parallel_system_total_discharge_energy:%d\r\n",eventDisplay.parallel_system_total_discharge_energy);
			    }
				break;

			case 56:
			case 57://28
				eventDisplay.parallel_system_total_charging_time_high=data;
				break;
			case 58:
			case 59://29
				{
					eventDisplay.parallel_system_total_charging_time_low= data;
					eventDisplay.parallel_system_total_charging_time=eventDisplay.parallel_system_total_charging_time_high <<4 ||eventDisplay.parallel_system_total_charging_time_low;
						
				}
				break;
			case 60:
			case 61://30
			 	eventDisplay.parallel_system_total_discharge_time_high= data;
				break;
			case 62:
			case 63://31
				{
					eventDisplay.parallel_system_total_discharge_time_low= data ;
				    eventDisplay.parallel_system_total_discharge_time=eventDisplay.parallel_system_total_discharge_time_high<<4||eventDisplay.parallel_system_total_discharge_time_low;
				}
				break ;
			case 64:
			case 65://32
				eventDisplay.parallel_system_standby_time_high= data ;
				break ;
			case 66:
			case 67://33
				{
					eventDisplay.parallel_system_standby_time_low= data ;
					eventDisplay.parallel_system_standby_time=eventDisplay.parallel_system_standby_time_high<<4||eventDisplay.parallel_system_standby_time_low;
				}
				break ;
			case 68:
			case 69://34
				for(int i = 7 ; i >= 0 ; i--)
				{
					switch(i) {
						case 7:
						case 6:
						case 5:
						case 4:
						case 3:
							if(data & (1 << i))
								eventDisplay.system_alarm_state=1;
							else 
								eventDisplay.system_alarm_state=0;
							break;
						case 2:
							if(data & (1 << i))
								eventDisplay.system_fault_state=1;
							else 
								eventDisplay.system_fault_state=0;
							break;
					    case 1:
							if(data & (1 << i))
								eventDisplay.wifi_state=1;
							else 
								eventDisplay.wifi_state=0;
							break;
						case 0:

							if(data & (1 << i))
								eventDisplay.bt_state=1;
							else 
								eventDisplay.bt_state=0;
							break;
						}
					
				}
				break ;
			case 70:
			case 71: // 35
				eventDisplay.charging_animation_range=data;
				break;
				
			case 72:
			case 73: // 36
				eventDisplay.discharging_animation_range=data;
				break;
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
			case 98:
			case 99: // 49
				data=data>200?200:data;
				eventDisplay.system_alarm_nums= data ;
				system_alarm_nums=eventDisplay.system_alarm_nums;
				if(system_alarm_nums>0)
				{
					if(system_alarm_nums%120==0)
						system_alarm_counts=system_alarm_nums/120;
					else
						system_alarm_counts=system_alarm_nums/120+1;
						
				}
				else{
					system_alarm_counts=0;
					memset(eventDisplay.system_alarms,0,2048);
					}

			//	printf("eventDisplay.system_alarm_nums:%d,system_alarm_counts:%d\r\n",eventDisplay.system_alarm_nums,system_alarm_counts);
				break;
			case 100:
			case 101: // 50

				data=data>200?200:data;
				
				eventDisplay.system_fault_nums= data ;
				system_fault_nums=eventDisplay.system_fault_nums;
				if(system_fault_nums>0)
				{
					if(system_fault_nums%120==0)
						system_fault_counts=system_fault_nums/120;
					else
						system_fault_counts=system_fault_nums/120+1;
						
				}
				else
				{
					system_fault_counts=0;
					memset(eventDisplay.system_faults,0,2048);
				}
				//printf("eventDisplay.system_fault_nums:%d,system_fault_nums:%d\r\n",eventDisplay.system_fault_nums,system_fault_nums);
					
				break;			
				
			default:
				break ;
		}		
	}
//extern void cluster_update(void);
//cluster_update();
//	printf("eventDisplay.cluster1_battery_num:%d,%d,%d,soc:%d\r\n",eventDisplay.cluster1_battery_num
//	,eventDisplay.cluster2_battery_num,eventDisplay.cluster3_battery_num,eventDisplay.cluster1_battery1_soc);
	memset(exteralBuf, 0, EXTERNAL_BUFFER_SIZE);
}
#endif

void send_fault_or_alarm(int add,int nums)
{
	unsigned int   write_datalen=0;	
	modbus_read_reg_info_st reg_info;	
	//modbus_write_reg_info_st  write_s_reg;	
	//modbus_rtu_reg_data_val_st data_val;	
	memset(event_str_send, 0, MAX_OUTDATA_SIZE);
	memset(&reg_info, 0, sizeof(modbus_read_reg_info_st));	
	reg_info.slave_addr     = 0x02;	
	#ifdef USER_MODBUS
	reg_info.func_code      = E_FUNC_CODE_READ_COILS;
	#else
	reg_info.func_code      = E_FUNC_CODE_READ_HOLDING_REGISTERS;
	#endif
	reg_info.register_addr  = add;					
	reg_info.register_cnt   =nums;					
	readDataLen=nums;	

//	printf("send_fault :add:%d,nums:%d\r\n",add,nums);
	modbus_rtu_pack_read_reg(&reg_info, event_str_send, &write_datalen);
	rt_device_write(event_uart_serial, 0, event_str_send, (sizeof(event_str_send) - 1));

	if(add>=501)
		isFaultSend=true;
	else
		isAlarmSend=true;
	aicos_msleep(100);
}
void event_uart_serial_thread_entry(void *parameter)
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
    while (!g_eventexit)
    {
	#if 1

		if(isFault)
		{
			if(system_fault_nums>0){

				if(system_fault_counts==1)
				{
					send_fault_or_alarm(501+system_fault_count*120,system_fault_nums);
					isFault=false;
					system_fault_count=0;
				}
				else{
					if(system_fault_count==system_fault_counts-1){
						send_fault_or_alarm(501+system_fault_count*120,system_fault_nums%120);
						isFault=false;
						system_fault_count=0;
					}else
						send_fault_or_alarm(501+system_fault_count*120,120);
					system_fault_count++;
				}
			}	  				
		}
		else if(isAlarm)
		{		 
			//	isAlarm=false;				
			if(system_alarm_nums>0)
			{
				if(system_alarm_nums>0)
				{
					if(system_alarm_counts==1)
					{
						send_fault_or_alarm(51+system_alarm_count*120,system_alarm_nums);
						isAlarm=false;
						system_alarm_count=0;
					}
					else{
						if(system_alarm_count==system_alarm_counts-1){
							send_fault_or_alarm(51+system_alarm_count*120,system_alarm_nums%120);
							isAlarm=false;
							system_alarm_count=0;
						}else
							send_fault_or_alarm(51+system_alarm_count*120,120);
						system_alarm_count++;
					}
				}	  
			}
		 } 			
		 else
		{
			memset(event_str_send, 0, MAX_OUTDATA_SIZE);
			memset(&reg_info, 0, sizeof(modbus_read_reg_info_st));	
			reg_info.slave_addr     = 0x02;					
			#ifdef USER_MODBUS
			reg_info.func_code      = E_FUNC_CODE_READ_COILS;
			#else
			reg_info.func_code      = E_FUNC_CODE_READ_HOLDING_REGISTERS;
			#endif
			reg_info.register_addr  = 0;					
			reg_info.register_cnt   =51 ;					
			readDataLen=51;
			
			modbus_rtu_pack_read_reg(&reg_info, event_str_send, &write_datalen);

			writetick = rt_tick_get();
			if ((writetick - lasttick) >= 500)
			{
				 lasttick  = writetick;
				 rt_device_write(event_uart_serial, 0, event_str_send, (sizeof(event_str_send) - 1));
				isDataSend=true;
				//printf("sendbuff:");
				//modbus_log_hex_print(event_str_send, write_datalen);
			
			}
	        aicos_msleep(100);
		}
	#endif
		 memset(str_receive, 0, EXTERNAL_BUFFER_SIZE);
		 tick = rt_tick_get();
		 readLen = rt_device_read(event_uart_serial, -1, &str_receive, EXTERNAL_BUFFER_SIZE);
 		//printf("readlen:%d\r\n",readLen);
		//printf("#inDataBuf#readLen:%d,g_eventexit:%d\r\n",readLen,g_eventexit);
		//modbus_log_hex_print(str_receive, readLen);
		
		if (readLen > 0) 
		{
			//printf("readbuff:");
			//modbus_log_hex_print(str_receive, readLen);
				
			int real_len=readLen;
			#ifdef USER_MODBUS
			if ((str_receive[0] == 0x02) && (str_receive[1] == 0x01))
			#else
			if ((str_receive[0] == 0x02) && (str_receive[1] == 0x03))
			#endif
			{	readtick  = tick;
				isStartPack=true;
				//readlen_one=real_len;
				#ifndef USER_MODBUS
					real_data_len=str_receive[2];
					#else
					char buf[4];
					memcpy(buf, str_receive+2, 2);
					int data = HexArrayToDec(buf, 2);
					real_data_len=data;
					//printf("data:%d,real_data_len:%d\r\n",data,real_data_len);
					#endif
					
				
				cmdPos=count=0;
				memset(exteralBuf, 0, EXTERNAL_BUFFER_SIZE);
				memset(cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
				while (real_len--)
				{
					cmdBuf[cmdPos++] = str_receive[count];
					count++;
				}
				//modbus_log_hex_print(cmdBuf, cmdPos);
				if(cmdPos==(real_data_len+DATA_HEAD_LEN))
				{
						//printf("###########inDataBuf############count:%d,readDataLen:%d\r\n",count,readDataLen);
					  //  modbus_log_hex_print(cmdBuf, cmdPos);
						#if 1
						memset(&data_val,0,sizeof(modbus_rtu_reg_data_val_st));
						if(modbus_rtu_unpack_read_register(&reg_info,cmdBuf,cmdPos,&data_val)==M_MODBUS_OK)
						{
							//printf("###########modbus_rtu_unpack_read_register M_MODBUS_OK \r\n");
							count=0;
							cmdPos=0;
							isStartPack=false;
							real_data_len=0;
							timeout=0;
							isCmdCompleted=true;
							if (!eventDisplay.dataisvalid)
								printf("1 eventDisplay.dataisvalid will change to 1\n") ;
							eventDisplay.dataisvalid = 1 ;
						}
						else
						{
							//printf("###########modbus_rtu_unpack_read_register M_MODBUS_ERR \r\n");
							isCmdCompleted=false;
							count=0;
							cmdPos=0;
							real_data_len=0;
							isStartPack=false;
							timeout++;
							if(timeout>2500)
							{
								printf("1 eventDisplay.dataisvalid will change to 0 %d\n", __LINE__) ;
								eventDisplay.dataisvalid = 0 ;
								
								//memset(&eventDisplay, 0, sizeof(EventDisplay));
							}
						}
						#endif
						
					}				
			}
			else{
					if(isStartPack)
					{
						count=0;
						while (real_len--)
						{
							cmdBuf[cmdPos++] = str_receive[count];
							count++;
						}
					//	modbus_log_hex_print(str_receive, count);
						if(cmdPos==(real_data_len+DATA_HEAD_LEN))
						{
								//printf("###########inDataBuf############count:%d,aio_readDataLen:%d\r\n",count,aio_readDataLen);
							    //modbus_log_hex_print(aio_cmdBuf, aio_cmdPos);
								#if 1
								memset(&data_val,0,sizeof(modbus_rtu_reg_data_val_st));
								if(modbus_rtu_unpack_read_register(&reg_info,cmdBuf,cmdPos,&data_val)==M_MODBUS_OK)
								{
									//printf("###########modbus_rtu_unpack_read_register M_MODBUS_OK \r\n");
									count=0;
									cmdPos=0;
									isStartPack=false;
									real_data_len=0;
									timeout=0;
									isCmdCompleted=true;
									if (!eventDisplay.dataisvalid)
										printf("2 eventDisplay.dataisvalid will change to 1\n") ;
									eventDisplay.dataisvalid = 1 ;
								}
								else
								{
									//printf("###########modbus_rtu_unpack_read_register M_MODBUS_ERR \r\n");
									isCmdCompleted=false;
									count=0;
									cmdPos=0;
									real_data_len=0;
									isStartPack=false;
									timeout++;
									if(timeout>2500)
									{
										printf("2 eventDisplay.dataisvalid will change to 0 %d\n", __LINE__) ;
										eventDisplay.dataisvalid = 0 ;
										
										//memset(&eventDisplay, 0, sizeof(EventDisplay));
									}
								}
								#endif
								
							}
					}
					else
						{
							printf("###########read from UART1############ readLen:%d, readDataLen:%d, %d\r\n", readLen, readDataLen, __LINE__);
		
							for (int ttt = 0 ; ttt < readLen; ttt++) {
								printf("%02X ", str_receive[ttt]) ;
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
			  cmdPos=0;
			  timeout=0;
			  isCmdCompleted=false;
			  printf("exteralDisplay.dataisvalid will change to 0 %d\n", __LINE__) ;
			  eventDisplay.dataisvalid = 0 ;
			 	
		}
		#endif

		// If read data is completed, start to parse data
		if (isCmdCompleted)
		{
			
			isCmdCompleted=false;
			//printf("###########data_val.read_register_cnt:%d \r\n",data_val.read_register_cnt);
			//modbus_log_hex_print(data_val.reg_data.storage_reg_value,data_val.read_register_cnt);		
			switch(cmdBuf[1])
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
				case 0x01://read
				#else
				case 0x03://read
				#endif
				{ 
					#if 1
					unsigned char cmdLen;
					#ifndef USER_MODBUS
					cmdLen= cmdBuf[2];
					#else
					char buf[4];
					memcpy(buf, cmdBuf+2, 2);
					int data = HexArrayToDec(buf, 2);
					cmdLen=data;
					//printf("data:%d,cmdLen:%d\r\n",data,cmdLen);
					#endif
					
					if (cmdLen >= 102 &&isDataSend){
						isDataSend=false;
						memcpy(exteralBuf, cmdBuf + DATA_START_ID, cmdLen );
						//memcpy(exteralBuf, cmdBuf + 3, cmdLen );
						parseData(exteralBuf, cmdLen);	
						
					   if(system_alarm_nums>0&&system_fault_nums<=0)
					   	{
					     	isAlarm=true;
					   		isFault=false;
					   	}
  					   else if((system_fault_nums>0&&system_alarm_nums<=0)
					   	||(system_fault_nums>0&&system_alarm_nums>0))
					   	{
					     	isFault=true;
					   		isAlarm=false;
					   	}
					   
						
					}
					else if(cmdLen==readDataLen*2 &&isFaultSend )
					{
						isFaultSend=false;
						memcpy(exteralBuf, cmdBuf + DATA_START_ID, cmdLen );
						//memcpy(exteralBuf, cmdBuf + 3, cmdLen );
						parseFaultData(exteralBuf, cmdLen);	
						
						if(!isFault&&(system_alarm_nums>0))
							isAlarm=true;
						else
							isAlarm=false;
					}
					else if(cmdLen==readDataLen*2&&isAlarmSend)
					{
						isAlarmSend=false;
						memcpy(exteralBuf, cmdBuf + DATA_START_ID, cmdLen );
						parseAlarmData(exteralBuf, cmdLen);	
					}
					#endif
				}
					break;			
				
				default:
					break;
			}
			memset(cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
			isCmdCompleted = false;
		}
		aicos_msleep(100);
		//usleep(10000);
    }

    rt_sem_detach(&event_rx_sem);
    g_eventexit = 1;
    rt_device_close(event_uart_serial);
}

int uart_run(const char *uart_name)
{
    rt_err_t ret = RT_EOK;
    g_eventexit = 0;

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
	 
  	memset(&eventDisplay,0,sizeof(EventDisplay));
	printf("uart_run :%s\r\n",uart_name);
    event_uart_serial = rt_device_find(uart_name);
    if (!event_uart_serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    rt_sem_init(&event_rx_sem, "event_rx_sem", 0, RT_IPC_FLAG_FIFO);

    ret = rt_device_open(event_uart_serial, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);

    if (ret != RT_EOK)
    {
        rt_kprintf("open %s failed : %d !\n", uart_name, ret);
        return RT_ERROR;
    }

    rt_device_set_rx_indicate(event_uart_serial, eventuart_input);

    // NOTE: thread stack-size at least for 1024*2 Bytes !!!
    rt_thread_t thread = rt_thread_create("event_uart_serial", event_uart_serial_thread_entry, RT_NULL, 1024*4, 15, 10);

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

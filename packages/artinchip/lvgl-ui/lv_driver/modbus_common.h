/**************************************************************************
**  名称: modbus_common.h头文件
*   日期: 
*   作者:
*   描述:
*   修改记录: 
***************************************************************************/
#ifndef __MODBUS_COMMON_H__
#define __MODBUS_COMMON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
/**************************************************************************
** 宏定义
**************************************************************************/
/* 简化类型定义 */
/*typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;*/


#define M_MODBUS_OK		(0)			/* 成功 */
#define M_MODBUS_ERR	(-1)		/* 失败 */

#ifndef bool
typedef unsigned char bool;
#endif

#ifndef false
#define false			(uint8_t)(0)
#endif

#ifndef true
#define true			(uint8_t)(1)
#endif

#define DEBUG			(1)

/* 打印重定义 */
#ifdef DEBUG
#define M_MODBUS_LOG_DEBUG(format,...)  printf("[%s][%d] debug: "format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define M_MODBUS_LOG_WARN(format,...)  	printf("[%s][%d] warn: "format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define M_MODBUS_LOG_ERROR(format,...)  printf("[%s][%d] error: "format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define M_MODBUS_LOG_INFO(format,...)  	printf("[%s][%d] info: "format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define M_MODBUS_TRACE_IN()				printf("[%s][%s][%d] trace in\n", __FILE__, __FUNCTION__, __LINE__)
#define M_MODBUS_TRACE_OUT()			printf("[%s][%s][%d] trace out\n", __FILE__, __FUNCTION__, __LINE__)
#else
#define M_MODBUS_LOG_DEBUG(format,...)
#define M_MODBUS_LOG_WARN(format,...)
#define M_MODBUS_LOG_ERROR(format,...)
#define M_MODBUS_LOG_INFO(format,...)
#define M_MODBUS_TRACE_IN()
#define M_MODBUS_TRACE_OUT()
#endif


/**************************************************************************
** 结构体声明
**************************************************************************/
/* 寄存器错误码应答 */
typedef enum _MODBUS_ERROR_CODE_E
{
	E_CODE_NO_ERR = 0,
	E_CODE_ILLEGAL_FUNC_ERR,						/* 非法功能错误 */
	E_CODE_ILLEGAL_REG_ADDR_ERR,					/* 非法寄存器数据地址 */
	E_CODE_ILLEGAL_REG_VAL_ERR,						/* 非法寄存器数据值 */
	E_CODE_SLAVER_FAULT_ERR,						/* 从设备故障 */
	E_CODE_DEALING_CONFIRM_ERR,						/* 正在确认 */
	E_CODE_OTHER_ERR,								/* 其他错误 */
} MODBUS_ERROR_CODE_E;

/* 寄存器类型 */
typedef enum _MODBUS_FUNC_CODE_TYPE_E
{
	E_FUNC_CODE_READ_COILS = 0x01,					/* 读线圈状态 */
	E_FUNC_CODE_READ_DISCRETE_INPUTS = 0x02,		/* 读离散输入状态 */
	E_FUNC_CODE_READ_HOLDING_REGISTERS = 0x03,		/* 读保持寄存器 */
	E_FUNC_CODE_READ_INPUT_REGISTERS = 0x04,		/* 读输入寄存器 */
	E_FUNC_CODE_WRITE_SINGLE_COIL = 0x05,			/* 写单个线圈 */
	E_FUNC_CODE_WRITE_SINGLE_REGISTER = 0x06,		/* 写单个保持寄存器 */
	E_FUNC_CODE_READ_EXCEPTION_STATUS = 0x07,		/* 读异常状态 */
	E_FUNC_CODE_WRITE_MULTIPLE_COILS = 0x0F,		/* 写多个线圈 */
	E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS = 0x10,	/* 写多个保持寄存器 */
	E_FUNC_CODE_REPORT_SLAVE_ID = 0x11,				/* 报告从机标识 */	
} MODBUS_FUNC_CODE_TYPE_E;

/* 支持三种协议方式, modbus ASCII, RTU, RTU_TCP */
typedef enum _MODBUS_PROTOCOL_TYPE_E
{
	E_START_PROTOCOL_TYPE = 0,
	E_ASCII_PROTOCOL_TYPE,
	E_RTU_PROTOCOL_TYPE,
	E_RTU_TCP_PROTOCOL_TYPE,
} MODBUS_PROTOCOL_TYPE_E;

/* 波特率 */
typedef enum _MODBUS_BAUD_E
{
	E_BAUD_2400BPS = 2400,
	E_BAUD_4800BPS = 4800,
	E_BAUD_9600BPS = 9600,
	E_BAUD_14400BPS = 14400,
	E_BAUD_19200BPS = 19200,
	E_BAUD_28800BPS = 28800,
	E_BAUD_38400BPS = 38400,
	E_BAUD_57600BPS = 56700,
	E_BAUD_115200BPS = 115200,
	E_BAUD_128000BPS = 128000,
	E_BAUD_256000BPS = 256000,
} MODBUS_BAUD_E;

/* 数据位 */
typedef enum _MODBUS_DATA_BIT_E
{
	E_DATA_4BITS = 4,
	E_DATA_5BITS = 5,
	E_DATA_6BITS = 6,
	E_DATA_7BITS = 7,
	E_DATA_8BITS = 8,
} MODBUS_DATA_BIT_E;

/* 停止位 */
typedef enum _MODBUS_STOP_BIT_E
{
	E_STOP_1V0BIT = 0,
	E_STOP_1V5BITS,
	E_STOP_2V0BITS,
} MODBUS_STOP_BIT_E;

/* 校验符 */
typedef enum _MODBUS_CHECK_E
{
	E_CHECK_NONE = 0,
	E_CHECK_EVEN,
	E_CHECK_ODD,
	E_CHECK_MARK,
	E_CHECK_SPACK,
} MODBUS_CHECK_E;

/* modbus的驱动口的相关参数 */
typedef struct _modbus_com_params_st
{
	char *device;				/* 硬件设备 */
	int32_t baud;				/* 波特率 */
	uint8_t data_bit;			/* 数据位 */
	uint8_t stop_bit;			/* 停止位 */
	char parity;				/* 校验符 */
} modbus_com_params_st;

/**************************************************************************
** 函数声明
**************************************************************************/
/**************************************************************************
* 函  数: void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
* 描  述: 打印hex数据
* 入  参: uint8_t *data : 需要打印的数据流
		  uint32_t datalen : 数据长度
* 出  参: void
* 返回值: void
**************************************************************************/
void modbus_log_hex_print(uint8_t *data, uint32_t datalen);

/**************************************************************************
* 函  数: int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
* 描  述: 计算crc校验函数
* 入  参: uint8_t *data : 计算crc校验的数据
		  uint32_t datalen : 计算crc校验的数据长度
* 出  参: uint16_t *crc : 计算crc校验值
* 返回值: int32_t : M_MODBUS_OK  - 成功
					M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

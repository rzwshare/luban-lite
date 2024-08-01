/**************************************************************************
**  ����: modbus_common.hͷ�ļ�
*   ����: 
*   ����:
*   ����:
*   �޸ļ�¼: 
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
** �궨��
**************************************************************************/
/* �����Ͷ��� */
/*typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;*/


#define M_MODBUS_OK		(0)			/* �ɹ� */
#define M_MODBUS_ERR	(-1)		/* ʧ�� */

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

/* ��ӡ�ض��� */
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
** �ṹ������
**************************************************************************/
/* �Ĵ���������Ӧ�� */
typedef enum _MODBUS_ERROR_CODE_E
{
	E_CODE_NO_ERR = 0,
	E_CODE_ILLEGAL_FUNC_ERR,						/* �Ƿ����ܴ��� */
	E_CODE_ILLEGAL_REG_ADDR_ERR,					/* �Ƿ��Ĵ������ݵ�ַ */
	E_CODE_ILLEGAL_REG_VAL_ERR,						/* �Ƿ��Ĵ�������ֵ */
	E_CODE_SLAVER_FAULT_ERR,						/* ���豸���� */
	E_CODE_DEALING_CONFIRM_ERR,						/* ����ȷ�� */
	E_CODE_OTHER_ERR,								/* �������� */
} MODBUS_ERROR_CODE_E;

/* �Ĵ������� */
typedef enum _MODBUS_FUNC_CODE_TYPE_E
{
	E_FUNC_CODE_READ_COILS = 0x01,					/* ����Ȧ״̬ */
	E_FUNC_CODE_READ_DISCRETE_INPUTS = 0x02,		/* ����ɢ����״̬ */
	E_FUNC_CODE_READ_HOLDING_REGISTERS = 0x03,		/* �����ּĴ��� */
	E_FUNC_CODE_READ_INPUT_REGISTERS = 0x04,		/* ������Ĵ��� */
	E_FUNC_CODE_WRITE_SINGLE_COIL = 0x05,			/* д������Ȧ */
	E_FUNC_CODE_WRITE_SINGLE_REGISTER = 0x06,		/* д�������ּĴ��� */
	E_FUNC_CODE_READ_EXCEPTION_STATUS = 0x07,		/* ���쳣״̬ */
	E_FUNC_CODE_WRITE_MULTIPLE_COILS = 0x0F,		/* д�����Ȧ */
	E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS = 0x10,	/* д������ּĴ��� */
	E_FUNC_CODE_REPORT_SLAVE_ID = 0x11,				/* ����ӻ���ʶ */	
} MODBUS_FUNC_CODE_TYPE_E;

/* ֧������Э�鷽ʽ, modbus ASCII, RTU, RTU_TCP */
typedef enum _MODBUS_PROTOCOL_TYPE_E
{
	E_START_PROTOCOL_TYPE = 0,
	E_ASCII_PROTOCOL_TYPE,
	E_RTU_PROTOCOL_TYPE,
	E_RTU_TCP_PROTOCOL_TYPE,
} MODBUS_PROTOCOL_TYPE_E;

/* ������ */
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

/* ����λ */
typedef enum _MODBUS_DATA_BIT_E
{
	E_DATA_4BITS = 4,
	E_DATA_5BITS = 5,
	E_DATA_6BITS = 6,
	E_DATA_7BITS = 7,
	E_DATA_8BITS = 8,
} MODBUS_DATA_BIT_E;

/* ֹͣλ */
typedef enum _MODBUS_STOP_BIT_E
{
	E_STOP_1V0BIT = 0,
	E_STOP_1V5BITS,
	E_STOP_2V0BITS,
} MODBUS_STOP_BIT_E;

/* У��� */
typedef enum _MODBUS_CHECK_E
{
	E_CHECK_NONE = 0,
	E_CHECK_EVEN,
	E_CHECK_ODD,
	E_CHECK_MARK,
	E_CHECK_SPACK,
} MODBUS_CHECK_E;

/* modbus�������ڵ���ز��� */
typedef struct _modbus_com_params_st
{
	char *device;				/* Ӳ���豸 */
	int32_t baud;				/* ������ */
	uint8_t data_bit;			/* ����λ */
	uint8_t stop_bit;			/* ֹͣλ */
	char parity;				/* У��� */
} modbus_com_params_st;

/**************************************************************************
** ��������
**************************************************************************/
/**************************************************************************
* ��  ��: void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
* ��  ��: ��ӡhex����
* ��  ��: uint8_t *data : ��Ҫ��ӡ��������
		  uint32_t datalen : ���ݳ���
* ��  ��: void
* ����ֵ: void
**************************************************************************/
void modbus_log_hex_print(uint8_t *data, uint32_t datalen);

/**************************************************************************
* ��  ��: int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
* ��  ��: ����crcУ�麯��
* ��  ��: uint8_t *data : ����crcУ�������
		  uint32_t datalen : ����crcУ������ݳ���
* ��  ��: uint16_t *crc : ����crcУ��ֵ
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
					M_MODBUS_ERR - ʧ��
**************************************************************************/
int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

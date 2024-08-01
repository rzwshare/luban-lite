/**************************************************************************
**  ����: modbus_rtu.hͷ�ļ�
*   ����: 
*   ����:
*   ����:
*   �޸ļ�¼: 
***************************************************************************/
#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "modbus_common.h"

/**************************************************************************
** �궨��
**************************************************************************/

/**************************************************************************
** �ṹ������
**************************************************************************/
/* ����ʧ�ܹ�����Ӧ��ṹ�� */
typedef struct _modbus_rtu_fail_st
{
	uint8_t slave_addr;
	uint8_t err_func_code;
	uint8_t except_code;
	uint16_t crc16;
} __attribute__((packed)) modbus_rtu_fail_st;

/* modbus rtu ��д�Ĵ�����ӦЭ��ṹ */
typedef struct _modbus_rtu_ack_data_rw_st
{
	uint8_t slave_addr;			/* ���豸��ַ */
	uint8_t func_code; 			/* ������ */
	uint8_t datalen;			/* ��Ӧ������ */
	uint8_t *data;				/* ��Ӧ���� */
} __attribute__((packed)) modbus_rtu_ack_data_rw_st;

/****************************************************************************
** ���Ĵ����ṹ��
*****************************************************************************/
/* ���ӻ��Ĵ����Ľṹ��(֧��1-126) */
typedef struct _modbus_read_reg_info_st
{
	 uint8_t slave_addr;							/* ���豸��ַ */
	 MODBUS_FUNC_CODE_TYPE_E func_code; 			/* ������ */
	 uint16_t register_addr;						/* �Ĵ����׵�ַ */
	 uint16_t register_cnt; 						/* �Ĵ�����Ŀ */
} modbus_read_reg_info_st;

/* modbus rtu ���Ĵ���Э��ṹ */
typedef struct _modbus_rtu_read_reg_st
{
	uint8_t slave_addr;			/* ���豸��ַ */
	uint8_t func_code; 			/* ������ */
	uint16_t register_addr; 	/* ���Ĵ�����ַ */
	uint16_t register_cnt;		/* �Ĵ�����Ŀ */
	uint16_t crc;				/* crcУ�� */
} __attribute__((packed)) modbus_rtu_read_reg_st;

/* �������ֶ�״̬�Ĵ��������Ƕ�����Ĵ��� */
typedef enum _MODBUS_RTU_READ_REG_TYPE_E
{
	E_MODBUS_RTU_READ_STATUS_REG = 0,
	E_MODBUS_RTU_READ_STORAGE_REG,
} MODBUS_RTU_READ_REG_TYPE_E;

/* ���ڱ����״̬�Ĵ����ͱ���Ĵ���������ֵ��Ϣ */
typedef union _modbus_rtu_reg_data_u
{
	uint16_t *storage_reg_value;
	uint8_t *status_reg_value;
} modbus_rtu_reg_data_u;

typedef struct _modbus_rtu_reg_data_val_st
{
	MODBUS_RTU_READ_REG_TYPE_E read_reg_type;	/* ״̬�Ĵ��������Ǵ洢�Ĵ��� */
	uint16_t start_register_addr;				/* �Ĵ�����ʼ��ַ */
	uint8_t read_register_cnt;					/* �Ĵ������� */
	modbus_rtu_reg_data_u reg_data;				/* ����ļĴ�������ֵ */
} modbus_rtu_reg_data_val_st;

/****************************************************************************
** д�Ĵ����ṹ��
****************************************************************************/
/* ����д�Ĵ�����ֵ */
typedef union _modbus_rtu_write_reg_val_st
{
	uint16_t *register_val; 					/* д�Ĵ�����ֵ */
	uint8_t *multiple_status_reg_val;			/* ���״̬�Ĵ�����ֵ */
} modbus_rtu_write_reg_val_st;

/* ����д״̬/�洢�Ĵ��� */
typedef struct _modbus_rtu_write_reg_info_st
{
	uint8_t slave_addr; 						/* ���豸��ַ */
	MODBUS_FUNC_CODE_TYPE_E func_code; 			/* ������ */
	uint16_t register_addr;						/* �Ĵ����׵�ַ */
	uint8_t register_cnt; 						/* д�Ĵ������� */
	modbus_rtu_write_reg_val_st reg_val;		/* �Ĵ�����ֵ */
} modbus_rtu_write_reg_info_st;
/*д�Ĵ����ṹ��*/
typedef struct _modbus_write_reg_info_st
{
	 uint8_t slave_addr;							/* ���豸��ַ */
	 MODBUS_FUNC_CODE_TYPE_E func_code; 			/* ������ */
	 uint16_t register_addr;						/* �Ĵ����׵�ַ */
	 uint16_t register_data; 						/* д�Ĵ�����ֵ */
} modbus_write_reg_info_st;

/* ����д�����Ĵ��� */
typedef struct _modbus_rtu_ack_write_signal_reg_st
{
	uint8_t slave_addr;							/* ���豸��ַ */
	uint8_t func_code;							/* ������ */
	uint16_t register_addr;						/* �Ĵ����׵�ַ */
	uint16_t register_data;						/* д�Ĵ�����ֵ */
	uint16_t crc;								/* crcУ�� */
} __attribute__((packed)) modbus_rtu_ack_write_signal_reg_st;

/* ����д���״̬�Ĵ���Ӧ��ṹ */
typedef struct _modbus_rtu_ack_write_multiple_status_reg_st
{
	uint8_t slave_addr;							/* ���豸��ַ */
	uint8_t func_code;							/* ������ */
	uint16_t register_addr;						/* �Ĵ����׵�ַ */
	uint16_t register_cnt;						/* �Ĵ������� */
	uint8_t register_data_len;					/* �Ĵ�������ֵ���� */
	uint16_t crc;								/* crcУ�� */
} __attribute__((packed)) modbus_rtu_ack_write_multiple_state_reg_st;

/* ����д���״̬�Ĵ���Ӧ��ṹ */
typedef struct _modbus_rtu_ack_write_multiple_storage_reg_st
{
	uint8_t slave_addr;							/* ���豸��ַ */
	uint8_t func_code;							/* ������ */
	uint16_t register_addr;						/* �Ĵ����׵�ַ */
	uint16_t register_cnt;						/* �Ĵ������� */
	uint16_t crc;								/* crcУ�� */
} __attribute__((packed)) modbus_rtu_ack_write_multiple_storage_reg_st;


/**************************************************************************
** ��������
**************************************************************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

/**************************************************************************
**  ����: modbus_rtu.c�ļ�
*   ����: 
*   ����:
*   ����:
*	1.����Ȧ����(������-- 0x01) -- ��λ���ж�
*   2.����ɢ����(������-- 0x02) -- ��λ���ж�
*
*   �޸ļ�¼: 
***************************************************************************/
#include "modbus_rtu.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
//#include "winsock2.h"

#if 0
unsigned int htonl(unsigned int  x)

{

	unsigned int  y;

	((unsigned  char*)&y)[0]= ((unsigned  char*)&x)[3];
	((unsigned  char*)&y)[1]= ((unsigned  char*)&x)[2];
	((unsigned  char*)&y)[2]= ((unsigned  char*)&x)[1];
	((unsigned  char*)&y)[3]= ((unsigned  char*)&x)[0];

	return y;

}

unsigned int nhtonl(unsigned int  x)

{

	return htonl(x);

}

unsigned short htons(unsigned short x)

{

	unsigned short y;

	((unsigned  char*)&y)[0]= ((unsigned  char*)&x)[1];
	((unsigned  char*)&y)[1]= ((unsigned  char*)&x)[0];

	return y;

}

 

unsigned short nhtons(unsigned short x)

{
	return htons(x);

}
#else
// �����ʹ�С�˻���
#define BigLittleSwap16(A)  ((((uint16_t)(A) & 0xff00) >> 8)|(((uint16_t)(A) & 0x00ff) << 8))

 

// �����ʹ�С�˻���

#define BigLittleSwap32(A)  ((((uint32_t)(A)&0xff000000) >> 24)|(((uint32_t)(A)&0x00ff0000) >> 8)|(((uint32_t)(A)&0x0000ff00) << 8)|(((uint32_t)(A)&0x000000ff) << 24))

 
// ������˷���1��С�˷���0
int checkCPUendian(void)
{
       union{
              unsigned long int i;
              unsigned char s[4];
       }c;

       c.i = 0x12345678;
       return (0x12 == c.s[0]);

}

// ģ��htonl�����������ֽ���ת�����ֽ���
unsigned long int htonl(unsigned long int h)
{
   // ������Ϊ��ˣ��������ֽ���ͬ��ֱ�ӷ���
   // ������ΪС�ˣ�ת���ɴ���ٷ���
   return checkCPUendian() ? h : BigLittleSwap32(h);

}

// ģ��ntohl�����������ֽ���ת�����ֽ���
unsigned long int ntohl(unsigned long int n)
{
       // ������Ϊ��ˣ��������ֽ���ͬ��ֱ�ӷ���
       // ������ΪС�ˣ���������ת����С���ٷ���
       return checkCPUendian() ? n : BigLittleSwap32(n);
}

// ģ��htons�����������ֽ���ת�����ֽ���
unsigned short int htons(unsigned short int h)
{
       // ������Ϊ��ˣ��������ֽ���ͬ��ֱ�ӷ���
       // ������ΪС�ˣ�ת���ɴ���ٷ���
       return checkCPUendian() ? h : BigLittleSwap16(h);
}

// ģ��ntohs�����������ֽ���ת�����ֽ���
unsigned short int ntohs(unsigned short int n)
{
       // ������Ϊ��ˣ��������ֽ���ͬ��ֱ�ӷ���
       // ������ΪС�ˣ���������ת����С���ٷ���
       return checkCPUendian() ? n : BigLittleSwap16(n);
}
#endif

	/**************************************************************************
	** �궨��
	**************************************************************************/
#define M_MODBUS_MAX_REGISTER_CNT	(127)		/* ��д�Ĵ��������� */

modbus_com_params_st g_modbus_com;

	/**************************************************************************
	** �ṹ������
	**************************************************************************/

	/**************************************************************************
	** ȫ�ֱ�������
	**************************************************************************/

	/**************************************************************************
	** ��������
	**************************************************************************/
/*************************************************************************	
	* ��  ��: int32_t 	modbus_rtu_pack_read_reg(modbus_read_reg_info_st *reg_info, uint8_t *data, uint32_t *datalen)
	* ��  ��: ��ModbusЭ�飬������Ĵ���
	* ��  ��: ��������Э�������������
      typedef struct _modbus_rtu_read_reg_st
	  {
		 uint8_t slave_addr;
		 uint8_t func_code;
		 uint16_t register_addr;
	     uint16_t register_cnt;
	     uint16_t crc;
	  } __attribute__((packed)) modbus_rtu_read_reg_st;
	  ��������ļĴ�����Ϣ��������ݲ���
	* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
	* ��  ��: uint8_t *data : ��modbusЭ�������������
      uint32_t *datalen : ����������
	* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                M_MODBUS_ERR - ʧ��
	**************************************************************************/
	int32_t modbus_rtu_pack_read_reg
	(
		modbus_read_reg_info_st *reg_info,
		uint8_t *data,
		uint32_t *datalen
	)
	{
	int32_t ret = M_MODBUS_ERR;
	modbus_rtu_read_reg_st read_reg;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ���У�� */
		if ((NULL == reg_info) || (NULL == data) || (NULL == datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	 /* ��modbusЭ����� */
	(void)memset(&read_reg, 0, sizeof(read_reg));
	read_reg.slave_addr = reg_info->slave_addr;
	read_reg.func_code = (uint8_t)reg_info->func_code;

	 /* ����Ĵ�����ַ */
	read_reg.register_addr = reg_info->register_addr;

	/* �Ĵ�����Ŀ */
	if ((0 == reg_info->register_cnt) || (M_MODBUS_MAX_REGISTER_CNT < reg_info->register_cnt))
	{
		M_MODBUS_LOG_ERROR("register count[%d] is over out of range", reg_info->register_cnt);
		break;
	}
	read_reg.register_cnt = reg_info->register_cnt;

	/* ��С��ת�� */
	read_reg.register_addr = htons(read_reg.register_addr);		
	read_reg.register_cnt = htons(read_reg.register_cnt);

	/* crcУ�� */
	ret = modbus_calc_crc((uint8_t *)&read_reg, sizeof(read_reg) - sizeof(uint16_t), &read_reg.crc);
	if (M_MODBUS_OK != ret)
	{
		M_MODBUS_LOG_ERROR("calc crc is failed, ret = %d", ret);
		break;
	}
	//printf("read_reg.crc:(%x)",read_reg.crc);
	(void)memcpy(data, (uint8_t *)&read_reg, sizeof(modbus_rtu_read_reg_st));
	*datalen = sizeof(modbus_rtu_read_reg_st);

	/* ��ӡ������� */
	//modbus_log_hex_print(data, *datalen);
	//M_MODBUS_LOG_DEBUG("package read status register data ok");

	ret = M_MODBUS_OK;
	}
	while (0);

	//M_MODBUS_TRACE_OUT();

	return ret;
}

/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_read_status_reg(modbus_read_reg_info_st *reg_info,
	        modbus_rtu_ack_data_rw_st *data_ack, modbus_rtu_reg_data_val_st *data_val)
* ��  ��: ��ModbusЭ�飬������ȡ״̬�Ĵ�������Ӧ����
* ��  ��: ��������Э��������н��������������Ӧ������ֵ
          typedef struct _modbus_rtu_ack_data_rw_st
		  {
			 uint8_t slave_addr;
			 uint8_t func_code;
			 uint8_t datalen;
			 uint8_t *data;
		  } __attribute__((packed)) modbus_rtu_ack_data_rw_st;		  
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  modbus_rtu_ack_data_rw_st *data_ack : ���յ�״̬�Ĵ�������Ϣ
* ��  ��: modbus_rtu_reg_data_val_st *data_val : ���������/״̬�Ĵ���������ֵ��Ϣ
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
static int32_t modbus_rtu_unpack_read_status_reg
(
	modbus_read_reg_info_st *reg_info,
	modbus_rtu_ack_data_rw_st *data_ack,
	modbus_rtu_reg_data_val_st *data_val
)
{
	int32_t ret = M_MODBUS_ERR;

	//M_MODBUS_TRACE_IN();

	do
	{
		if ((NULL == reg_info) || (NULL == data_ack) || (NULL == data_val))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

		/* ��״̬�Ĵ��� */
		if (reg_info->register_cnt != (uint16_t)data_ack->datalen)
		{
			M_MODBUS_LOG_ERROR("read status register num[%x] not equal recv datalen[%x]",
						reg_info->register_cnt, data_ack->datalen);
			break;
		}

		/* ���浽�ṹ���� */
		data_val->read_reg_type = E_MODBUS_RTU_READ_STATUS_REG;
		data_val->start_register_addr = reg_info->register_addr;
		data_val->read_register_cnt = reg_info->register_cnt / 8 + (reg_info->register_cnt % 8) ? 1 : 0;
		data_val->reg_data.status_reg_value = (uint8_t *)malloc(data_val->read_register_cnt);
		if (NULL == data_val->reg_data.status_reg_value)
		{
			M_MODBUS_LOG_ERROR("malloc read status register[%d] space is failed", data_val->read_register_cnt);
			break;
		}

		(void)memset(data_val->reg_data.status_reg_value, 0, data_val->read_register_cnt);
		(void)memcpy(data_val->reg_data.status_reg_value, data_ack->data, data_val->read_register_cnt);

		//M_MODBUS_LOG_DEBUG("recv read status register data ok");

	ret = M_MODBUS_OK;
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}


/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_read_storage_reg(modbus_read_reg_info_st *reg_info,
	        modbus_rtu_ack_data_rw_st *data_ack, modbus_rtu_reg_data_val_st *data_val)
* ��  ��: ��ModbusЭ�飬������ȡ�洢�Ĵ�������Ӧ����
* ��  ��: ��������Э��������н��������������Ӧ������ֵ
          typedef struct _modbus_rtu_ack_data_rw_st
		  {
			 uint8_t slave_addr;
			 uint8_t func_code;
			 uint8_t datalen;
			 uint8_t *data;
		  } __attribute__((packed)) modbus_rtu_ack_data_rw_st;		  
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  modbus_rtu_ack_data_rw_st *data_ack : ���յ�״̬�Ĵ�������Ϣ
* ��  ��: modbus_rtu_reg_data_val_st *data_val : ���������/״̬�Ĵ���������ֵ��Ϣ
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
static int32_t modbus_rtu_unpack_read_storage_reg
(
	modbus_read_reg_info_st *reg_info,
	modbus_rtu_ack_data_rw_st *data_ack,
	modbus_rtu_reg_data_val_st *data_val
)
{
	uint16_t index = 0;
	int32_t ret = M_MODBUS_ERR;

	//M_MODBUS_TRACE_IN();

	do
	{
		if ((NULL == reg_info) || (NULL == data_ack) || (NULL == data_val))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	/* ������Ĵ��� */
	if ((reg_info->register_cnt * 2) != (uint16_t)data_ack->datalen)
	{
		M_MODBUS_LOG_ERROR("read register num[%x *��] not equal recv datalen[%x]", reg_info->register_cnt, data_ack->datalen);
		break;
	}

	/* ������ȡ���ṹ���� */
	data_val->read_reg_type = E_MODBUS_RTU_READ_STORAGE_REG;
	data_val->start_register_addr = reg_info->register_addr;
	data_val->read_register_cnt = data_ack->datalen / 2;
	data_val->reg_data.storage_reg_value = (uint16_t *)malloc(data_val->read_register_cnt);
	if (NULL == data_val->reg_data.storage_reg_value)
	{
		M_MODBUS_LOG_ERROR("malloc read storage register[%d] space is failed", data_val->read_register_cnt);
		break;
	}

	(void)memset(data_val->reg_data.storage_reg_value, 0, data_val->read_register_cnt);
	for (index = 0; index < reg_info->register_cnt; index++)
	
	{
			data_val->reg_data.storage_reg_value[index] = data_ack->data[index * 2];
			data_val->reg_data.storage_reg_value[index] <<= 8;
			data_val->reg_data.storage_reg_value[index] |= data_ack->data[index * 2 + 1];
	}

	//M_MODBUS_LOG_DEBUG("recv read storage register data ok");
	ret = M_MODBUS_OK;
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}


/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_read_register(modbus_read_reg_info_st *reg_info,
                                  uint8_t *data, uint32_t datalen, uint16_t *pOutBuf)
* ��  ��: ��ModbusЭ�飬������Ӧ������
* ��  ��: ��������Э��������н��������������Ӧ������ֵ
          typedef struct _modbus_rtu_ack_data_rw_st
		  {
			 uint8_t slave_addr;
			 uint8_t func_code;
			 uint8_t datalen;
			 uint8_t *data;
		  } __attribute__((packed)) modbus_rtu_ack_data_rw_st;		  
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  uint8_t *data : ��modbusЭ�������������
          uint32_t datalen : ����������
* ��  ��: modbus_rtu_reg_data_val_st *data_val : ���������/״̬�Ĵ���������ֵ��Ϣ��
		  �ڵ��ú󣬳ɹ���ȡ��Ĵ��������ݺ���Ҫ�ͷ�����ļĴ�����ֵ
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
int32_t modbus_rtu_unpack_read_register
(
	modbus_read_reg_info_st *reg_info,
	uint8_t *data,
	uint32_t datalen,
	modbus_rtu_reg_data_val_st *data_val
)
{
	int32_t ret = M_MODBUS_ERR;
	uint16_t crc_val = 0;
	uint16_t recv_crc_val = 0;
	modbus_rtu_ack_data_rw_st *data_ack = NULL;
	modbus_rtu_fail_st *fail_info = NULL;
	MODBUS_ERROR_CODE_E err_code = E_CODE_NO_ERR;

	//M_MODBUS_TRACE_IN();
	do
	{
		/* ���У�� */
		if ((NULL == reg_info) || (NULL == data) || (NULL == data_val) || (3 > datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}
		data_ack = (modbus_rtu_ack_data_rw_st *)data;

		/* �ӵ�ַУ�� */
		if (reg_info->slave_addr != data_ack->slave_addr)
		{
			M_MODBUS_LOG_ERROR("pack read_reg[%x] not equal recv read_reg[%x]",
								reg_info->slave_addr, data_ack->slave_addr);
			break;
		}

		/* CRCУ�� */
		ret = modbus_calc_crc(data, datalen - 2, &crc_val);
		if (M_MODBUS_OK != ret)
		{
			M_MODBUS_LOG_ERROR("recv read_reg ack calc crc failed");
			break;
		}
		crc_val = ((crc_val & 0x00ff) << 8) | ((crc_val & 0xff00) >> 8);
	 	//printf("crc_val:%x\r\n",crc_val);
		recv_crc_val = (uint16_t)(((uint16_t)data[datalen - 2] << 8) | (uint16_t)data[datalen - 1]);
		if (crc_val != recv_crc_val)
		{
			M_MODBUS_LOG_ERROR("recv read_reg(%x) not equal calc crc(%x)", recv_crc_val, crc_val);
			ret = M_MODBUS_ERR;
			break;
		}

		/* ����������� */
		if ((uint8_t)reg_info->func_code != data_ack->func_code)
		{
			if (((uint8_t)reg_info->func_code | 0x80) == data_ack->func_code)
			{
				fail_info = (modbus_rtu_fail_st *)data;

				/* ���ص��Ǵ����� */
				switch (fail_info->except_code)
				{
					case 0x01:
					case 0x02:
					case 0x03:
					case 0x04:
					case 0x05:
					{
						err_code = (MODBUS_ERROR_CODE_E)fail_info->except_code;
						break;
					}
					default:
					{
						err_code = E_CODE_OTHER_ERR;
						break;
					}
				}
			}
			else
			{
				err_code = E_CODE_OTHER_ERR;
			}

				M_MODBUS_LOG_WARN("read register data, func code[%x] is fail ack value[%x]",
				data_ack->func_code, fail_info->except_code);

			ret = M_MODBUS_OK;
			break;
	}
#if 0
	switch (reg_info->func_code)
	{
		case E_FUNC_CODE_READ_COILS:
		case E_FUNC_CODE_READ_DISCRETE_INPUTS:
		{
			/* ��״̬�Ĵ��� */
			ret = modbus_rtu_unpack_read_status_reg(reg_info, data_ack, data_val);
			break;
		}
		case E_FUNC_CODE_READ_HOLDING_REGISTERS:
		case E_FUNC_CODE_READ_INPUT_REGISTERS:
		{
			/* ���洢�Ĵ��� */
			ret = modbus_rtu_unpack_read_storage_reg(reg_info, data_ack, data_val);
			break;
		}
		default:
		{
			/* ���� */
			M_MODBUS_LOG_WARN("not recv read register data, func code[%x]", reg_info->func_code);
			ret = M_MODBUS_ERR;
			break;
		}
	}
	#else
	ret = M_MODBUS_OK;
	#endif
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}
/*************************************************************************	
	* ��  ��: int32_t 	modbus_rtu_pack_write_signal_reg(modbus_write_reg_info_st *reg_info,uint8_t *data,uint32_t *datalen)
	* ��  ��: ��ModbusЭ�飬���д�����Ĵ���
	* ��  ��: ��������Э�������������
     typedef struct _modbus_rtu_ack_write_signal_reg_st
	{
		uint8_t slave_addr;							
		uint8_t func_code;							
		uint16_t register_addr;						
		uint16_t register_data;						
		uint16_t crc;								
	} __attribute__((packed)) modbus_rtu_ack_write_signal_reg_st;

	  ��������ļĴ�����Ϣ��������ݲ���
	* ��  ��: modbus_write_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
	* ��  ��: uint8_t *data : ��modbusЭ�������������
      uint32_t *datalen : ����������
	* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                M_MODBUS_ERR - ʧ��
	**************************************************************************/
	int32_t modbus_rtu_pack_write_signal_reg
	(
		modbus_write_reg_info_st *reg_info,
		uint8_t *data,
		uint32_t *datalen
	)
	{
	int32_t ret = M_MODBUS_ERR;
	modbus_rtu_ack_write_signal_reg_st write_reg;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ���У�� */
		if ((NULL == reg_info) || (NULL == data) || (NULL == datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	 /* ��modbusЭ����� */
	 
	 (void)memset(&write_reg, 0, sizeof(write_reg));
	write_reg.slave_addr= reg_info->slave_addr;
	write_reg.func_code= (uint8_t)reg_info->func_code;

	 /* ����Ĵ�����ַ */
	write_reg.register_addr = reg_info->register_addr;
	
	
	write_reg.register_data= reg_info->register_data;

	/* ��С��ת�� */
	write_reg.register_addr = htons(write_reg.register_addr);		
	write_reg.register_data= htons(write_reg.register_data);

	/* crcУ�� */

	ret = modbus_calc_crc((uint8_t *)&write_reg, sizeof(write_reg) - sizeof(uint16_t), &write_reg.crc);
	if (M_MODBUS_OK != ret)
	{
		M_MODBUS_LOG_ERROR("calc crc is failed, ret = %d", ret);
		break;
	}
	//printf("write_reg.crc:(%x)",write_reg.crc);
	(void)memcpy(data, (uint8_t *)&write_reg, sizeof(modbus_rtu_ack_write_signal_reg_st));
	*datalen = sizeof(modbus_rtu_ack_write_signal_reg_st);

	/* ��ӡ������� */
	//modbus_log_hex_print(data, *datalen);
	//M_MODBUS_LOG_DEBUG("package write status register data ok");

	ret = M_MODBUS_OK;
	}
	while (0);

	//M_MODBUS_TRACE_OUT();

	return ret;
}
/**************************************************************************
* ��  ��: int32_t modbus_rtu_pack_write_reg(modbus_rtu_write_reg_info_st *reg_info,
                        uint8_t *data, uint32_t *datalen)
* ��  ��: ��ModbusЭ�飬���д�Ĵ�����ֵ
* ��  ��: 
		���ڵ���״̬/�洢�Ĵ�����˵,�����ʽ,���ֽڵķ�ʽ: 
		����->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
		Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
		���ڶ��״̬�Ĵ�����˵,�����ʽ,��bit�ķ�ʽ: 
		����->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
		Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + crcc(2)
		���ڶ���洢�Ĵ�����˵�������ʽ�����ֽڵķ�ʽ:
		����->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
		Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + crcc(2)
* ��  ��: modbus_register_info_st *reg_info : ����д�Ĵ�����Ϣ
* ��  ��: uint8_t *data : ��modbusЭ�������������
          uint32_t *datalen : ����������
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
int32_t modbus_rtu_pack_write_reg
(
	modbus_rtu_write_reg_info_st *reg_info,
	uint8_t *data,
	uint32_t *datalen
)
{
	int32_t ret = M_MODBUS_ERR;
	uint16_t crc_val = 0;
	uint8_t index = 0;
	uint8_t templen = 0;
	uint8_t register_val_len = 0;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ���У�� */
		if ((NULL == reg_info) || (NULL == data) || (NULL == datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	/* �Ĵ�������Ϊ0��ֱ���˳� */
	if (0x00 == reg_info->register_cnt)
	{
		M_MODBUS_LOG_ERROR("input param write register count is 0x00, exit.");
		break;
	}

	/* ��modbusЭ����� */
	data[templen++] = reg_info->slave_addr;
	data[templen++] = (uint8_t)reg_info->func_code;

	/* ����Ĵ�����ַ */
	data[templen++] = (uint8_t)((reg_info->register_addr >> 8) & 0xff);
	data[templen++] = (uint8_t)(reg_info->register_addr & 0xff);

	if (0x01 == reg_info->register_cnt)
	{
		/* ����״̬/����Ĵ��� */
		data[templen++] = (uint8_t)((reg_info->reg_val.register_val[0] >> 8) & 0xff);
		data[templen++] = (uint8_t)(reg_info->reg_val.register_val[0] & 0xff);
	}
	else
	{
		/* д�Ĵ������� */
		data[templen++] = 0x00;
		data[templen++] = reg_info->register_cnt;

		if (E_FUNC_CODE_WRITE_MULTIPLE_COILS == reg_info->func_code)
		{
			/* д�����Ȧ���ֽ������� */
			register_val_len = reg_info->register_cnt / 8 + (reg_info->register_cnt % 8 ? 1 : 0);
			data[templen++] = register_val_len;

			/* �����Ȧ״ֵ̬ */
			(void)memcpy(&data[templen], reg_info->reg_val.multiple_status_reg_val, register_val_len);
			templen += register_val_len;
		}
		else if (E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS == reg_info->func_code)
		{
			/* д����Ĵ����ֽڳ��� */
			data[templen++] = reg_info->register_cnt * 2;
			
			for (index = 0; index < reg_info->register_cnt; index++)
			{
				data[templen++] = (uint8_t)((reg_info->reg_val.register_val[index] >> 8) & 0xff);
				data[templen++] = (uint8_t)(reg_info->reg_val.register_val[index] & 0xff);
			}
		}
		else
		{
			M_MODBUS_LOG_ERROR("write multiple register, func code[%d] error", reg_info->func_code);
			break;
		}
	}

	/* crcУ�� */
	ret = modbus_calc_crc(data, templen, &crc_val);
	if (M_MODBUS_OK != ret)
	{
		M_MODBUS_LOG_ERROR("calc crc is failed, ret = %d", ret);
		break;
	}

	data[templen++] = (uint8_t)((crc_val >> 8) & 0xff);
	data[templen++] = (uint8_t)(crc_val & 0xff);

	*datalen = templen;

	/* ��ӡ������� */
	//modbus_log_hex_print(data, *datalen);
	//M_MODBUS_LOG_DEBUG("package write register data ok");
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}


/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_write_signal_reg(modbus_read_reg_info_st *reg_info,
	        uint8_t *data, uint32_t datalen)
* ��  ��: ��ModbusЭ�飬������ȡ����״̬/�洢�Ĵ�������Ӧ����
* ��  ��: ���ڵ���״̬/�洢�Ĵ�����˵,�����ʽ,���ֽڵķ�ʽ: 
		  ����->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
		  Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)	
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  uint8_t *data : ��modbusЭ����յ�������
          uint32_t datalen : ����������
* ��  ��: void
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
static int32_t modbus_rtu_unpack_write_signal_reg
(
	modbus_rtu_write_reg_info_st *reg_info,
	uint8_t *data, 
	uint32_t datalen
)
{
	int32_t ret = M_MODBUS_ERR;
	modbus_rtu_ack_write_signal_reg_st *write_signal_reg = NULL;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ����ж� */
		if ((NULL == reg_info) || (NULL == data) || (3 >= datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	write_signal_reg = (modbus_rtu_ack_write_signal_reg_st *)data;

	/* �Ĵ�����ַ�ͼĴ���ֵ��У�� */
	write_signal_reg->register_addr = ntohs(write_signal_reg->register_addr);
	write_signal_reg->register_data = ntohs(write_signal_reg->register_data);

	if (reg_info->register_addr != write_signal_reg->register_addr)
	{
		M_MODBUS_LOG_ERROR("recv write_signal_reg addr(%x) != recv write signal addr(%x)",
			reg_info->register_addr, write_signal_reg->register_addr);
		break;
	}

	if (reg_info->reg_val.register_val[0] != write_signal_reg->register_data)
	{
		M_MODBUS_LOG_ERROR("recv write_signal_reg val(%x) != recv write signal val(%x)",
			reg_info->reg_val.register_val[0], write_signal_reg->register_data);
		break;
	}

	//M_MODBUS_LOG_DEBUG("recv write signal register data ok");
	ret = M_MODBUS_OK;
	break;
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}

/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_write_multiple_state_reg(
           modbus_read_reg_info_st *reg_info, uint8_t *data, uint32_t datalen)
* ��  ��: ��ModbusЭ�飬������ȡ���״̬�Ĵ�������Ӧ����
* ��  ��: ���ڶ��״̬�Ĵ�����˵,�����ʽ,��bit�ķ�ʽ: 
		  ����->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
		  Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + crcc(2)
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  uint8_t *data : ��modbusЭ����յ�������
          uint32_t datalen : ����������
* ��  ��: void
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
static int32_t modbus_rtu_unpack_write_multiple_state_reg
(
	modbus_rtu_write_reg_info_st *reg_info,
	uint8_t *data, 
	uint32_t datalen
)
{
	int32_t ret = M_MODBUS_ERR;
	uint8_t write_data_len = 0;
	modbus_rtu_ack_write_multiple_state_reg_st *write_state_reg = NULL;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ����ж� */
		if ((NULL == reg_info) || (NULL == data) || (3 >= datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	write_state_reg = (modbus_rtu_ack_write_multiple_state_reg_st *)data;

	/* �Ĵ�����ַ�ͼĴ����������Ĵ������Ƚ���У��*/
	write_state_reg->register_addr = ntohs(write_state_reg->register_addr);
	write_state_reg->register_cnt = ntohs(write_state_reg->register_cnt);
	
	if (reg_info->register_addr != write_state_reg->register_addr)
	{
		M_MODBUS_LOG_ERROR("write_multiple_status_reg addr(%x) != recv write multiple_status addr(%x)",
			reg_info->register_addr, write_state_reg->register_addr);
		break;
	}

	if ((uint16_t)reg_info->register_cnt != write_state_reg->register_cnt)
	{
		M_MODBUS_LOG_ERROR("write_multiple_status_cnt(%x) != recv write multiple_status cnt(%x)",
			reg_info->register_cnt, write_state_reg->register_cnt);
		break;
	}

	write_data_len = reg_info->register_cnt / 8 + (reg_info->register_cnt % 8 ? 1 : 0);
	if (write_data_len != write_state_reg->register_data_len)
	{
		M_MODBUS_LOG_ERROR("write_multiple_status_data_len(%x) != recv write multiple_status data len(%x)",
			write_data_len, write_state_reg->register_data_len);
		break;
	}

	//M_MODBUS_LOG_DEBUG("recv write multiple_status data ok");
	ret = M_MODBUS_OK;
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}

/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_write_multiple_storage_reg(
           modbus_read_reg_info_st *reg_info, uint8_t *data, uint32_t datalen)
* ��  ��: ��ModbusЭ�飬������ȡ����洢�Ĵ�������Ӧ����
* ��  ��: ���ڶ���洢�Ĵ�����˵�������ʽ�����ֽڵķ�ʽ:
		  ����->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
		  Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + crcc(2)
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  uint8_t *data : ��modbusЭ����յ�������
          uint32_t datalen : ����������
* ��  ��: void
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
static int32_t modbus_rtu_unpack_write_multiple_storage_reg
(
	modbus_rtu_write_reg_info_st *reg_info,
	uint8_t *data, 
	uint32_t datalen
)
{
	int32_t ret = M_MODBUS_ERR;
	modbus_rtu_ack_write_multiple_storage_reg_st *write_storage_reg = NULL;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ����ж� */
		if ((NULL == reg_info) || (NULL == data) || (3 >= datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	write_storage_reg = (modbus_rtu_ack_write_multiple_storage_reg_st *)data;

	/* �Ĵ�����ַ�ͼĴ����������Ĵ������Ƚ���У��*/
	write_storage_reg->register_addr = ntohs(write_storage_reg->register_addr);
	write_storage_reg->register_cnt = ntohs(write_storage_reg->register_cnt);
	
	if (reg_info->register_addr != write_storage_reg->register_addr)
	{
		M_MODBUS_LOG_ERROR("write_multiple_storage_reg addr(%x) != recv write multiple_storage addr(%x)",
			reg_info->register_addr, write_storage_reg->register_addr);
		break;
	}

	if ((uint16_t)reg_info->register_cnt != write_storage_reg->register_cnt)
	{
		M_MODBUS_LOG_ERROR("write_multiple_storage_cnt(%x) != recv write multiple_storage cnt(%x)",
			reg_info->register_cnt, write_storage_reg->register_cnt);
		break;
	}

	//M_MODBUS_LOG_DEBUG("recv write multiple_storage data ok");
	ret = M_MODBUS_OK;
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
}

/**************************************************************************
* ��  ��: int32_t modbus_rtu_unpack_write_reg(modbus_read_reg_info_st *reg_info,
	        uint8_t *data, uint32_t datalen)
* ��  ��: ��ModbusЭ�飬������ȡ�Ĵ�������Ӧ����
* ��  ��: ���ڵ���״̬/�洢�Ĵ�����˵,�����ʽ,���ֽڵķ�ʽ: 
		  ����->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
		  Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)	
		  ���ڶ��״̬�Ĵ�����˵,�����ʽ,��bit�ķ�ʽ: 
		  ����->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
		  Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + crcc(2)
		  ���ڶ���洢�Ĵ�����˵�������ʽ�����ֽڵķ�ʽ:
		  ����->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
		  Ӧ��->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + crcc(2)
* ��  ��: modbus_read_reg_info_st *reg_info : �����д�Ĵ�����Ϣ
		  uint8_t *data : ��modbusЭ����յ�������
          uint32_t datalen : ����������
* ��  ��: void
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
static int32_t modbus_rtu_unpack_write_reg
(
	modbus_rtu_write_reg_info_st *reg_info,
	uint8_t *data,
	uint32_t datalen
)
{
	int32_t ret = M_MODBUS_ERR;
	uint16_t crc_val = 0;
	uint16_t recv_crc_val = 0;
	MODBUS_ERROR_CODE_E err_code = E_CODE_NO_ERR;
	modbus_rtu_fail_st *fail_info = NULL;
	modbus_rtu_ack_data_rw_st *data_ack = NULL;

	//M_MODBUS_TRACE_IN();

	do
	{
		/* ����ж� */
		if ((NULL == reg_info) || (NULL == data) || (3 >= datalen))
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	data_ack = (modbus_rtu_ack_data_rw_st *)data;

	/* ��ַ�Ƿ�һ�� */
	if (reg_info->slave_addr != data_ack->slave_addr)
	{
		M_MODBUS_LOG_ERROR("write register slave addr[%x] != recv data ack slave addr[%x]",
			reg_info->slave_addr, data_ack->slave_addr);
		break;
	}

	/* crcУ�� */
	ret = modbus_calc_crc(data, datalen - 2, &crc_val);
	if (M_MODBUS_OK != ret)
	{
		M_MODBUS_LOG_ERROR("recv write_signal_reg ack calc crc failed");
		break;
	}

	recv_crc_val = (uint16_t)(((uint16_t)data[datalen - 2] << 8) | (uint16_t)data[datalen - 1]);
	if (crc_val != recv_crc_val)
	{
		M_MODBUS_LOG_ERROR("recv write_signal_reg_crc(%x) not equal calc crc(%x)", recv_crc_val, crc_val);
		ret = M_MODBUS_ERR;
		break;
	}

	/* ����������� */
	if ((uint8_t)reg_info->func_code != data_ack->func_code)
	{
		if (((uint8_t)reg_info->func_code | 0x80) == data_ack->func_code)
		{
			fail_info = (modbus_rtu_fail_st *)data;

			/* ���ص��Ǵ����� */
			switch (fail_info->except_code)
			{
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				{
					err_code = (MODBUS_ERROR_CODE_E)fail_info->except_code;
					break;
				}
				default:
				{
					err_code = E_CODE_OTHER_ERR;
					break;
				}
			}
		}
		else
		{
			err_code = E_CODE_OTHER_ERR;
		}

		M_MODBUS_LOG_WARN("write signal register data, func code[%x] is fail ack value[%x]",
			reg_info->func_code, data_ack->func_code);

		ret = M_MODBUS_ERR;
		break;
	}

	switch (reg_info->func_code)
	{
		case E_FUNC_CODE_WRITE_SINGLE_COIL:
		case E_FUNC_CODE_WRITE_SINGLE_REGISTER:
		{
			ret = modbus_rtu_unpack_write_signal_reg(reg_info, data, datalen);
		}
		case E_FUNC_CODE_WRITE_MULTIPLE_COILS:
		{
			ret = modbus_rtu_unpack_write_multiple_state_reg(reg_info, data, datalen);
		}
		case E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS:
		{
			ret = modbus_rtu_unpack_write_multiple_storage_reg(reg_info, data, datalen);
		}
		default:
		{
			/* ���� */
			M_MODBUS_LOG_WARN("not recv write register data, func code[%x]", reg_info->func_code);
			ret = M_MODBUS_ERR;
		}
	}
}
while (0);

//M_MODBUS_TRACE_OUT();

return ret;
} 

/**************************************************************************
* ��  ��: int32_t modbus_rtu_init(modbus_com_params_st *com_param)
* ��  ��: ��ʼ��modbus rtu ͨ�ſڲ���
* ��  ��: modbus_com_params_st *com_param : ͨ�ſڲ�����Ϣ
* ��  ��: void
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
int32_t modbus_rtu_init(modbus_com_params_st *com_param)
{
	int32_t ret = M_MODBUS_ERR;

	M_MODBUS_TRACE_IN();

	do
	{
		if (NULL == com_param)
		{
			M_MODBUS_LOG_ERROR("input param is error, null point");
			break;
		}

	/* ������ʼ�� */
	(void)memset(&g_modbus_com, 0, sizeof(g_modbus_com));
	if (strlen(com_param->device) > 0)
	{
		(void)memcpy(g_modbus_com.device, com_param->device, strlen(com_param->device));
	}

	/* ������ */
	if ((E_BAUD_2400BPS > com_param->baud) || (E_BAUD_256000BPS < com_param->baud))
	{
		g_modbus_com.baud = E_BAUD_9600BPS;
	}
	else
	{
		g_modbus_com.baud = com_param->baud;
	}

	/* ����λ */
	if ((E_DATA_4BITS > com_param->data_bit) || (E_DATA_8BITS < com_param->data_bit))
	{
		g_modbus_com.data_bit = E_DATA_8BITS;
	}
	else
	{
		g_modbus_com.data_bit = com_param->data_bit;
	}

	/* ֹͣλ */		
	g_modbus_com.stop_bit = com_param->stop_bit;
	g_modbus_com.parity = com_param->parity;
}
while (0);

M_MODBUS_TRACE_OUT();

return ret;
}

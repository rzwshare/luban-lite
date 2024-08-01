//## modbus_common.c�ļ���

/**************************************************************************
**  ����: modbus_common.c�ļ�
*   ����: 
*   ����:
*   ����:
*   �޸ļ�¼: 
***************************************************************************/
#include "modbus_common.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
//#include <netinet/in.h>

/**************************************************************************
** �궨��
**************************************************************************/
#define M_MODBUS_POLY_VALUE			(0x1021)	/* ����Ķ���ʽֵ */
#define M_MODBUS_CRC_SEED_VALUE		(0xFFFF)	/* Ԥ��ֵ */

/**************************************************************************
** �ṹ������
**************************************************************************/

/**************************************************************************
** ��������
**************************************************************************/

/**************************************************************************
** ��������
**************************************************************************/
/**************************************************************************
* ��  ��: void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
* ��  ��: ��ӡhex����
* ��  ��: ����data: 0x12 0x34 0x56 0x00 0x78 0x90; datalen: 6
		  ��ӡ���: "hex_data[6]: 12 34 56 00 78 90"
* ��  ��: uint8_t *data : ��Ҫ��ӡ��������
		  uint32_t datalen : ���ݳ���
* ��  ��: void
* ����ֵ: void
**************************************************************************/
void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
{
	uint32_t index = 0;
	uint32_t temp_data_len = datalen * 3 + 1;
	uint32_t templen = 0;
	char *temp_data = NULL;

	if ((NULL == data) || (0 == datalen))
	{
		M_MODBUS_LOG_ERROR("print hex data log failed, input param is error.");
		return;
	}

	temp_data = (char*)malloc(temp_data_len);
	if (NULL == temp_data)
	{
		M_MODBUS_LOG_ERROR("print hex data log, malloc data[%d] space failed.", temp_data_len);
		return;
	}

	(void)memset(temp_data, 0, temp_data_len);
	for (index = 0; index < datalen; index++)
	{
		templen += sprintf(&temp_data[templen], "%02x ", data[index]);
	}

	printf("hex_data[%d]: %s\n", datalen, temp_data);

	free(temp_data);
	temp_data = NULL;
}

/**************************************************************************
* ��  ��: int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
* ��  ��: ����crcУ�麯��
* ��  ��: ����data: 0x12 0x23 0x46 0x99; datalen = 4;
		  ����õ������ֽڵ�crcУ����: XXXX
* ��  ��: uint8_t *data : ����crcУ�������
          uint32_t datalen : ����crcУ������ݳ���
* ��  ��: uint16_t *crc : ����crcУ��ֵ
* ����ֵ: int32_t : M_MODBUS_OK  - �ɹ�
                    M_MODBUS_ERR - ʧ��
**************************************************************************/
#if 0
int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
{
	uint16_t crc_val = M_MODBUS_CRC_SEED_VALUE;
	uint32_t i = 0;
	uint32_t j = 0;

	if ((NULL == data) || (0 == datalen))
	{
		M_MODBUS_LOG_ERROR("input param is error");
		return M_MODBUS_ERR;
	}

	for (i = datalen; i > 0; i--)
	{
		crc_val = crc_val ^ (*data++ << 8);

		for (j = 0; j < 8; j++)
		{
			if (crc_val & 0x8000)
			{
				crc_val = (crc_val << 1) ^ M_MODBUS_POLY_VALUE;
			}
			else
			{
				crc_val <<= 1;
			}
		}
	}

	*crc = crc_val;

	return M_MODBUS_OK;
}
#else


int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
{
	uint16_t crc_val = M_MODBUS_CRC_SEED_VALUE;
	uint32_t i = 0;
	uint32_t j = 0;
	if ((NULL == data) || (0 == datalen))
	{
		M_MODBUS_LOG_ERROR("input param is error");
		return M_MODBUS_ERR;
	}
	for(i=0;i<datalen;++i)
    {
        crc_val  ^= *(data+i);                    
        for(j=0;j<8;++j)                            
        {
            if((crc_val & 0x01) == 0x01)          
            {
                 crc_val = (crc_val >> 1)^0xA001; 
            }else 
            {
                crc_val >>= 1;                    
            }           
        }
    } 
	*crc = crc_val;

	return M_MODBUS_OK;
}

#endif




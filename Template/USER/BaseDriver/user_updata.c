#include "user_updata.h"
#include "main.h"

static uint32_t updata_shift = 0; // ƫ����


/**********************************************************************
 * @ ������  �� Erase_Updata_Program_Area
 * @ ����˵���� ������������׼��д��
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Erase_Updata_Program_Area(void)
{
  FLASH_Unlock();
	FLASH_EraseSector(Updata_Program_SECTOR_1, VoltageRange_3);
  FLASH_EraseSector(Updata_Program_SECTOR_2, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ ������  �� Reset_Updata_Flag
 * @ ����˵���� �������±�־λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Reset_Updata_Flag(void)
{
  FLASH_Unlock();
	FLASH_EraseSector(Flag_SECTOR, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ ������  �� Check_Program
 * @ ����˵���� �������Ƿ���ȫд�룬trueΪ����ȷд�룬�ֽ�Ϊ��λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
bool Check_Program(uint32_t buff_add, uint32_t updata_program_add, uint32_t size)
{
	if (0 == memcmp((uint8_t *)buff_add, (uint8_t *)updata_program_add, size))
		return true;
	else
		return false;
}


/**********************************************************************
 * @ ������  �� Program_Updata_Area
 * @ ����˵���� �����ջ��������д��app flash��sizeΪ�ֽ�,updata_program_add����Ҫ��
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Program_Updata_Area(uint32_t updata_program_add, uint32_t buff_add, const uint32_t size) 
{ /*�����updata_program_add�����߸��³����ַ��Ϊ����6 buff_addΪ��λ������������ sizeΪ1024�ֽ�*/
	FLASH_Unlock();                 
	for (int i = 0; i < size; i++) // д��
		FLASH_ProgramByte(updata_program_add + updata_shift + i, *(uint8_t *)(buff_add + i));  //����Ӧ�ĵ�ַ�ϱ��һ���ֽ�
	
	if (true == Check_Program(updata_program_add + updata_shift, buff_add, size)) // У��
#ifdef USER_JLINK						
		SEGGER_RTT_printf(0, "write right!!\n");
#endif 		
	else
#ifdef USER_JLINK				
		SEGGER_RTT_printf(0, "write error!!\n");
#endif 
	
	updata_shift += size;          //����д����ƫ����
	FLASH_Lock();                  
}

/**********************************************************************
 * @ ������  �� Set_Updata_Flag
 * @ ����˵���� ������λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Set_Updata_Flag(void)
{
	FLASH_Unlock();
	FLASH_ProgramWord(Flag_Address, 0x00000000);  //����Ӧ�ĵ�ַ���һ����
	FLASH_Lock();
}


/**********************************************************************
 * @ ������  �� System_Reset
 * @ ����˵���� �����λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void System_Reset(void)
{
	__set_FAULTMASK(1); //�ر������ж�
	NVIC_SystemReset(); //���������λ
}

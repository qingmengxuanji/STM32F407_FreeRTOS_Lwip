#include "main.h"

int main(void)
{ 
#ifdef USER_JLINK		
	SEGGER_RTT_Init();
	DWT->CTRL=(1<<0);
#endif 

#ifdef USER_JLINK				
		SEGGER_RTT_printf(0,"Bootloader Init\n");
#endif 			
  LED_Init();
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
	LED4_OFF;	
	user_delayMs(1000);
	
	if (Check_Flag(Flag_Address) == false){
		
#ifdef USER_JLINK				
		SEGGER_RTT_printf(0,"Check_Flag(Flag_Address) == false \n");
#endif 		
			LED1_OFF;
			LED2_ON;
			user_delayMs(500);
			LED1_ON;
			LED2_OFF;
			user_delayMs(500);	
		
		  Erase_BackApp_Area();//����ԭ����APP����
			Program_Updata(Backdata_Program_Address, App_Address, Backdata_Program_SIZE);	
		  
		  Erase_App_Area();
			Program_Updata(App_Address, Updata_Program_Address, Updata_Program_SIZE);
			if (Check_Program(App_Address, Updata_Program_Address, Updata_Program_SIZE) == false)
			{	
					// ʧ�ܣ�����			
				Reset_Updata_Flag();
				Erase_App_Area();
				Program_Updata(App_Address, Backdata_Program_Address, Backdata_Program_SIZE);			
				
				LED1_OFF;
				LED2_OFF;
				user_delayMs(1000);
			}
			else
			{
				Reset_Updata_Flag();
				Erase_Updata_Program_Area();
				LED1_ON;
				LED2_ON;
				user_delayMs(1000);
				// ���³ɹ�,���Բ�����updata����ֻ������־λ����
			}		
		
		
	}
	else{
#ifdef USER_JLINK				
		SEGGER_RTT_printf(0," Go_to_App(App_Address) \n");
#endif 	
	}
	__disable_irq();
	Go_to_App(App_Address);
	while(1){
#ifdef USER_JLINK				
		SEGGER_RTT_printf(0," Go_to_App(App_Address) ERROR\n");
#endif 		
		LED1_ON;
		LED2_ON;
		user_delayMs(1000);
		LED1_OFF;
		LED2_OFF;		
		user_delayMs(1000);		
	}
	
}

/**********************************************************************
 * @ ������  �� Check_Flag
 * @ ����˵���� falseΪ�и���
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
bool Check_Flag(uint32_t flag_add)
{
	if (*(uint32_t *)flag_add == 0xFFFFFFFF)
		return true;
	else
		return false;
}

/**********************************************************************
 * @ ������  �� Erase_BackApp_Area
 * @ ����˵���� ��������8��9
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Erase_BackApp_Area(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(Backdata_Program_SECTOR_1, VoltageRange_3);
  FLASH_EraseSector(Backdata_Program_SECTOR_2, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ ������  �� Erase_App_Area
 * @ ����˵���� ��������4,5
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Erase_App_Area(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(App_SECTOR_1, VoltageRange_3);
  FLASH_EraseSector(App_SECTOR_2, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ ������  �� Erase_App_Area
 * @ ����˵���� ��������6,7
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
 * @ ������  �� Program_Updata
 * @ ����˵���� �ֽ�Ϊ��λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Program_Updata(uint32_t app_add, uint32_t updata_program_add, uint32_t size)
{
	FLASH_Unlock();
	for (int i = 0; i < size; i++)
		FLASH_ProgramByte(app_add + i, *(uint8_t *)(updata_program_add + i));
  FLASH_Lock();
}


/**********************************************************************
 * @ ������  �� Check_Program
 * @ ����˵���� ����Ƿ���ȫд�룬turnΪ����ȷд�룬�ֽ�Ϊ��λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
bool Check_Program(uint32_t app_add, uint32_t updata_program_add, uint32_t size)
{
	if (0 == memcmp((uint32_t *)app_add, (uint32_t *)updata_program_add, size))
		return true;
	else
		return false;
}

/**********************************************************************
 * @ ������  �� Go_to_App
 * @ ����˵���� ��ת��App
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Go_to_App(uint32_t app_add)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_Offset);       // �ж�������
	__set_MSP(*(__IO uint32_t *)(app_add));					        // ��ȡ����ջָ��
	__set_CONTROL(0);
	(*((int (*)())(*(__IO uint32_t *)(app_add + 4))))();		  // ����app��ʼִ��
}

/**********************************************************************
 * @ ������  �� Reset_Updata_Flag
 * @ ����˵���� ��λ���� ��־λ
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
 * @ ������  �� Force_Set_Flag
 * @ ����˵���� ��λ���� ��־λ
 * @ ����    ��   
 * @ ����ֵ  �� ��
********************************************************************/
void Force_Set_Flag(void) // ǿ���ñ�־λ��������֤
{
	FLASH_Unlock();
	FLASH_ProgramWord(Flag_Address, 0x00000000);
  FLASH_Lock();	
}


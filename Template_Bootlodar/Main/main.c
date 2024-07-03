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
		
		  Erase_BackApp_Area();//备份原来的APP数据
			Program_Updata(Backdata_Program_Address, App_Address, Backdata_Program_SIZE);	
		  
		  Erase_App_Area();
			Program_Updata(App_Address, Updata_Program_Address, Updata_Program_SIZE);
			if (Check_Program(App_Address, Updata_Program_Address, Updata_Program_SIZE) == false)
			{	
					// 失败，重载			
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
				// 更新成功,可以不擦除updata区，只擦除标志位即可
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
 * @ 函数名  ： Check_Flag
 * @ 功能说明： false为有更新
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
bool Check_Flag(uint32_t flag_add)
{
	if (*(uint32_t *)flag_add == 0xFFFFFFFF)
		return true;
	else
		return false;
}

/**********************************************************************
 * @ 函数名  ： Erase_BackApp_Area
 * @ 功能说明： 擦除扇区8，9
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Erase_BackApp_Area(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(Backdata_Program_SECTOR_1, VoltageRange_3);
  FLASH_EraseSector(Backdata_Program_SECTOR_2, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ 函数名  ： Erase_App_Area
 * @ 功能说明： 擦除扇区4,5
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Erase_App_Area(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(App_SECTOR_1, VoltageRange_3);
  FLASH_EraseSector(App_SECTOR_2, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ 函数名  ： Erase_App_Area
 * @ 功能说明： 擦除扇区6,7
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Erase_Updata_Program_Area(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(Updata_Program_SECTOR_1, VoltageRange_3);
  FLASH_EraseSector(Updata_Program_SECTOR_2, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ 函数名  ： Program_Updata
 * @ 功能说明： 字节为单位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Program_Updata(uint32_t app_add, uint32_t updata_program_add, uint32_t size)
{
	FLASH_Unlock();
	for (int i = 0; i < size; i++)
		FLASH_ProgramByte(app_add + i, *(uint8_t *)(updata_program_add + i));
  FLASH_Lock();
}


/**********************************************************************
 * @ 函数名  ： Check_Program
 * @ 功能说明： 检查是否完全写入，turn为已正确写入，字节为单位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
bool Check_Program(uint32_t app_add, uint32_t updata_program_add, uint32_t size)
{
	if (0 == memcmp((uint32_t *)app_add, (uint32_t *)updata_program_add, size))
		return true;
	else
		return false;
}

/**********************************************************************
 * @ 函数名  ： Go_to_App
 * @ 功能说明： 跳转到App
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Go_to_App(uint32_t app_add)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_Offset);       // 中断向量表
	__set_MSP(*(__IO uint32_t *)(app_add));					        // 获取主堆栈指针
	__set_CONTROL(0);
	(*((int (*)())(*(__IO uint32_t *)(app_add + 4))))();		  // 跳至app开始执行
}

/**********************************************************************
 * @ 函数名  ： Reset_Updata_Flag
 * @ 功能说明： 复位更新 标志位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Reset_Updata_Flag(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(Flag_SECTOR, VoltageRange_3);
  FLASH_Lock();
}

/**********************************************************************
 * @ 函数名  ： Force_Set_Flag
 * @ 功能说明： 复位更新 标志位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Force_Set_Flag(void) // 强制置标志位，用于验证
{
	FLASH_Unlock();
	FLASH_ProgramWord(Flag_Address, 0x00000000);
  FLASH_Lock();	
}


#include "user_updata.h"
#include "main.h"

static uint32_t updata_shift = 0; // 偏移量


/**********************************************************************
 * @ 函数名  ： Erase_Updata_Program_Area
 * @ 功能说明： 擦除升级区，准备写入
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
 * @ 函数名  ： Reset_Updata_Flag
 * @ 功能说明： 擦除更新标志位
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
 * @ 函数名  ： Check_Program
 * @ 功能说明： 检查更新是否完全写入，true为已正确写入，字节为单位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
bool Check_Program(uint32_t buff_add, uint32_t updata_program_add, uint32_t size)
{
	if (0 == memcmp((uint8_t *)buff_add, (uint8_t *)updata_program_add, size))
		return true;
	else
		return false;
}


/**********************************************************************
 * @ 函数名  ： Program_Updata_Area
 * @ 功能说明： 将接收缓存的数据写入app flash，size为字节,updata_program_add不需要动
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Program_Updata_Area(uint32_t updata_program_add, uint32_t buff_add, const uint32_t size) 
{ /*传输的updata_program_add（在线更新程序地址）为扇区6 buff_add为上位机传来的数据 size为1024字节*/
	FLASH_Unlock();                 
	for (int i = 0; i < size; i++) // 写入
		FLASH_ProgramByte(updata_program_add + updata_shift + i, *(uint8_t *)(buff_add + i));  //在相应的地址上编程一个字节
	
	if (true == Check_Program(updata_program_add + updata_shift, buff_add, size)) // 校验
#ifdef USER_JLINK						
		SEGGER_RTT_printf(0, "write right!!\n");
#endif 		
	else
#ifdef USER_JLINK				
		SEGGER_RTT_printf(0, "write error!!\n");
#endif 
	
	updata_shift += size;          //计算写入后的偏移量
	FLASH_Lock();                  
}

/**********************************************************************
 * @ 函数名  ： Set_Updata_Flag
 * @ 功能说明： 更新置位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void Set_Updata_Flag(void)
{
	FLASH_Unlock();
	FLASH_ProgramWord(Flag_Address, 0x00000000);  //在相应的地址编程一个字
	FLASH_Lock();
}


/**********************************************************************
 * @ 函数名  ： System_Reset
 * @ 功能说明： 软件复位
 * @ 参数    ：   
 * @ 返回值  ： 无
********************************************************************/
void System_Reset(void)
{
	__set_FAULTMASK(1); //关闭所有中断
	NVIC_SystemReset(); //进行软件复位
}

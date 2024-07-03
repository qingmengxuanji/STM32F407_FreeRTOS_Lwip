/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"

#include "user_led.h"
#include "user_delay.h"


//#define USER_JLINK
#ifdef USER_JLINK
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#endif

#define APP_Offset   0x10000                      //�ж�������ƫ����
#define App_Address (0x08000000 + 0x10000)			  // App��ʼ��ַ
#define Flag_Address (0x08000000 + 0xC000)			  // ���³����־
#define Updata_Program_Address (0x08000000 + 0x40000) // ���³������
#define Backdata_Program_Address (0x08000000 + 0x80000) // ����ԭ�ȵĳ������

#define App_SIZE (0x2FFFF)			  // �ֽ�
#define Updata_Program_SIZE (0x2FFFF) // �ֽ�,ֱ��ȫ��д��
#define Backdata_Program_SIZE (0x2FFFF) // �ֽ�,ֱ��ȫ��д��


#define Flag_SECTOR  FLASH_Sector_3

#define App_SECTOR_1 FLASH_Sector_4
#define App_SECTOR_2 FLASH_Sector_5

#define Updata_Program_SECTOR_1 FLASH_Sector_6
#define Updata_Program_SECTOR_2 FLASH_Sector_7

#define Backdata_Program_SECTOR_1 FLASH_Sector_8
#define Backdata_Program_SECTOR_2 FLASH_Sector_9


bool Check_Flag(uint32_t flag_add);
void Erase_BackApp_Area(void);
void Erase_App_Area(void);
void Erase_Updata_Program_Area(void);
void Program_Updata(uint32_t app_add, uint32_t updata_program_add, uint32_t size);
bool Check_Program(uint32_t app_add, uint32_t updata_program_add, uint32_t size);
void Go_to_App(uint32_t app_add);
void Reset_Updata_Flag(void);
void Force_Set_Flag(void); 

#endif /* __MAIN_H */


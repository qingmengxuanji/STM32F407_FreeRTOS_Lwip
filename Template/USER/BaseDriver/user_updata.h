#ifndef __updata_H
#define __updata_H
#include "main.h"
/*
STM32F4
����������
����0				0x0800 0000 - 0x0800 3FFF			16KB			//��������
����1				0x0800 4000 - 0x0800 7FFF			16KB			//��������
����2				0x0800 8000 - 0x0800 BFFF			16KB			//��������
����3				0x0800 C000 - 0x0800 FFFF			16KB 			//���ȣ�ֱ��ʹ�ñ�־��

����4				0x0801 0000 - 0x0801 FFFF			64KB			//APP
����5				0x0802 0000 - 0x0803 FFFF			128KB			//APP							192K
����6				0x0804 0000 - 0x0805 FFFF			128KB			//UPDATA SAVE
����7				0x0806 0000 �C 0x0807 FFFF			128KB			//UPDATA SAVE  		192K

����8				0x0808 0000 - 0x0809 FFFF			128KB			//UPDATA SAVE
����9				0x080A 0000 �C 0x080B FFFF			128KB			//UPDATA SAVE
����10				0x080C 0000 - 0x080D FFFF		  	128KB
����11				0x080E 0000 �C 0x080F FFFF	  		128KB

*/
#define APP_Offset   0x20000                          //�ж�������ƫ����
#define App_Address (0x08000000 + 0x20000)            // App��ʼ��ַ    0x08010000
#define Flag_Address (0x08000000 + 0xC000)            // ���³����־   0x0800C000  --> ����3
#define Updata_Program_Address (0x08000000 + 0x60000) // ���³������   0x08040000  --> ����6

#define Backdata_Program_Address (0x08000000 + 0xA0000) // ����ԭ�ȵĳ������

#define Updata_Program_SECTOR_1 FLASH_Sector_7     //����7
#define Updata_Program_SECTOR_2 FLASH_Sector_8     //����8
#define Flag_SECTOR             FLASH_Sector_3     //����3

void Erase_Updata_Program_Area(void);
void Reset_Updata_Flag(void);
bool Check_Program(uint32_t buff_add, uint32_t updata_program_add, uint32_t size);
void Program_Updata_Area(uint32_t updata_program_add, uint32_t buff_add, const uint32_t size) ;
void Set_Updata_Flag(void);
extern void System_Reset(void);

#endif




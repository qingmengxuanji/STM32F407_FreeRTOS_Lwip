#ifndef __COMMON_H
#define __COMMON_H 			   
#include "stm32f4xx.h" 


//����Ϊ��ຯ��
void WFI_SET(void);		   //ִ��WFIָ��
void INTX_DISABLE(void); //�ر������ж�
void INTX_ENABLE(void);	 //���������ж�
void MSR_MSP(uint32_t addr);	 //���ö�ջ��ַ 


#endif
































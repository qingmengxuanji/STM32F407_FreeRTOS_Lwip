#ifndef __COMMON_H
#define __COMMON_H 			   
#include "stm32f4xx.h" 


//以下为汇编函数
void WFI_SET(void);		   //执行WFI指令
void INTX_DISABLE(void); //关闭所有中断
void INTX_ENABLE(void);	 //开启所有中断
void MSR_MSP(uint32_t addr);	 //设置堆栈地址 


#endif
































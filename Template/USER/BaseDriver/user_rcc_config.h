#ifndef __USER_RCC_CONFIG_H
#define __USER_RCC_CONFIG_H
#include "stm32f4xx_rcc.h"

extern void SystemClock_Config(void);
void GetSystemClockSource(void);
void GetSystemClockFrequency(void);
void SetClock_HSE(uint32_t m, uint32_t n, uint32_t p, uint32_t q);
void Error_Handler(void);

#endif



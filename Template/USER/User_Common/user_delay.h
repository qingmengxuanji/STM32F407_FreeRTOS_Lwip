#ifndef _user_delay_H
#define _user_delay_H
#include "stdint.h"

#define  DEM_CR      *(volatile uint32_t *)0xE000EDFC
#define  DWT_CR      *(volatile uint32_t *)0xE0001000
#define  DWT_CYCCNT  *(volatile uint32_t *)0xE0001004
#define  DEM_CR_TRCENA                   (1 << 24)
#define  DWT_CR_CYCCNTENA                (1 <<  0)

extern void user_delayUs(int time);
extern void user_delayMs(int time);


extern void delay_us(uint32_t nus);
extern void delay_ms(uint16_t nms);

extern void DWT_Init(void);
extern void DWT_DelayUS(uint32_t _ulDelayTime);
extern void DWT_DelayMS(uint32_t _ulDelayTime);

#endif


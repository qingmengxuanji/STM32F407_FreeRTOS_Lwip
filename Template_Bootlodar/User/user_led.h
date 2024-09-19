#ifndef __LED_H
#define __LED_H

#define LED1_ON   GPIO_SetBits(GPIOE , GPIO_Pin_3);
#define LED1_OFF  GPIO_ResetBits(GPIOE , GPIO_Pin_3);

#define LED2_ON   GPIO_SetBits(GPIOE , GPIO_Pin_4);
#define LED2_OFF  GPIO_ResetBits(GPIOE , GPIO_Pin_4);

#define LED3_ON   GPIO_SetBits(GPIOG , GPIO_Pin_9);
#define LED3_OFF  GPIO_ResetBits(GPIOG , GPIO_Pin_9);

#define LED4_ON   GPIO_SetBits(GPIOA , GPIO_Pin_1);
#define LED4_OFF  GPIO_ResetBits(GPIOA , GPIO_Pin_1);

//函数声明
void LED_Init(void);//初始化

#endif

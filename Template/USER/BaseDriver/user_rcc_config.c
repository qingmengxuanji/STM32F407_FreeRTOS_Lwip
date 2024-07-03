#include "user_rcc_config.h"
#include "SEGGER_RTT.h"
#include "main.h"
//#include "stm32f4xx.h"
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
#ifdef USER_JLINK				
		SEGGER_RTT_printf(0,"ERR\n");
#endif 		
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	GetSystemClockFrequency();
	GetSystemClockSource();                      //获取系统时钟源选择     
	SetClock_HSE(8,240,2,4);                     //启明开发板用8M的外部晶振 (（HSE/M）*N/P )  
	GetSystemClockFrequency();
}


/**
  * @brief  Sets the system clock frequency
  * @retval None
  */
void SetClock_HSE(uint32_t m ,uint32_t n , uint32_t p , uint32_t q)
{
  volatile uint32_t HSEStartUpStatus=0;
	RCC_HSEConfig(RCC_HSE_ON);                   //使能HSE振荡器 ，开启外部晶振
	HSEStartUpStatus = RCC_WaitForHSEStartUp();  //等待HSE振荡器稳定
	if(HSEStartUpStatus == SUCCESS){
		/*
		** 在系统运行中更改系统时钟的话需要先将时钟源切换到其他，并关闭PLL，再进行PLL配置
		*/
		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);   //将系统时钟切换到HSE振荡器时钟
		while(RCC_GetSYSCLKSource()!=0x04){}      //等待HSE被选为系统时钟
		RCC_PLLCmd(DISABLE);                      //禁止PLL
		
		RCC_HCLKConfig(RCC_SYSCLK_Div1);          // HCLK  = SYSCLK/1
    RCC_PCLK1Config(RCC_HCLK_Div4);           // PCLK1 = HCLK/4
    RCC_PCLK2Config(RCC_HCLK_Div2);			      // PCLK2 = HCLK/2
			
	  RCC_PLLConfig(RCC_PLLSource_HSE,m,n,p,q); //配置PLL
			
		RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==RESET){}  //等待PLL稳定

    /* 开启OVER-RIDE模式 ，以能达到更高频率 */		
//    PWR->CR |= PWR_CR_ODEN;
//    while( ( PWR->CSR & PWR_CSR_ODRDY )   == 0 ) {}
//    PWR->CR |= PWR_CR_ODSWEN;
//    while( ( PWR->CSR & PWR_CSR_ODSWRDY ) == 0 ) {}
    
		/* 配置Flash 预取指，指令缓存，数据缓存 ，和等待状态 */
//		FLASH->ACR = FLASH_ACR_PRFTEN
//			          |FLASH_ACR_ICEN
//			          |FLASH_ACR_DCEN
//                |FLASH_ACR_LATENCY_5WS;
			
   RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //把PLLCLK切换为系统时钟
   while( RCC_GetSYSCLKSource()!=0x08 ){}     //等待PLLCLK选择为系统时钟 			
	}

  else//HSE振荡器启动出错处理
  {
		Error_Handler();
	}		
}

/**
  * @brief  Gets the system clock source
  * @retval None
  */
void GetSystemClockSource(void)
{
	uint8_t SysclkStatus =0;
	SysclkStatus = RCC_GetSYSCLKSource();
#ifdef USER_JLINK			
	switch(SysclkStatus)
	{
		case 0x00:
				SEGGER_RTT_printf(0,"SysclkStatus==0x00: HSI used as system clock\n");
			break;
		case 0x04:
				SEGGER_RTT_printf(0,"SysclkStatus==0x04: HSE used as system clock\n");
			break;
		case 0x08:
				SEGGER_RTT_printf(0,"SysclkStatus==0x08: PLL used as system clock\n");
			break;
	}
#endif 	
}	



/**
  * @brief  Gets the system clock frequency
  * @retval None
  */
void GetSystemClockFrequency(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
#ifdef USER_JLINK		
	SEGGER_RTT_printf(0, "RCC_Clocks.SYSCLK_Frequency %d\n" ,RCC_Clocks.SYSCLK_Frequency);
	SEGGER_RTT_printf(0, "RCC_Clocks.HCLK_Frequency   %d\n" ,RCC_Clocks.HCLK_Frequency);	
	SEGGER_RTT_printf(0, "RCC_Clocks.PCLK1_Frequency  %d\n" ,RCC_Clocks.PCLK1_Frequency);
	SEGGER_RTT_printf(0, "RCC_Clocks.PCLK2_Frequency  %d\n" ,RCC_Clocks.PCLK2_Frequency);
#endif 		
}	

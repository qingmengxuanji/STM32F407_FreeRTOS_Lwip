/**
  ******************************************************************************
  * @file    Project/STM32F2xx_StdPeriph_Template/main.h 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    13-April-2012
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2012 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
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


///*FreeRTOS相关*/
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"



//#include "sys_arch.h"
#include "user_Lan8720.h"
#include "user_tcp_server.h"


/*项目相关*/
#include "led.h"
#include "user_delay.h"
#include "user_SysType.h"
#include "user_ETHCommunication.h"
#include "user_rcc_config.h"
#include "user_updata.h"



#define USER_JLINK
/*SYSTEMVIEW相关*/
#ifdef USER_JLINK
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#endif

extern void TCPIP_Init(void);



#endif /* __MAIN_H */


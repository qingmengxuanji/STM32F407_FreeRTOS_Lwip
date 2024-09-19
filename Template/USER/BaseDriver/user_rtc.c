#include "main.h"
#include "user_rtc.h"



/**********************************************************************
 * @ 函数名  ： RTC_Configuration
 * @ 功能说明： 配置RTC
 * @ 参数    ：
 * @ 返回值  ： 无
********************************************************************/
void RTC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
    PWR_BackupAccessCmd(ENABLE);	 //使能后备寄存器
}


#include "main.h"
#include "user_rtc.h"



/**********************************************************************
 * @ ������  �� RTC_Configuration
 * @ ����˵���� ����RTC
 * @ ����    ��
 * @ ����ֵ  �� ��
********************************************************************/
void RTC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//ʹ��PWRʱ��
    PWR_BackupAccessCmd(ENABLE);	 //ʹ�ܺ󱸼Ĵ���
}


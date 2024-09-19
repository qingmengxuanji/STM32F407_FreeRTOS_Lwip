#include "main.h"
__align(16) uint16_t SendData[16650] __attribute__((section("CCMRAM_SendData")));//�����ĵط�
/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle =NULL; // ������
static TaskHandle_t LED1_Task_Handle =NULL ;
static TaskHandle_t LED2_Task_Handle =NULL ;

static void AppTaskCreate(void *p);
static void LED1_Task(void* parameter);
static void LED2_Task(void* parameter);



int main(void)
{ 	
	SystemInit();
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_Offset);
	__enable_irq();
#ifdef USER_JLINK		
	SEGGER_RTT_Init();
	DWT->CTRL=(1<<0);
#endif 
	SystemClock_Config();
	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )128,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )16, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */  		
	
}

/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
void AppTaskCreate(void *p)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //����ϵͳ�ж����ȼ�����4	
	LED_Init();		    //��ʼ��LED�˿�
	
	ETH_BSP_Config();
	TCPIP_Init();				   // LWIP��ʼ��
	TCP_Server_Init();			   // �����̳߳�ʼ��


  /* ����LED_Task���� */
  xTaskCreate((TaskFunction_t )LED1_Task, /* ������ں��� */
                        (const char*    )"LED1_Task",/* �������� */
                        (uint16_t       )128,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&LED1_Task_Handle);/* ������ƿ�ָ�� */

  
	/* ����LED_Task���� */
  xTaskCreate((TaskFunction_t )LED2_Task, /* ������ں��� */
                        (const char*    )"LED2_Task",/* �������� */
                        (uint16_t       )128,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )3,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&LED2_Task_Handle);/* ������ƿ�ָ�� */
  
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  while (1)
		;
}



/**********************************************************************
  * @ ������  �� LED_Task
  * @ ����˵���� LED_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
void LED1_Task(void* parameter)
{

    while (1)
    {
        GPIO_ToggleBits(GPIOE , GPIO_Pin_3);
        vTaskDelay(50);   
    }
}

/**********************************************************************
  * @ ������  �� LED_Task
  * @ ����˵���� LED_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
void LED2_Task(void* parameter)
{
    while (1)
    {
        GPIO_ToggleBits(GPIOE , GPIO_Pin_4);
        vTaskDelay(200);   
    }
}



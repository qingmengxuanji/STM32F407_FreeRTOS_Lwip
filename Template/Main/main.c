#include "main.h"
__align(16) uint16_t SendData[16650] __attribute__((section("CCMRAM_SendData")));//打包存的地方
/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle =NULL; // 任务句柄
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
	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )128,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )16, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */  		
	
}

/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
void AppTaskCreate(void *p)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //设置系统中断优先级分组4	
	LED_Init();		    //初始化LED端口
	
	ETH_BSP_Config();
	TCPIP_Init();				   // LWIP初始化
	TCP_Server_Init();			   // 接收线程初始化


  /* 创建LED_Task任务 */
  xTaskCreate((TaskFunction_t )LED1_Task, /* 任务入口函数 */
                        (const char*    )"LED1_Task",/* 任务名字 */
                        (uint16_t       )128,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&LED1_Task_Handle);/* 任务控制块指针 */

  
	/* 创建LED_Task任务 */
  xTaskCreate((TaskFunction_t )LED2_Task, /* 任务入口函数 */
                        (const char*    )"LED2_Task",/* 任务名字 */
                        (uint16_t       )128,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&LED2_Task_Handle);/* 任务控制块指针 */
  
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  while (1)
		;
}



/**********************************************************************
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
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
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
void LED2_Task(void* parameter)
{
    while (1)
    {
        GPIO_ToggleBits(GPIOE , GPIO_Pin_4);
        vTaskDelay(200);   
    }
}



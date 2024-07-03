#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "lwip/err.h"

#include "ethernetif.h"
//#include "ethernet/ethernet.h"

#include "stm32f4x7_eth.h"


	
#define netifINTERFACE_TASK_STACK_SIZE		(1024)	//ethernetif_input的任务堆栈
#define netifINTERFACE_TASK_PRIORITY		9//(configMAX_PRIORITIES-1)//ethernetif_input的任务优先级
#define emacBLOCK_TIME_WAITING_FOR_INPUT	((portTickType)100)//输入阻塞的等待时间
#define netifGUARD_BLOCK_TIME				((portTickType)250)//块的保护时间

//网卡名
#define IFNAME0 's'
#define IFNAME1 't'

static struct netif *s_pxNetIf = NULL;
//以太网Rx的二值信号量
xSemaphoreHandle s_xSemaphore = NULL;
//以太网Rx & Tx DMA描述符

extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
//以太网收发缓冲区

//extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE],Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; 
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 
//跟踪当前发送和接收描述符的全局指针

//extern ETH_DMADESCTypeDef *DMATxDescToSet,*DMARxDescToGet;
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;
//最后接收帧信息的全局指针

extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;
//以太网数据接收处理任务
static void ethernetif_input( void * pvParameters );

/**
* In this function, the hardware should be initialized.
* Called from ethernetif_init().
*
* @param netif the already initialized lwip network interface structure
*        for this ethernetif
*/
//extern uint8_t IP_ADDR3;
static void low_level_init(struct netif *netif)
{
	uint32_t i;

	netif->hwaddr_len = ETHARP_HWADDR_LEN;//设置nettif MAC硬件地址长度
	//初始化MAC地址,设置什么地址由用户自己设置,但是不能与网络中其他设备MAC地址重复
	netif->hwaddr[0]=MAC_ADDR0; 
	netif->hwaddr[1]=MAC_ADDR1; 
	netif->hwaddr[2]=MAC_ADDR2;
	netif->hwaddr[3]=MAC_ADDR3;   
	netif->hwaddr[4]=MAC_ADDR4;
	netif->hwaddr[5]=MAC_ADDR5;

	netif->mtu = 1500;//nettif最大传输单位
	//网卡状态信息标志位，是很重要的控制字段，它包括网卡功能使能、广播
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP|NETIF_FLAG_LINK_UP;//接受广播地址和ARP流量
	
	s_pxNetIf =netif;

	//创建用于通知以太网帧接收的二进制信号量
	if(s_xSemaphore == NULL)
	{
		vSemaphoreCreateBinary(s_xSemaphore);
		xSemaphoreTake(s_xSemaphore,0);
	}

	ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr);//初始化以太网MAC中的MAC地址 
	
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);//初始化Tx描述符列表:链式模式
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);//初始化Rx描述符列表:链式模式

	//初始化Tx描述符列表:链式模式
	{ 
		for(i=0; i<ETH_RXBUFNB; i++)
		{
			ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
		}
	}

#ifdef CHECKSUM_BY_HARDWARE
	//使能Tx帧的校验和插入
	{
		for(i=0; i<ETH_TXBUFNB; i++)
		{
			ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
		}
	} 
#endif

	//创建处理ETH MAC的任务
	xTaskCreate(ethernetif_input,"Eth_if", netifINTERFACE_TASK_STACK_SIZE, NULL,
			  netifINTERFACE_TASK_PRIORITY,NULL);
	
    ETH_Start();//使能MAC和DMA传输和接收  
}


/**
* This function should do the actual transmission of the packet. The packet is
* contained in the pbuf that is passed to the function. This pbuf
* might be chained.
*
* @param netif the lwip network interface structure for this ethernetif
* @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
* @return ERR_OK if the packet could be sent
*         an err_t value if the packet couldn't be sent
*
* @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
*       strange results. You might consider waiting for space in the DMA queue
*       to become availale since the stack doesn't retry to send a packet
*       dropped because of memory failure (except for the TCP timers).
*/

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	static xSemaphoreHandle xTxSemaphore = NULL;
	err_t errval;
	struct pbuf *q;
	uint8_t *buffer ;
	__IO ETH_DMADESCTypeDef *DmaTxDesc;
	uint16_t framelength = 0;
	uint32_t bufferoffset = 0;
	uint32_t byteslefttocopy = 0;
	uint32_t payloadoffset = 0;

	if(xTxSemaphore == NULL) 
	{
		vSemaphoreCreateBinary(xTxSemaphore);
//		xSemaphoreGive(xTxSemaphore);
	}
	if(xSemaphoreTake(xTxSemaphore, netifGUARD_BLOCK_TIME))
	{
		DmaTxDesc = DMATxDescToSet;
		buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);
		bufferoffset = 0;

#if ETH_PAD_SIZE
	  pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif
		
		for(q = p; q != NULL; q = q->next)//从pbuf中拷贝要发送的数据 
		{
			//判断此发送描述符是否有效，即判断此发送描述符是否归以太网DMA所有
			if((DmaTxDesc->Status & ETH_DMATxDesc_OWN) != (uint32_t)RESET)
			{
				errval=ERR_USE;
				goto error;
			}
			//获取当前lwIP缓冲区中的字节
			byteslefttocopy = q->len;
			payloadoffset = 0;

			/*将pbuf中要发送的数据写入到以太网发送描述符中，有时候我们要发送的数据可能大于一个以太网
				描述符的Tx Buffer，因此我们需要分多次将数据拷贝到多个发送描述符中*/
			while( (byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE )
			{
				//将数据拷贝到以太网发送描述符的Tx Buffer中
				memcpy((u8_t*)((u8_t*)buffer+bufferoffset),(u8_t*)((u8_t*)q->payload+payloadoffset),
						(ETH_TX_BUF_SIZE-bufferoffset));
				//DmaTxDsc指向下一个发送描述符
				DmaTxDesc = (ETH_DMADESCTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);
				//检查新的发送描述符是否有效
				if((DmaTxDesc->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)
				{
					errval = ERR_USE;
					goto error;
				}
				buffer = (u8 *)(DmaTxDesc->Buffer1Addr);//更新buffer地址，指向新的发送描述符的Tx Buffer
			
				byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
				payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
				framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
				bufferoffset = 0;
			}

		  //拷贝剩余的数据
		  memcpy( (u8_t*)((u8_t*)buffer+bufferoffset),(u8_t*)((u8_t*)q->payload+payloadoffset),byteslefttocopy );
		  bufferoffset = bufferoffset + byteslefttocopy;
		  framelength = framelength + byteslefttocopy;
		}

		//当所有要发送的数据都放进发送描述符的Tx Buffer以后就可发送此帧了
		ETH_Prepare_Transmit_Descriptors(framelength);

		errval = ERR_OK;
error:
		//发送缓冲区发生下溢，一旦发送缓冲区发生下溢TxDMA会进入挂起状态
		if((ETH->DMASR&ETH_DMASR_TUS)!= (uint32_t)RESET)
		{
			
			ETH->DMASR = ETH_DMASR_TUS;//清除下溢标志
			/* 当发送帧中出现下溢错误的时候TxDMA会挂起，这时候需要向DMATPDR寄存器
			 随便写入一个值来将其唤醒，此处我们写0 */
			ETH->DMATPDR=0;
		}
		xSemaphoreGive(xTxSemaphore);
	}
	
	return errval;
}

/**
* Should allocate a pbuf and transfer the bytes of the incoming
* packet from the interface into the pbuf.
*
* @param netif the lwip network interface structure for this ethernetif
* @return a pbuf filled with the received packet (including MAC header)
*         NULL on memory error
*/
static struct pbuf * low_level_input(struct netif *netif)
{
	struct pbuf *p= NULL, *q;
	u32_t len;
	FrameTypeDef frame;
	u8 *buffer;
	__IO ETH_DMADESCTypeDef *DMARxDesc;
	uint32_t bufferoffset = 0;
	uint32_t payloadoffset = 0;
	uint32_t byteslefttocopy = 0;
	uint32_t i=0;  

	frame = ETH_Get_Received_Frame_interrupt();//接收一帧数据帧

	//获取数据包的大小并将其放入“len”变量中
	len = frame.length;
	buffer = (u8 *)frame.buffer;
	//从Lwip缓冲池中分配一个pbuf链
	if(len > 0) p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	//pbuf链分配成功
	if (p != NULL)
	{
		DMARxDesc = frame.descriptor;//获取接收描述符链表中的第一个描述符 
		bufferoffset = 0;
		for(q = p; q != NULL; q = q->next)
		{
			byteslefttocopy = q->len;
			payloadoffset = 0;

			//检查当前pbuf中要复制的字节长度是否大于Rx缓冲区大小
			while( (byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE )
			{
				//将数据复制到pbuf
				memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), 
						(u8_t*)((u8_t*)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));

				//指向下一个描述符
				DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
				buffer = (unsigned char *)(DMARxDesc->Buffer1Addr);

				byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
				payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
				bufferoffset = 0;
			}

		//复制pbuf中的剩余数据
		memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), 
			(u8_t*)((u8_t*)buffer + bufferoffset), byteslefttocopy);
		bufferoffset = bufferoffset + byteslefttocopy;
		}
	}
  
	DMARxDesc =frame.descriptor;//向DMA释放描述符

	//在Rx描述符中设置自己的位:将缓冲区返回给DMA
	for(i=0; i<DMA_RX_FRAME_infos->Seg_Count; i++)
	{  
		DMARxDesc->Status = ETH_DMARxDesc_OWN;
		DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
	}
	DMA_RX_FRAME_infos->Seg_Count =0;//清除段计数
  
	//当设置Rx缓冲区不可用标志时:清除它并恢复接收
	if((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
	{
		ETH->DMASR = ETH_DMASR_RBUS;//清除接收缓冲区不可用标志
		/* 当接收缓冲区不可用的时候 RxDMA 会进去挂起状态，
			通过向 DMARPDR 写入任意一个值来唤醒 Rx DMA */
		ETH->DMARPDR = 0;
	}
	return p;
}

/**
* This function is the ethernetif_input task, it is processed when a packet 
* is ready to be read from the interface. It uses the function low_level_input() 
* that should handle the actual reception of bytes from the network
* interface. Then the type of the received packet is determined and
* the appropriate input function is called.
*
* @param netif the lwip network interface structure for this ethernetif
*/
void ethernetif_input( void * pvParameters )
{
  struct pbuf *p;
  
  for(;;)
  {
    if(xSemaphoreTake(s_xSemaphore, emacBLOCK_TIME_WAITING_FOR_INPUT)==pdTRUE)
    {
TRY_GET_NEXT_FRAME:
      p = low_level_input(s_pxNetIf);
      if(p != NULL)
      {
        if(ERR_OK!=s_pxNetIf->input(p,s_pxNetIf))
        {
          pbuf_free(p);
        }
        else
        {
          goto TRY_GET_NEXT_FRAME;
        }
      }
    }
  }
}

/**
* Should be called at the beginning of the program to set up the
* network interface. It calls the function low_level_init() to do the
* actual setup of the hardware.
*
* This function should be passed as a parameter to netif_add().
*
* @param netif the lwip network interface structure for this ethernetif
* @return ERR_OK if the loopif is initialized
*         ERR_MEM if private data couldn't be allocated
*         any other err_t on error
*/



/*
*该函数是直接拿来用即可，如果没有特别的需求，基本不需要怎么修改它，它是 LwIP
*中默认的网卡初始化函数，内部封装了 low_level_init()函数
*/
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}



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


	
#define netifINTERFACE_TASK_STACK_SIZE		(1024)	//ethernetif_input�������ջ
#define netifINTERFACE_TASK_PRIORITY		9//(configMAX_PRIORITIES-1)//ethernetif_input���������ȼ�
#define emacBLOCK_TIME_WAITING_FOR_INPUT	((portTickType)100)//���������ĵȴ�ʱ��
#define netifGUARD_BLOCK_TIME				((portTickType)250)//��ı���ʱ��

//������
#define IFNAME0 's'
#define IFNAME1 't'

static struct netif *s_pxNetIf = NULL;
//��̫��Rx�Ķ�ֵ�ź���
xSemaphoreHandle s_xSemaphore = NULL;
//��̫��Rx & Tx DMA������

extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
//��̫���շ�������

//extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE],Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; 
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 
//���ٵ�ǰ���ͺͽ�����������ȫ��ָ��

//extern ETH_DMADESCTypeDef *DMATxDescToSet,*DMARxDescToGet;
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;
//������֡��Ϣ��ȫ��ָ��

extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;
//��̫�����ݽ��մ�������
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

	netif->hwaddr_len = ETHARP_HWADDR_LEN;//����nettif MACӲ����ַ����
	//��ʼ��MAC��ַ,����ʲô��ַ���û��Լ�����,���ǲ����������������豸MAC��ַ�ظ�
	netif->hwaddr[0]=MAC_ADDR0; 
	netif->hwaddr[1]=MAC_ADDR1; 
	netif->hwaddr[2]=MAC_ADDR2;
	netif->hwaddr[3]=MAC_ADDR3;   
	netif->hwaddr[4]=MAC_ADDR4;
	netif->hwaddr[5]=MAC_ADDR5;

	netif->mtu = 1500;//nettif����䵥λ
	//����״̬��Ϣ��־λ���Ǻ���Ҫ�Ŀ����ֶΣ���������������ʹ�ܡ��㲥
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP|NETIF_FLAG_LINK_UP;//���ܹ㲥��ַ��ARP����
	
	s_pxNetIf =netif;

	//��������֪ͨ��̫��֡���յĶ������ź���
	if(s_xSemaphore == NULL)
	{
		vSemaphoreCreateBinary(s_xSemaphore);
		xSemaphoreTake(s_xSemaphore,0);
	}

	ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr);//��ʼ����̫��MAC�е�MAC��ַ 
	
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);//��ʼ��Tx�������б�:��ʽģʽ
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);//��ʼ��Rx�������б�:��ʽģʽ

	//��ʼ��Tx�������б�:��ʽģʽ
	{ 
		for(i=0; i<ETH_RXBUFNB; i++)
		{
			ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
		}
	}

#ifdef CHECKSUM_BY_HARDWARE
	//ʹ��Tx֡��У��Ͳ���
	{
		for(i=0; i<ETH_TXBUFNB; i++)
		{
			ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
		}
	} 
#endif

	//��������ETH MAC������
	xTaskCreate(ethernetif_input,"Eth_if", netifINTERFACE_TASK_STACK_SIZE, NULL,
			  netifINTERFACE_TASK_PRIORITY,NULL);
	
    ETH_Start();//ʹ��MAC��DMA����ͽ���  
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
		
		for(q = p; q != NULL; q = q->next)//��pbuf�п���Ҫ���͵����� 
		{
			//�жϴ˷����������Ƿ���Ч�����жϴ˷����������Ƿ����̫��DMA����
			if((DmaTxDesc->Status & ETH_DMATxDesc_OWN) != (uint32_t)RESET)
			{
				errval=ERR_USE;
				goto error;
			}
			//��ȡ��ǰlwIP�������е��ֽ�
			byteslefttocopy = q->len;
			payloadoffset = 0;

			/*��pbuf��Ҫ���͵�����д�뵽��̫�������������У���ʱ������Ҫ���͵����ݿ��ܴ���һ����̫��
				��������Tx Buffer�����������Ҫ�ֶ�ν����ݿ��������������������*/
			while( (byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE )
			{
				//�����ݿ�������̫��������������Tx Buffer��
				memcpy((u8_t*)((u8_t*)buffer+bufferoffset),(u8_t*)((u8_t*)q->payload+payloadoffset),
						(ETH_TX_BUF_SIZE-bufferoffset));
				//DmaTxDscָ����һ������������
				DmaTxDesc = (ETH_DMADESCTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);
				//����µķ����������Ƿ���Ч
				if((DmaTxDesc->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)
				{
					errval = ERR_USE;
					goto error;
				}
				buffer = (u8 *)(DmaTxDesc->Buffer1Addr);//����buffer��ַ��ָ���µķ�����������Tx Buffer
			
				byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
				payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
				framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
				bufferoffset = 0;
			}

		  //����ʣ�������
		  memcpy( (u8_t*)((u8_t*)buffer+bufferoffset),(u8_t*)((u8_t*)q->payload+payloadoffset),byteslefttocopy );
		  bufferoffset = bufferoffset + byteslefttocopy;
		  framelength = framelength + byteslefttocopy;
		}

		//������Ҫ���͵����ݶ��Ž�������������Tx Buffer�Ժ�Ϳɷ��ʹ�֡��
		ETH_Prepare_Transmit_Descriptors(framelength);

		errval = ERR_OK;
error:
		//���ͻ������������磬һ�����ͻ�������������TxDMA��������״̬
		if((ETH->DMASR&ETH_DMASR_TUS)!= (uint32_t)RESET)
		{
			
			ETH->DMASR = ETH_DMASR_TUS;//��������־
			/* ������֡�г�����������ʱ��TxDMA�������ʱ����Ҫ��DMATPDR�Ĵ���
			 ���д��һ��ֵ�����份�ѣ��˴�����д0 */
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

	frame = ETH_Get_Received_Frame_interrupt();//����һ֡����֡

	//��ȡ���ݰ��Ĵ�С��������롰len��������
	len = frame.length;
	buffer = (u8 *)frame.buffer;
	//��Lwip������з���һ��pbuf��
	if(len > 0) p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	//pbuf������ɹ�
	if (p != NULL)
	{
		DMARxDesc = frame.descriptor;//��ȡ���������������еĵ�һ�������� 
		bufferoffset = 0;
		for(q = p; q != NULL; q = q->next)
		{
			byteslefttocopy = q->len;
			payloadoffset = 0;

			//��鵱ǰpbuf��Ҫ���Ƶ��ֽڳ����Ƿ����Rx��������С
			while( (byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE )
			{
				//�����ݸ��Ƶ�pbuf
				memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), 
						(u8_t*)((u8_t*)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));

				//ָ����һ��������
				DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
				buffer = (unsigned char *)(DMARxDesc->Buffer1Addr);

				byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
				payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
				bufferoffset = 0;
			}

		//����pbuf�е�ʣ������
		memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), 
			(u8_t*)((u8_t*)buffer + bufferoffset), byteslefttocopy);
		bufferoffset = bufferoffset + byteslefttocopy;
		}
	}
  
	DMARxDesc =frame.descriptor;//��DMA�ͷ�������

	//��Rx�������������Լ���λ:�����������ظ�DMA
	for(i=0; i<DMA_RX_FRAME_infos->Seg_Count; i++)
	{  
		DMARxDesc->Status = ETH_DMARxDesc_OWN;
		DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
	}
	DMA_RX_FRAME_infos->Seg_Count =0;//����μ���
  
	//������Rx�����������ñ�־ʱ:��������ָ�����
	if((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
	{
		ETH->DMASR = ETH_DMASR_RBUS;//������ջ����������ñ�־
		/* �����ջ����������õ�ʱ�� RxDMA ���ȥ����״̬��
			ͨ���� DMARPDR д������һ��ֵ������ Rx DMA */
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
*�ú�����ֱ�������ü��ɣ����û���ر�����󣬻�������Ҫ��ô�޸��������� LwIP
*��Ĭ�ϵ�������ʼ���������ڲ���װ�� low_level_init()����
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



#ifndef __LAN8720_H
#define __LAN8720_H
#include "common.h"
#include "stm32f4x7_eth.h"

//void ETH_GPIO_Config(void);
//static void ETH_MACDMA_Config(void);
#ifdef __cplusplus
 extern "C" {
#endif

#define MII_MODE	1

/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DP83848_PHY_ADDRESS       0x01 

#define LAN8720_PHY_ADDRESS     	0x00				//LAN8720 PHY–æ∆¨µÿ÷∑.

/* Specific defines for EXTI line, used to manage Ethernet link status */
#define ETH_LINK_EXTI_LINE             EXTI_Line14
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOB
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource14
#define ETH_LINK_EXTI_IRQn             EXTI15_10_IRQn 
/* PB14 */
#define ETH_LINK_PIN                   GPIO_Pin_14
#define ETH_LINK_GPIO_PORT             GPIOB
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOB
/* PHY registers */
#define PHY_MICR                  0x11 /* MII Interrupt Control Register */
#define PHY_MICR_INT_EN           ((uint16_t)0x0002) /* PHY Enable interrupts */
#define PHY_MICR_INT_OE           ((uint16_t)0x0001) /* PHY Enable output interrupt events */
#define PHY_MISR                  0x12 /* MII Interrupt Status and Misc. Control Register */
#define PHY_MISR_LINK_INT_EN      ((uint16_t)0x0020) /* Enable Interrupt on change of link status */
#define PHY_LINK_STATUS           ((uint16_t)0x2000) /* PHY link status interrupt mask */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void  ETH_BSP_Config(void);
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress);
void Eth_Link_EXTIConfig(void);
void Eth_Link_ITHandler(uint16_t PHYAddress);
void ETH_NVIC_Config(void);

#ifdef __cplusplus
}
#endif


#endif 

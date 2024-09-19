#ifndef _user_ETHCOMMUNICATION_H
#define _user_ETHCOMMUNICATION_H
#include "main.h"



extern struct netconn *remote_netconn;
/***************************************************************************
****************************************************************************/
#define PC_CMD_MAX (sizeof(CMD_Assemble) / sizeof(CMD_Type))

typedef struct CMD_TYPE
{
    char *CMD_String;
    void (*Run_Function)(void *parameter);
} CMD_Type;

extern void EthernetReceiveSuccess(uint32_t ParameterLen, uint16_t *Send);
extern void EthernetReceiveFailedString(uint16_t Error);
extern void EthernetInterfaceHandler(void);
extern void PC_DATAHandler(void);
extern uint32_t Crs(uint16_t *buf, uint32_t len);


void GETEDITION(void *parameter);
void ONLINEUPDATE1(void *parameter);
void ONLINEUPDATE2(void *parameter);


extern void EthernetReceiveSuccessString_Updata(uint32_t ParameterLen, uint16_t *Send);
extern void SendUpData(void *parameter);
/***************************************************************************
****************************************************************************/

#endif

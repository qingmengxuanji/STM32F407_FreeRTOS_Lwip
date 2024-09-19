#ifndef __user_SysType_H
#define __user_SysType_H
#include "stdint.h"



#define MAX_REC_CMDSIZE 10     // 上位机接收数据命令长度，固定10个字，20个字节
#define FALSE 0
#define TRUE 1

union DATA_BYTE
{
    uint32_t data;
    struct
    {
        uint16_t lbyte;
        uint16_t hbyte;
    } bytes;
};


struct ETH_TYPE_REG
{
    union DATA_BYTE ETH_ReceiveLength;
    union DATA_BYTE ETH_Length;

    union DATA_BYTE Ssum_L;
    union DATA_BYTE Ssum_H;

    uint16_t Rec_Comand[MAX_REC_CMDSIZE];
    uint16_t Data[600];
};

struct SYS_CTRL
{
    struct ETH_TYPE_REG Ethernet;
};

extern struct SYS_CTRL SysCtrl;

struct TIMER_REG
{
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
    uint8_t Day;
    uint8_t Momth;
    uint8_t Year;
    uint8_t  LastSecond;
    uint8_t  LastMinute;
};

#endif


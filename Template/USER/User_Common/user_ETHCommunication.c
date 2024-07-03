#include "main.h"

#include "api.h"
#define TransmitEthernetTxMessage(temp, num) netconn_write(remote_netconn, temp, (num * 2), NETCONN_COPY);
extern uint16_t SendData[16650];
//extern uint16_t KbData[200];
extern struct TIMER_REG SysTime;
// 下位机软件版本号
const uint8_t SoftVersionString_QDB[] = "I:_STM;V1.1.0_20240703_TEST_YZL"; // QDB:驱动板 
const CMD_Type CMD_Assemble[] = {
	  {"GETDATA"  , SendUpData},          // 发送老化数据
    //****************通用命令协议-1*****************/
    {"GETEDITION", GETEDITION},       // 读取下位机版本号命令
    {"ONLINEUPDATE1", ONLINEUPDATE1}, //
    {"ONLINEUPDATE2", ONLINEUPDATE2}, //
};
/******************************************************************
函数名称：
函数功能:以太网数据处理函数
输入参数：
输出参数：
返回值：
注意：
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
void EthernetInterfaceHandler(void)
{
    uint16_t i;
    uint32_t dSum, rSum;

    SysCtrl.Ethernet.ETH_ReceiveLength.bytes.lbyte = SysCtrl.Ethernet.Data[13];
    SysCtrl.Ethernet.ETH_ReceiveLength.bytes.hbyte = SysCtrl.Ethernet.Data[14];
    SysCtrl.Ethernet.ETH_ReceiveLength.data /= 2;
    if (SysCtrl.Ethernet.ETH_ReceiveLength.data > 600)
    {
        EthernetReceiveFailedString(0);
        return;
    }
    if (SysCtrl.Ethernet.Data[SysCtrl.Ethernet.ETH_ReceiveLength.data - 1] == 0x5E5D && SysCtrl.Ethernet.Data[SysCtrl.Ethernet.ETH_ReceiveLength.data - 2] == 0x5C5A)
    {
        dSum = Crs(SysCtrl.Ethernet.Data, SysCtrl.Ethernet.ETH_ReceiveLength.data - 4);
        rSum = SysCtrl.Ethernet.Data[SysCtrl.Ethernet.ETH_ReceiveLength.data - 4] + (SysCtrl.Ethernet.Data[SysCtrl.Ethernet.ETH_ReceiveLength.data - 3] << 16);
    }

    if (dSum == rSum)
    {
        for (i = 0; i < MAX_REC_CMDSIZE; i++)
        {
            SysCtrl.Ethernet.Rec_Comand[i] = SysCtrl.Ethernet.Data[2 + i];
        }
        PC_DATAHandler();
    }
    else
    {
        EthernetReceiveFailedString(0);
    }
    memset(SysCtrl.Ethernet.Data, 0, sizeof(SysCtrl.Ethernet.Data));
}
/******************************************************************
函数名称：
函数功能:解析收到的数据命令
输入参数：
输出参数：
返回值：
注意：
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
void PC_DATAHandler(void)
{
    bool verify_bool;
    uint16_t i, j = 0;
    uint16_t ptr[30];
    char *Command = CMD_Assemble[0].CMD_String;
    uint16_t CMD_Count = 0;

    for (i = 0; i < MAX_REC_CMDSIZE; i++)
    {
        ptr[j++] = (SysCtrl.Ethernet.Data[i + 2] & 0x00FF);
        ptr[j++] = ((SysCtrl.Ethernet.Data[i + 2] >> 8) & 0x00FF);
    }
    ptr[j++] = 0;
    ptr[j++] = 0;

    while (CMD_Count < PC_CMD_MAX)
    {

        verify_bool = TRUE;
        for (i = 0; Command[i]; i++)
        {
            if (Command[i] != ptr[i])
            {
                verify_bool = FALSE;
                break;
            }
        }
        if (ptr[i] != 0)
        {
            verify_bool = FALSE;
        }

        if (verify_bool == TRUE)
        {
            user_delayUs(100);
            (*CMD_Assemble[CMD_Count].Run_Function)((void *)(&SysCtrl.Ethernet.Data[15]));
        }
        CMD_Count++;
        if (CMD_Count < PC_CMD_MAX)
        {
            Command = CMD_Assemble[CMD_Count].CMD_String;
        }
        else
        {
        }
    }
}

/******************************************************************
函数名称：
函数功能:数据接收成功标志，传输给上位机
输入参数：
输出参数：
返回值：
注意：上传参数长度为零时不需要上参数首地址,数据长度过大时默认恢复校验和为0
           ParameterLen为0时表示无上传数据，此时Send可以为任意值
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
void EthernetReceiveSuccessString(uint32_t ParameterLen, uint16_t *Send)
{
    uint16_t temp[40], num = 0, i;
    uint32_t SUM;
    union DATA_BYTE TEMP;

    temp[num++] = 0xBBAA;
    temp[num++] = 0xDDCC;
    for (i = 0; i < MAX_REC_CMDSIZE; i++)
    {
        temp[num++] = SysCtrl.Ethernet.Rec_Comand[i];
    }
    temp[num++] = 0;
    temp[num++] = 0xc0a8;
    temp[num++] = (0x7b << 8) | IP_ADDR3;
    temp[num++] = 0x3388;
    TEMP.data = ParameterLen * 2 + (num * 2) + 12; // 参数长度+这行之前的长度+这行之后除去命令的长度
    temp[num++] = TEMP.bytes.lbyte;
    temp[num++] = TEMP.bytes.hbyte;
    SUM = Crs(temp, num);
    TransmitEthernetTxMessage(temp, num);
    if (0 != ParameterLen)
    {
        if (ParameterLen <= 1460)
        {
            TransmitEthernetTxMessage(Send, ParameterLen);
            if (ParameterLen < 1000)
            {
                SUM += Crs(Send, ParameterLen);
            }
            else
            {
                SUM = 0;
            }
        }
        else
        {
            uint32_t Len = ParameterLen;

            for (i = 0; Len > 1460; i++)
            {
                TransmitEthernetTxMessage((Send + 1460 * i), 1460);
                Len -= 1460;
            }
            TransmitEthernetTxMessage((Send + ParameterLen - Len), Len);
            SUM = 0;
        }
    }
    num = 0;
    temp[num++] = SUM;
    temp[num++] = SUM >> 16;
    temp[num++] = 0x5C5A;
    temp[num++] = 0x5E5D;
    TransmitEthernetTxMessage(temp, num);
}


/***********************************************************************
  * @ 函数名  ： EthernetReceiveSuccessString_Updata
  * @ 功能说明： 传输FLASH中存储的HEX文件
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
void EthernetReceiveSuccessString_Updata(uint32_t ParameterLen, uint16_t *Send)
{
    uint16_t temp[40], num = 0, i;
    uint32_t SUM;
	  
	  uint32_t PackageCnt=0;
    if (0 != ParameterLen)
    {
        if (ParameterLen <= 1460)
        {
            TransmitEthernetTxMessage(Send, ParameterLen);
            if (ParameterLen < 1000)
            {
                SUM += Crs(Send, ParameterLen);
            }
            else
            {
                SUM = 0;
            }
        }
        else
        {
            uint32_t Len = ParameterLen;
					  PackageCnt =  Len/1460;

            for (i = 0; Len > 1460; i++)
            {
                TransmitEthernetTxMessage((Send + 1460 * i), 1460);
                Len -= 1460;
            }
            TransmitEthernetTxMessage((Send + ParameterLen - Len), Len);
            SUM = 0;
        }
    }
    num = 0;
    temp[num++] = SUM;
    temp[num++] = SUM >> 16;
    temp[num++] = 0x5C5A;
    temp[num++] = 0x5E5D;
    TransmitEthernetTxMessage(temp, num);
}
/******************************************************************
函数名称：
函数功能:数据接受失败标志，传输给上位机
输入参数：
输出参数：
返回值：
注意：
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
void EthernetReceiveFailedString(uint16_t Error)
{
    uint16_t temp[28], num = 0, i;
    uint32_t SUM;
    temp[num++] = 0xBBAA;
    temp[num++] = 0xDDCC;
    for (i = 0; i < MAX_REC_CMDSIZE; i++)
    {
        temp[num++] = SysCtrl.Ethernet.Rec_Comand[i];
    }
    temp[num++] = 0;
    temp[num++] = 0xc0a8;
    temp[num++] = (0x7b >> 8) | IP_ADDR3;
    temp[num++] = 0x1144;
    temp[num++] = 46;
    temp[num++] = 0x0000;
    temp[num++] = Error; // 后面改成错误码
    SUM = Crs(temp, num);
    temp[num++] = SUM;
    temp[num++] = SUM >> 16;
    temp[num++] = 0x5C5A;
    temp[num++] = 0x5E5D;
    TransmitEthernetTxMessage(temp, num);
}
/******************************************************************
函数名称：
函数功能:校验和计算
输入参数：
输出参数：
返回值：
注意：校验和 = 命令 + 数据，len按字来算，而不是按字节算
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
uint32_t Crs(uint16_t *buf, uint32_t len)
{
    uint32_t i, sum = 0;
    uint16_t *ptr;

    if (len > 1536)
    {
        return 0;
    }
    ptr = buf;
    for (i = 0; i < len; i++)
    {
        sum += ((*(ptr + i)) & 0x00ff);
        sum += ((*(ptr + i)) >> 8);
    }

    return sum;
}

/******************************************************************
函数名称：
函数功能:获取程序版本号
输入参数：
输出参数：
返回值：
注意：
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
void GETEDITION(void *parameter)
{
    uint32_t parity;
    uint16_t i, j, len;
    uint16_t SoftVerSend[22];

    len = (sizeof(SoftVersionString_QDB) / 2); // 36

    for (i = 0, j = 0; i < len; i++)
    {
        SoftVerSend[i] = SoftVersionString_QDB[j++];
        SoftVerSend[i] += (SoftVersionString_QDB[j++] << 8);
    }
    parity = Crs(SoftVerSend, len);
    SoftVerSend[(len + 1)] = parity;
    SoftVerSend[(len + 2)] = (parity >> 16);

    EthernetReceiveSuccessString((len + 2), &SoftVerSend[0]);
}
/******************************************************************
函数名称：
函数功能:在线更新第一条命令获取整包数据长度
输入参数：
输出参数：
返回值：
注意：
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
// 在线更新已完成
static uint16_t pack_amount = 0;
void ONLINEUPDATE1(void *parameter)
{
    union DATA_BYTE updata_size;

    updata_size.bytes.lbyte = *((uint16_t *)parameter)++;
    updata_size.bytes.hbyte = *((uint16_t *)parameter)++;
    pack_amount = updata_size.data / 0x400 + 1;
    if (updata_size.data % 0x400 == 0)
        pack_amount--;
    // 获取包数量
    Erase_Updata_Program_Area(); // 擦除升级程序存放区
    Reset_Updata_Flag();         // 擦除更新标志位

    EthernetReceiveSuccessString(0, 0);
}
/******************************************************************
函数名称：
函数功能:在线更新第二条命令接收更新的数据
输入参数：
输出参数：
返回值：
注意：
******************************************************************/
/******************************************************************
函数更改历史记录：
******************************************************************/
void ONLINEUPDATE2(void *parameter)
{
	
    uint16_t pack_num = *((uint16_t *)parameter - 3); // 获取包编号
    // 开始写入
#ifdef USER_JLINK		
		SEGGER_RTT_printf(0, "pack_num %d\n" ,pack_num);
#endif 		
    Program_Updata_Area(Updata_Program_Address, (uint32_t)parameter, 1024);  //将接收缓存的数据写入app flash，size为字节,updata_program_add不需要动
    if (pack_amount - pack_num == 0) // 最后一包已结束
    {
        Set_Updata_Flag(); // 更新标志置位
			  EthernetReceiveSuccessString(0, 0); //此处回复是最后一包，之后需复位程序
				user_delayMs(1000);
        System_Reset();
    }

    EthernetReceiveSuccessString(0, 0);     //每一包都需要回复
			

}

/***********************************************************************
  * @ 函数名  ： SendUpData
	* @ 功能说明： 发送
   aa bb cc dd 47 45 54 44 41 54   
   41 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   
   26 00 00 00 2e 05 00 00 5a 5c 5d 5e 
	 这条命令语句时候，将会把单片机之前存储在flash中的运行文件打包发出
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
//发送本机下位机程序
void SendUpData(void *parameter)
{
//#define Backdata_Program_Address (0x08000000 + 0x80000) // 备份原先的程序放置
    uint16_t *sendptr, i;
    uint32_t length = 0;	
	  uint16_t *sptr;		
		for( uint32_t PackCnt=0;PackCnt<192;PackCnt++ ){			
			sendptr = (uint16_t *)(SendData);
			*sendptr++ = PackCnt+1;		
			 length = 0;
			 sptr = (uint16_t *)(Backdata_Program_Address+0x400*PackCnt);
			for (uint16_t i=(PackCnt*512);i<(512*PackCnt+512);i++) //0-1023 0x08080400 +0x3ff
			{
				*sendptr++ = *sptr++;
			}
			length = (uint32_t)(sendptr - (uint16_t *)(SendData));
			sendptr = (uint16_t *)(SendData);
			EthernetReceiveSuccessString_Updata(length, sendptr);			
		  vTaskDelay(1000);
		}
}


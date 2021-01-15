#ifndef USR_GPRS_H
#define USR_GPRS_H

#include "usr_main.h"

#define	STATUS_ACC_ON		((u32)0x00000001)	//位置信息包车辆状态ACC开
#define	STATUS_HAVE_GPS		((u32)0x00000002)	//位置信息包车辆状态GPS OK
#define	STATUS_GPS_S		((u32)0x00000004)	//位置信息包车辆状态GPS 0 北纬 1 南纬
#define	STATUS_GPS_W		((u32)0x00000008)	//位置信息包车辆状态GPS 0 东经 1 西经

#ifndef _GPRS_TYPE
#define _GPRS_TYPE

typedef enum{
	GPRS_NULL,
	LOGIN,DATA,BKDATA,UPGRESULT,AGPSDATA,
	HAND
}GPRS_TYPE;
	
#endif

#define DATABUFLEN 		500
#define SCIBUFLEN  		1500
#define UPDRADELEN 		30
#define TFTPLEN    		30
#define GPRSCONTLEN		200

#define SPEED_GPS		0


#define	MODULE_OFF_MODE		0			//设备休眠期间：1，模块进入关机模式,；0模块进入休眠模式
#define DEEP_SLEEP_MODE		0			//深度休眠模式，在振动停止一段时间后，设备不在周期性唤醒上传数据

//#define IMEI_MANUAL	"861118010103569"
//#define IMEI_MANUAL	"358511159000052"
#define IMEI_MANUAL	"000000000000004"
//#define IMEI_MANUAL	""
#define PLAT_TYPE	1		//平台类型，1是益文的，2是宝康源


#ifndef _GPRS_SEND
#define _GPRS_SEND
typedef struct{
	unsigned char posCnt;     	//定位包发送计数	
	unsigned short posTimer;  	//定位包上传间隔倒计时
	unsigned char posFlag;	  	//需要发送定位包发送标志
	unsigned char handFlag;		//需要发送心跳包标志
	unsigned short handsendcnt;	//已发送的心跳包数量
}GPRS_SEND;
#endif

extern GPRS_TYPE  GprsType;
extern GPRS_SEND  GprsSend;
extern unsigned short ConnectDelayCnt;
extern unsigned char  Upd_command_len;
extern unsigned short At_Timeout_Cnt;
extern unsigned int  ActiveTimer;  
extern unsigned char WifiCnt;


extern char GprsSendBuf[DATABUFLEN];
extern char AtSendbuf[SCIBUFLEN];
extern char UpgradeSendBuf[UPDRADELEN];
extern char UserIDBuf[16];

extern char CCID[21];
extern char IMEI[16];	
extern char Lac[5];
extern char Cid[9];
extern char Wifi_Content[200];
extern u32  speed_gprs;
extern u8   WifiCnt;
extern u8	WifiScanDelay;

int Usr_Atoi(char *pSrc);
void GPRS_SaveBreakPoint(void);
void StrAscii2Hex(char *pSrc,char *pDst,u16 Srclen);
void WIRELESS_GprsReceive(char *pSrc);
void WIRELESS_Handle(void);

u32 Ascii2Hex(char *pSrc,unsigned char srcLen);

u32 HEX2BCD_FOR_U32(u16 Val_HEX);
u16 HEX2BCD(u8 Val_HEX);
void Ascii2BCD(char *pSrc,unsigned char *pDst);
u16 Ascii2BCD_u16(char *pSrc,unsigned char len);
void Hex2StrAscii(char *pSrc,char *pDst,u16 Srclen);
void Itoa(unsigned char src, char dst[]);
int Usr_Atoi(char *pSrc);
u16 WIRELESS_GprsSendPacket(GPRS_TYPE switch_tmp);



#endif

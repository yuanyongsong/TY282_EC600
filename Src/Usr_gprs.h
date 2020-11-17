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
	LOGIN,DATA,BKDATA,UPGRESULT
}GPRS_TYPE;
	
#endif

#define DATABUFLEN 		500
#define SCIBUFLEN  		1500
#define UPDRADELEN 		30
#define TFTPLEN    		30
#define GPRSCONTLEN		200

#define USR_FOR_JP		0				//切换在中国使用和日本使用开关，0为中国使用，1为日本使用
#define SENSOR_3		1
#define NO_SLEEP		0
#define TEST_USE		1

#if USR_FOR_JP
	#define IMEI_MANUAL	    ""
	#define JP_AT_USE		1
	#define USE_SOFTSIM		0
#else
	#define IMEI_MANUAL	"861118010103569"
	//#define IMEI_MANUAL	"352656103473735"
	//#define IMEI_MANUAL	"999999999999999"
	//#define IMEI_MANUAL	""
	#define USE_SOFTSIM		0

#endif

#ifndef _GPRS_SEND
#define _GPRS_SEND
typedef struct{
	unsigned char posCnt;     //定位包发送计数	
	unsigned short posTimer;  //定位包上传间隔倒计时
	unsigned char posFlag;
}GPRS_SEND;
#endif

extern GPRS_TYPE  GprsType;
extern GPRS_SEND  GprsSend;
extern unsigned short ConnectDelayCnt;
extern unsigned char  Upd_command_len;
extern unsigned short At_Timeout_Cnt;
extern unsigned int  ActiveTimer;  
extern char GprsSendBuf[DATABUFLEN];
extern char AtSendbuf[SCIBUFLEN];
extern char UpgradeSendBuf[UPDRADELEN];
extern char UserIDBuf[16];

extern char CCID[21];
extern char IMEI[16];	
extern u32  speed_gprs;

int Usr_Atoi(char *pSrc);
void GPRS_Send_CC888(void);
void GPRS_Send_CC828(void);
void GPRS_SaveBreakPoint(void);
void StrAscii2Hex(char *pSrc,char *pDst,u16 Srclen);
void WIRELESS_GprsReceive_CC828(char *pSrc,u16 len);
void WIRELESS_GprsReceive(char *pSrc,u16 len);
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

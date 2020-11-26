#ifndef USR_AT_H
#define USR_AT_H

#include "usr_main.h"

#ifndef _AT_TYPE
#define _AT_TYPE
typedef enum{
	AT_NULL=0,
	AT_ATE,AT_CLIP,AT_COLP,AT_CMGF,AT_CSCS,              			 		//1-5
	AT_CLVL,AT_CREG,AT_CGREG,AT_QISEND,							//6-10 			 
	AT_QIDEACT,AT_CMGRD,AT_CMGR,AT_CSMP,AT_CNMI,   							//11-15      		  	
	AT_CMGS,AT_CBC,AT_SMSEND,AT_ATH,  							//16-20       					
	AT_CSDH,AT_MCELL_1,AT_MCELL_2,AT_GSN,AT_SAPBR_P,				//26-30
	AT_FTPCID,AT_FTPTYPE,AT_FTPSERV,AT_FTPUN,AT_FTPPW,						//31-35
	AT_FTPGETNAME,AT_FTPGETPATH,AT_FTPGET_1,AT_FTPGET_2,AT_ATD,				//36-40
	AT_CSQ,AT_FTPSIZE,AT_DDET,AT_CDNSGIP,AT_HTTPINIT,						//41-45
	AT_HTTPPARA_1,AT_HTTPPARA_2,AT_HTTPTERM,AT_HTTPACTION,AT_CNMP,			//46-50
	AT_HTTPREAD,AT_CFUN_4,AT_CFUN_1,AT_CGSN,AT_CALM,						//51-55
	AT_STTONE,AT_ATA,AT_CREC_4,AT_CREC_5,AT_CMIC_0,							//56-60
	AT_CLVL_H,AT_CMEE,AT_QGPS,AT_QGPSCFG,AT_CMGD,							//61-65
	AT_AT,AT_QICSGP,AT_QIREGAPP,AT_QIOPEN,AT_QIACT,
	AT_QISTATE,AT_CSMP_EN,AT_CVHU,AT_GPRSEND,							//66-70
	AT_MIPOPEN,AT_SENDCHK,AT_MIPCLOSE,AT_CPMS,AT_CSCS_EN,						//71-75
	AT_QCELLLOC,AT_QIDNSGIP_XTRA,AT_QGPSEND,AT_QGPSGNMEA,AT_COPS_CHECK,	        //76-80
	AT_QURCCFG,AT_COPS,AT_QIDNSGIP,AT_QGPSXTRA_1,AT_QGPSXTRA_2,           //81-85
	AT_QHTTPCFG,AT_QHTTPURL,AT_QHTTPURL_D,AT_QHTTPGET,AT_QHTTPREAD,         //86-90
	AT_QGPSXTRATIME,AT_QGPSXTRADATA,AT_QFDEL,AT_GPRSSTOP,AT_QNTP,           //91-95
	AT_QSCLK,AT_QGPSXTRADATA_CK,AT_QMIC,AT_QCFG,AT_QDAI,AT_CSCLK,           //96-100,
	AT_MIPNTP,AT_CCLK,AT_GTSET,AT_CLCC,AT_GTAUDGAIN,                        //101-105
	AT_GTSET_1,AT_CPIN,AT_CIPMUX,AT_CK_BTHOST,
	AT_BTPWR_ON,AT_BTACPT,AT_BTSEND,AT_BTDISCONN,AT_BTHOST,
	AT_BTPWR_OFF,AT_BTSPPSEND,AT_BTPAIR,AT_WAKEUP,AT_CSCLK_CLOSE,
	AT_IPR,AT_CCID,AT_BTSTATUS,AT_CENG_SET,AT_CENG_CK,AT_BTSCAN,
	AT_QIMODE,AT_QILOCIP,AT_QIDNSIP,AT_QGNSSC,AT_QBTGATSREG,
	AT_QBTGATSS,AT_QBTGATSC_1,AT_QBTGATSD,AT_QBTGATSC_2,AT_QBTGATSST,
	AT_QBTGATSRSP,AT_QINDI,AT_QIRD_0,AT_QIRD_1,AT_QIFGCNT,AT_QGNSSTS,
	AT_QGNSSEPO,AT_QGEPOAID,AT_QICLOSE_AGPS,AT_CIPSHUT,AT_QICLOSE,
	AT_CGREG_SET
}AT_TYPE;
#endif

#ifndef _AT_ERROR
#define _AT_ERROR
typedef struct{
	unsigned char PsSingalEorCnt;     //ps信号相关出错计数
	unsigned char GprsConnectEorCnt;  //GPRS连接过程中相关出错计数
	unsigned char GprsSendEorCnt;     //GPRS数据发送过程中相关出错计数
	unsigned char FtpConnectEorCnt;   //FTP连接过程中相关出错计数
	unsigned char FtpGetEorCnt;       //FTP读取数据过程中出错计数
	unsigned char NoSingalCnt;        //无信号时检查注册计数
}AT_ERROR;
#endif

//模块重启原因
#define CANT_ATTACH_NET			1		//无法附着到网络
#define CONNECT_SERVICE_FAILED	2		//连接服务器过程中出错
#define NO_SIMCARD				3		//没有SIM卡或者SIM卡检测出错
#define MODUAL_INFO_ERROR		4		//其他读取模块信息出错
#define MODUAL_NOACK			5		//模块超过三次无应答

extern AT_TYPE  AtType;
extern AT_ERROR AtError;

extern unsigned char baseTimeSec;
extern unsigned char baseTimeMin;
extern unsigned char baseTimeHor;

extern unsigned  short GprsDataLen;
extern unsigned char InitCmdTimes;
extern unsigned short GprsRecDataLen;
extern unsigned short BatVoltage;
extern unsigned char Rssi; 

extern char MccMnc[7];				
	

void AT_SendPacket(AT_TYPE temType,char * pDst);
unsigned char AT_InitReceive(AT_TYPE *temType,char * pSrc);
unsigned char AT_Receive(AT_TYPE *temType, char * pSrc);
void Flag_check(void);


#endif



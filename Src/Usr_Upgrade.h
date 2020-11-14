#ifndef USR_UPGRADE_H
#define USR_UPGRADE_H

#include "usr_main.h"


#ifndef _FS_UPG
#define _FS_UPG
typedef struct{
	char   	Ok[3];			  			//为"OK"表示从flash读出的数据有效，不是"OK"表示此flash没有初始化
	char 	AppFilePath[100];   		 	//远程升级文件在http服务器上的文件路径
	char 	AppFileName[50];   		 	//远程升级文件在http服务器上的文件名
	char   	AppIpAdress[50];   			//远程升级用http地址
	char   	HttpError[32];				//http下载文件中出现的错误原因

	unsigned char   UpgNeedSendGprs;	//0x01 时要求GPRS回复升级结果	
	unsigned char   UpgEnJamp;			//0xaa为程序已经更新完毕，可以跳转，0x55为程序接收完毕
	u32   			AppLenBuf;			//接收数据OK,存EXFLASH长度。
}FS_UPG;

#endif

#ifndef _UPG_INFO
#define _UPG_INFO
typedef struct{
	unsigned char 	AppReadComplete:1;	//文件已经读取完成
	unsigned char 	UpgrateFail:1;		//远程升级动作失败
	unsigned char 	NeedCheckUploadState:1;		//下载文件过程中需要周期性查询下载状态
	unsigned char 	NeedUpdata:1;		//开始远程升级
	unsigned char 	NeedDiscontHttp:1;	//需要断开HTTP连接

	unsigned char 	AppDownloadOk;		//文件已经成功下载完成
	unsigned short 	UpgPacketNums;
	unsigned int  	UpgExFlashAddr;		//APP在Flash存储地址
	unsigned char 	Verify_ok;			//MD5校验一致
	unsigned char 	RetryCnt;			//因网络原因请求失败重试测试
	unsigned char 	RetryWaitCnt;		//重试等待时间间隔
	unsigned char 	Md5decrypt[17];
}UPG_INFO;
#endif

extern UPG_INFO UpgInfo;
extern FS_UPG FsUpg;
extern char Md5FileAsc[50];

void WIRELESS_UpgradeReceive(char *pSrc);
void UpgInfo_InitValue(void);
void FSUPG_InitValue(void);

#endif

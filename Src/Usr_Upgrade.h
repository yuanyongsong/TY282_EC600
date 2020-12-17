#ifndef USR_UPGRADE_H
#define USR_UPGRADE_H

#include "usr_main.h"




#ifndef _UPG_INFO
#define _UPG_INFO
typedef struct{
	unsigned char 	AppReadComplete:1;			//文件已经读取完成
	unsigned char 	UpgrateFail:1;				//远程升级动作失败
	unsigned char 	NeedCheckUploadState:1;		//下载文件过程中需要周期性查询下载状态
	unsigned char 	NeedUpdata:1;				//开始远程升级
	unsigned char 	NeedDiscontHttp:1;			//需要断开HTTP连接
	unsigned char 	NeedWaitUpgrade:1;			//需要等到指定时间后开始升级
	unsigned char 	HaveGetRankData:1;			//已经获取到了随机升级倒计时数

	unsigned char 	AppDownloadOk;		//文件已经成功下载完成
	unsigned short 	UpgPacketNums;
	unsigned int  	UpgFlashAddr;		//APP在Flash存储地址
	unsigned char 	Verify_ok;			//MD5校验一致
	unsigned char 	RetryCnt;			//因网络原因请求失败重试测试
	unsigned char 	RetryWaitCnt;		//重试等待时间间隔
	unsigned char 	Md5decrypt[17];
}UPG_INFO;
#endif

extern UPG_INFO UpgInfo;
extern char Md5FileAsc[50];

void WIRELESS_UpgradeReceive(char *pSrc);
void UpgInfo_InitValue(void);
void FSUPG_InitValue(void);

#endif

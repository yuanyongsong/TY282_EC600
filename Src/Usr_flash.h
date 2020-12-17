#ifndef USR_FLASH_H
#define USR_FLASH_H

#include "usr_main.h"

#define STM32_FLASH_SIZE 	128 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_BASE 	0x08000000 	//STM32 FLASH的起始地址
//#define BEAKPIONT_ADDR	 0X08020000 - 0x2800	//断点存储位置起始，在最后10k地址处，大小8k用于存放断点

//设置FLASH保存数据地址(必须为偶数)
#define FLASH_SAVE_ADDR  	0X08020000 - 0x800		//STMG070RB flash为128K，使用末尾2K作为数据存储区
//远程升级文件存放位置0x08011800，即地70k的位置开始，所以，要求程序大小不得大于57k(12k的BootLoader和末尾2k的数据)
#define FOAT_SAVE_ADDR  	0x08011800	 //远程升级文件存储的地址段为0x08011800-0x0801F800 共56k，28个扇区


#ifndef _FS_UPG
#define _FS_UPG
typedef struct{
	char   	Ok[3];			  			//为"OK"表示从flash读出的数据有效，不是"OK"表示此flash没有初始化
	char 	AppFilePath[100];   		 //远程升级文件在http服务器上的文件路径
	char 	AppFileName[50];   		 	//远程升级文件在http服务器上的文件名
	char   	AppIpAdress[50];   			//远程升级用http地址
	char   	HttpError[32];				//http下载文件中出现的错误原因

	unsigned char   UpgNeedSendGprs;	//0x01 时要求GPRS回复升级结果	
	unsigned char   UpgEnJamp;			//0xaa为程序已经更新完毕，可以跳转，0x55为程序接收完毕
	u32   			AppLenBuf;			//接收数据OK,存EXFLASH长度。
}FS_UPG;

#endif


#ifndef _FS
#define _FS
typedef struct{
	char   Ok[3];			   			//为"OK"表示从flash读出的数据有效，不是"OK"表示此flash没有初始化
	char   UserID[16];
	char   DeviceImei[16];
	char   IpPort[8];
	char   AppIpPort[6];       			//远程升级用FTP端口号
	char   IpAdress[50];				//服务器IP或者域名
	char   AppIpAdress[50];   		 	//远程升级用http的IP或地址
	char   UbloxIp[21];					//请求ublox星历时的IP
	char   UbloxPort[7];				//请求ublox星历时的端口
	char   ApnName[32];
	char   GprsUserName[32];    		//gprs用户名
	char   GprsPassWord[16];    		//gprs密码
	char   MccMnc[7];          			//最后的MCCMNC
	char   LatitudeLast[14]; 			//最后一次定位纬度，整数4位，小数4位，不足补0,最后有'N'或'S'
	char   LongitudeLast[14];			//最后一次定位经度，整数5位，小数4位，不足补0,最后有'E'或'W'
	FS_UPG FsUpg;						//远程升级相关

	unsigned char   Sensor;				//灵敏度值
	unsigned char   ModeSet;            //设备模式设置
	unsigned char 	HaveCertificate;	//模块是否有烧录证书，0xAA为已经烧录，其他为未烧录
	unsigned char   HaveSetApn;			//非零表示客户有短信设置apn
	unsigned short  BKSavedCnt;         //已存断点计数，最大为500
	unsigned int    BkSavedLen;         //已存断点字长计数，单位byte
	unsigned short  BkSendCnt;          //已发断点计数
	unsigned int  	BkSendLen;          //已发断点字长计数，单位byte
	unsigned int  	Interval;           //定位包上传间隔,单位是秒	
	unsigned char   ShutDownHour;		//关机时间段，小时
	unsigned char   ShutDownMin;		//关机时间段，分钟
	unsigned char   BootHour;			//开机时间段，小时
	unsigned char   BootMin;			//开机时间段，分钟

}FS;
#endif


extern FS Fs;


void FS_InitValue(void);
void FS_UpdateValue(void);
void FS_FactroyValueFile(void);
void STMFLASH_WriteFs(u32 WriteAddr,u64 *pBuffer,u16 NumToWrite);
void STMFLASH_Read(u32 ReadAddr, u32 *pBuffer, u16 NumToRead);
void FSUPG_FactroyValue(void);
void STMFLASH_Write_NoCheck(u32 WriteAddr, u64 *pBuffer, u16 NumToWrite);
#endif



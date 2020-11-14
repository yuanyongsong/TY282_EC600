#include "usr_main.h"
#include "stm32g0xx_ll_flash.h"
//使用stm32的flash模拟eeprom
FS Fs;

//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32 *)faddr;
}

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针

void STMFLASH_Read(u32 ReadAddr, u32 *pBuffer, u16 NumToRead)
{
	#if 1
	u16 i;
	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr); 		//读取2个字节.
		ReadAddr += 4;								  	//偏移2个字节.
	}
	#endif
}

//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:字(32位)数
void STMFLASH_Write_NoCheck(u32 WriteAddr, u64 *pBuffer, u16 NumToWrite)
{
	#if 1
	u16 i;
	LL_FLASH_EnableProgram();						//使能写入（允许写入）
	while (LL_FLASH_IsActiveFlag_BSY()) {}
	for (i = 0; i < NumToWrite; i++)
	{
		LL_FLASH_Program(ProgaraType_DATA64, WriteAddr, pBuffer[i]);
		WriteAddr += 8; 							//地址增加4.
	}
	while (LL_FLASH_IsActiveFlag_BSY()) {}
	LL_FLASH_DisenableProgram();					//禁止写入
	#endif
}


#define STM_SECTOR_SIZE 2048						//STM32G070一个扇区2K大小

//此函数专用于写Fs数据(地址正是flash的最后一个扇区)
void STMFLASH_WriteFs(u32 WriteAddr, u64 *pBuffer, u16 NumToWrite)
{
	#if 1
	u32 secpos;  							//扇区地址
	u32 offaddr; 							//去掉0X08000000后的地址
	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
	{
		return; 							//非法地址
	}
	LL_Flash_Unlock();						//解锁
	offaddr = WriteAddr - STM32_FLASH_BASE; //实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;		//所在扇区地址  0~63 for STM32G070RB

	LL_Flash_PageErase(secpos); 			//擦除这个扇区
	STMFLASH_Write_NoCheck(WriteAddr, pBuffer, NumToWrite);		  //写已经擦除了的,直接写入扇区剩余区间.

	LL_Flash_Lock(); 							//上锁
	#endif
}

//只是初始化变量
static void FS_FactroyValue(void)
{
	strcpy(Fs.Ok, "OK");

	memset(Fs.IpAdress, '\0', 50);
	memset(Fs.IpPort, '\0', 8);
	memset(Fs.MccMnc, '\0', 7);
	memset(Fs.ApnName, '\0', 32);
	memset(Fs.GprsUserName, '\0', 32);
	memset(Fs.GprsPassWord, '\0', 16);
	memset(Fs.LatitudeLast, '\0', 14);
	memset(Fs.LongitudeLast, '\0', 14);

	strcpy(Fs.IpPort, "7788"); 
	strcpy(Fs.IpAdress, "bky.appxmg.com");
	strcpy(Fs.ApnName, "mtc.gen"); 
	strcpy(Fs.GprsUserName, "mtc"); 
	strcpy(Fs.GprsPassWord, "mtc"); 


	Fs.BKSavedCnt = 0;
	Fs.BkSendCnt = 0;
	Fs.BkSendLen = 0;
	Fs.Interval = 120;
	Fs.HaveSetApn = 0;
	Fs.Sensor = 4;

	Flag.HaveGetMccMnc = 0;
}

void FSUPG_FactroyValue(void)
{

	strcpy(FsUpg.Ok, "OK");
	memset(FsUpg.AppFilePath, '\0', 50);
	memset(FsUpg.AppFileName, '\0', 50);
	memset(FsUpg.AppIpAdress, '\0', 50);
	memset(FsUpg.HttpError, '\0', 32);

	FsUpg.UpgEnJamp = 0x00;
	FsUpg.UpgNeedSendGprs = 0x00;
	FsUpg.AppLenBuf = 0;

//	EXFLASH_Write((u8 *)&FsUpg, FLASH_UPG_ADDR, sizeof(FsUpg));
}

//将参数文件初始化
void FS_FactroyValueFile(void)
{
	FS_FactroyValue();
	Flag.NeedUpdateFs = 1;
	FS_UpdateValue();
}

void FS_InitValue(void)
{

	memset(&Fs, 0, sizeof(Fs));
	STMFLASH_Read(FLASH_SAVE_ADDR, (u32 *)&Fs, (u16)(sizeof(Fs))/4);
//	EXFLASH_ReadArray(FLASH_UPG_ADDR, (char *)&FsUpg, sizeof(FsUpg));
	
	//flash空白 要初始化
//	if ('O' != Fs.Ok[0] || 'K' != Fs.Ok[1])
	if(strcmp(Fs.IpAdress,"bky.appxmg.com") != 0)
	{
		strcpy(Fs.UserID, "999999000004"); 				//这个变量在w686中暂时不使用
		printf("\r\nFormat the eeprom\r\n");
		FS_FactroyValue();
		Flag.NeedUpdateFs = 1;
		return;
	}

	if (FsUpg.UpgEnJamp == 0xaa)
	{
//		Flag.UpgrateAppSuccess = 1;
		FsUpg.UpgNeedSendGprs = 0;
		Flag.NeedUpgradeResultResponse = 1;
	}
	else if (FsUpg.UpgNeedSendGprs)
	{
		FsUpg.UpgNeedSendGprs = 0;
		Flag.NeedUpgradeResultResponse = 1;
	}


	printf("\r\n------Device parameters as follows:------\r\n\r\n");
	#if USR_FOR_JP
	printf("Device suitable for Japan\r\n\r\n");
	#else
	printf("Device suitable for China\r\n\r\n");
	#endif
	printf("Device IMEI: 	 %s\r\n",Fs.DeviceImei);
	printf("Fs.IpAdress: 	 %s\r\n",Fs.IpAdress);
	printf("Fs.IpPort:       %s\r\n",Fs.IpPort);
	printf("Fs.ApnName: 	 %s\r\n",Fs.ApnName);
	printf("Fs.GprsUserName: %s\r\n",Fs.GprsUserName);
	printf("Fs.GprsPassWord: %s\r\n",Fs.GprsPassWord);
	printf("\r\n-----------------------------------------\r\n");
}

void FS_UpdateValue(void)
{
	if (!Flag.NeedUpdateFs)
	{
		return;
	}

	Flag.NeedUpdateFs = 0;
	STMFLASH_WriteFs(FLASH_SAVE_ADDR, (u64 *)&Fs, (u16)(sizeof(Fs))/8);
}

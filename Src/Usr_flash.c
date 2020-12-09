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
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr); //读取2个字节.
		ReadAddr += 4;							  //偏移2个字节.
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
	LL_FLASH_EnableProgram(); //使能写入（允许写入）
	while (LL_FLASH_IsActiveFlag_BSY())
	{
	}
	for (i = 0; i < NumToWrite; i++)
	{
		LL_FLASH_Program(ProgaraType_DATA32, WriteAddr, pBuffer[i]);
		WriteAddr += 8; //地址增加4.
	}
	while (LL_FLASH_IsActiveFlag_BSY())
	{
	}
	LL_FLASH_DisenableProgram(); //禁止写入
#endif
}

#define STM_SECTOR_SIZE 2048 //STM32G070一个扇区2K大小

//此函数专用于写Fs数据(地址正是flash的最后一个扇区)
void STMFLASH_WriteFs(u32 WriteAddr, u64 *pBuffer, u16 NumToWrite)
{
#if 1
	u32 secpos;	 //扇区地址
	u32 offaddr; //去掉0X08000000后的地址
	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
	{
		return; //非法地址
	}
	LL_Flash_Unlock();						//解锁
	offaddr = WriteAddr - STM32_FLASH_BASE; //实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;		//所在扇区地址  0~63 for STM32G070RB

	LL_Flash_PageErase(secpos);								//擦除这个扇区
	STMFLASH_Write_NoCheck(WriteAddr, pBuffer, NumToWrite); //写已经擦除了的,直接写入扇区剩余区间.

	LL_Flash_Lock(); //上锁
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
	memset(Fs.UbloxIp, '\0', 21);
	memset(Fs.UbloxPort, '\0', 7);

	strcpy(Fs.IpPort, "7788");
	strcpy(Fs.IpAdress, "bky.appxmg.com");
	strcpy(Fs.ApnName, "mtc.gen");
	strcpy(Fs.GprsUserName, "mtc");
	strcpy(Fs.GprsPassWord, "mtc");
	strcpy(Fs.UbloxIp,"www.gnss-aide.com");
	strcpy(Fs.UbloxPort,"2621");

	Fs.BKSavedCnt = 0;
	Fs.BkSendCnt = 0;
	Fs.BkSendLen = 0;
	Fs.Interval = 180;
	Fs.HaveSetApn = 0;
	Fs.Sensor = 0x10;
	Fs.ModeSet = 0;
	Fs.ShutDownHour = 0xFF;
	Fs.ShutDownMin = 0xFF;
	Fs.BootHour = 0xFF;
	Fs.BootMin = 0xFF;

	
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
	if (strcmp(Fs.IpAdress, "bky.appxmg.com") != 0)
	{
		strcpy(Fs.UserID, "999999000004"); //这个变量在w686中暂时不使用
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

	printf("Device suitable for China\r\n\r\n");

	printf("Device IMEI: 	 %s\r\n", Fs.DeviceImei);
	printf("Fs.IpAdress: 	 %s\r\n", Fs.IpAdress);
	printf("Fs.IpPort:       %s\r\n", Fs.IpPort);
	printf("Fs.ApnName: 	 %s\r\n", Fs.ApnName);
	printf("Fs.GprsUserName: %s\r\n", Fs.GprsUserName);
	printf("Fs.GprsPassWord: %s\r\n", Fs.GprsPassWord);
	printf("\r\n-----------------------------------------\r\n");
}

void FS_UpdateValue(void)
{
	u16 Fs_len = 0;

	if (!Flag.NeedUpdateFs)
	{
		return;
	}

	Flag.NeedUpdateFs = 0;

	//由于STM32G070的内部flash只能以8byte写入，这个就导致如果Fs大小不是8byte整数倍时会写入不完全，需要+1把剩下的也加进去
	Fs_len = sizeof(Fs);
	if((Fs_len % 8) != 0)
	{
		STMFLASH_WriteFs(FLASH_SAVE_ADDR, (u64 *)&Fs, Fs_len/8 + 1);
	}
	else
	{
		STMFLASH_WriteFs(FLASH_SAVE_ADDR, (u64 *)&Fs, Fs_len/8);
	}
	
}

//擦除用于保存断点扇区，一共8k，预计可以保存100个断点
void BreakPiont_Save_Init(void)
{
	LL_Flash_PageErase(BEAKPIONT_ADDR / STM_SECTOR_SIZE);
	LL_Flash_PageErase(BEAKPIONT_ADDR / STM_SECTOR_SIZE + 1);
	LL_Flash_PageErase(BEAKPIONT_ADDR / STM_SECTOR_SIZE + 2);
	LL_Flash_PageErase(BEAKPIONT_ADDR / STM_SECTOR_SIZE + 3);
}
#if 0
void EXFLSAH_SaveBreakPoint(void)
{
	//已达到最大断点保存数目，并且没有上传完，不再保存新断点
	if (Fs.BKSavedCnt >= 100 && Fs.BKSavedCnt != Fs.BkSendCnt)
		return;

	//如果保存的断点已经上传完或没保存断点,格式化flash
	if (Fs.BKSavedCnt == Fs.BkSendCnt)
	{
		Fs.BKSavedCnt = 0;
		Fs.BkSendCnt = 0;
		Fs.BkSendLen = 0;
		Fs.BkSavedLen = BEAKPIONT_ADDR;
		BreakPiont_Save_Init();
		Flag.NeedUpdateFs = 1;
	}

	STMFLASH_WriteFs(Fs.BkSavedLen, (uint8_t *)GprsSendBuf, strlen(GprsSendBuf));
	Fs.BkSavedLen += strlen(GprsSendBuf);

	Fs.BKSavedCnt += 1;
	Flag.NeedUpdateFs = 1;

	printf("\r\nsave break points,SavedCnt:%d,BkSavedLen:%d,DataSendBuf:\r\n%s\r\n", Fs.BKSavedCnt, Fs.BkSavedLen, GprsSendBuf);
}

//读取断点数据到GprsSendBuf
//每次读取GprsSendBuf长度的数据，然后找到末尾字符，在后面插入\0作为结束
unsigned char EXFLASH_ReadBreakPoint(void)
{
	//	static unsigned char readtimes=0;
	char *p1 = NULL;
	unsigned char i;

	//断点已经发送完，返回0
	if (Fs.BKSavedCnt <= Fs.BkSendCnt)
	{
		if (Fs.BkSendCnt || Fs.BKSavedCnt)
		{
			Fs.BKSavedCnt = 0;
			Fs.BkSendCnt = 0;
			Fs.BkSendLen = 0;
			Fs.BkSavedLen = 0;
			BreakPiont_Save_Init();
			Flag.NeedUpdateFs = 1;
		}
		return 0;
	}

	memset(GprsSendBuf, '\0', DATABUFLEN);
	STMFLASH_Read((unsigned int)Fs.BkSendLen, (uint32_t *)GprsSendBuf, DATABUFLEN/4);

	//MQTT数据以'{'开头，以'}'结尾
	if (GprsSendBuf[0] != 'S')
	{
		printf("read BK data error 1");
		Fs.BKSavedCnt = 0;
		Fs.BkSendCnt = 0;
		Fs.BkSendLen = 0;
		Fs.BkSavedLen = 0;

		EXFLASH_Init();
		Flag.NeedUpdateFs = 1;
		return 0;
	}

	if ((p1 = strstr(GprsSendBuf, "$")) != NULL)
	{
		i = (unsigned char)((p1 - GprsSendBuf) + 7);
		Breakpointleng = i;
		Fs.BkSendLen += i;
		GprsSendBuf[i] = '\0';
	}
	else
	{
		printf("read BK data error 2");
		Fs.BKSavedCnt = Fs.BkSendCnt = 0;
		Fs.BkSendLen = 0;
		Fs.BkSavedLen = 0;

		EXFLASH_Init();
		Flag.NeedUpdateFs = 1;
		return 0;
	}

	Fs.BkSendCnt += 1;
	Flag.NeedUpdateFs = 1;

	printf("\r\nread break points,BkReadCnt:%d,BkReadLen:%d,DataSendBuf:\r\n%s\r\n", Fs.BkSendCnt, Fs.BkSendLen, GprsSendBuf);

	return 1;
}
#endif


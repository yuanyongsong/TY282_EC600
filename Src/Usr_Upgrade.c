#include "usr_main.h"

UPG_INFO UpgInfo;

unsigned char DataRecBuf[1025];
char FileMd5[20];					//给定的MD5校验值
char Md5FileAsc[50] = {0};			//服务器下发的MD5校验值
void Md5StrToHex(const char *str, unsigned char *pDst);

void Asc2Bcd(u8 *bcd, const char *asc, u8 len)
{
	u8 i, ch;

	len >>= 1;
	for (i = 0; i < len; i++)
	{
		ch = (*asc++) << 4;
		ch |= (*asc++) & '\x0F';
		*bcd++ = ch;
	}
}

void STMFLASH_Write_Upg(u32 WriteAddr, u64 *pBuffer, u16 NumToWrite)
{
	LL_Flash_Unlock();						//解锁

	STMFLASH_Write_NoCheck(WriteAddr, pBuffer, NumToWrite); //写已经擦除了的,直接写入扇区剩余区间.

	LL_Flash_Lock(); //上锁
}

void WIRELESS_UpgradeReceive(char *pSrc)
{
	char *p0 = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
	u16	PacketLen = 0;
//	u8	leftlen = 0;
	u16	j = 0;
	u32 offaddr = 0;
	u8 	secpos = 0;


	p0 = strstr(pSrc, "CONNECT ");
	p1 = strstr(p0, "\r\n");
	p0 += 8;
	PacketLen = Ascii2BCD_u16(p0, p1-p0);
	p1 += 2;
	if (UpgInfo.UpgPacketNums == 0)
	{
		UpgInfo.UpgFlashAddr = FOAT_SAVE_ADDR;

		//一个扇区56k，这里格式化28个扇区用于存储远程升级数据
		LL_Flash_Unlock();									//解锁
		offaddr = UpgInfo.UpgFlashAddr - STM32_FLASH_BASE;  //实际偏移地址.
		secpos = offaddr / 2048;							//所在扇区地址  0~63 for STM32G070RB
		for(j = 0;j < 28;j ++)
		{
			LL_Flash_PageErase(secpos + j);					//擦除这个扇区
		}		
		LL_Flash_Lock(); 									//上锁		
	}

		

	if(PacketLen != 1024)				//最后一包数据
	{
		AtType = AT_NULL;
		UpgInfo.AppReadComplete = 1;

		//写入小于1024的数据，先将数据用0xFF补充到1k大小
		p2 = p1 + PacketLen;
		for(j = 0;j <1024 - PacketLen;j ++)	 	*(p2 + j) = 0xFF;

		STMFLASH_Write_Upg(UpgInfo.UpgFlashAddr,(u64 *)p1,1024/8);
#if 0
		if(PacketLen % 8 == 0)
		{
			STMFLASH_Write_Upg(UpgInfo.UpgFlashAddr,(u64 *)p1,PacketLen/8);
		}
		else		//在不足8字节的情况下，用0xFF补足8字节
		{
			leftlen = 8 - PacketLen % 8;			//计算需要补几个字节
			p2 = p1 + PacketLen;
			for(j = 0;j < leftlen;j ++)	 	*(p2 + j) = 0xFF;

			STMFLASH_Write_Upg(UpgInfo.UpgFlashAddr,(u64 *)p1,PacketLen/8 + 1);
		}
#endif		
		memset(DataRecBuf,0,sizeof(DataRecBuf));
		STMFLASH_Read(UpgInfo.UpgFlashAddr,(u32 *)DataRecBuf,PacketLen/4);

		for (j = 0; j <= 10; j++) //保存失败，重试10次
		{
			if (memcmp(p1, DataRecBuf, PacketLen)) //保存数据错误
			{
				printf("\r\n保存数据错误! retry...%d", j);
				STMFLASH_Write_Upg(UpgInfo.UpgFlashAddr, (u64 *)p1, PacketLen/8);
				memset(DataRecBuf, 0, 1024);
				LL_mDelay(20);
				STMFLASH_Read(UpgInfo.UpgFlashAddr,(u32 *)DataRecBuf,PacketLen/4);
			}
			else
				break;
		}
		if (j > 10)
		{
			memset(Fs.FsUpg.HttpError, 0, sizeof(Fs.FsUpg.HttpError));
			strcpy(Fs.FsUpg.HttpError, "Save ExFlash Error! Please try again later.\r\n");
			Fs.FsUpg.UpgNeedSendGprs = 1;
			printf("\r\n保存数据错误!");
			Flag.IsUpgrate = 0;
			UpgInfo.UpgrateFail = 1;
			return;
		}	
	#if 1
			Flag.NeedUpdateFs = 1;	
			FS_UpdateValue();	

			printf("\r\nComplete updata file download,Reset device!\r\n");
			Flag.NeedModuleOff=1;
			Usr_ModuleTurnOff();
			NVIC_SystemReset();   
	#else
		UpgInfo.UpgFlashAddr+=PacketLen;
		MD5Update(&Upgmd5,(unsigned char *)p1,PacketLen);
		MD5Final(&Upgmd5,UpgInfo.Md5decrypt);					//获取最终MD5结果
		for(j=0;j<16;j++)  printf("%02x",UpgInfo.Md5decrypt[j]);
		Md5StrToHex(Md5FileAsc,(u8 *)FileMd5);
		
		if(memcmp(FileMd5,UpgInfo.Md5decrypt,16)==0)				//比较MD5计算值和给定值，相等则写入标志并重启
		{
			Fs.FsUpg.UpgEnJamp=0x55;
			Fs.FsUpg.AppLenBuf=UpgInfo.UpgFlashAddr;
			memset(Fs.FsUpg.HttpError,0,sizeof(Fs.FsUpg.HttpError));
			strcpy(Fs.FsUpg.HttpError,"MD5 verify OK!\r\n");		
			Flag.NeedUpdateFs = 1;	
			FS_UpdateValue();	

			printf("\r\nComplete updata file download,Reset device!\r\n");
			Flag.NeedModuleOff=1;
			Usr_ModuleTurnOff();
			NVIC_SystemReset();    
		}
		else													//失败
		{
			printf("\r\nverify Error! Please try again\r\n");
			memset(Fs.FsUpg.HttpError,0,sizeof(Fs.FsUpg.HttpError));
			strcpy(Fs.FsUpg.HttpError,"verify Error! Please try again\r\n");
			UpgInfo.UpgrateFail = 1;
			return;
		}
	#endif
	}
	else
	{
		STMFLASH_Write_Upg(UpgInfo.UpgFlashAddr,(u64 *)p1,1024/8);
		memset(DataRecBuf,0,sizeof(DataRecBuf));
		STMFLASH_Read(UpgInfo.UpgFlashAddr,(u32 *)DataRecBuf,1024/4);

		for (j = 0; j <= 10; j++) 						//保存失败，重试10次
		{
			if (memcmp(p1, DataRecBuf, 1024)) 		//保存数据错误
			{
				printf("\r\n保存数据错误! retry...%d", j);
				EXFLASH_WriteBuffer((u8 *)p1, UpgInfo.UpgFlashAddr, 1024);
				memset(DataRecBuf, 0, 1024);
				LL_mDelay(20);
				STMFLASH_Read(UpgInfo.UpgFlashAddr,(u32 *)DataRecBuf,1024/4);
			}
			else
				break;
		}
		if (j > 10)
		{
			memset(Fs.FsUpg.HttpError, 0, sizeof(Fs.FsUpg.HttpError));
			strcpy(Fs.FsUpg.HttpError, "Save ExFlash Error! Please try again later.\r\n");
			Fs.FsUpg.UpgNeedSendGprs = 1;
			printf("\r\n保存数据错误!");
			Flag.IsUpgrate = 0;
			UpgInfo.UpgrateFail = 1;
			return;
		}

		printf("receive updata packet number:%d\r\n", UpgInfo.UpgPacketNums);
		UpgInfo.UpgPacketNums++;
		UpgInfo.UpgFlashAddr += 1024;

		MD5Update(&Upgmd5, (unsigned char *)p1, 1024);
	}

}

void Md5StrToHex(const char *str, unsigned char *pDst)
{
	unsigned char i = 0;
	unsigned char sum = 0;
	char tmp;

	while (str[i] != '\0')
	{
		sum = 0;
		tmp = str[i++];
		if (tmp >= '0' && tmp <= '9')
		{
			sum |= tmp - '0';
		}
		else if (tmp <= 'f' && tmp >= 'a')
		{
			sum |= tmp - 'a' + 10;
		}
		else if (tmp <= 'F' && tmp >= 'A')
		{
			sum |= tmp - 'A' + 10;
		}
		sum = sum << 4;

		if (str[i] == '\0')
			break;

		tmp = str[i++];
		if (tmp >= '0' && tmp <= '9')
		{
			sum |= tmp - '0';
		}
		else if (tmp <= 'f' && tmp >= 'a')
		{
			sum |= tmp - 'a' + 10;
		}
		else if (tmp <= 'F' && tmp >= 'A')
		{
			sum |= tmp - 'A' + 10;
		}

		*pDst++ = sum;

		*pDst = 0;
	}
}




void FSUPG_InitValue(void)
{
	strcpy(Fs.FsUpg.Ok,"OK");
	memset(Fs.FsUpg.AppFilePath,'\0',50);
	memset(Fs.FsUpg.AppFileName,'\0',50);
	memset(Fs.FsUpg.AppIpAdress,'\0',50);
	memset(Fs.FsUpg.HttpError,'\0',32);

	Fs.FsUpg.UpgNeedSendGprs=0x00;
	Fs.FsUpg.UpgEnJamp=0x00;
	Fs.FsUpg.AppLenBuf=0x00;

    Flag.NeedUpdateFs = 1;
}


void UpgInfo_InitValue(void)
{
	UpgInfo.AppReadComplete = 0;
	UpgInfo.UpgrateFail = 0;
	UpgInfo.NeedCheckUploadState = 0;
	UpgInfo.NeedUpdata = 0;
	UpgInfo.AppDownloadOk = 0;
	UpgInfo.UpgPacketNums = 0;
	UpgInfo.UpgFlashAddr = 0;
	UpgInfo.Verify_ok = 0;
	memset(UpgInfo.Md5decrypt,'\0',17);
}


#include "usr_main.h"

FS_UPG FsUpg;
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

void WIRELESS_UpgradeReceive(char *pSrc)
{
	char *p0 = NULL;
	char *p1 = NULL;
	u16	PacketLen = 0;
	u8	j = 0;


	p0 = strstr(pSrc, "+CFSRFILE:");
	p1 = strstr(p0, "\r\n");
	p0 += 11;
	PacketLen = Ascii2BCD_u16(p0, p1-p0);
	p1 += 2;
	if (UpgInfo.UpgPacketNums == 0)
	{
		UpgInfo.UpgExFlashAddr = EXFLASH_APP1_ADDR;

		//一个扇区64k，这里格式化三个扇区用于存储远程升级数据
		EXFLASH_EraseSector(EXFLASH_APP1_ADDR);				
		EXFLASH_EraseSector(EXFLASH_APP1_ADDR + (64*1024));			
		EXFLASH_EraseSector(EXFLASH_APP1_ADDR + (128*1024));			
	}

		
	

	if(PacketLen != 1024)				//最后一包数据
	{
		AtType = AT_NULL;
		UpgInfo.AppReadComplete = 1;
		//EXFLASH_WriteBuffer((u8 *)p1,UpgInfo.UpgExFlashAddr,PacketLen);
		memset(DataRecBuf,0,sizeof(DataRecBuf));
		//EXFLASH_ReadBuffer(DataRecBuf,UpgInfo.UpgExFlashAddr,PacketLen);

		for (j = 0; j <= 10; j++) //保存失败，重试40次
		{
			if (memcmp(p1, DataRecBuf, PacketLen)) //保存数据错误
			{
				printf("\r\n保存数据错误! retry...%d", j);
				//EXFLASH_WriteBuffer((u8 *)p1, UpgInfo.UpgExFlashAddr, PacketLen);
				memset(DataRecBuf, 0, 1024);
				LL_mDelay(20);
				//EXFLASH_ReadBuffer(DataRecBuf,UpgInfo.UpgExFlashAddr,PacketLen);
			}
			else
				break;
		}
		if (j > 10)
		{
			memset(FsUpg.HttpError, 0, sizeof(FsUpg.HttpError));
			strcpy(FsUpg.HttpError, "Save ExFlash Error! Please try again later.\r\n");
			FsUpg.UpgNeedSendGprs = 1;
			printf("\r\n保存数据错误!");
			Flag.IsUpgrate = 0;
			UpgInfo.UpgrateFail = 1;
			return;
		}	

		UpgInfo.UpgExFlashAddr+=PacketLen;
		MD5Update(&Upgmd5,(unsigned char *)p1,PacketLen);
		MD5Final(&Upgmd5,UpgInfo.Md5decrypt);					//获取最终MD5结果
		for(j=0;j<16;j++)  printf("%02x",UpgInfo.Md5decrypt[j]);
		Md5StrToHex(Md5FileAsc,(u8 *)FileMd5);
		
		if(memcmp(FileMd5,UpgInfo.Md5decrypt,16)==0)				//比较MD5计算值和给定值，相等则写入标志并重启
//		if(1)														//暂时不比较MD5校验
		{
			FsUpg.UpgEnJamp=0x55;
			FsUpg.AppLenBuf=UpgInfo.UpgExFlashAddr;
			memset(FsUpg.HttpError,0,sizeof(FsUpg.HttpError));
			strcpy(FsUpg.HttpError,"MD5 verify OK!\r\n");		

			EXFLASH_EraseSector(FLASH_UPG_ADDR);
			//EXFLASH_WriteBuffer((u8 *)&FsUpg,FLASH_UPG_ADDR,sizeof(FsUpg));					
		   
			printf("\r\nComplete updata file download,reset device!\r\n");
			Flag.NeedModuleOff=1;
			Usr_ModuleTurnOff();

			NVIC_SystemReset();    
		}
		else													//失败
		{
			printf("\r\nverify Error! Please try again\r\n");
			memset(FsUpg.HttpError,0,sizeof(FsUpg.HttpError));
			strcpy(FsUpg.HttpError,"verify Error! Please try again\r\n");
			UpgInfo.UpgrateFail = 1;
			return;
		}
	}
	else
	{
		//EXFLASH_WriteBuffer((u8 *)p1, UpgInfo.UpgExFlashAddr, 1024);
		memset(DataRecBuf,0,sizeof(DataRecBuf));
		//EXFLASH_ReadBuffer(DataRecBuf,UpgInfo.UpgExFlashAddr,1024);

		for (j = 0; j <= 10; j++) 						//保存失败，重试10次
		{
			if (memcmp(p1, DataRecBuf, 1024)) 		//保存数据错误
			{
				printf("\r\n保存数据错误! retry...%d", j);
				//EXFLASH_WriteBuffer((u8 *)p1, UpgInfo.UpgExFlashAddr, 1024);
				memset(DataRecBuf, 0, 1024);
				LL_mDelay(20);
				//EXFLASH_ReadBuffer(DataRecBuf,UpgInfo.UpgExFlashAddr,1024);
			}
			else
				break;
		}
		if (j > 10)
		{
			memset(FsUpg.HttpError, 0, sizeof(FsUpg.HttpError));
			strcpy(FsUpg.HttpError, "Save ExFlash Error! Please try again later.\r\n");
			FsUpg.UpgNeedSendGprs = 1;
			printf("\r\n保存数据错误!");
			Flag.IsUpgrate = 0;
			UpgInfo.UpgrateFail = 1;
			return;
		}

		printf("receive updata packet number:%d\r\n", UpgInfo.UpgPacketNums);
		UpgInfo.UpgPacketNums++;
		UpgInfo.UpgExFlashAddr += 1024;

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

	strcpy(FsUpg.Ok,"OK");
	memset(FsUpg.AppFilePath,'\0',50);
	memset(FsUpg.AppFileName,'\0',50);
	memset(FsUpg.AppIpAdress,'\0',50);
	memset(FsUpg.HttpError,'\0',32);

	FsUpg.UpgNeedSendGprs=0x00;
	FsUpg.UpgEnJamp=0x00;
	FsUpg.AppLenBuf=0x00;

    //EXFLASH_WriteBuffer((u8 *)&FsUpg,FLASH_UPG_ADDR,sizeof(FsUpg));
}


void UpgInfo_InitValue(void)
{
	UpgInfo.AppReadComplete = 0;
	UpgInfo.UpgrateFail = 0;
	UpgInfo.NeedCheckUploadState = 0;
	UpgInfo.NeedUpdata = 0;
	UpgInfo.AppDownloadOk = 0;
	UpgInfo.UpgPacketNums = 0;
	UpgInfo.UpgExFlashAddr = 0;
	UpgInfo.Verify_ok = 0;
	memset(UpgInfo.Md5decrypt,'\0',17);
}


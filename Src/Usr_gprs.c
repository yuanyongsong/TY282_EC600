#include "usr_main.h"

GPRS_TYPE GprsType;
GPRS_SEND GprsSend;

unsigned short ConnectDelayCnt; //由于网络问题，重连服务器中间等待时间
unsigned char Upd_command_len;	//远程升级时，发送数据包长度
unsigned short At_Timeout_Cnt;	//等待AT超时的时间，用于等待非标准超时时间时使用
unsigned short PacketSerialNum;	//发送数据包流水号
unsigned int ActiveTimer;
unsigned char WifiCnt;			//搜索到的wifi数量
unsigned char FindDeviceCnt;	//收到寻找设备指令，LED闪灯计时
u32  speed_gprs;
char UserIDBuf[16];
char GprsSendBuf[DATABUFLEN];
char UpgradeSendBuf[UPDRADELEN];
char CCID[21];
char AtSendbuf[SCIBUFLEN]; 		/*定义一个数组存储发送数据*/
char GprsContent[GPRSCONTLEN];
char IMEI[16];
char Lac[5];					//基站LAC
char Cid[9];					//基站CID
char IMSI[20];					//卡的IMSI号
char SIMNUMB[20];				//SIM卡卡号
char Wifi_Content[200];			//搜索到的wifi的MAC地址
char GprsCommand[20];			//Gprs下发的命令
u8   WifiCnt;					//搜索到的WiFi个数
u8	 WifiScanDelay;				//wifi搜索时间间隔

int Usr_Atoi(char *pSrc)
{
	signed int i = 0;
	signed char f = 1;

	if ('-' == *pSrc)
	{
		f = -1;
		pSrc++;
	}
	while ((*pSrc >= '0') && (*pSrc <= '9'))
	{
		i *= 10;
		i += *pSrc - '0';
		pSrc++;
	}
	i *= f;

	return i;
}

u16 HEX2BCD(u8 Val_HEX)
{
	u16 Val_BCD;
	u8 temp;
	temp = Val_HEX % 100;
	Val_BCD = ((u16)Val_HEX) / 100 << 8;
	Val_BCD = Val_BCD | temp / 10 << 4;
	Val_BCD = Val_BCD | temp % 10;
	return Val_BCD;
}

u32 HEX2BCD_FOR_U32(u16 Val_HEX)
{
	u32 Val_BCD;
	u32 temp;
	temp = Val_HEX % 10000;
	Val_BCD = (u32)(Val_HEX) / 10000 << 16;
	Val_BCD = Val_BCD | temp / 1000 << 12;
	temp = Val_HEX % 1000;
	Val_BCD = Val_BCD | temp / 100 << 8;
	temp = Val_HEX % 100;
	Val_BCD = Val_BCD | temp / 10 << 4;
	Val_BCD = Val_BCD | temp % 10;
	return Val_BCD;
}


void Ascii2BCD(char *pSrc, unsigned char *pDst)
{
	char i, f, k = 0, temp;

	for (i = 0; i < 2; i++)
	{
		temp = 0;
		f = 1;
		if (pSrc[i] >= '0' && pSrc[i] <= '9')
			temp = pSrc[i] - 0x30;
		//		else if(pSrc[i]>='a' && pSrc[i]<='f') 	temp=pSrc[i]-0x57;
		//		else if(pSrc[i]>='A' && pSrc[i]<='F') 	temp=pSrc[i]-0x37;
		else
			f = 0;

		if (f)
		{
			if (0 == k % 2)
			{
				*pDst = (unsigned char)(temp * 10);
			}
			else
			{
				*pDst += (unsigned char)temp;
			}
			k++;
		}
	}
}

u16 Ascii2BCD_u16(char *pSrc, unsigned char len)
{
	unsigned char i;
	u16 result = 0, temp = 0;

	for (i = 0; i < len; i++)
	{
		result *= 10;
		if (pSrc[i] >= '0' && pSrc[i] <= '9')
			temp = pSrc[i] - 0x30;
		else
			return 0;

		result += temp;
	}

	return result;
}

//将字符串"41 01 83 07 FF 00"转换到hex值
u32 Ascii2Hex(char *pSrc, unsigned char srcLen)
{
	char i;
	u32 Data_leng = 0, temp = 0;
	for (i = 0; i < srcLen; i++)
	{
		Data_leng <<= 4;
		if (pSrc[i] >= '0' && pSrc[i] <= '9')
			temp = pSrc[i] - 0x30;
		else if (pSrc[i] >= 'a' && pSrc[i] <= 'f')
			temp = pSrc[i] - 0x57;
		else if (pSrc[i] >= 'A' && pSrc[i] <= 'F')
			temp = pSrc[i] - 0x37;
		else
			return 0xFF;
		Data_leng += temp;
	}
	return Data_leng;
}
//将指定长度的字符串转换为HEX
void StrAscii2Hex(char *pSrc, char *pDst, u16 Srclen)
{
	char f, temp;
	u16 Dstlen = 0, i;
	if (Srclen % 2 != 0)
		return;

	for (i = 0; i < Srclen; i++)
	{
		temp = 0;
		f = 1;
		if (pSrc[i] >= '0' && pSrc[i] <= '9')
			temp = pSrc[i] - 0x30;
		else if (pSrc[i] >= 'a' && pSrc[i] <= 'f')
			temp = pSrc[i] - 0x57;
		else if (pSrc[i] >= 'A' && pSrc[i] <= 'F')
			temp = pSrc[i] - 0x37;
		else
			f = 0;

		if (f)
		{
			if (0 == i % 2)
			{
				pDst[Dstlen] = (unsigned char)(temp << 4);
			}
			else
			{
				pDst[Dstlen] += (unsigned char)temp;
				Dstlen++;
			}
		}
	}
}

void U32ToBCDStrAscii(u32 Src, char *pDst)
{
	u8 data_temp = 0;
	u32 BCD_temp = 0, Mask = 0;
	u8 Frist_not_0 = 0, i = 0, j = 7;
	BCD_temp = HEX2BCD_FOR_U32((u16)Src);
	i = 8;
	Mask = 0xF0000000;
	while (i--)
	{
		data_temp = (u8)((BCD_temp & Mask) >> i * 4);
		if ((data_temp != 0) || (Frist_not_0 == 1))
		{
			pDst[7 - j] = data_temp + 0x30;
			Frist_not_0 = 1;
			j--;
		}
		Mask = Mask >> 4;
	}
}
void Hex2StrAscii(char *pSrc, char *pDst, u16 Srclen)
{
	char f, temp, temp0;
	u16 Dstlen = 0, i, j = 0;

	Dstlen = Srclen * 2;
	f = 1;
	for (i = 0; i < Dstlen; i++)
	{

		if (0 == i % 2)
		{
			temp0 = pSrc[j] & 0xF0;
			temp0 = temp0 >> 4;
			if (temp0 <= 0x9)
				temp = temp0 + 0x30;
			else if ((temp0 >= 0x0A) && (temp0 <= 0x0F))
				temp = temp0 + 0x37;
			else
				f = 0;
		}
		else
		{
			temp0 = pSrc[j] & 0x0F;
			if (temp0 <= 0x9)
				temp = temp0 + 0x30;
			else if ((temp0 >= 0x0A) && (temp0 <= 0x0F))
				temp = temp0 + 0x37;
			else
				f = 0;

			j++;
		}

		if (f)
			pDst[i] = temp;
	}
}

//将i个字节的pSrcAcsii码转换成BCD码存放在pDst
void Acsii2Bcd(char *pSrc, char *pDst, unsigned char i)
{
	unsigned char j = 0;
	unsigned char l = 0;

	if (0 == i || NULL == pSrc || NULL == pDst)
	{
		return;
	}

	ChangeToUpper(pSrc, i);

	while (i--)
	{
		if (pSrc[j] >= '0' && pSrc[j] <= '9')
		{
			l = pSrc[j] - '0';
		}
		else if (pSrc[j] >= 'A' && pSrc[j] <= 'F')
		{
			l = pSrc[j] - 'A' + 10;
		}
		else
		{
			return;
		}

		if (0 == (j % 2))
		{
			*pDst = l << 4;
		}
		else
		{
			*pDst++ |= l;
		}

		j++;
	}
}

u16 WIRELESS_GprsSendPacket(GPRS_TYPE switch_tmp)
{
	u16 data_len = 0;				//打包完成后整包数据长度
	u16 content_len = 0;			//数据包中内容长度
	u16 Degrees = 0;
	u8  Elevation = 0;

	char latTmp[14] = {0};
	char lonTmp[14] = {0};
	u8 bat_percente = 0;
	u8 csq_temp = 0;


	unsigned char gpsValid = 'A';

	LBS_Num = 1;


	memset(GprsSendBuf, '\0', DATABUFLEN);
	memset(GprsContent, '\0', GPRSCONTLEN);

	//电池电量低于3.5v认为是没电，高于4.1v认为是满电，中间电量按照电压线性计算
	if (BatVoltage_Adc < 3500)
		bat_percente = 0;
	else
		BatVoltage_Adc -= 3500;

	if (BatVoltage_Adc > 600)
		bat_percente = 100;
	else
		bat_percente = BatVoltage_Adc / 6;

	if (Flag.HaveGPS)
	{
		gpsValid = 'A';
	}
	else
	{
		gpsValid = 'V';
	}

	if((Rssi > 30)&&(Rssi != 99))
	{
		csq_temp = 100;
	}
	else if((Rssi <= 30) && (Rssi >= 1))
	{
		csq_temp = (u8)((float)Rssi * 3.3);
	}
	else
	{
		csq_temp = 0;
	}

	PacketSerialNum ++;

	if (Flag.LatFlag)
	{
		strcpy(latTmp, "-");
		strcat(latTmp, Fs.LatitudeLast);
	}
	else
	{
		strcpy(latTmp, Fs.LatitudeLast);
	}
	MinuteToDegree(latTmp);

	if (Flag.LonFlag)
	{
		strcpy(lonTmp, "-");
		strcat(lonTmp, Fs.LongitudeLast);
	}
	else
	{
		strcpy(lonTmp, Fs.LongitudeLast);
	}
	MinuteToDegree(lonTmp);

	latTmp[9] = 0;
	lonTmp[10] = 0;

	Degrees=(u16)atoi(DegreesTmp);
	Elevation = (u16)CurElevation;

	UTCToBeijing(Rtc);

	switch (switch_tmp)
	{
	case AGPSDATA:
		sprintf(GprsSendBuf, "user=qyjem@qq.com;pwd=qgjqgj88;cmd=full;lat=%s;lon=%s;pacc=1000;",latTmp,lonTmp);
		break;

	case DATA:
		if(Flag.HaveGPS)
		{
			sprintf(GprsContent, "LOCA:G;CELL:1,%s,%s,%s,%s,%d;GDATA:%c,%d,%02d%02d%02d%02d%02d%02d,%s,%s,%d,%d,%d;ALERT:%04X;STATUS:%d,%d",
					Mcc,Mnc,Lac,Cid,csq_temp,gpsValid,CurSateCnt,Rtc_BJ.year,Rtc_BJ.mon,Rtc_BJ.day, 
					Rtc_BJ.hour,Rtc_BJ.min,Rtc_BJ.sec,latTmp,lonTmp,speedByte,Degrees,Elevation,VehSta,bat_percente,csq_temp);
		}
		else if(Flag.GetScanWifi)
		{
			Flag.GetScanWifi = 0;
			
			sprintf(GprsContent, "LOCA:W;WIFI:%d,%s;ALERT:%04X;STATUS:%d,%d;WAY:0",
					WifiCnt,Wifi_Content,VehSta,bat_percente,csq_temp);			
		}

		break;

	case UPGRESULT:

		break;

	case RESPDATA:				//应答平台下发的指令
		sprintf(GprsContent, "RET,%s,1",GprsCommand);	
		break;


	case DEVINFODATA:
		sprintf(GprsContent, "INFO,ITEM:w282;VER:%s;PLMN:%s%s;IMEI:%s;IMSI:%s;ICCID:%s;OWNER:%s",
		Edition,CCID,Mcc,Mnc,IMEI,IMSI,CCID);
		break;

	case HAND:
		sprintf(GprsContent, "SYNC:%04X;STATUS:%d,%d",GprsSend.handsendcnt,bat_percente,csq_temp);
		break;

	default:
		break;
	}
	if(switch_tmp != AGPSDATA)
	{
		content_len = strlen(GprsContent);
		sprintf(GprsSendBuf, "S168#%s#%04X#%04X#%s$",Fs.UserID,PacketSerialNum,content_len,GprsContent);
	}

	data_len = strlen(GprsSendBuf);
	return data_len;
}

void GPRS_Send_Handle(void)
{

	if (Flag.NeedLogIn)
	{
		Flag.NeedLogIn = 0;
		AtType = AT_QISEND;
		GprsType = LOGIN;
		printf("Send login data to mqtt service\r\n");
		return;
	}

	//各类报警发送 
	if (HaveAlarmGprsType)
	{

		if(Flag.HaveGPS || Flag.GetScanWifi)
		{
			AtType = AT_QISEND;
			GprsType = DATA;

			if (HaveAlarmGprsType & HAVE_SHOCK_ALARM)
			{
				VehSta |= HAVE_SHOCK_ALARM;
			}
			if (HaveAlarmGprsType & HAVE_LOWPWR_ALARM)
			{
				VehSta |= HAVE_LOWPWR_ALARM;
			}

			HaveAlarmGprsType = 0;
		}
		else if((WifiScanDelay == 0)&&(!Flag.GetScanWifi))
		{
			Flag.NeedScanWifi = 1;
			WifiScanDelay = 10;
			return;
		}
	}
	//已经发送，清除报警位
	else
	{
		VehSta = 0;
	}

	if (((GprsSend.posCnt > 0) || (Flag.NoSleepMode)) && (GprsSend.posFlag))
	{
		//GPS定位到了立即使用GPS上传位置信息，如果一直没有定位到，在休眠前10秒上传wifi信息
		if(Flag.HaveGPS)
		{
			AtType = AT_QISEND;
			GprsType = DATA;	
			GprsSend.posCnt = 0;
			GprsSend.posFlag = 0;	
			return;
		}
		else if(!Flag.HaveGPS && Flag.GetScanWifi)
		{
			AtType = AT_QISEND;
			GprsType = DATA;	
			GprsSend.posCnt = 0;
			GprsSend.posFlag = 0;	
			return;
		}
		else if(!Flag.HaveGPS && !Flag.GetScanWifi && (WifiScanDelay == 0))
		{
			if(Flag.NoSleepMode)				//如果是不休眠模式，到上传时间没有GPS的情况下，上传wifi
			{
				Flag.NeedScanWifi = 1;
				WifiScanDelay = 10;
			}
			else if(ActiveTimer <= 10)			//在休眠模式下，在没有GPS的情况下，在进入休眠前20秒查询wifi
			{
				Flag.NeedScanWifi = 1;
				WifiScanDelay = 10;
			}
			else if(Flag.NeedLocateByGprs)
			{
				Flag.NeedLocateByGprs = 0;
				Flag.NeedScanWifi = 1;
				WifiScanDelay = 10;
			}
		}
	}


	if(GprsSend.handFlag)
	{
		AtType = AT_QISEND;
		GprsType = HAND;
		GprsSend.handFlag = 0;
		GprsSend.handsendcnt ++;
		return;		
	}
/*
	if (EXFLASH_ReadBreakPoint())
	{
		AtType = AT_QISEND;
		GprsType = BKDATA;
		Flag.IsSendingBk = 1;
		return;
	}
*/
	if(Flag.NeedSendUpgResult)
	{
		AtType = AT_QISEND;
		GprsType = UPGRESULT;
		Flag.NeedSendUpgResult = 0;	
		return;	
	}

	if(Flag.NeedRspGprs)
	{
		Flag.NeedRspGprs = 0;
		AtType = AT_QISEND;
		GprsType = RESPDATA;
		return;
	}

	if(Flag.NeedRspDevInfo)
	{
		Flag.NeedRspDevInfo = 0;
		AtType = AT_QISEND;
		GprsType = DEVINFODATA;
		return;
	}
}

#
//定位包保存
void GPRS_SaveBreakPoint(void)
{
#if 0
	if ((Flag.GprsConnectOk == 0 || Flag.PsSignalOk == 0) && Flag.ReadySaveBreak && GprsSend.posCnt > 0)
	{
		Flag.ReadySaveBreak = 0;

		if (GprsSend.posCnt > 0)
		{
			GprsSend.posCnt--;
		}
		GprsType = DATA;
		WIRELESS_GprsSendPacket(GprsType);
//		EXFLSAH_SaveBreakPoint();
	}
#endif
}

//例:将"41424344" 转换为"ABCD"
void HexStrToStr(const char *pSrc, char *pDst)
{
	u8 tmp;

	while ('\0' != *pSrc)
	{
		tmp = 0;

		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			tmp = *pSrc - '0';
		}
		else if ((*pSrc >= 'a') && (*pSrc <= 'z'))
		{
			tmp = *pSrc - 'a' + 10;
		}
		else if ((*pSrc >= 'A') && (*pSrc <= 'Z'))
		{
			tmp = *pSrc - 'A' + 10;
		}
		tmp <<= 4;
		pSrc++;

		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			tmp |= *pSrc - '0';
		}
		else if ((*pSrc >= 'a') && (*pSrc <= 'z'))
		{
			tmp |= *pSrc - 'a' + 10;
		}
		else if ((*pSrc >= 'A') && (*pSrc <= 'Z'))
		{
			tmp |= *pSrc - 'A' + 10;
		}

		*pDst++ = tmp;
		pSrc++;
	}
}

void Itoa(unsigned char src, char dst[])
{
	unsigned char i = 0;
	int tmp = src;

	while ((tmp /= 10) > 0)
	{
		i++;
	}
	i++;
	dst[i--] = '\0';

	do
	{
		dst[i] = src % 10 + '0';
		src /= 10;
	} while (i--);
}

void WIRELESS_GprsReceive(char *pSrc)
{
	char *p0 = NULL;
	char *p1 = NULL;
	char gprs_content[100] = {0};

	p0 = strstr(pSrc,"S168");
	p1 = strstr(p0,"$");
	if(p0 != NULL)
	{
		if(p1 != NULL)
		{
			strncpy(gprs_content, p0, p1 - p0 + 1);
			printf("\r\nRecvice the data form service: %s\r\n",gprs_content);

		}
		else
		{
			printf("\r\nRecvice data format incorrect!\r\n");
			return;
		}
		//修改传感器灵敏度
		if((p0 = strstr(pSrc,"GSENSOR,")) != NULL)
		{
			p1 = strstr(p0,",");
			if(p1 - p0 <= sizeof(GprsCommand))
			{
				memset(GprsCommand,0,sizeof(GprsCommand));
				strncpy(GprsCommand, p0, p1 - p0);
			}
			else
			{
				return;
			}

			p0 += 8;
			if(*p0 == '0')
			{
				G_Sensor_Pwr(0);
				Fs.ModeSet |= NO_SENSOR;
			} 	
			else
			{
				if(*p0 == '1') 			Fs.Sensor = SENSOR_HIGH;
				else if(*p0 == '2') 	Fs.Sensor = SENSOR_NORMAL;
				else if(*p0 == '3') 	Fs.Sensor = SENSOR_LOW;

				Flag.GsensorNeedInit = 1;
			}	
			Flag.NeedUpdateFs = 1;		
		}
		//修改上传时间间隔
		else if((p0 = strstr(pSrc,"UP,")) != NULL)
		{
			p1 = strstr(p0,",");
			if(p1 - p0 <= sizeof(GprsCommand))
			{
				memset(GprsCommand,0,sizeof(GprsCommand));
				strncpy(GprsCommand, p0, p1 - p0);
			}
			else
			{
				return;
			}

			p0 += 3;
			Fs.Interval = (u32)atoi(p0);
			if(Fs.Interval < 5)
			{
				Fs.Interval = 5;
			}
			if(Fs.Interval <=120)
			{
				Flag.NoSleepMode = 1;			
			}
			
			Flag.NeedUpdateFs = 1;		
			Flag.NeedRspGprs = 1;		
		}
		//查询设备
		else if((p0 = strstr(pSrc,"PARAM")) != NULL)
		{
			Flag.NeedRspDevInfo = 1;			
		}
		//重启
		else if((p0 = strstr(pSrc,"REBOOT")) != NULL)
		{
			ResetLeftCnt = 3;			
		}
		//寻找设备
		else if((p0 = strstr(pSrc,"FINDME")) != NULL)
		{
			p1 = strstr(p0,"$");
			if(p1 - p0 <= sizeof(GprsCommand))
			{
				memset(GprsCommand,0,sizeof(GprsCommand));
				strncpy(GprsCommand, p0, p1 - p0);
			}
			else
			{
				return;
			}		
			FindDeviceCnt = 60;
			Flag.NeedRspGprs = 1;
		}
		//关机
		else if((p0 = strstr(pSrc,"POWERDN")) != NULL)
		{
			Flag.NeedShutDown = 1;
		}
		//立即定位
		else if((p0 = strstr(pSrc,"JUST")) != NULL)
		{
			GprsSend.posCnt = 1;
			GprsSend.posFlag = 1;
			Flag.NeedLocateByGprs = 1;
		}
		//自动开关机
		else if((p0 = strstr(pSrc,"APOF,")) != NULL)
		{
			p1 = strstr(p0,",");
			if(p1 - p0 <= sizeof(GprsCommand))
			{
				memset(GprsCommand,0,sizeof(GprsCommand));
				strncpy(GprsCommand, p0, p1 - p0);
			}
			else
			{
				return;
			}

			p0 += 5;

			if(*p0 == '0')			Fs.ModeSet &= ~AUTO_SHUTDOWN;
			else if(*p0 == '1')		Fs.ModeSet |= AUTO_SHUTDOWN;
			else 					return;

			p0 = strstr(p0,",");
			p0 ++;

			Fs.ShutDownHour = (u8)atoi(p0);

			p0 = strstr(p0,",");
			p0 ++;

			Fs.ShutDownMin = (u8)atoi(p0);

			p0 = strstr(p0,",");
			p0 ++;

			Fs.BootHour = (u8)atoi(p0);

			if(Fs.BootHour < Fs.ShutDownHour)			//如果时间跨天了，开机时间加24
			{
				Fs.BootHour += 24;
			}

			p0 = strstr(p0,",");
			p0 ++;

			Fs.BootMin = (u8)atoi(p0);

			Flag.NeedUpdateFs = 1;
			Flag.NeedRspGprs = 1;
		}
		//恢复出厂设置
		else if((p0 = strstr(pSrc,"FACTORY")) != NULL)
		{
			p1 = strstr(p0,",");
			if(p1 - p0 <= sizeof(GprsCommand))
			{
				memset(GprsCommand,0,sizeof(GprsCommand));
				strncpy(GprsCommand, p0, p1 - p0);
			}
			else
			{
				return;
			}

			Flag.NeedClrValueFile = 1;
			ResetLeftCnt = 3;	
			Flag.NeedRspGprs = 1;
		}
	}
}

void WIRELESS_Handle(void)
{
	u16 at_timeout_temp;

	if (At_Timeout_Cnt > 0)
	{
		at_timeout_temp = At_Timeout_Cnt;
	}
	else if (Flag.AtInitCmd)
	{
		at_timeout_temp = 10; //初始化指令通常不需要等待很久
	}
	else
	{
		at_timeout_temp = 75;
	}

	//AT指令延时时间到仍没回复，复位模块
	if (AtType != AT_NULL && Flag.WaitAtAck && !AtDelayCnt)
	{
		delay_ms(500);
		printf("\r\n---->wait AT response cnt:");
		printf("%d(%d)\r\n", ResetCnt, AtType);

		if (++ResetCnt > at_timeout_temp) //延时1分钟
		{
			ResetCnt = 0;
			Flag.PsSignalOk = 0;
			if ((DATA == GprsType) || (LOGIN == GprsType))
			{
				GprsDataLen = WIRELESS_GprsSendPacket(GprsType);
	//			EXFLSAH_SaveBreakPoint();
			}
			//3次AT无回应重启模块
			if (++AtTimeOutCnt > 2)
			{
				AtTimeOutCnt = 0;
				At_Timeout_Cnt = 0;
				NeedModuleReset = MODUAL_NOACK;
			}

			AtType = AT_NULL;
			Flag.WaitAtAck = 0;
			AtDelayCnt = 0;
		}

		//获取星历过程中联网异常，及时关闭 20140812_1
		if ((ResetCnt > 35) && Flag.AskUbloxData)
		{
			Flag.WaitAtAck = 0;
			AtDelayCnt = 0;
			ResetCnt = 0;
			CantConectAgpsCnt++;

			if (CantConectAgpsCnt < 3)			//三次失败后不再连AGPS服务器
				Flag.NeedReloadAgps = 1;
			else
				printf("\r\nConnect AGPS Service Fail!\r\n");
			Flag.AskUbloxData = 0;
			AtType = AT_QICLOSE_AGPS;
		}
	}
	else
	{
		ResetCnt = 0;
	}

	//AT指令错误处理
	//增加Flag.ModuleWakeUp判断，避免设备在发送休眠指令后，以及唤醒前，程序还是会进入AT_Eorror()发送
	if (Flag.ModuleOn && AtType == AT_NULL && !Flag.WaitAtAck && Flag.WakeUpMode)
	{
		Flag_check();
	}

	//AT指令发送
	if (Flag.ModuleOn && AtType != AT_NULL && !Flag.WaitAtAck)
	{
		if (!AtDelayCnt)
		{

			AT_SendPacket(AtType, AtSendbuf);
			UART_Send(AT_PORT, (u8 *)AtSendbuf, strlen(AtSendbuf));

			AtDelayCnt = 2;
			Flag.WaitAtAck = TRUE;

			printf("\r\nsend AT(%d):\r\n", AtType);
			printf(AtSendbuf);
			memset(AtSendbuf, '\0', SCIBUFLEN);
		}
	}

	if (Flag.PsSignalOk && AtType == AT_NULL && Flag.GprsConnectOk && !Flag.IsUpgrate)
	{
		GPRS_Send_Handle();	
	}
}

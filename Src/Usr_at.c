#include "usr_main.h"

#define USR_TEST_PLAM		1		//使用测试服

AT_TYPE AtType; 					//给AtType赋值的函数要在没有AT指令通信时调用，
									//赋值语句后要有break或return,以免影响同函数其它对AtType的赋值
AT_ERROR AtError;


unsigned char InitCmdTimes;			//初始AT指令时某条指令出错次数，重试三次失败，跳过该指令
unsigned char AtErrorTimes;			//计数某些AT指令连续出错次数，超过一定次数时需要重启操作
unsigned short GprsDataLen;		    //发送GPRS数据长度
unsigned char CheckSimError;		//CPIN检查SIM卡出错次数，连续三次出错会重启模块


char BatValue[5];
unsigned char Rssi; 			//gsm信号强度原始数据
unsigned short BatVoltage;		//电池电压，这里使用模块供电电压作为电池电压计算电池剩余电量
char CsqValue[12];
char Mcc[7];
char Mnc[7];
unsigned short MCC_hex;			//MCC十六进制值
unsigned short MNC_hex;			//MNC十六进制值



//向GSM内核发送AT指令前，打包要发送的AT指令
void AT_SendPacket(AT_TYPE temType, char *pDst)
{
	switch (temType)
	{
	case AT_CPIN:
		strcpy(pDst, "AT+CPIN?\r\n");
		break;

	case AT_ATE:
		strcpy(pDst, "ATE0\r\n");
		break;

	case AT_CLIP:
		strcpy(pDst, "AT+CLIP=1\r\n");
		break;

	case AT_COLP:
		strcpy(pDst, "AT+COLP=1\r\n");
		break;

	case AT_CMGF:
		strcpy(pDst, "AT+CMGF=1\r\n");
		break;

	case AT_CPMS:
		strcpy(pDst, "AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n");
		break;

	case AT_CIPMUX:
		strcpy(pDst, "AT+QIMUX=1\r\n");
		break;

	case AT_QINDI:
		strcpy(pDst, "AT+QINDI=2\r\n");
		break; 
		
	case AT_CCID:
		strcpy(pDst, "AT+CCID\r\n");
		break;

	case AT_COPS:
		strcpy(pDst, "AT+COPS=0,2\r\n");
		break;

	case AT_COPS_CHECK:
		strcpy(pDst, "AT+COPS?\r\n");
		break;

	case AT_GSN:
		strcpy(pDst, "AT+GSN\r\n");
		break;

	case AT_CSCS: 
		strcpy(pDst, "AT+CSCS=\"IRA\"\r\n"); 
		break;

	case AT_CLVL:
		strcpy(pDst, "AT+CLVL=3\r\n");
		break;

	case AT_CGSN:
		strcpy(pDst, "AT+CGSN\r\n");
		break;

	case AT_CSDH:
		strcpy(pDst, "AT+CSDH=0\r\n");
		break;

	case AT_CSMP:
		strcpy(pDst, "AT+CSMP=17,167,0,0\r\n"); 
		break;

	case AT_CNMI:
		strcpy(pDst, "AT+CNMI=2,1\r\n");
		break;

	case AT_QURCCFG:
		strcpy(pDst, "AT+QURCCFG=\"urcport\",\"uart1\"\r\n");
		break;

	case AT_QCFG:
		strcpy(pDst, "AT+QCFG=\"urc/ri/other\",\"off\",120\r\n"); //关闭有网络上报时RI脚电平变化
	//	strcpy(pDst, "AT+QCFG=\"urc/ri/other\",\"pulse\",200\r\n");			//开启有网络上报时RI脚电平变化
		break;

	case AT_CREG:
		strcpy(pDst, "AT+CREG?\r\n");
		break;

	case AT_CGREG_SET:
		strcpy(pDst, "AT+CGREG=2\r\n");
		break;

	case AT_CGREG:
		strcpy(pDst, "AT+CGREG?\r\n");
		break;

	case AT_QICSGP:
		if (Fs.GprsUserName[0] && Fs.GprsPassWord[0])
		{
			sprintf(pDst, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\"\r\n", Fs.ApnName, Fs.GprsUserName, Fs.GprsPassWord);
		}
		else if (Fs.GprsUserName[0]) 
		{
			sprintf(pDst, "AT+QICSGP=1,1,\"%s\",\"%s\"\r\n", Fs.ApnName, Fs.GprsUserName);
		} 
		else if (Fs.ApnName[0])
		{
			sprintf(pDst, "AT+QICSGP=1,1,\"%s\"\r\n", Fs.ApnName);
		}
		else
		{
			strcpy(pDst, "AT+QICSGP=1,1,\"cmnet\"\r\n");
		}
		break;

	case AT_QIACT:
		strcpy(pDst, "AT+QIACT=1\r\n");
		break;

	case AT_QISTATE:
		strcpy(pDst, "AT+QISTATE=0,1\r\n");
		break;

	case AT_QNTP:
		strcpy(pDst, "AT+QNTP=1,\"ntp1.aliyun.com\"\r\n");
		TIMER_AtDelay(3);
		break;

	case AT_CCLK:
		strcpy(pDst, "AT+CCLK?\r\n");
		TIMER_AtDelay(3);
		break;

	case AT_QIOPEN:
		if (Flag.AskUbloxData)
		{
			sprintf(pDst, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%s,0,1\r\n", Fs.UbloxIp, Fs.UbloxPort);
			Flag.ConUblox = 1;
		}		
		TIMER_AtDelay(2);
		break;

	case AT_QISEND:
		if(Flag.AskUbloxData)
		{
			GprsType = AGPSDATA;
			GprsDataLen = WIRELESS_GprsSendPacket(GprsType);
			sprintf(pDst, "AT+QISEND=0,%d\r\n", GprsDataLen);	
		}
		break;

	case AT_GPRSEND:
		memcpy(pDst, GprsSendBuf, GprsDataLen);
		TIMER_AtDelay(5);
		break;

	case AT_QIDEACT:
		strcpy(pDst, "AT+QIDEACT=1\r\n");
		TIMER_AtDelay(2);
		break;

	case AT_QICLOSE_AGPS:
		strcpy(pDst, "AT+QICLOSE=0\r\n");
		TIMER_AtDelay(2);
		break;

	case AT_QICLOSE:
		strcpy(pDst, "AT+QICLOSE=1\r\n");
		TIMER_AtDelay(2);
		break;

	case AT_CBC:
		strcpy(pDst, "AT+CBC\r\n");
		break;
	#if 0
	case AT_CMGR:
		sprintf(pDst, "AT+CMGR=%d\r\n", SmsIndex);
		Flag.HaveSms = 0;
		break;

	case AT_CMGD:
		sprintf(pDst, "AT+CMGD=%d,4\r\n", SmsIndex);
		break;



	case AT_CMGS:
			sprintf(pDst, "AT+CMGS=\"%s\"\r\n", GsmSms.phone);
		break;

	case AT_SMSEND:
		sprintf(pDst, "%s\x1A", SmsBuf);
		TIMER_AtDelay(5);
		break;

	case AT_ATH:
		strcpy(pDst, "ATH\r\n");
		break;

	case AT_ATD:
		sprintf(pDst, "ATD%s;\r\n", GsmSms.phone);
		Flag.IsDialing = 1;
		Flag.OnPhone = 0;
		break;
#endif

	case AT_CSQ:
		strcpy(pDst, "AT+CSQ\r\n");
		break;

	case AT_ATA:
		strcpy(pDst, "ATA\r\n");
		break;

	case AT_AT:
		strcpy(pDst, "AT\r\n");
		break;

	case AT_CSCLK:
		strcpy(pDst, "AT+QSCLK=1\r\n");
		break;

	case AT_GTSET:
		strcpy(pDst, "at+gtset=\"callbreak\",0\r\n");
		break;

	case AT_GTSET_1:
		strcpy(pDst, "at+gtset=\"IPRFMT\",2\r\n");
		break;

	case AT_CLCC:
		strcpy(pDst, "at+clcc\r\n");
		break;

	case AT_GTAUDGAIN:
		strcpy(pDst, "at+gtaudgain=5,249\r\n");
		TIMER_AtDelay(3);

	case AT_CENG_CK:
		strcpy(pDst, "AT+QENG?\r\n"); //开启工程师模式，MC20AT指令，这里不用
		TIMER_AtDelay(3);
		break;

	case AT_QWIFISCAN:
		strcpy(pDst, "AT+QWIFISCAN=20000,3,5,10,1\r\n"); //wifi扫描
		TIMER_AtDelay(3);
		break;

	case AT_QHTTPURL:
		strcpy(pDst, "AT+QHTTPURL=36,80\r\n"); 			//设置链接地址长度及超时时间
		TIMER_AtDelay(3);
		break;

	case AT_HTTPURL:
		strcpy(pDst, "https://www.zzhb.org.cn/gas/e/report\r\n"); 	//发送链接
		TIMER_AtDelay(3);
		break;

	case AT_QHTTPGET:
		strcpy(pDst, "AT+QHTTPGET=80\r\n"); 			//get链接内容
		TIMER_AtDelay(3);
		break;

	case AT_QHTTPCFG:									//设置用户去配置消息头内容
		strcpy(pDst, "AT+QHTTPCFG=\"requestheader\",1\r\n"); 			
		TIMER_AtDelay(3);
		break;

	case AT_QHTTPPOST:									//向网页post数据
		GprsDataLen = WIRELESS_GprsSendPacket(GprsType);
		sprintf(pDst, "AT+QHTTPPOST=%d,30,30\r\n",GprsDataLen); 			
		TIMER_AtDelay(3);
		break;

	case AT_HTTPPOST:									//向网页post数据
		sprintf(pDst, "%s\r\n",GprsSendBuf); 			
		TIMER_AtDelay(3);
		break;

	case AT_QHTTPREAD:
		strcpy(pDst, "AT+QHTTPREAD=80\r\n"); 			//读取发送结果
		TIMER_AtDelay(3);
		break;

	case AT_QHTTPREADFILE:
		strcpy(pDst, "AT+QHTTPREADFILE=\"FOTA.bin\",80\r\n"); 			//保存下载文件
		TIMER_AtDelay(3);
		break;

	case AT_QFOPEN:
		strcpy(pDst, "AT+QFOPEN=\"FOTA_TEST.bin\",0\r\n"); 			//打开文件
		TIMER_AtDelay(3);
		break;

	case AT_QFREAD:
		strcpy(pDst, "AT+QFREAD=1,1024\r\n"); 				//读取文件指定长度
		TIMER_AtDelay(3);
		break;

	case AT_QFCLOSE:
		strcpy(pDst, "AT+QFCLOSE=1\r\n"); 					//关闭文件
		TIMER_AtDelay(3);
		break;

	default:
		break;
	}
}

unsigned char AT_InitReceive(AT_TYPE *temType, char *pSrc)
{
//	char *ptem = NULL, *p1 = NULL;
	unsigned char back = 0;
	if (strstr(pSrc, "OK") || InitCmdTimes >= 2)
	{
		back = 1;
		InitCmdTimes = 0;
		AtDelayCnt = 0;
		if (*temType == AT_ATE)
		{
			*temType = AT_AT;
		}
		else if (*temType == AT_AT)
		{
			*temType = AT_CLIP;
		}		
		else if (*temType == AT_CLIP)
		{
			*temType = AT_COLP;
		}
		else if (*temType == AT_COLP)
		{
			*temType = AT_CMGF;
		}
		else if (*temType == AT_CMGF)
		{
			*temType = AT_CNMI;
		}
		else if (*temType == AT_CNMI)
		{
			*temType = AT_CGSN;
		}
		else if (*temType == AT_CGSN)
		{
			*temType = AT_CSCLK;
		}
		else if (*temType == AT_CSCLK)
		{
			*temType = AT_CSDH;
		}
		else if (*temType == AT_CSDH)
		{
			*temType = AT_QURCCFG;
		}
		else if (*temType == AT_QURCCFG)
		{
			*temType = AT_CPMS;
		}
		else if (*temType == AT_CPMS)
		{
			*temType = AT_COPS;
		}
		else if (*temType == AT_COPS)
		{
			*temType = AT_QCFG;
		}
		else if (*temType == AT_QCFG)
		{
			*temType = AT_CGREG;
			Flag.AtInitCmd = 0;
			Flag.AtInitFinish = 1;
		}
	}
	else if (strstr(pSrc, "ERROR"))
	{
		back = 1;
		AtDelayCnt = 5;
		InitCmdTimes++;
	}
	return back;
}

//处理GSM内核回应AT指令的数据
unsigned char AT_Receive(AT_TYPE *temType, char *pSrc)
{
	char *ptem, *p1;
	unsigned short i;
	char temp[10] = {0};
	unsigned char back = 0;
	static unsigned char error1 = 0, error2 = 0, error3 = 0;
	switch (*temType)
	{
	case AT_CPIN:
		if (strstr(pSrc, "READY"))
		{
			AtDelayCnt = 0;
			back = 1;
			*temType = AT_NULL;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			AtDelayCnt = 0;
			back = 1;
			CheckSimError++;
			if (CheckSimError > 3)
			{
				CheckSimError = 0;
				NeedModuleReset = NO_SIMCARD;
			}
		}
		break;

	case AT_CSCS:
		if (strstr(pSrc, "OK"))
		{
			*temType = AT_CSMP;
			back = 1;
			AtDelayCnt = 0;
		}
		else if (strstr(pSrc, "ERROR"))
		{
//			Flag.IsCnSms = 0;
			*temType = AT_NULL;
			back = 1;
		}
		break;

	case AT_CSCS_EN:
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_CSMP_EN;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_CSMP:
		if (strstr(pSrc, "OK"))
		{
			*temType = AT_CMGS; 
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_CSMP_EN:
		if (strstr(pSrc, "OK"))
		{
			*temType = AT_NULL;
			back = 1;
		}
		break;

	case AT_CSCLK:
		if (strstr(pSrc, "OK"))
		{
			*temType = AT_NULL;
			Flag.WakeUpMode = 0;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_CGREG:
		if (strstr(pSrc, "+CGREG:"))
		{
<<<<<<< HEAD
			if (strstr(pSrc, "2,1,") || strstr(pSrc, "2,5,"))
=======
			if (strstr(pSrc, ",1") || strstr(pSrc, ",5"))
>>>>>>> cfc6897120fc08673b392d5bd1225f628af94601
			{

				if ((Flag.PsSignalChk && Flag.GprsConnectOk) || (ConnectDelayCnt > 0))
				{
					*temType = AT_NULL;
				}
				else
				{
					*temType = AT_QICSGP;
				}

				Flag.PsSignalOk = 1; 
				AtDelayCnt = 0;
				AtError.PsSingalEorCnt = 0; 
											//Flag.EorhaveShutGprs=0;	
			}
			else
			{
				*temType = AT_NULL;
				Flag.PsSignalOk = 0; 
				AtError.PsSingalEorCnt++;	  

				if (AtError.PsSingalEorCnt > 10) 
				{
					AtError.PsSingalEorCnt = 0;

					if (!Flag.InNoSignNoRstMd)		//如果不是在认定没有信号的地方，重启模块
					{
						NeedModuleReset = CANT_ATTACH_NET;
					}
				}

				if (Flag.Insleeping && Flag.InNoSignNoRstMd) //设备处于没有信号的地方，查询没有网络，直接进入休眠
				{
					ActiveTimer = 4;
//					Flag.SendHandInsleeping = 0;
				}
			}
		}
		else if (strstr(pSrc, "OK"))
		{
			*temType = AT_NULL;
		}
		Flag.PsSignalChk = 0; 
		back = 1;
		break;

	case AT_QICSGP:

		if (strstr(pSrc, "OK"))
		{
			*temType = AT_QIACT;
			AtDelayCnt = 0;
			back = 1;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtError.GprsConnectEorCnt++;
			back = 1;
		}
		break;

	case AT_CCID:
		if ((p1 = strstr(pSrc, "+CCID:")) != NULL)
		{
			memset(CCID, '\0', 21);
			p1 += 7;
			strncpy(CCID, p1, 20);

			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;
		}
		break;


	case AT_QIACT:
		if (strstr(pSrc, "OK"))
		{
			Flag.IsContextAct = 1;
			if (Flag.AskUbloxData)
			{
				*temType = AT_QIOPEN;
			}
			else
			{
				*temType = AT_QHTTPCFG;
			}
			
			AtDelayCnt = 0;
			back = 1;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			back = 1;
			AtDelayCnt = 0;
			if (Flag.IsContextAct) //场景已经激活
				*temType = AT_QHTTPCFG;
			else
			{
				*temType = AT_NULL;
				AtError.GprsConnectEorCnt++;
			}
		}
		break;

	case AT_QISTATE:
#if 1
		if (strstr(pSrc, "+QISTATE:"))
		{
			if (!Flag.GprsConnectOk) 
			{
				*temType = AT_QIOPEN;
			}
			else
			{
				*temType = AT_NULL;
			}

			AtDelayCnt = 0;
			back = 1;
		}
		else if (strstr(pSrc, "OK"))
		{
			*temType = AT_QICSGP;
			AtDelayCnt = 0;
			back = 1;
		}

		else if (strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			back = 1;
		}

#endif
		break;
	case AT_QNTP: 

		if ((p1 = strstr(pSrc, "+QNTP:")) != NULL)
		{
			if (((p1 = strstr(pSrc, "+QNTP: 0,")) != NULL) && ((p1 = strstr(p1, ",\"")) != NULL))
			{
				TIME_UpdateRtcByNtp(pSrc);
			}

			AtDelayCnt = 0;
			*temType = AT_NULL;
			back = 1;

		}
		else 
		{
			AtDelayCnt = 0;
			*temType = AT_NULL;
			back = 1;
		}
		break;

	case AT_CCLK:
		//模块时间未同步时会上报初始时间+CCLK: "80/01/06,00:00:32+00"		，不可以使用
		if ((p1 = strstr(pSrc, "+CCLK:")) != NULL)
		{
			if ((p1 = strstr(pSrc, "70/01")) != NULL) //不保存初始时间
			{
				AtDelayCnt = 0;
				*temType = AT_NULL;
				back = 1;
				break;
			}
			TIME_UpdateRtcByNtp(pSrc);

			if (!Flag.Insleeping)
			{
				if (CompareAgpsRct(Rtc, RtcAgpsBackup))
				{
					Flag.NeedReloadAgps = 1;
				}
			}
			AtDelayCnt = 0;
			*temType = AT_NULL;
			back = 1;
		}
		else if (strstr(pSrc, "OK"))
		{
			back = 1;
		}
		else
		{
			AtDelayCnt = 0;
			*temType = AT_NULL;
			back = 1;
		}
		break;

	case AT_QIOPEN:
		if (strstr(pSrc, "+QIOPEN: 0,0"))
		{
			*temType = AT_QISEND;
			AtDelayCnt = 0;
			error3 = 0;
			back = 1;
		}
		else if ((strstr(pSrc, "OK")) && (strstr(pSrc, "+QIOPEN:") == NULL))
		{
			back = 0;
		}
		else if (strstr(pSrc, "+QIOPEN: 1,563")) //有时会出现
		{
			*temType = AT_QICLOSE;
			back = 1;
		}
		else
		{
			ConnectDelayCnt = 15;
			*temType = AT_NULL;
			error3++;

			if (Flag.AskUbloxData)
			{
				Flag.AskUbloxData = 0;
			}

			if (error3 > 5)
			{
				error3 = 0;
				NeedModuleReset = CONNECT_SERVICE_FAILED;
			}
			back = 1;
		}
		break;

	case AT_QURCCFG:
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_CREG:
		if (strstr(pSrc, "CREG:"))
		{
			if (strstr(pSrc, "0,1") || strstr(pSrc, "0,5"))
			{

			}
			else
			{

			}
			*temType = AT_NULL;
			back = 1;
		}
		else if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			back = 1;
		}

		break;
	case AT_QISEND:

		if (strstr(pSrc, ">"))
		{
			*temType = AT_GPRSEND;
			AtDelayCnt = 0;
			error1 = 0;
			back = 1;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			Flag.NeedCloseGprs = 1;
			if ((DATA == GprsType) || (LOGIN == GprsType)) //20151209_4
			{
				GprsDataLen = WIRELESS_GprsSendPacket(GprsType);
	//			EXFLSAH_SaveBreakPoint();
			}

			error1++;
			if (error1 > 5)
			{
				error1 = 0;
				NeedModuleReset = CONNECT_SERVICE_FAILED;
			}

			if (Flag.AskUbloxData && Flag.ConUblox)
			{
				Flag.AskUbloxData = 0;
				Flag.NeedCloseAgpsConnect = 1;
			}

			back = 1;
			*temType = AT_NULL;
		}
		else
		{
			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;			/* code */
		}
		
		break;

	case AT_GPRSEND:
		if ((p1 = strstr(pSrc, "OK")) != NULL)
		{
			if (Flag.AskUbloxData && Flag.ConUblox) 
			{
				Flag.AskUbloxData = 0;
				WaitUbloxCnt = 10;		//发送成功后，等待10秒，如果超时未收到，断开AGPS链接
			}

			//如果是休眠期间周期性唤醒时上传数据成功，ActiveTimer = 2即刻进入休眠
			if(((Flag.Insleeping) || (GprsSend.posCnt == 0)) && (!Flag.NoSleepMode))
			{
				ActiveTimer = 5;
			}	
			
			error2 = 0;
			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;	
		}
		else if (strstr(pSrc, "ERROR") || strstr(pSrc, "SEND FAIL"))
		{
			Flag.NeedCloseGprs = 1;
			if ((DATA == GprsType) || (LOGIN == GprsType)) //20151209_4  20160321_1
			{
				GprsDataLen = WIRELESS_GprsSendPacket(GprsType);
	//			EXFLSAH_SaveBreakPoint();
			}

			if (Flag.AskUbloxData)
			{
				Flag.AskUbloxData = 0;
			}

			error2++;
			if (error2 > 5)
			{
				error2 = 0;
				NeedModuleReset = 2;
			}

			*temType = AT_NULL;
			back = 1;
		}
		else if (strstr(pSrc, "CLOSED"))
		{
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_QICLOSE:
	{
		*temType = AT_QIDEACT;
		AtDelayCnt = 0;
		back = 1;
		break;
	}

	case AT_QICLOSE_AGPS:
		if (strstr(pSrc, "OK"))
		{
			Flag.ConUblox = 0;
			AtDelayCnt = 0;
			back = 1;
		}
		else
		{
			AtDelayCnt = 0;
			back = 1;
		}

		if (!Flag.GprsConnectOk)	//如果是开机时，先获取AGPS数据，获取完成之后就开始连接服务器
		{
			*temType = AT_QHTTPCFG;
		}
		else
		{
			*temType = AT_NULL;
		}
		break;	

	case AT_QIDEACT:
	{
		Flag.GprsConnectOk = 0;
		Flag.IsContextAct = 0;
		AtDelayCnt = 5;
		*temType = AT_NULL;
		back = 1;

		break;
	}


	case AT_COPS:
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR") || strstr(pSrc, "ABORTED"))
		{
			*temType = AT_COPS_CHECK;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_COPS_CHECK:
	{
		char data_temp[10] = {0};
		//+COPS: 0,2,"46000",3
		if ((p1 = strstr(pSrc, "+COPS:")) != NULL)
		{
			AtDelayCnt = 0;
			back = 1;
			*temType = AT_NULL;

			p1 = strstr(pSrc, "\"");
			p1 ++;
			ptem = strstr(p1, "\"");
			if(ptem - p1 < 6)
			{
				memset(Mcc,0,sizeof(Mcc));
				strncpy(Mcc,p1,3);

				memset(Mnc,0,sizeof(Mnc));
				strncpy(Mnc,p1+3,2);	

				BcdStr2HexStr(Mcc,data_temp);
				memset(Mcc,0,sizeof(Mcc));
				strcpy(Mcc,data_temp);

				memset(data_temp,0,sizeof(data_temp));

				BcdStr2HexStr(Mnc,data_temp);
				memset(Mnc,0,sizeof(Mnc));
				strcpy(Mnc,data_temp);				
			}

		}
		else if (strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		else if (strstr(pSrc, "OK"))
		{
			back = 1;
		}
	}
	break;

	case AT_GSN:
		if (strstr(pSrc, "OK"))
		{
			ptem = strstr(pSrc,"\r\n");
			if(ptem != NULL)
			{
				ptem += 2;
				if((*ptem >= '0')&&(*ptem <= '9'))
				{
					memset(IMEI,0,16);
					if(strlen(IMEI_MANUAL) == 15)
					{
						strcpy(IMEI,IMEI_MANUAL);
					}
					else
					{
						strncpy(IMEI,ptem,15);
						if(strcmp(Fs.DeviceImei,IMEI) != 0)
						{
							strncpy(Fs.DeviceImei,IMEI,sizeof(Fs.DeviceImei));
							Flag.NeedUpdateFs = 1;
						}
					}
				}
			}

			*temType = AT_NULL;		
			back = 1;
		}
		else
		{
			*temType = AT_NULL;
			back = 1;
		}
		break;
	case AT_CMGR:
	{
		*temType = AT_NULL;
		AtDelayCnt = 0;
		back = 1;
	}
	break;

	case AT_CMGD:
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;
	case AT_CBC:
		if (strstr(pSrc, "+CBC:"))
		{
			//+CBC: 0,90,4066	R_UC20
			*temType = AT_NULL;

			ptem = strrchr(pSrc, ',') + 1;
			memset(BatValue, '\0', 5);
			strncpy(BatValue, ptem, 4);
			if (strcmp(BatValue, "3650") < 0)
				Flag.BattLow = 1;
			else
				Flag.BattLow = 0;
			back = 1;
		}
		else if (strstr(pSrc, "OK"))
		{
			back = 1;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			back = 1;
		}

		break;
	case AT_CMGS:
		if (strstr(pSrc, ">"))
		{
			*temType = AT_SMSEND;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_SMSEND:
		if (strstr(pSrc, "+CMGS:") || strstr(pSrc, "OK"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_ATH:
		if (strstr(pSrc, "OK"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_MCELL_1:
		if (strstr(pSrc, "+MCELL:"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;
	case AT_MCELL_2:
//		if (strstr(pSrc, "+MCELL:"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_ATD:
		if (strstr(pSrc, "OK"))
		{
			*temType = AT_NULL;
			back = 1;
		}
		else if (strstr(pSrc, "ERROR"))
		{
//			Flag.IsDialing = 0;
//			Flag.OnPhone = 0;
			*temType = AT_NULL;
			back = 1;
		}

		break;
	case AT_CSQ:
		if ((p1 = strstr(pSrc, "+CSQ:")) != NULL)
		{
			memset(CsqValue, '\0', 12);
			memset(temp, '\0', 10);
			p1 += 5;
			if (*p1 == ' ')
				p1++;
			ptem = strchr(p1, ',');

			if(ptem == NULL)
			break;

			strncpy(temp, p1, (ptem - p1));
			i = Usr_Atoi(temp);
			Rssi = i;
			if (i == 99)
			{
				strcpy(CsqValue, "csq unknown");
			}
			else
			{
				i = 113 - (2 * i);

				CsqValue[0] = '-';
				Itoa(i, CsqValue + 1);
				strcat(CsqValue, " dBm");
			}
			*temType = AT_NULL;
			back = 1;
		}
		else if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			back = 1;
		}

		break;
	case AT_ATE:
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}

		break;
	case AT_ATA:
		if (strstr(pSrc, "OK"))
		{
//			Flag.OnPhone = 1;
//			Flag.IsDialing = 0;
			*temType = AT_NULL;
			back = 1;
		}
		else
		{
			*temType = AT_NULL;
			back = 1;
		}
		break;

	case AT_AT:
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;

	case AT_CLCC:
		if (strstr(pSrc, "+CLCC: 1,0,0"))
		{
			*temType = AT_GTAUDGAIN;
			AtDelayCnt = 0;
			back = 1;
		}
		else
		{
			*temType = AT_NULL;
			AtDelayCnt = 0;
			back = 1;
		}
		break;
	case AT_GTAUDGAIN: //调节音频增益
		if (strstr(pSrc, "OK") || strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			back = 1;
			AtDelayCnt = 0;
		}
		break;

	case AT_CENG_CK:
		if (strstr(pSrc, "+QENG:") != NULL)
		{
//			BLS_DataProcess(pSrc);
		}
		else
		{
		}
		back = 1;
		AtDelayCnt = 0;
		*temType = AT_NULL;
		break;

	case AT_QWIFISCAN:
		//+QWIFISCAN:(-,-,-68,"A8:0C:63:D2:99:B4"6)
		

		if(strstr(pSrc, "+QWIFISCAN:") != NULL)
		{
			char wifi_csq[4] = {0};			//wifi信号强度
			
			WifiCnt = 0;	
			memset(Wifi_Content,0,sizeof(Wifi_Content));
			p1 = pSrc;

			while (strstr(p1, "+QWIFISCAN:") != NULL)
			{
				p1 = strstr(p1, ",");
				p1 ++;
				p1 = strstr(p1, ",");
				p1 ++;
				ptem = strstr(p1, ",");

				if(ptem - p1 > 4)		break;

				memset(wifi_csq,0,sizeof(wifi_csq));
				strncat(wifi_csq,p1,ptem - p1);

				p1 = strstr(p1, "\"");
				p1 ++;
				ptem = strstr(p1, "\"");

				if(ptem - p1 > 20)		break;
		
				strncat(Wifi_Content,p1,ptem - p1);
				strcat(Wifi_Content,",");
				strcat(Wifi_Content,wifi_csq);
				strcat(Wifi_Content,",");

				WifiCnt ++;
			}

			for(i = 0;i < strlen(Wifi_Content);i++)
			{
				if(Wifi_Content[i] == ':')	Wifi_Content[i] = '-';
			}
			Wifi_Content[i - 1] = 0;		//去掉末尾的','

			if(WifiCnt > 0)
			{
				Flag.GetScanWifi = 1;
			}
			
			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;
		}
		else if(strstr(pSrc, "ERROR") != NULL)
		{
			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;			
		}
<<<<<<< HEAD
=======
	break;

	case AT_QHTTPCFG:
		if(strstr(pSrc, "OK") != NULL)
		{
			*temType = AT_QHTTPURL;
			back = 1;
			AtDelayCnt = 0;
		}	
		else if (strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtError.GprsConnectEorCnt++;
			back = 1;
		}
>>>>>>> cfc6897120fc08673b392d5bd1225f628af94601
	break;

	case AT_QHTTPURL:
		if(strstr(pSrc, "CONNECT") != NULL)
		{
			*temType = AT_HTTPURL;
			back = 1;
			AtDelayCnt = 0;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			*temType = AT_NULL;
			AtError.GprsConnectEorCnt++;
			back = 1;
		}
		break;

	case AT_HTTPURL:
		if(strstr(pSrc, "OK") != NULL)
		{
			Flag.GprsConnectOk = 1;
			*temType = AT_NULL;
			error3 = 0;
			back = 1;
			AtDelayCnt = 0;
		}
		else
		{
			ConnectDelayCnt = 15;
			*temType = AT_NULL;
			error3++;
			if (error3 > 5)
			{
				error3 = 0;
				NeedModuleReset = CONNECT_SERVICE_FAILED;
			}
			back = 1;			
		}
		break;

	case AT_QHTTPPOST:
		if(strstr(pSrc, "CONNECT") != NULL)
		{
			*temType = AT_HTTPPOST;
			error1 = 0;
			back = 1;
			AtDelayCnt = 0;
		}
		else if (strstr(pSrc, "ERROR"))
		{
			Flag.NeedCloseGprs = 1;

			error1++;
			if (error1 > 5)
			{
				error1 = 0;
				NeedModuleReset = CONNECT_SERVICE_FAILED;
			}

			back = 1;
			*temType = AT_NULL;
		}
		else
		{
			back = 1;
			AtDelayCnt = 0;
			*temType = AT_NULL;			
		}
		break;

	case AT_HTTPPOST:
		if(strstr(pSrc, "+QHTTPPOST: 0,200") != NULL)
		{
			*temType = AT_QHTTPREAD;
			error2 = 0;
			back = 1;
			AtDelayCnt = 0;

			//如果是有gps情况下发送的数据包，那么数据包有效，就可以直接进入休眠
			if(Flag.Insleeping && Flag.HaveGPS)
			{
				ActiveTimer = 3;
			}
		}
		else if (strstr(pSrc, "OK"))
		{
			back = 0;
		}
		else 
		{
			error2++;
			if (error2 > 5)
			{
				error2 = 0;
				NeedModuleReset = 2;
			}

			*temType = AT_NULL;
			back = 1;
		}
		break;

	case AT_QHTTPREAD:
		if(strstr(pSrc, "+QHTTPREAD:") != NULL)
		{
			*temType = AT_NULL;
			back = 1;
			AtDelayCnt = 0;
		}
		break;

	case AT_QHTTPGET:
		if(strstr(pSrc, "+QHTTPGET: 0,200") != NULL)
		{
			*temType = AT_QHTTPREADFILE;
			back = 1;
			AtDelayCnt = 0;
		}
		break;

	case AT_QHTTPREADFILE:
		if(strstr(pSrc, "+QHTTPREADFILE: 0") != NULL)
		{
			*temType = AT_QFOPEN;
			back = 1;
			AtDelayCnt = 0;
		}
		break;

	case AT_QFOPEN:
		if(strstr(pSrc, "+QFOPEN: 1") != NULL)
		{
			*temType = AT_QFREAD;
			back = 1;
			AtDelayCnt = 0;
		}
		break;

	case AT_QFREAD:
		if((strstr(pSrc, "CONNECT") != NULL)&&(UpgInfo.AppReadComplete == 0))
		{
			WIRELESS_UpgradeReceive(pSrc);
		}
		else if(UpgInfo.AppReadComplete)	//文件已经读取完成
		{
			*temType = AT_NULL;
		}	
		else								//文件读取出错		
		{
			*temType = AT_NULL;
			UpgInfo.UpgrateFail = 1;
		}
		AtDelayCnt = 0;
		back = 1;	
		break;

	case AT_QFCLOSE:
		if(strstr(pSrc, "OK") != NULL)
		{
			*temType = AT_NULL;
			back = 1;
			AtDelayCnt = 0;
		}
		break;

	
	default:
		break;
	}

	if (baseTimeSec > 18 && !Flag.IsUpgrate && (AT_NULL == *temType) && strstr(pSrc, "AT+")) //20150403_2  20150416_1 20150610_12
	{
		*temType = AT_ATE;
		AtDelayCnt = 0;
		back = 1;
	}

	if (AT_NULL == *temType)
		AtDelayCnt = 0;

	return back;
}

void Flag_check(void)
{
	if (AtType != AT_NULL)
	{
		return;
	}
	//模块不响应AT指令时 只发AT测试	
	if (AtTimeOutCnt)
	{
		AtType = AT_AT;
		return;
	}

	if (Flag.SendAtWithoutRDY)
	{
		Flag.SendAtWithoutRDY = 0;
		Flag.PsSignalOk = 0; 
		Flag.WaitAtAck = FALSE;
		Flag.HaveSmsReady = 1;		//将SMSREADY设置为1，避免后面SMSREADY报上来，又重新初始化一遍

		AtType = AT_ATE; 	//开始初始化模块
		Flag.AtInitCmd = 1;
		printf("Haven't recvive SMS Ready from GSM module,start AT command without SMS Ready\r\n");
		return;
	}

	if(Flag.AtInitFinish == 0)				//模块没有初始化完成前，不查询下面内容
	{
		return;
	}

	if (Flag.NeedCloseGprs)
	{
		Flag.NeedCloseGprs = 0;
		AtType = AT_QICLOSE;
		printf("\r\nDisconnect the connect to service!\r\n");
		return;
	}
#if 0
	if(UpgInfo.NeedCheckUploadState)
	{
		UpgInfo.NeedCheckUploadState = 0;
		AtType = AT_HTTPTOFSRL;
		return;
	}

	if(UpgInfo.AppDownloadOk)
	{
		UpgInfo.AppDownloadOk = 0;
		AtType = AT_CFSINIT;
		return;		
	}
#endif
	if(Flag.IsUpgrate)
	{
		return;
	}

	//定时检查ps是否注册
	if (Flag.PsSignalChk && !Flag.IsUpgrate)
	{
		AtType = AT_CGREG;
		return;
	}

	//联网之后查询到网络注册丢失，断开连接
	if (!Flag.PsSignalOk && Flag.GprsConnectOk)
	{
		Flag.NeedCloseGprs = 1;
		printf("\r\nDisconnect the Service because of attch to net failed!\r\n");
		return;
	}

	//查电量
	if (Flag.BatChk && !Flag.IsUpgrate)
	{
		Flag.BatChk = 0;
		AtType = AT_CBC;
		return;
	}

	//查信号强度
	if (Flag.CsqChk && !Flag.IsUpgrate)
	{
		Flag.CsqChk = 0;
		AtType = AT_CSQ;
		return;
	}

	if(Flag.NeedSetNtp && Flag.GprsConnectOk)
	{
		Flag.NeedSetNtp = 0;
		AtType = AT_QNTP;
		return;
	}

	if (Flag.NtpGetCCLK)
	{
		Flag.NtpGetCCLK = 0;
		AtType = AT_CCLK;
		return;
	}

	if (Flag.NeedGetIMEI)
	{
		Flag.NeedGetIMEI = 0;
		AtType = AT_GSN;
		return;
	}

	if (Flag.NeedcheckCCID)
	{
		Flag.NeedcheckCCID = 0;
		AtType = AT_CCID;
		return;
	}

	if (Flag.NeedCheckSIM && !Flag.PsSignalOk)
	{
		Flag.NeedCheckSIM = 0;
		AtType = AT_CPIN;
		return;
	}

	if (AtError.GprsConnectEorCnt > 10)
	{
		AtError.GprsConnectEorCnt = 0;
		NeedModuleReset = CONNECT_SERVICE_FAILED;
		return;
	}

	if ((Flag.NeedReloadAgps && Flag.IsContextAct)&&(Fs.LongitudeLast[0] != 0xFF)&&(Fs.LongitudeLast[0] != 0))
	{
		Flag.NeedReloadAgps = 0;
		Flag.AskUbloxData = 1;
		AtType = AT_QIOPEN;
		return;
	}

	if (Flag.NeedCloseAgpsConnect)
	{
		Flag.NeedCloseAgpsConnect = 0;
		AtType = AT_QICLOSE_AGPS;
		return;
	}

	if (Flag.NeedGetMccMnc && Flag.PsSignalOk)
	{
		Flag.NeedGetMccMnc = 0;
		AtType = AT_COPS_CHECK;
		return;
	}

	if (Flag.NeedScanWifi)
	{
		Flag.NeedScanWifi = 0;
		AtType = AT_QWIFISCAN;
		return;
	}

	//网络激活状态，并且已经给服务器应答开始升级指令，可以开始升级
	if((UpgInfo.NeedUpdata)&&(Flag.IsContextAct))
	{
		UpgInfo.NeedUpdata = 0;
		AtType = AT_QHTTPURL;
		return;
	}
}


#include "usr_main.h"

Rtc_st Rtc;

unsigned char baseTimeCnt;
unsigned char baseTimeSec;
unsigned char baseTimeMin;
unsigned char baseTimeHor;
unsigned int baseSecCnt;
unsigned int Timestamp;		//unix时间（时间戳）
unsigned char ResetLeftCnt; //该变量为重启设备倒计时。被赋值之后递减，为0时重启模块。需要延时重启时使用，例如更新Fs数据后需要重启
unsigned int AtDelayCnt; 	//AT指令发送成功后延时多久发送下一条指令，通常AT指令处理处理完成后会清零该位，取消等待
unsigned short IntervalTemp; //用来暂存定时上传时间间隔

unsigned char AT_CBC_IntervalTemp; 	//电池电量采样间隔

const unsigned char arr_nDays[12] = 	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const unsigned char Leap_month_day[12]=	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //闰年 



void TIMER_AtDelay(unsigned int i) //1对应1s 专用于AT指令延时
{
	AtDelayCnt = i;
}

void delay_us(unsigned int us)
{
	LL_uDelay(us);
}

//参数小于1864ms
void delay_ms(unsigned int ms)
{
	LL_mDelay(ms);
}
 
void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
	if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
	{
		
	};

	/* HSE configuration and activation */
	LL_RCC_HSE_Enable();
	while(LL_RCC_HSE_IsReady() != 1)
	{
	};

	/* Main PLL configuration and activation */
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 8, LL_RCC_PLLR_DIV_2);
	LL_RCC_PLL_Enable();
	LL_RCC_PLL_EnableDomain_SYS();
	while(LL_RCC_PLL_IsReady() != 1)
	{
	};

	/* Set AHB prescaler*/
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

	/* Sysclk activation on the main PLL */
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
	{
	};

	/* Set APB1 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_Init1msTick(32000000);
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_SetSystemCoreClock(32000000);
	/* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
	LL_SetSystemCoreClock(32000000);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
	LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
	LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_SYSCLK);
  
}

//主频32M
void Usr_TIM3_Init(void)
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

    /* TIM3 interrupt Init */
    NVIC_SetPriority(TIM3_IRQn, 3);
    NVIC_EnableIRQ(TIM3_IRQn);

    //32M分频3200，计数10000次，产生100ms一次的中断。其他组合配置没有这个精度高
    TIM_InitStruct.Prescaler = 3200;        
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 1000;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM3, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM3);
    LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM3);
    LL_TIM_EnableIT_UPDATE(TIM3);
	LL_TIM_EnableCounter(TIM3);
}

void TIMER_Init(void)
{
	Usr_TIM3_Init(); 
}


//各种以秒为单位进行变量的加减
void TIMER_SecCntHandle(void)
{
	//秒累计
	if (++baseTimeSec > 59)
	{
		//分累计
		baseTimeSec = 0;
		if (baseTimeMin > 59)
		{
			//时累计
			baseTimeMin = 0;
			if (++baseTimeHor > 23) //20150129_1
			{
				baseTimeHor = 0;
			}
		}
	}


	if((baseTimeSec % 30 == 0) && (Flag.InCharging == 0))
	{
		Flag.SensorLed = 1;
	}
	
	WatchDogCnt++;
	if (WatchDogCnt > 30)
	{
		NVIC_SystemReset();
	}

	if (Flag.PwrOnModule)
	{
		if (++CheckModeCnt > 15)
		{
			Flag.SendAtWithoutRDY = 1;
			Flag.ModuleOn = 1;
			Flag.PwrOnModule = 0;
			CheckModeCnt = 0;
		}
	}

	baseSecCnt ++;



	if((UpgInfo.RetryWaitCnt > 0) && (UpgInfo.RetryCnt > 0))
	{
		UpgInfo.RetryWaitCnt --;
		if(UpgInfo.RetryWaitCnt == 0)
		{
			UpgInfo.RetryCnt --;
			UpgInfo.NeedUpdata = 1;
		}
	}


	if ((baseSecCnt % 10 == 4) && (baseSecCnt > 4))
	{
		Flag.CsqChk = 1;		 //查信号强度
		Flag.Bma250NeedInit = 1; //查振动传感器状态
	}
	
	//如果设备没有附着上网络，10秒检测一次CGREG
	if(Flag.PsSignalOk == 0)	
	{	
		if (baseSecCnt % 10 == 2) 
		{
			Flag.PsSignalChk = 1; 		//查询CGREG
			Flag.NeedCheckSIM = 1;		//查询SIM卡状态
		}	
	}
	else
	{
		if (baseSecCnt % 30 == 5)
		{
			Flag.PsSignalChk = 1; 
		}
	}

	if((Flag.IsUpgrate)&&(baseSecCnt % 30 == 4))
	{
		UpgInfo.NeedCheckUploadState = 1;
	}
	
	if(DC_DET)
	{
		Flag.InCharging = 1;
	}
	else
	{
		Flag.InCharging = 0;
	}

	//如果没有同步时间，30检查一次时间，如果时间已经同步，10分钟更新一次时间
	if(Flag.HaveSynRtc == 0)
	{
		if (baseSecCnt % 30 == 6) 
		{
			Flag.NtpGetCCLK = 1;
		}
	}
	else
	{
		if (baseSecCnt % 600 == 6) 
		{
			Flag.NtpGetCCLK = 1;
		}
	}

	//查电量
	if(AT_CBC_IntervalTemp <3)
	{
		AT_CBC_IntervalTemp = 20;
	}

	if (baseSecCnt % AT_CBC_IntervalTemp == 2) 
	{
//		Flag.BatChk = 1; 		
	}

	//查电量
	if (baseSecCnt % 300 == 6) 
	{
		Flag.NeedcheckCCID = 1; 		
	}

	if (ResetLeftCnt > 0)
	{
		ResetLeftCnt--;
		if (ResetLeftCnt == 0)
			Flag.NeedDeviceRst = 1;
	}


	if (!Flag.NoSleep && ActiveTimer > 0 && !Flag.IsUpgrate)
	{
		ActiveTimer--;
	}

	if (AtDelayCnt > 0)
	{
		AtDelayCnt--;
	}

	if (ModePwrDownCnt > 0)
	{
		ModePwrDownCnt--;
	}

	if (GprsSend.posTimer < IntervalTemp)
	{
		GprsSend.posTimer++;
	}
	else
	{
		Flag.ReadySaveBreak = 1;
		GprsSend.posTimer = 0;
		Flag.NeedcheckLBS = 1;
		GprsSend.posFlag = 1;
		#if NO_SLEEP
		GprsSend.posCnt = 1;
		#endif
		printf("It's the time to send MQTT sensor data...\r\n");
	}


	if(ConnectDelayCnt > 0)
	{
		ConnectDelayCnt--;
	}

}

//各种以100毫秒为单位进行变量的加减
void TIMER_BaseCntHandle(void)
{
	if (Uart1RecCnt > 0)
	{
		Uart1RecCnt--;
	}
	if (Uart2RecCnt > 0)
	{
		Uart2RecCnt--;
	}
	if (Uart3RecCnt > 0)
	{
		Uart3RecCnt--;
	}
	if (Uart4RecCnt > 0)
	{
		Uart4RecCnt--;
	}

	if (++ledCnt > 32)
		ledCnt = 0; //周期为4s


	if (Flag.ModuleSleep)
	{
		RED_OFF; 
		GREEN_OFF;
	}
	else if (Flag.IsUpgrate)
	{
		RED_NEG;
		GREEN_OFF;
	}

	else if(Flag.InCharging)
	{
		RED_ON;
	}

	else
	{
		switch (ledCnt)
		{
		case 0:
			GREEN_ON;
			break;
		case 3:
			if (!Flag.HaveGPS)
				GREEN_ON;
			break;
		case 6:
			if (Flag.BattLow)
				GREEN_ON;
			break;
		case 9:
			if (Flag.BattLow)
				GREEN_ON;
			break;
		case 12:
				RED_ON;
			break;

		case 15:
			if (!Flag.GprsConnectOk)
				RED_ON;
			break;
		case 18:
			break;
		case 21:
			break;
		default:
			GREEN_OFF;
			RED_OFF;
			break;
		}
	}
}

unsigned char IsLeap(int nYear )
{
	if ( nYear % 400 == 0 )  return 1;
	else if ( nYear % 100 == 0 ) return 0;
	else if ( nYear % 4 == 0 )  return 1;
	else return 0;
}

//将指定时区时间转换成unix时间（时间戳）
uint32_t Get_Timestamp(Rtc_st *beijingTime,u8 TIMEZONE)
{
	uint32_t daynum=0, SecNum=0; //保存北京时间到起始时间的天数
	uint16_t tempYear=1970, tempMonth=0;
 

	//1.年的天数 
	while(tempYear < (beijingTime->year + 2000)) 
	{
		daynum += 365 + IsLeap(tempYear);	
		tempYear++;
	}
	//2.月的天数
 	while(tempMonth < beijingTime->mon-1) 
 	{
        if(IsLeap(beijingTime->year)){ //闰年
            daynum += Leap_month_day[tempMonth];
        }
        else{
		    daynum += arr_nDays[tempMonth];
        }
		tempMonth++;
	}
    //3.天数
	daynum += (beijingTime->day-1);
 
    //4.时分秒
    SecNum = daynum*24*60*60;    
    SecNum += beijingTime->hour*60*60;    
    SecNum += beijingTime->min*60;    
    SecNum += beijingTime->sec;
 
    //5.时区调整
    SecNum -= TIMEZONE*60*60;
 
    return SecNum;
}

//+CCLK: "20/09/04,15:03:00+32"
void TIME_UpdateRtcByNtp(char *pSrc)
{
	char *p1 = NULL;
	u8 time_zone = 0;

	p1 = strstr(pSrc, "+CCLK: \"") + 8;
	Rtc.year = (unsigned char)Usr_Atoi(p1);
	p1 = strchr(p1, '/') + 1;
	Rtc.mon = (unsigned char)Usr_Atoi(p1);
	p1 = strchr(p1, '/') + 1;
	Rtc.day = (unsigned char)Usr_Atoi(p1);
	p1 = strchr(p1, ',') + 1;
	Rtc.hour = (unsigned char)Usr_Atoi(p1);
	p1 = strchr(p1, ':') + 1;
	Rtc.min = (unsigned char)Usr_Atoi(p1);
	p1 = strchr(p1, ':') + 1;
	Rtc.sec = (unsigned char)Usr_Atoi(p1);
	p1 = strchr(p1, '+') + 1;
	time_zone = (unsigned char)Usr_Atoi(p1);

	time_zone = time_zone/4;
	Timestamp = Get_Timestamp(&Rtc,time_zone);
	printf("The Timestamp is %d\r\n",Timestamp);

	if (!Flag.HaveSynRtc)
	{
		Flag.HaveSynRtc = 1;						 
	}
}


void TIMER_RtcHandle(void)
{
	int nYear;
	char nSolarMonth,nMonthDay;

	nYear=Rtc.year;
	nSolarMonth=Rtc.mon-1;

	if(nSolarMonth==1)
	{
		nMonthDay = IsLeap(nYear) + 28;
	}
	else
	{
		nMonthDay = arr_nDays[nSolarMonth];
	}
	
	Timestamp ++;

	if(++Rtc.sec>59)
	{
		Rtc.sec=0;
		if(++Rtc.min>59)
		{
			Rtc.min=0;
			if(++Rtc.hour>23)
			{
				Rtc.hour=0;
				Rtc.day++;
				if(Rtc.day>nMonthDay)
				{
					Rtc.day-=nMonthDay;
					Rtc.mon++;
					if(Rtc.mon>12)
					{
						Rtc.mon-=12;
						Rtc.year++;
					}
				}
			}
		}
	}
}



/*
 * 函数名称: RTC_Wake_Init
 * 函数说明: RTC唤醒初始化（需要在时钟初始化中打开LSE）
 * 输入参数: 无
 * 返回参数: 无
 */
void RTC_Wake_Init(u16 sec)
{
	/* 设置RTC时钟源 */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTC);
	LL_PWR_EnableBkUpAccess();									//使能对后备寄存器的访问
	/* 使能RTC时钟 */
	LL_RCC_LSI_Enable();
	//等待LSI稳定
	while(LL_RCC_LSI_IsReady() != 1)
	{
	}
	//选择LSI为RTC外设时钟
	LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
	LL_RCC_EnableRTC();
	/* 设置预分频 */
	LL_RTC_SetAsynchPrescaler(RTC, 0x7F);
	LL_RTC_SetSynchPrescaler(RTC, 0xFF);

	/* 失能RTC写保护 */
	LL_RTC_DisableWriteProtection(RTC);
	/* 修改重装载值时需要先禁止唤醒定时器 */
	LL_RTC_WAKEUP_Disable(RTC);
	
	/* 等待WUTWF置1 */
	while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1)
	{
	}
	/* 设置重装载值 */
	LL_RTC_WAKEUP_SetAutoReload(RTC, sec);
	/* 选择唤醒时钟 */
	LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_CKSPRE);
	/* 使能内部唤醒线 */
	LL_PWR_EnableInternWU();  
	/* 配置唤醒中断线21 */
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_19);
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_19); 
	/* 配置唤醒中断优先级 */
	NVIC_SetPriority(RTC_TAMP_IRQn, 2);
	NVIC_EnableIRQ(RTC_TAMP_IRQn);
	/* 清除唤醒标志 */
	LL_RTC_ClearFlag_WUT(RTC);   
	//使能wakeup和中断
	LL_RTC_EnableIT_WUT(RTC);
	LL_RTC_WAKEUP_Enable(RTC);  
	/* 使能写保护 */
	LL_RTC_EnableWriteProtection(RTC);

}
 

void RTC_TAMP_IRQHandler(void)
{
	if(LL_RTC_IsActiveFlag_WUT(RTC) != RESET)
	{
		LL_RTC_ClearFlag_WUT(RTC);
		LL_EXTI_ClearRisingFlag_0_31(LL_EXTI_LINE_19); 
		printf("\r\n----------come to rtc interrupt----------\r\n");
		if (Flag.ModuleSleep)
        {
            Flag.ModuleSleep = 0;
            Flag.ModuleWakeup = 1;
			Flag.IrNoNeedWakeUp = 0;
            ActiveTimer = 100;
			Timestamp += 300;
        } 
	}
}

void TIM3_IRQHandler(void)
{
    if(LL_TIM_IsActiveFlag_UPDATE(TIM3) == SET)
    {
        LL_TIM_ClearFlag_UPDATE(TIM3); 

        if (++baseTimeCnt > 9)
        {
			baseTimeCnt = 0;
			TIMER_RtcHandle();
			TIMER_SecCntHandle();
        }
        TIMER_BaseCntHandle(); 
    }
}

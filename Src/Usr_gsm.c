#include "usr_main.h"

unsigned char ResetMouldeCnt_1; //模块因为没有信号重启次数


void Usr_ModuleGoSleep(void)
{
	#if NO_SLEEP
	return;
	#endif
	
	if (AT_NULL != AtType ||ActiveTimer || Flag.ModuleWakeup ) //Flag.NeedAlarmCall在还有报警电话未拨打时不进入休眠
	{
		return;
	}

	Flag.ModuleWakeup = 0;
	Flag.Insleeping = 1;
	Flag.ModuleSleep = 1;
	Flag.NeedModuleOff = 1;
	GREEN_OFF;
	RED_OFF;
	Usr_ModuleTurnOff();
	printf("\r\nsystem sleep!\r\n");
	LL_mDelay(100);		//留一个时间窗口给串口打印数据
	POWER_OFF;
	GPS_OFF;
	G_Sensor_Pwr(0);
	Sys_Setting_Before_StopMode();
sleep:
	LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
	LL_LPM_EnableDeepSleep();
	__WFI();

	if(Flag.IrNoNeedWakeUp)
	{
		goto sleep;
	}
#if 1
	Usr_InitHardware();
	G_Sensor_Pwr(1);
#else
    SystemClock_Config();
	StopMode_TurnOn_Some_GPIOs();
	UART_Init();
	TIMER_Init();
	IIC_Init();
//	EXFLASH_SpiInit();
#endif
}

void Usr_ModuleWakeUp(void)
{

	if (!Flag.ModuleWakeup)
		return;

	Flag.ModuleWakeup = 0;
//	Flag.BatChk = 1;
	Flag.HaveSynRtc = 0;
	Flag.NtpGetCCLK = 1;

	GprsSend.posCnt = 1;
	GprsSend.posFlag = 1;
	Flag.NeedModuleOn = 1;

	LL_mDelay(200);

	printf("\r\nmodule wake up!\r\n");
	Flag.ModuleSleep = 0;
	Flag.NeedSendToSleep = 1;
	AtType = AT_NULL;
}

void Usr_ModuleTurnOn(void)
{
	unsigned char i = 3;

	if (!Flag.NeedModuleOn)
	{
		return;
	}
 
	Flag.NeedModuleOn = 0;

	printf("Ready turn on the GSM module\r\n");
	POWER_OFF;
	delay_ms(1000);

	POWER_ON; 		
	while (i--)
	{
		delay_ms(1000);
		printf("-");
	}

	//重新上电
	GSM_KEY_OFF; //模块PWRKEY为高
	delay_ms(300);
	GSM_KEY_ON; //模块PWRKEY为低
	delay_ms(1500);
	GSM_KEY_OFF; //模块PWRKEY为高
	delay_ms(300);

	delay_ms(2000);		//延时两秒，之后准备连续发送AT指令唤醒模块
//	Flag.NeedWakeMdByAt = 1;
	Flag.RcvAtAckOK = 0;
	Flag.HaveSmsReady = 0;
	Flag.HaveGetCCID = 1;
	Flag.PwrOnModule = 1;
	Flag.NeedSetNtp = 1;
	Flag.AtInitFinish = 0;

	ActiveTimer = ACTIVE_TIME;	//模块开机后需要较长时间的初始化和联网，保证进入休眠前完成这些操作
}

void Usr_ModuleTurnOff(void)
{
	if (!Flag.NeedModuleOff)
	{
		return;
	}
	printf("Ready turn off the GSM module\r\n");
	Flag.HaveSmsReady = 0;
	Flag.NeedModuleOff = 0;
	Flag.ModuleOn = 0;
	Flag.PsSignalOk = 0; 
	Flag.ModePwrDownNormal = 0;
	Flag.IsContextAct = 0;
	Flag.GprsConnectOk = 0;
	Flag.WaitAtAck = 0;
	Flag.AtInitFinish = 0;
	Flag.HaveSynRtc = 0;
	AtDelayCnt = 0;

	//先拉低引脚关模块
	GSM_KEY_OFF; //模块PWRKEY为高
	delay_ms(500);
	GSM_KEY_ON; //模块PWRKEY为低
	delay_ms(1500);
	GSM_KEY_OFF; //模块PWRKEY为高

	Flag.HavePwdMode = 1;
	ModePwrDownCnt = 10;
	AtType = AT_NULL;

	GREEN_ON;
	while (Flag.HavePwdMode)
	{
		if ((ModePwrDownCnt <= 0) || Flag.ModePwrDownNormal)
		{
			POWER_OFF;
			GREEN_OFF;
			delay_ms(500);
			Flag.HavePwdMode = 0;
			printf("\r\nModule turn off!\r\n");
		}
		LL_mDelay(100);
	}
	UART_AtInit();	//有的时候关机前AT串口接收到了数据没有处理，重启开机后，这个数据会干扰程序判断
}

void Usr_ModuleReset(void)
{
	//有需要时重启模块
	if (NeedModuleReset)
	{
		//如果是因为持续附着不到网络重启模块，连续重启三次后，认为处于没有网络的地方，不再重启。如检测到震动，认为有可能离开
		//该区域，回复检测和重启机制
		if (NeedModuleReset == CANT_ATTACH_NET)		
		{
			printf("Can't registere net for a long time,need restart GSM module\r\n");
			ResetMouldeCnt_1++;
			if ((ResetMouldeCnt_1 >= 2) && (NoShockCnt > 300))
			{
				Flag.InNoSignNoRstMd = 1;
			}
		}
		else if(NeedModuleReset == CONNECT_SERVICE_FAILED)
		{
			printf("There some wrong in connect to service,need restart GSM module\r\n");
		}
		else if(NeedModuleReset == NO_SIMCARD)
		{
			printf("Didn't read SIM card,need restart GSM module\r\n");
		}
		else if(NeedModuleReset == MODUAL_INFO_ERROR)
		{
			printf("SIM7080G turn on but no ack,need restart module\r\n");
		}
		else if(NeedModuleReset == MODUAL_NOACK)
		{
			printf("GSM module no response,need restart GSM module\r\n");
		}

		printf("\r\n---->module reset:%d\r\n", NeedModuleReset);
		NeedModuleReset = 0;

		if (Flag.NeedClrValueFile)
		{
			Flag.NeedClrValueFile = 0;
			FS_FactroyValueFile();
		}

		Flag.NeedModuleOff = 1;
		Usr_ModuleTurnOff();

		Flag.NeedModuleOn = 1;
		Usr_ModuleTurnOn();
	}
}

//设备关机: gsm模块断电 单片机进入休眠
//或设备重启
void Usr_DevicePwrHandle(void)
{
	if (Flag.NeedClrValueFile) 
	{
		Flag.NeedClrValueFile = 0;
		FS_FactroyValueFile();
	}

	if (Flag.NeedDeviceRst)
	{
		Flag.NeedDeviceRst = 0;
		printf("\r\nDevice RESET!");

		Flag.NeedModuleOff = 1;
		Usr_ModuleTurnOff();

		NVIC_SystemReset();
	}
}

void Usr_Device_ShutDown(void)
{
	if(Flag.NeedShutDown == 0)
	{
		return;
	}


	printf("\r\nLow battery,Device ShutDown!\r\n");

	Flag.NeedModuleOff = 1;
	Usr_ModuleTurnOff();				//关模块
	GPIO_Init_Before_Shutdown();		//设置GPIO状态

	LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
	LL_LPM_EnableDeepSleep();			//进入停止模式
	__WFI();

	NVIC_SystemReset();					//外部充电中断唤醒后直接重启
}

void Usr_DeviceContral(void)
{
	Usr_ModuleGoSleep(); 

	Usr_ModuleWakeUp(); 

	Usr_ModuleTurnOn();

	Usr_ModuleReset();

	Usr_DevicePwrHandle();

	Usr_Device_ShutDown();
}

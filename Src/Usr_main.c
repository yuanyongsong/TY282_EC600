
#include "usr_main.h"

/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
FLAG Flag;
unsigned short ResetCnt = 0;
unsigned short WaitAtTime;		//等待AT超时时间，默认是75，可以根据不同AT指令修改
unsigned char WatchDogCnt = 0; 
unsigned char AtTimeOutCnt; 	//AT超时次数，超时三次重启模块
unsigned char NeedModuleReset;
unsigned int  NoShockCnt;		//没有振动计时，用于处理地下停车场长期无网络时不循环重启模块问题及进入深度睡眠
unsigned char ModePwrDownCnt;	//执行关机操作后等待模块回应关机消息倒计时
unsigned char CheckModeCnt;		//模块开机后，等待主动上报内容，超过10秒，跳过等待，直接开始发送AT指令
unsigned char WorkMode;			//挖掘机工作模式：1为静止模式，2为怠速模式，3为工作模式
unsigned short IdlingKeepTime;	//怠速持续时间，超过5分钟认为是进入工作状态
const unsigned char SoftwareBuilt[50] = {0};
char Edition[50] = {0};


u8 Built_year[5] = {'\0'};
u8 Built_mon[3] = {'\0'};
u8 Built_day[3] = {'\0'};
u8 Built_hour[3] = {'\0'};
u8 Built_min[3] = {'\0'};

void Usr_InitHardware(void);
void Usr_InitValue(void);
void Flag_Check(void);

void time_convert(void)
{
	Built_year[0] = BUILD_YEAR_CH2;
	Built_year[1] = BUILD_YEAR_CH3;

	Built_mon[0] = BUILD_MONTH_CH0;
	Built_mon[1] = BUILD_MONTH_CH1;

	Built_day[0] = BUILD_DAY_CH0;
	Built_day[1] = BUILD_DAY_CH1;

	Built_hour[0] = BUILD_HOUR_CH0;
	Built_hour[1] = BUILD_HOUR_CH1;

	Built_min[0] = BUILD_MIN_CH0;
	Built_min[1] = BUILD_MIN_CH1;
}

int main(void)
{
	time_convert();
	Usr_InitHardware();
	printf("\r\n============================================");
	printf("\r\n================w609_EC600S=================");
	sprintf(Edition, "w609_EC600S_%s%s%s%s%s", Built_year, Built_mon, Built_day, Built_hour, Built_min);
	printf("\r\n==software built time:%s %s==", __DATE__, __TIME__);
	printf("\r\n============================================\r\n");
	Usr_InitValue();
	while (1)
	{
		Usr_DeviceContral();
		UART_Handle();
		WIRELESS_Handle();
		GPS_Handle();
		FS_UpdateValue();
		Flag_Check();
		WatchDogCnt = 0;
	}
}

void Usr_InitHardware(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    SystemClock_Config();
	//开启停止模式下的debug时钟会使功耗增加28mA左右，同时可以在停止模式下烧录程序。该设置掉电保存
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DBGMCU);		//开启停止模式下debug时钟
	LL_DBGMCU_EnableDBGStopMode();								//使能停止模式时debug功能
	LL_DBGMCU_DisableDBGStopMode();

	GPIO_init();
	UART_Init();
	TIMER_Init();
	IIC_Init();

	//注意这里，ADC从停止模式唤醒时，不需要再次初始化
	if(Flag.Insleeping == 0)
	{
		Adc_init();
	}
}


void Usr_InitValue(void)
{
//	unsigned char i;

	GprsSend.posCnt = 1;
	GprsSend.posFlag = 1;

//	Flag.NeedLogIn = 1;
	Flag.NeedGpsOpen = 1;
	Flag.NeedSendToSleep = 1;
	Flag.NeedModuleOn = 1;
	Flag.NeedcheckLBS = 1;
	Flag.PorOldOn = 1; 		//电池供电没有接外部供电时，不产生断电报警，只有在接过供电再断电时才产生断电报警
	Flag.WakeUpMode = 1;
	Flag.NeedcheckCCID = 1;
	Flag.NtpGetCCLK = 1;
	Flag.NeedSetNtp = 1;
	Flag.NeedGetIMEI = 1;
	Flag.NeedcheckCCID = 1;
	Flag.NeedReloadAgps = 1;
	Flag.NeedGetMccMnc = 1;
	AT_CBC_IntervalTemp = 20;
//	Flag.NeedScanWifi = 1;
	GprsSend.handFlag = 1;		//开机时需要先发送一个心跳包，在设备连接上平台时就可以发送一个数据

	ActiveTimer = ACTIVE_TIME;
	WorkMode = 2;				//设备开机认为是在怠速状态
	AtType = AT_NULL;
	Flag.WaitAtAck = 0;
	WatchDogCnt = 0;

	FS_InitValue();			

	//如果设备ID没有初始化，这里自动串口参数设置模式，等待通过串口初始化
	if(memcmp(Fs.UserID,"00000000000",11) == 0)		
	{
		Flag.DeviceInSetting = 1;
		printf("Device ID not set,please set device ID frist!\r\n");
	}	

	if(Fs.ModeSet & SETTING_MODE)
	{
		Flag.DeviceInSetting = 1;
	}

	while(Flag.DeviceInSetting)
	{
		if (Flag.Uart3HaveData && !Uart3RecCnt)
		{
			Debug_Receive();
			UART_DebugInit();
		}
		WatchDogCnt = 0;
		delay_ms(10);
	}

	memset(&Fs.FsUpg,0,sizeof(Fs.FsUpg));
  	//EXFLASH_ReadBuffer((u8 *)&FsUpg,FLASH_UPG_ADDR,sizeof(FsUpg));
	//远程升级成功,需要发送升级成功数据
	if(Fs.FsUpg.UpgEnJamp==0xAA)		
	{
		Flag.NeedSendUpgResult = 1;
		//清除升级结果
		memset(&Fs.FsUpg,0,sizeof(Fs.FsUpg));
		Flag.NeedUpdateFs = 1;
		printf("\r\nUpgrade App success!\r\n");
	}

	if(Fs.HaveCertificate == 0xAA)
	{
		Flag.ModuleHasCA = 1;
	}

	//如果灵敏度值过低，例如等于0，传感器会一直输出高电平，单片机无法检测到中断
	if(Fs.Sensor < 0x02)		
	{
		Fs.Sensor = 0x4;
	}

	memset(UserIDBuf,0, sizeof(UserIDBuf));
	strncpy(UserIDBuf,Fs.UserID, sizeof(UserIDBuf)); 

	//以下是需要获取到配置参数才能初始化的设备外设

	RTC_Wake_Init(60);				//1分钟产生一次闹钟事件

	if(G_Sensor_init())
	{
		Flag.GsensorInitOk = 1;
		printf("G sensor init ok\r\n");
	}
	else
	{
		printf("G sensor init failed\r\n");
	}
	
}

void Flag_Check(void)
{
	if(UpgInfo.UpgrateFail)
	{
		Flag.IsUpgrate = 0;
		Flag.UpgrateFailed = 1;
		Flag.NeedSendUpgResult = 1;
		UpgInfo_InitValue();
	}

	if(Flag.NeedGetBatVoltage)
	{
		Flag.NeedGetBatVoltage = 0;
		BatVoltage_Adc = (u32)Adc_Value_Get();
		BatVoltage_Adc = (u16)(BatVoltage_Adc * 478/100);		//转换成电池电压,1M和270k分压，采样值*（1.27/0.27）=采样值*4.7,修正到4.78
		printf("\r\nThe battery voltage is %d mv\r\n",BatVoltage_Adc);
	}

	if(UpgInfo.NeedWaitUpgrade)
	{
		UpgInfo.HaveGetRankData = 0;
		UpgInfo.NeedWaitUpgrade = 0;
		UpgInfo.NeedUpdata = 1;				//需要开始升级
		UpgInfo.RetryCnt = 2;				//升级失败重复次数

		printf("Need upgrade the device,upgrade file name is: %s\r\n",Fs.FsUpg.AppFilePath);

		// Flag.NeedResponseFrist = 1;			//需要首先应答平台消息后在开始升级
		// Flag.NeedSendResponse = 1;
		// sprintf(RespServiceBuf,"Fota file name is :%s,ready upgrade...",FsUpg.AppFilePath);
	}
	
	if(Flag.NeedPrintf)
	{
		Flag.NeedPrintf = 0;
		printf("ActiveTimer:%d\r\n",ActiveTimer);
	}

	if(Flag.GsensorNeedInit)
	{
		Flag.GsensorNeedInit = 0;
		if(G_Sensor_init())
		{
			Flag.GsensorInitOk = 1;
			printf("G sensor init ok\r\n");
		}
		else
		{
			printf("G sensor init failed\r\n");
		}
	}

}


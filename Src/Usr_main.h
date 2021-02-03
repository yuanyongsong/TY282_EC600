#ifndef USR_MAIN_H
#define USR_MAIN_H

#include "stm32g0xx_ll_adc.h"
#include "stm32g0xx.h"
#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_spi.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_rtc.h"
#include "stm32g0xx_ll_flash.h"

#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "Usr_at.h"
#include "Usr_flash.h"
#include "Usr_gprs.h"
#include "Usr_gpio.h"
#include "Usr_timer.h"
#include "Usr_uart.h"
#include "Usr_exflash.h"
#include "Usr_gsm.h"
#include "Usr_Upgrade.h"
#include "Usr_iic.h"
#include "Usr_gps.h"
#include "Usr_adc.h"
#include "Usr_Upgrade.h"
#include "MD5.h"


#define SENSOR_HIGH         40
#define SENSOR_NORMAL       60
#define SENSOR_LOW          80

#define NO_SENSOR           0x01
#define AUTO_SHUTDOWN       0x02

#define SETTING_MODE		0x04

typedef enum
{
FALSE =0,
TRUE =!FALSE
} bool;

#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define BUILD_MONTH_CH0 \
((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_MONTH_CH1 \
( \
(BUILD_MONTH_IS_JAN) ? '1' : \
(BUILD_MONTH_IS_FEB) ? '2' : \
(BUILD_MONTH_IS_MAR) ? '3' : \
(BUILD_MONTH_IS_APR) ? '4' : \
(BUILD_MONTH_IS_MAY) ? '5' : \
(BUILD_MONTH_IS_JUN) ? '6' : \
(BUILD_MONTH_IS_JUL) ? '7' : \
(BUILD_MONTH_IS_AUG) ? '8' : \
(BUILD_MONTH_IS_SEP) ? '9' : \
(BUILD_MONTH_IS_OCT) ? '0' : \
(BUILD_MONTH_IS_NOV) ? '1' : \
(BUILD_MONTH_IS_DEC) ? '2' : \
/* error default */ '?' \
)

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])



#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])

#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])

#define BUILD_SEC_CH0 (__TIME__[6])
#define BUILD_SEC_CH1 (__TIME__[7])



#ifndef _FLAG_
#define _FLAG_
typedef struct FLAG_
{
	unsigned char PsSignalOk:1; 	  //置位表示ps网络(internet)注册成功
	unsigned char BattLow:1;          //电池电压低
	unsigned char BatChk:1;
	unsigned char GprsConnectOk:1;	  //tcp已连接 或请求基站定位时已经打开网络
	unsigned char PsSignalChk:1;	  //定期检查ps网络注册，主要是用来区分CGREG是由于需要联网触发还是周期查询
	unsigned char NeedCloseGprs:1;    //需要断开gprs连接
	unsigned char CheckSignal:1;  
	unsigned char ReadySaveBreak:1;   //准备保存定位包数据，如果此时网络是未链接的，保存数据，如果网络是链接的，清零
	unsigned char ConnectReady:1;     //置位表示需要或可以进行GPRS连接

	unsigned char NeedUpdateFs:1;     //需要更新Fs结构体
	unsigned char FeedbackGprs:1;     //需要应答平台下发数据
	unsigned char ModuleSleep:1;      //模块进入休眠标记
	unsigned char NeedGetMccMnc:1;    //已经获取MCCMNC
	unsigned char HaveGetMccMnc:1;    //已经获取MCCMNC
	unsigned char HaveCommonMccMnc:1; //已经同步MCCMNC
	unsigned char UpgrateAppOk:1;      //升级成功标志
	unsigned char CsqChk:1;     	  //定期查CSQ信号强度

	unsigned char IsSendingBk:1;      //正在发送断点时，也要保存当前的位置
	unsigned char NeedClrValueFile:1; //初始化参数
	unsigned char OtherSendPosi:1; 	  //其它需要上传数据包的情况

	unsigned char WaitAtAck; 		  //置位等待at指令的回复
	unsigned char ConNet:1;        	  //实际TCP连接为Fs.IpAdress


	unsigned char NoSleep:1;          //有任务处理，暂时不休眠	  20140902_5
	unsigned char HaveSynRtc:1;		  //已经将rtc时间与当前时区同步 20141107_2
	unsigned char IsUpgrate:1;        //远程升级模式
	unsigned char UpgrateFailed:1;    //远程升级失败
	unsigned char ModuleOn:1; 		  //模块成功开机
	unsigned char PwrOnModule:1;      //已经执行模块开机动作，至于模块有没有成功开机还需要继续判断，模块没有RDY上报时使用
	unsigned char HavePwrOff:1;		  //设备关机
	unsigned char HaveDcIn:1;		  //有dc输入 充电
	unsigned char NeedModuleOff:1;	  //gsm模块关机
	unsigned char NeedModuleOn:1;	  //gsm模块开机
	unsigned char Mode3WakeUp:1;	  //模式3设置时间到开机
	unsigned char NeedDeviceRst:1;	  //设备重启 一般用于不可预见异常 如模块死机等

	unsigned char ModuleWakeup:1;	  //模块退出休眠
	unsigned char Insleeping:1 ;       //设备处于休眠状态，只是中途唤醒
	unsigned char IrNoNeedWakeUp:1;	  //红外从休眠模式退出时不需要唤醒系统
	unsigned char GsensorNeedInit:1;  //需要初始化Gsnesor
	unsigned char GsensorInitOk:1;	  //Gsensor初始化成功
	unsigned char IsContextAct:1;	  //置位表示场景被激活
	unsigned char HaveSmsReady:1;	  //已经出现READY,再次出现ready时不重新走初始化AT流程
	unsigned char AtInitCmd:1;
	unsigned char AtInitFinish:1;
	unsigned char GpsOneDataOk:1;


    unsigned char NeedSendToSleep:1;
	unsigned char NeedCloseNetForSleep:1;     	//进入睡眠模式前，关闭网络
	unsigned char PorOldOn:1;       			//初始供电情况标志。为1代表无外部供电，电池启动工作。
	unsigned char NtpGetCCLK:1;					//通过发送CCLK查询网络时间
	unsigned char NeedSetNtp:1;					//需要设置NTP服务器
	unsigned char NeedcheckCCID:1;
	unsigned char HaveGetCCID:1;
	unsigned char WakeUpMode:1;                	//模式从休眠中唤醒
	unsigned char NeedUpgrade:1;	
	unsigned char Sendupgradestart:1;
	unsigned char NeedUpgradeResultResponse:1;       //远程升级结果应答
	unsigned char recvgprs:1;                  //有gprs接收到数据
	unsigned char NeedcheckLBS:1;              //查询基站位置
	unsigned char HavePwdMode:1;               //已经发送关机，等待模块上报正常关机
	unsigned char ModePwrDownNormal:1;         //收到模块关机应答，可以断电
	unsigned char InNoSignNoRstMd:1;           //设备处于没有信号的地方，不再因为没有信号重启模块
	unsigned char NeedPrintTftpError_1:1;      //需要打印TFTP升级中的错误
	unsigned char NeedPrintTftpError_2:1;      //需要打印TFTP升级中的错误
	unsigned char SendAtWithoutRDY:1;          //在等待一断时间后，还没有收到RDY，直接开始发送AT指令
	unsigned char DeviceIsPwrOff:1;			   //设备当前处于关机模式
	unsigned char StopModeOpenGprs:1;		   //停止模式期间链接网络标志
	unsigned char HaveShock:1;       		   //无震动标志位。用于判断是否为连续震动，工作模式一唤醒设备时使用
	unsigned char BaseShock:1;				   //连续震动时间段内有震动标志


	unsigned char NoSimShutdown:1;			   //为1表示当前时因为没有SIM卡关机的
	unsigned char NeedCheckSIM:1;			   //需要检查SIM卡状态
	unsigned char Mode4WakeUp:1;			   //模式4时，休眠后指定时间后自动唤醒标志
	unsigned char NeedTerminalResponse:1;      //需要应答平台（通用应答）
	unsigned char NeedUpgradeResponse:1;       //远程升级应答
	unsigned char GPRSChangeid:1;			   //GPRS下发修改ID，这个时候需要用旧的ID应答，不能立即更新IDbuffer
	unsigned char NoNeedAct:1;                 //收到下发指令是需要做动作，为0时不需要动作，只需要应答  在收到重复下发控制指令时使用	
	unsigned char ModuleHasCA:1;			   //模块已经烧录了CA证书，连接服务器时无需再次烧录	

	unsigned char Uart1HaveData:1;          	//串口1有数据
	unsigned char Uart2HaveData:1;	 	    	//串口2有数据	
	unsigned char Uart3HaveData:1;          	//串口3有数据
    unsigned char Uart4HaveData:1;          	//串口4有数据

	unsigned char NeedWakeMdByAt:1;				//需要在开机后通过连续发送AT指令唤醒模块
	unsigned char RcvAtAckOK:1;          		//开机后连续发送AT指令，接收到模块应答OK
	unsigned char NeedGetIMEI:1;				//需要查询IMEI
	unsigned char NeedLogIn:1;					//需要发送登入包
	unsigned char NeedChangeSoftSim:1;			//需要修改softsim卡号
	unsigned char InCharging:1;					//设备出于充电状态
	unsigned char ChargeOver:1;					//设备充满电
	unsigned char SensorLed:1;					//传感器指示sensor灯，为1时表示需要亮一下

	unsigned char SysShutDown:1;				//系统关机
	unsigned char NeedShutDown:1;				//电池电压达到或者低于设定值时，关机
	unsigned char NeedSendUpgResult:1;			//需要发送升级结果

	unsigned char HaveGPS:1;          			//置位表示GPS信号有效 实时性的
	unsigned char SetDegreeData:1;    			//保存第一个角度数据，用于以后角度变化判断
	unsigned char IsGpsOn:1;          			//标记是否给gps模块上电
	unsigned char NeedReloadAgps:1;   			//需要重新加载AGPS数据
	unsigned char GpsReseting:1;      			//GPS重启中，此时AGPS需要重新加载，配合AGPS使用
	unsigned char HaveGetGpsOk:1;	  			//置位表示已获取最新定位 有些应用获取最后定位即可关gps
	unsigned char NeedResetGps:1;	  			//重启gps
	unsigned char GpsGprmcOk:1;					//获取到GPRM数据
	unsigned char GpsGpggaOk:1;					//获取到GPGG数据
	unsigned char NeedGpsOpen:1;      			//需要开gps
	unsigned char LatFlag:1;          			//0:N北纬 1:S南纬
	unsigned char LonFlag:1;          			//0:E东经 1:W西经	
	unsigned char FristGetGps:1;      			//首次获取到gps，用于获取系统时间

	unsigned char AskUbloxData:1;	  			//请求ublox assistonnowline
	unsigned char ConUblox:1;         			//TCP连接到ublox服务器
	unsigned char NeedCloseAgpsConnect:1;   	//需要关闭AGPS网络
	unsigned char NoSleepMode:1;				//当设定的上传时间间隔小于一定值时，设备不休眠
	unsigned char RtcInterrupt:1;				//有RTC中断触发标志，用来模拟设备进入stop模式

	unsigned char NeedGetBatVoltage:1;			//需要查询ADC电池电压
	unsigned char DeviceInDeepSleep:1;			//设备进入深度休眠，不再唤醒
	unsigned char NeedScanWifi:1;				//需要扫描周围wifi
	unsigned char GetScanWifi:1;				//扫描到周围有效wifi信号
	unsigned char InNoShockSleep:1;				//进入周期性睡眠时间段时，设备休眠，无法通过振动传感器唤醒
	unsigned char NeedCloseGsensor:1;			//需要关闭G sensor
	unsigned char GsensorClose:1;				//G sensor关闭
	unsigned char NeedPrintf:1;					//需要打印数据
	
	unsigned char DeviceInSetting:1;			//设备出于设置模式

	unsigned char NeedSendCimi:1;				//需要查询IMSI
	unsigned char NeedSendCnum:1;				//需要查询SIM卡卡号
	unsigned char HaveSendLowPower:1;			//已经发送低电告警短信，未充电无需再次发送
	unsigned char NeedRspGprs:1;				//需要应答平台下发信息
	unsigned char NeedRspDevInfo:1;				//需要应答设备信息
	unsigned char NeedLocateByGprs:1;			//平台下发需要定位指令
	unsigned char InRealTimeLocate:1;			//实时数据定位标志

}FLAG;
#endif

extern unsigned char  AtTimeOutCnt;
extern unsigned char  WatchDogCnt;
extern unsigned short ResetCnt;
extern FLAG Flag;
extern unsigned char NeedModuleReset;
extern unsigned char  CheckModeCnt;
extern unsigned int  NoShockCnt;
extern unsigned char ModePwrDownCnt;
extern unsigned short WaitAtTime;
extern char Edition[50];

void Usr_ModuleWakeUp(void);
void Usr_ModuleTurnOff(void);
void Usr_InitHardware(void);

#endif




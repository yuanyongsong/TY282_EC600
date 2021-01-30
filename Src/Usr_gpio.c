#include "usr_main.h"

unsigned char KeyDefaultCnt;
unsigned char KeyPwrActCnt;
unsigned char ledCnt;
short         BreathCnt;
unsigned char BreathDir;
unsigned short BreathData;
unsigned char SysPoweKeyCnt;		//系统开关机按键按下计数
unsigned char SysPoweKeyTimer;		//系统开关机按键按下计时
unsigned char KeyShocksTimer;		//按键短按判断，判断条件是间隔时间小于1秒

void GPIO_Hand(void)
{
}

//振动和碰撞检测中断
void GPIO_ExintInit(void)
{

}

void Shock_DeInit(void)
{

}

void ALLGPIO_SET_AIN(void)
{

}

//所有GPIO在关机模式下全部配置成浮空输入，仅保持开机和SIM卡中断开启，如开机，将唤醒单片机，重启设备
void GPIO_ExintPwrInit(void)
{

}


void GPIO_init(void)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOD);


    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*二氧化碳传感器准备好引脚，低电平有效*/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);



    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    

    /*
    GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    */
    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /**/
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTA, LL_EXTI_CONFIG_LINE0);

    /**/
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTD, LL_EXTI_CONFIG_LINE3);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_3;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);


    /**/
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(GPIOD, LL_GPIO_PIN_3, LL_GPIO_PULL_UP);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_3, LL_GPIO_MODE_INPUT);

    NVIC_SetPriority(EXTI0_1_IRQn, 3);
    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI2_3_IRQn, 3);
    NVIC_EnableIRQ(EXTI2_3_IRQn);


    //模块数据接收中断
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTC, LL_EXTI_CONFIG_LINE13);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
    NVIC_EnableIRQ(EXTI4_15_IRQn);

    RED_ON;
    MODULE_WAKEUP_RESET;
}



//外部中断配置，用于系统进入休眠前配置，用于唤醒系统
void Exit_GPIO_Interrupt_Init(void)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOD);

    //红外中断PD1，上升沿触发
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTD, LL_EXTI_CONFIG_LINE1);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_1;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    LL_GPIO_SetPinPull(GPIOD, LL_GPIO_PIN_1, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_1, LL_GPIO_MODE_INPUT);
    NVIC_SetPriority(EXTI0_1_IRQn, 3);
    NVIC_EnableIRQ(EXTI0_1_IRQn);

    //模块数据接收中断
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTC, LL_EXTI_CONFIG_LINE13);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}


void GPIO_Init_Before_Shutdown(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_ALL);

    //不关闭PA13和PA14
    GPIO_InitStruct.Pin = (~(LL_GPIO_PIN_0|LL_GPIO_PIN_13|LL_GPIO_PIN_14)); 
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; 
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //蓝牙芯片供电使能，PB13
    GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //二氧化碳传感器供电使能脚，PC4；模块供电维持，PC7；
    GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //G sensor使能脚保持开启
    GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#if 1
    //关机前开启PB15，外部供电唤醒中断
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTB, LL_EXTI_CONFIG_LINE15);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_15;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_15, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_15, LL_GPIO_MODE_INPUT);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
#endif

    RTC_Close();         //需要关闭RTC
}




void StopMode_TurnOff_Some_GPIOs(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_ALL);

    //不关闭PA13和PA14
    GPIO_InitStruct.Pin = (~(LL_GPIO_PIN_0|LL_GPIO_PIN_13|LL_GPIO_PIN_14)); 
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; 
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //蓝牙芯片供电使能，PB13
    GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //模块供电维持，PC7；如果是设备休眠时模块休眠模式，应保留该位状态
#if MODULE_OFF_MODE
    GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
#else
    GPIO_InitStruct.Pin = (~(LL_GPIO_PIN_7));
#endif
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //G sensor使能脚保持开启
    GPIO_InitStruct.Pin = (~(LL_GPIO_PIN_3));
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#if 0
    //震动传感器脚维持中断
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTD, LL_EXTI_CONFIG_LINE3);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_3;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    LL_GPIO_SetPinPull(GPIOD, LL_GPIO_PIN_3, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_3, LL_GPIO_MODE_INPUT);
    NVIC_SetPriority(EXTI2_3_IRQn, 2);
    NVIC_EnableIRQ(EXTI2_3_IRQn);
#endif
}

void StopMode_TurnOn_Some_GPIOs(void)
{
    GPIO_init();
    POWER_ON;
}


//系统进入停止模式前需要设置的内容
void Sys_Setting_Before_StopMode(void)
{

	//LL_SYSTICK_DisableIT();
	StopMode_TurnOff_Some_GPIOs();
//	Exit_GPIO_Interrupt_Init();

}


void EXTI0_1_IRQHandler(void)
{

    if (LL_EXTI_IsActiveFallingFlag_0_31(LL_EXTI_LINE_0) != RESET)
    {
        LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_0);

        Flag.RtcInterrupt = 1;

        if(!Flag.SysShutDown)
        {
            if(KeyShocksTimer > 0)          //按键短按判断
            {
                KeyShocksTimer = 10;
                SysPoweKeyCnt ++;
            }
            else                            //如果触发时，上次按下已经超过1秒，重新计数
            {
                KeyShocksTimer = 10;
                SysPoweKeyCnt = 1;
            }
        }
    }

    if (LL_EXTI_IsActiveRisingFlag_0_31(LL_EXTI_LINE_1) != RESET)
    {
        LL_EXTI_ClearRisingFlag_0_31(LL_EXTI_LINE_1);
        if(baseSecCnt<2)   return;          //刚开机时会有一个中断产生，需要过滤

        if(Flag.IrNoNeedWakeUp == 0)
        {
            printf("\r\n----------> Infrared detected! <-----------\r\n\r\n");
        }

    }

}



void EXTI2_3_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFallingFlag_0_31(LL_EXTI_LINE_3) != RESET)
    {
        LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_3);
        if(Flag.ModuleSleep)
        {
            if(Flag.DeviceInDeepSleep)
            {
                WakeUpType = 3;
                Flag.DeviceInDeepSleep = 0; 
                Flag.ModuleSleep = 0;
                Flag.ModuleWakeup = 1;
			    ActiveTimer = 120;
                Flag.RtcInterrupt = 1;
            }
        }

        //如果是因为上传时间间隔短的不休眠模式，在持续无振动180秒后设备也需要进入休眠状态
        if(Flag.NoSleepMode)
        {
            if(Flag.ModuleSleep)
            {
                WakeUpType = 3;
                Flag.ModuleSleep = 0;
                Flag.ModuleWakeup = 1;
            }

            ActiveTimer = 180;
        }

        if(NoShockCnt >= 120)
        {
            HaveAlarmGprsType |= HAVE_SHOCK_ALARM;
            printf("Shock Actived, Need send gprs alarm\r\n");

            if(Flag.ModuleSleep)
            {
                WakeUpType = 3;
                Flag.ModuleSleep = 0;
                Flag.ModuleWakeup = 1;
                ActiveTimer = 180;
                Flag.RtcInterrupt = 1;
            }
        }

        NoShockCnt = 0;
    }
}


void EXTI4_15_IRQHandler(void)
{
    if (LL_EXTI_IsActiveRisingFlag_0_31(LL_EXTI_LINE_13) != RESET)
    {
        LL_EXTI_ClearRisingFlag_0_31(LL_EXTI_LINE_13);
    }

    if (LL_EXTI_IsActiveRisingFlag_0_31(LL_EXTI_LINE_15) != RESET)
    {
        LL_EXTI_ClearRisingFlag_0_31(LL_EXTI_LINE_15);
        Flag.NeedDeviceRst = 1;
    }
}


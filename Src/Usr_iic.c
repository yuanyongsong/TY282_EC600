#include "Usr_iic.h"


void IIC_GPIO_Init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct;

	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void IIC_Init(void)
{
	IIC_GPIO_Init();

	SDA_HIGH;
	delay_us(10);
	SCL_HIGH;
	delay_us(10);
}

//产生IIC起始信号
void IIC_Start(void)
{
	SET_SDA_OUT; //sda线输出
	SDA_HIGH;
	SCL_HIGH;
	delay_us(4);
	SDA_LOW; //START:when CLK is high,DATA change form high to low
	delay_us(4);
	SCL_LOW; //钳住I2C总线，准备发送或接收数据
}
//产生IIC停止信号
void IIC_Stop(void)
{
	SET_SDA_OUT; //sda线输出
	SCL_LOW;
	SDA_LOW; //STOP:when CLK is high DATA change form low to high
	delay_us(4);
	SCL_HIGH;
	SDA_HIGH; //发送I2C总线结束信号
	delay_us(4);
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	SET_SDA_IN; //SDA设置为输入
	SDA_HIGH;
	delay_us(1);
	SCL_HIGH;
	delay_us(1);
	while (SDA_ST)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	SCL_LOW; //时钟输出0
	return 0;
}
//产生ACK应答
void IIC_Ack(void)
{
	SCL_LOW;
	SET_SDA_OUT;
	SDA_LOW;
	delay_us(2);
	SCL_HIGH;
	delay_us(2);
	SCL_LOW;
}
//不产生ACK应答
void IIC_NAck(void)
{
	SCL_LOW;
	SET_SDA_OUT;
	SDA_HIGH;
	delay_us(2);
	SCL_HIGH;
	delay_us(2);
	SCL_LOW;
}
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(u8 txd)
{
	u8 t;
	SET_SDA_OUT;
	SCL_LOW; //拉低时钟开始数据传输
	for (t = 0; t < 8; t++)
	{
		//IIC_SDA=(txd&0x80)>>7;
		if ((txd & 0x80) >> 7)
			SDA_HIGH;
		else
			SDA_LOW;
		txd <<= 1;
		delay_us(2); //对TEA5767这三个延时都是必须的
		SCL_HIGH;
		delay_us(2);
		SCL_LOW;
		delay_us(2);
	}
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SET_SDA_IN; //SDA设置为输入
	for (i = 0; i < 8; i++)
	{
		SCL_LOW;
		delay_us(2);
		SCL_HIGH;
		receive <<= 1;
		if (SDA_ST)
			receive++;
		delay_us(1);
	}
	if (!ack)
		IIC_NAck(); //发送nACK
	else
		IIC_Ack(); //发送ACK
	return receive;
}

u8 G_Sensor_ReadBytes(u8 ReadAddr, char *dst, u8 len)
{
	u8 f = 0;

	IIC_Start();
	IIC_Send_Byte(GSENSOR_ID);
	f = IIC_Wait_Ack();
	if (0 == f)
	{
		IIC_Send_Byte(ReadAddr);
		f = IIC_Wait_Ack();
		if (0 == f)
		{
			IIC_Start();
			IIC_Send_Byte(GSENSOR_ID | 0x01);
			f = IIC_Wait_Ack();
			if (0 == f)
			{
				while (len--)
				{
					if (len > 0)
					{
						*dst++ = IIC_Read_Byte(1);
					}
					else
					{
						*dst++ = IIC_Read_Byte(0);
					}
				}
			}
		}
	}
	IIC_Stop();

	if (f)
		printf("\r\n--->get no ack");
	return f;
}

u8 G_Sensor_Read(u8 ReadAddr, u8 *dst)
{
	u8 f = 0;

	IIC_Start();
	IIC_Send_Byte(GSENSOR_ID);
	f = IIC_Wait_Ack();
	if (0 == f)
	{
		IIC_Send_Byte(ReadAddr);
		f = IIC_Wait_Ack();
		if (0 == f)
		{
			IIC_Start();
			IIC_Send_Byte(GSENSOR_ID | 0x01);
			f = IIC_Wait_Ack();
			if (0 == f)
			{
				*dst = IIC_Read_Byte(0);
			}
		}
	}
	IIC_Stop();

	if (f)
		printf("\r\n--->get no ack");
	return f;
}

u8 G_Sensor_Write(u8 WriteAddr, u8 DataToWrite)
{
	u8 f = 0;

	IIC_Start();
	IIC_Send_Byte(GSENSOR_ID);
	f = IIC_Wait_Ack();
	if (0 == f)
	{
		IIC_Send_Byte(WriteAddr);
		f = IIC_Wait_Ack();
		if (0 == f)
		{
			IIC_Send_Byte(DataToWrite);
			f = IIC_Wait_Ack();
		}
	}
	IIC_Stop();

	if (f)
		printf("\r\n--->get no ack");
	return f;
}

u8 G_Sensor_init(void)
{
	u8 ret;
	u8 add = 0, buf = 0;
	bool f = TRUE;

	IIC_Init();

	//核对 id
	add = 0x01;
	buf = 0;
	ret = G_Sensor_Read(add, &buf);
	if ((0x13 != buf) || (1 == ret))
	{
		f = FALSE;
	}

	//设置MSA301加速度值分辨率和加速度范围为正负2g
	add = 0x10;
	buf = 0x00;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}

	//设置低功耗模式及采样频率：正常模式，采样频率125Hz
	add = 0x11;
	buf = 0x10;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}

	//使能XYZ方向加速度中断
	add = 0x16;
	buf = 0x07;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}

	//int1的阈值
	add = 0x28;
	buf = Fs.Sensor;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}

	//将加速度中断映射到中断脚
	add = 0x19;
	buf = 0x04;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}	

	//配置锁存时间为25ms
	add = 0x21;
	buf = 0x00;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}

		//配置推拉输出，高电平有效
	add = 0x20;
	buf = 0x01;
	if (TRUE == f && (1 == (ret = G_Sensor_Write(add, buf))))
	{
		f = FALSE;
	}
	return f;
}


void G_Sensor_Pwr(u8 state)
{
	if(state)
		G_Sensor_Write(0x11,0x10);
	else	
		G_Sensor_Write(0x11,0x90);
}



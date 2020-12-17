#include "usr_main.h"

MOVE_DATA MoveData;

unsigned char VALUE_BUFF;
unsigned char speedByte;	 //保存速度值，单位km/h
unsigned short DegreeValue;  //保存首次用于角度变化判断的值，只有整数部分
unsigned char HaveMoveCnt;   //判断位移变化时，连续几次变化超过值才确认
unsigned char HaveDegreeCnt; //判断角度变化时，连续几次变化超过值才确认
unsigned short NoGpsTime;	//GPS开启，但是没有定位到计时
unsigned char VehSta[4];	 //天琴协议里4字节车辆状态

char HandTBuf[7] = {0};   //有加时区 bcd时间 hhmmss
char HandDBuf[7] = {0};   //有加时区 bcd日期 YYMMDD
char HandDBufTQ[7] = {0}; //有加时区 bcd日期 DDMMYY
char SpeedTmp[7] = {0};   //字符串格式，保存速度值，单位km/h
char DegreesTmp[7] = {0}; //ascii data 保留一位小数
char GprsSpeed[7] = {0};  //ascii data 保留一位小数 单位节

char TimeBuff[7];			 //保存原始时间数据不变
char DateBuff[7];			 //保存原始时间数据不变 DDMMYY
char LatitudeBuff[14];		 //raw ascii data 整数5位，小数4位，不足补0
char LongitudeBuff[14];		 //raw ascii data 整数4位，小数4位，不足补0
char LatitudeTmp[14] = {0};  //整数4位，小数4位，不足补0,最后有'N'或'S'
char LongitudeTmp[14] = {0}; //整数5位，小数4位，不足补0,最后有'E'或'W'
char SpeedBuff[7];			 //raw ascii data 保留一位小数
char DegreesBuff[7];		 //raw ascii data 保留一位小数
char LatitudeBuffAlarm[14];  //raw ascii data 整数5位，小数4位，不足补0,有'E' 各种报警判断用
char LongitudeBuffAlarm[14]; //raw ascii data 整数4位，小数4位，不足补0,有'E' 各种报警判断用
unsigned char CurSateCnt;	//最近一次定位的使用卫星数
float CurElevation;			 //最近一次定位的海拔高度，单位 米
unsigned int SchkLat = 0;
unsigned int SchkLon = 0;
unsigned char OpenGpsEorCnt = 0; //开gps失败计数
unsigned char OpenGpsCnt;		 //开gps失败时，延时
unsigned char RecGpsCnt;		 //gps接收检测计数，如没有接受到GPS串口有数据，该变量累加到30后重启GPS
unsigned char AgpsErrCnt;		 //获取Xtra失败计数
unsigned short NoGpsRestartCnt;  //在超过10分钟没有GPS信号重启GPS
char Gps_Sta_backup;			 //用来保存GPS当前定位状态，为1时定位，为0时没有定位。当状态有变化时，上传0020数据包
char Gps_S_back_up = 'N';
char Gps_W_back_up = 'E';
char Gps_W_S_Cnt;

unsigned char WaitUbloxCnt;
unsigned char CantConectAgpsCnt;

char LBS_Piont[68]; //最多容纳7个基站数据
unsigned char LBS_Num = 0;


void ChangeToUpper(char *pSrc, unsigned char maxLen)
{
	unsigned char I = 0;
	do
	{
		if (pSrc[I] >= 'a' && pSrc[I] <= 'z')
			pSrc[I] = pSrc[I] - 32;
		I++;
		maxLen--;
	} while (pSrc[I] != '\0' && pSrc[I] != '*' && maxLen);
}

void HexAtoI(char *src, unsigned long *dst)
{
	unsigned long i;

	ChangeToUpper(src, strlen(src));
	*dst = 0;
	while ('\0' != *src)
	{
		i = 0;

		if ((*src >= '0') && (*src <= '9'))
		{
			i = *src - '0';
		}
		else if ((*src >= 'A') && (*src <= 'Z'))
		{
			i = *src - 'A' + 10;
		}

		if (0 != i)
		{
			*dst = (*dst) * 16 + i;
		}
		src++;
	}
}

void GPS_Init(void)
{
	memset(SpeedBuff, '\0', 7);
	memset(DegreesBuff, '\0', 7);
	memset(TimeBuff, '\0', 7);
	memset(LatitudeBuff, '\0', 14);
	memset(LongitudeBuff, '\0', 14);
	memset(DateBuff, '\0', 7);
	memset(LongitudeTmp, '\0', 14);
	memset(LatitudeTmp, '\0', 14);
	speedByte = 0;
	VALUE_BUFF = 0;
	Flag.HaveGPS = 0;
}


void TIME_UpdateRTC(void)
{
	char datetem[7] = {0};
	char timetem[7] = {0};

	strcpy(datetem, DateBuff);
	strcpy(timetem, TimeBuff);

	Rtc.day = (datetem[0] - '0') * 10 + (datetem[1] - '0');
	Rtc.mon = (datetem[2] - '0') * 10 + (datetem[3] - '0');
	Rtc.year = (datetem[4] - '0') * 10 + (datetem[5] - '0');
	Rtc.hour = (timetem[0] - '0') * 10 + (timetem[1] - '0');
	Rtc.min = (timetem[2] - '0') * 10 + (timetem[3] - '0');
	Rtc.sec = (timetem[4] - '0') * 10 + (timetem[5] - '0');

	if (!Flag.HaveSynRtc) 
	{
		Flag.HaveSynRtc = 1;
	}
}

//将经纬度字符串 "ddmm.mmmm"转换为"dd.dddddd"
unsigned char MinuteToDegree(char *DM)
{
	unsigned long x = 0;
	unsigned char i, TEMP;
	unsigned char WhileCnt = 10;

	for (i = 0; (DM[i] != '.') && (WhileCnt > 0); i++)
	{
		if (i >= 6)
			return 0; //格式错误
	}

	DM[i] = DM[i - 1];
	DM[i - 1] = DM[i - 2];
	DM[i - 2] = '.';

	TEMP = i - 1; //记录转换后数据要存储的位置

	for (i = TEMP; i != TEMP + 6; i++)
	{
		x = x * 10 + DM[i] - '0';
	}

	x *= 10;
	x /= 6;

	for (i = TEMP + 5; i != TEMP - 1; i--)
	{
		DM[i] = x % 10 + '0';
		x /= 10;
	}
	return 1; //转换成功
}

//gps模块电源管理
/*
GPS在5分钟内没有定到位的情况下秒，需要断电重启，断电时间为5秒，重启之后需要重新加载AGPS数据：
OpenGpsCnt为断电时间倒计时
Flag.GpsReseting为GPS关闭未开启阶段标志位
*/
void GPS_Handle(void)
{
	if (Flag.NeedResetGps)
	{
		GPS_OFF;
		Flag.IsGpsOn = 0;
		OpenGpsCnt = 5;
		Flag.NeedResetGps = 0;
		Flag.GpsReseting = 1;

//		UART_DeInitUartGps();
		return;
	}
	else if (Flag.NeedGpsOpen)
	{
		if (!Flag.IsGpsOn && !OpenGpsCnt)
		{
			GPS_ON;

			if (Flag.GpsReseting)	
			{
				Flag.NeedReloadAgps = 1;
				Flag.GpsReseting = 0;

//				UART_InitUartGps();
			}
			RecGpsCnt = 0;
			Flag.IsGpsOn = 1;
			return;
		}
	}
	else if (ActiveTimer)
	{
		if (!Flag.IsGpsOn && !OpenGpsCnt && !Flag.Insleeping)
		{
			GPS_ON;

			if (Flag.GpsReseting)
			{
				Flag.NeedReloadAgps = 1;
				Flag.GpsReseting = 0;

//				UART_InitUartGps();
			}
			RecGpsCnt = 0;
			Flag.IsGpsOn = 1;
			return;
		}
	}
	else if (Flag.IsGpsOn)
	{
		GPS_OFF;
		Flag.HaveGPS = 0;
		Flag.IsGpsOn = 0;
		return;
	}
}

//返回以度为单位的经纬度值 2240.8176N
//float CHANGE_OO(char *pSrc,unsigned char TYPE)
float GPS_GetDegreeFrmStr(char *pSrc, unsigned char TYPE)
{
	unsigned char I = 0, J;
	//	unsigned char DD[4];
	float FEN = 0, DU = 0;

	//DU保存度数值，经度的度数有三位
	DU = pSrc[I++] - '0';
	DU *= 10;
	DU += pSrc[I++] - '0';
	if (TYPE)
	{
		DU *= 10;
		DU += pSrc[I++] - '0';
	}

	//FEN保存分值，单位为0.0001分
	while (pSrc[I] != '\0')
	{
		if (pSrc[I] == '.')
		{
			I++;
			J = 0;
			continue;
		}

		FEN *= 10;
		FEN += pSrc[I] - '0';
		I++;
		J++;
		if (pSrc[I] != '.')
		{
			if (pSrc[I] > '9' || pSrc[I] < '0')
				break;
		}
	}

	//J为分值中的小数点位数
	switch (J)
	{
	case 3:
		FEN = (FEN / 1000) / 60;
		DU = DU + FEN;
		break;
	case 2:
		FEN = (FEN / 100) / 60;
		DU = DU + FEN;
		break;
	case 1:
		FEN = (FEN / 10) / 60;
		DU = DU + FEN;
		break;
	default:
		FEN = (FEN / 10000) / 60;
		DU = DU + FEN;
		break;
	}
	//返回以度为单位的经纬度值
	return DU;
	//	return FEN;
}

//$GPRMC,041657.000,A,2240.8009,N,11408.8250,E,0.07,7.89,300414,,,A*6A
void GPS_UpdateData(char *pSrc)
{
	unsigned char pointer;
	char RX_BUF, DATA_COUNTS = 0;

	memset(TimeBuff, '\0', 7);

	while ((RX_BUF = *pSrc) != '\0' && RX_BUF != '*')
	{
		//RX_BUF=pSrc[0];
		if (RX_BUF == ',')
		{
			DATA_COUNTS++;
			if (DATA_COUNTS >= 10)
			{
				//GPS.data_ok=1;
			}
			RX_BUF = '\0'; // 为下一次接收做准备
			if ((DATA_COUNTS != 4) && (DATA_COUNTS != 6))
				pointer = 0;
			pSrc++;
			continue;
		}
		switch (DATA_COUNTS)
		{
		case 1:
			if (pointer < 6)
			{
				TimeBuff[pointer++] = RX_BUF;
			}
			TimeBuff[pointer] = '\0';
			break;
		case 2:
			VALUE_BUFF = RX_BUF;
			break;
		case 3:
			if (pointer < 9)
			{
				LatitudeBuff[pointer++] = RX_BUF;
				LatitudeBuff[pointer] = '\0';
			}
			break;
		case 4:

			if (RX_BUF == 'N')
				Flag.LatFlag = 0;
			else
				Flag.LatFlag = 1;

			if (pointer < 14)
			{
				LatitudeBuff[pointer++] = RX_BUF;
				LatitudeBuff[pointer] = '\0';
			}
			break;
		case 5:
			if (pointer < 10)
			{
				LongitudeBuff[pointer++] = RX_BUF;
				LongitudeBuff[pointer] = '\0';
			}
			break;
		case 6:

			if (RX_BUF == 'E')
				Flag.LonFlag = 0;
			else
				Flag.LonFlag = 1;

			if (pointer < 14)
			{
				LongitudeBuff[pointer++] = RX_BUF;
				LongitudeBuff[pointer] = '\0';
			}
			break;
		case 7:
			if (pointer < 7)
			{
				SpeedBuff[pointer++] = RX_BUF;
				SpeedBuff[pointer] = '\0';
			}
			break;
		case 8:
			if (pointer < 7)
			{
				DegreesBuff[pointer++] = RX_BUF;
				DegreesBuff[pointer] = '\0';
			}
			break;
		case 9:
			if (pointer < 6)
			{
				DateBuff[pointer++] = RX_BUF;
			}
			DateBuff[pointer] = '\0';
			break;
		default:
			break;
		}
		pSrc++;
	}
}
//将经纬度字符串整理为整数部分长为4或5位，小数部分为5位，不足数据补0
void chang_someone_data(char *DM, unsigned char len)
{
	unsigned char i, j, z, kk;
	char *ptr1; //,*ptr2;
	char temp[20];
	strcpy(temp, DM);
	//	strcpypgm2ram(temp,"9.5E");
	kk = strlen(temp);
	ptr1 = temp;

	//i值为整数部分位数
	for (i = 0; i < 5; i++)
	{
		if (*ptr1 == '\0' || *ptr1 == '.')
			break;
		ptr1++;
	}
	ptr1++;
	j = 0;

	//j值为小数部分位数
	while (*ptr1 != '\0')
	{
		j++;
		ptr1++;
	}

	//整数部分位数不足len,在数据字符串前补'0'
	if (i < len)
	{
		i = len - i;
		while (i--)
		{
			for (z = kk; z != 0; z--)
				temp[z] = temp[z - 1];
			temp[0] = '0';
			kk++;
			temp[kk] = 0;
		}
	}
	kk = strlen(temp);

	//小数部分位数不足5,在数据字符串后补'0'
	if (j != 5)
	{
		if (j < 5)
		{
			j = 5 - j;
			while (j--)
			{
				temp[kk] = temp[kk - 1];
				temp[kk - 1] = '0';
				kk++;
				temp[kk] = 0;
			}
		}
		else
		{
			j = j - 5;
			while (j--)
			{
				//temp[kk-2]=temp[kk-1];
				if ((temp[kk - 1] >= 5) && (temp[kk - 2] != '9'))
					temp[kk - 2]++;
				temp[kk - 1] = '\0';
				kk--;
			}
		}
	}

	if (len == 4)
	{
		if (Flag.LatFlag)
			temp[9] = 'S';
		else
			temp[9] = 'N';
		temp[10] = '\0';
	}
	if (len == 5)
	{
		if (Flag.LonFlag)
			temp[10] = 'W';
		else
			temp[10] = 'E';
		temp[11] = '\0';
	}

	strcpy(DM, temp);
}

/*
注意，由于纬度越高，纬度方向上相差相同的距离时经度变化范围越大，
以广东纬度小于30为例：经度0.03分约55m，约198km/h
因此需要根据纬度来区别对待

经度变化
但是在经度方向上，相同距离纬度变化一直是一样的。大约都是纬度变化0.06度对应104米

程序逻辑如下：

判断纬度合法性
将经纬度字符串转换为度分格式

计算本次度分格式的经纬度，与保存的经纬度之间的差值：A
获取实际GPS掉线时间T。(T为实际掉线时间，GPS关闭无输出时间不计算在内，该值GPS定位到时清零)

纬度判断：
如果A<T*k分，判断经度；
如果A>T*0.03分，返回0，舍弃数据；(0.03分为速度为180km/h时，1秒钟经纬度偏移度数)

经度判断：
确认当前定位的纬度位置，确认经度方向偏移50m对应的经度变化k；
如果A<T*k分，返回1，更新当前保存位置和时间;
如果A>T*k分，返回0，舍弃数据；

*/
unsigned char chkLatLon(void)
{
	char chkLat[7] = {0};
	char chkLon[7] = {0};
	unsigned int lat, lon, r1, r2, r3, lat_du; //lat_du是当前纬度，不同纬度情况下，相同距离，经度差别很大，需要区别处理
	unsigned char k = 0;					   //k为不同纬度下的系数
	char *p1, *p2;

	if (!LatitudeBuff[0])
		return 0;

	if ((p1 = strchr(LatitudeBuff, '.')) != NULL)
	{
		strncpy(chkLat, LatitudeBuff, (p1 - LatitudeBuff));
	}
	else
	{
		return 0;
	}

	if ((p2 = strchr(LongitudeBuff, '.')) != NULL)
	{
		strncpy(chkLon, LongitudeBuff, (p2 - LongitudeBuff));
	}
	else
	{
		return 0;
	}

	//首次调用该函数，给SchkLat，SchkLon赋值
	if (!SchkLat && !SchkLon)
	{
		r1 = Usr_Atoi(chkLat);
		r2 = r1 / 100;
		lat_du = r2;
		lat = r2 * 60 + r1 % 100; //获取分的整数部分

		p1++;
		r3 = Usr_Atoi(p1);
		r3 = r3 / 10;			   //获取到小数第三位，小数部分固定是5位
		SchkLat = lat * 1000 + r3; //单位为0.001分

		r1 = Usr_Atoi(chkLon);
		r2 = r1 / 100;
		lon = r2 * 60 + r1 % 100;

		p2++;
		r3 = Usr_Atoi(p2);
		r3 = r3 / 10;
		SchkLon = lon * 1000 + r3; //单位为0.001分

		return 1;
	}

	r1 = Usr_Atoi(chkLat);
	r2 = r1 / 100;
	lat_du = r2;
	lat = r2 * 60 + r1 % 100; //获取分的整数部分

	p1++;
	r3 = Usr_Atoi(p1);
	r3 = r3 / 10;
	lat = lat * 1000 + r3; //单位为0.001分

	r1 = Usr_Atoi(chkLon);
	r2 = r1 / 100;
	lon = r2 * 60 + r1 % 100;

	p2++;
	r3 = Usr_Atoi(p2);
	r3 = r3 / 10;

	lon = lon * 1000 + r3; //单位为0.001分

	if (SchkLat >= lat)
		r1 = SchkLat - lat;
	else
		r1 = lat - SchkLat;

	//相差k*NoGpsTime分，认为是跳点，舍弃
	if (speedByte < 180)
	{
		if (r1 >= 30 * NoGpsTime)
			return 0; //20150126_1
	}
	else
	{
		if (r1 >= 60 * NoGpsTime)
			return 0; //20150126_1
	}
	if (SchkLon >= lon)
		r1 = SchkLon - lon;
	else
		r1 = lon - SchkLon;

	if (lat_du < 30)
		k = 30;
	else if ((lat_du >= 30) && (lat_du < 40))
		k = 35;
	else if ((lat_du >= 40) && (lat_du < 50))
		k = 40;
	else if ((lat_du >= 50) && (lat_du < 60))
		k = 50;
	else if ((lat_du >= 60) && (lat_du < 70))
		k = 75;
	else if ((lat_du >= 70) && (lat_du < 80))
		k = 120;
	else
		return 1; //极高纬度地区，纬度变化变化率和经度变化呈现很大非线性，这里不予以跳点判断
	//相差0.006*NoGpsTime分，认为是跳点，舍弃
	if (speedByte < 180)
	{
		if (r1 >= k * NoGpsTime)
			return 0; //20150126_1
	}
	else
	{
		if (r1 >= 2 * k * NoGpsTime)
			return 0; //20150126_1
	}
	SchkLat = lat;
	SchkLon = lon;
	return 1;
}

//更新LAST_LAT_BUF和LAST_LON_BUF，并判断经纬度值变化是否大于1度
//变化大于3分，返回0
/*
unsigned char chkLatLon(void)
{	
	char chkLat[7]={0};
	char chkLon[7]={0};
	unsigned int  lat,lon,r1,r2;
	char *p1;
		
	if(!LatitudeBuff[0]) return 0;

	if((p1=strchr(LatitudeBuff,'.'))!=NULL)
	{
		strncpy(chkLat,LatitudeBuff,(p1-LatitudeBuff));
	}
	else
	{
		return 0;
	}

	if((p1=strchr(LongitudeBuff,'.'))!=NULL)
	{
		strncpy(chkLon,LongitudeBuff,(p1-LongitudeBuff));
	}
	else
	{
		return 0;
	}

	//首次调用该函数，给SchkLat，SchkLon赋值
	if(!SchkLat && !SchkLon)
	{
		r1=Usr_Atoi(chkLat);
		r2=r1/100;
		SchkLat=r2*60+r1%100; //将度数值转换为分数值
		
		r1=Usr_Atoi(chkLon);
		r2=r1/100;
		SchkLon=r2*60+r1%100;
		return 1;
	}

	r1=Usr_Atoi(chkLat);
	r2=r1/100;
	lat=r2*60+r1%100; 

	r1=Usr_Atoi(chkLon);
	r2=r1/100;
	lon=r2*60+r1%100;
		
	if(SchkLat>=lat) r1=SchkLat-lat;
	else			 r1=lat-SchkLat;
	//相差3分，认为是跳点，舍弃 
	if(r1>=3)  return 0;				//20150126_1

	if(SchkLon>=lon) r1=SchkLon-lon;
	else			 r1=lon-SchkLon;
	//相差3分，认为是跳点，舍弃 
	if(r1>=3)  return 0;				//20150126_1

	SchkLat=lat;
	SchkLon=lon;
	return 1;
	
}
*/
//计算并转换速度值,整数保存在speedByte，字符串形式保存在SPEED_TMP
void chane_speed(void)
{
	unsigned char I;
	float A;
	speedByte = 0;
	A = 0;
	for (I = 0; I < 7; I++)
	{ //最多读取前三位数据
		if (('0' <= SpeedBuff[I]) && (SpeedBuff[I] <= '9'))
		{
			A = A * 10 + (SpeedBuff[I] - '0');
		}
		else
		{ //格式错误,取消速度限制
			break;
		}
	}
	A *= 1.8;
	speedByte = (unsigned char)A;
	Itoa(speedByte, SpeedTmp);
	speed_gprs = (u16)speedByte * 10;
}
//转换SPEED_BUFF中字符串为整数部分三位，小数部分一位，不足补'0'
void make_speed(void)
{
	unsigned char i, j, z;
	i = 0;
	while (SpeedBuff[i] != '\0')
	{
		if (SpeedBuff[i] == '.')
			break;
		i++;
	}
	switch (i)
	{
	//例:由".123"变为"000.1"
	case 0:
		for (z = 0; z < 3; z++)
		{
			for (j = 4; j != 0; j--)
				SpeedBuff[j] = SpeedBuff[j - 1];
			SpeedBuff[0] = '0';
		}
		break;

	//例:由"1.12"变为"001.1"
	case 1:
		for (z = 0; z < 2; z++)
		{
			for (j = 4; j != 0; j--)
				SpeedBuff[j] = SpeedBuff[j - 1];
			SpeedBuff[0] = '0';
		}
		break;

	//例:由"11.1"变为"011.1"
	case 2:
		for (j = 4; j != 0; j--)
			SpeedBuff[j] = SpeedBuff[j - 1];
		SpeedBuff[0] = '0';
		break;
	default:
		SpeedBuff[3] = '.';
		break;
	}

	for (i = 0; i < 5; i++)
	{
		if (SpeedBuff[i] == '\0')
			SpeedBuff[i] = '0';
	}
	SpeedBuff[5] = '\0';
}

//转换DEGREES_BUFF中字符串为整数部分三位，小数部分两位，不足补'0'
void make_degrees(void)
{
	unsigned char i, j, z;
	i = 0;
	while (DegreesBuff[i] != '\0')
	{
		if (DegreesBuff[i] == '.')
			break;
		i++;
	}
	switch (i)
	{
	case 0:
		for (z = 0; z < 3; z++)
		{
			for (j = 5; j != 0; j--)
				DegreesBuff[j] = DegreesBuff[j - 1];
			DegreesBuff[0] = '0';
		}
		break;
	case 1:
		for (z = 0; z < 2; z++)
		{
			for (j = 5; j != 0; j--)
				DegreesBuff[j] = DegreesBuff[j - 1];
			DegreesBuff[0] = '0';
		}
		break;

	case 2:
		for (j = 5; j != 0; j--)
			DegreesBuff[j] = DegreesBuff[j - 1];
		DegreesBuff[0] = '0';
		break;
	default:
		DegreesBuff[3] = '.';
		break;
	}

	for (i = 0; i < 6; i++)
	{
		if (DegreesBuff[i] == '\0')
			DegreesBuff[i] = '0';
	}
	DegreesBuff[6] = '\0';
}
//由DDMMYY转为YYMMDD
void make_time(void)

{
	unsigned char i;
	char tmp;
	for (i = 0; i < 6; i++)
	{
		if (HandDBuf[i] == '\0')
		{
			HandDBuf[i] = '0';
		}
	}
	HandDBuf[i] = '\0';
	for (i = 0; i < 6; i++)
	{
		if (HandTBuf[i] == '\0')
		{
			HandTBuf[i] = '0';
		}
	}
	HandTBuf[i] = '\0';

	tmp = HandDBuf[0];
	HandDBuf[0] = HandDBuf[4];
	HandDBuf[4] = tmp;
	tmp = HandDBuf[1];
	HandDBuf[1] = HandDBuf[5];
	HandDBuf[5] = tmp;
}

void copyGpsdataToTmp(void)
{
	//DateBuff TimeBuff保存原始时间数据不变
	strcpy(HandDBuf, DateBuff);
	strcpy(HandTBuf, TimeBuff);

	make_time();

	strcpy(LongitudeTmp, LongitudeBuff);
	strcpy(LatitudeTmp, LatitudeBuff);
	strcpy(GprsSpeed, SpeedBuff);
	strcpy(DegreesTmp, DegreesBuff);

	HandDBufTQ[0] = HandDBuf[4];
	HandDBufTQ[1] = HandDBuf[5];
	HandDBufTQ[2] = HandDBuf[2];
	HandDBufTQ[3] = HandDBuf[3];
	HandDBufTQ[4] = HandDBuf[0];
	HandDBufTQ[5] = HandDBuf[1];

	strcpy(Fs.LatitudeLast, LatitudeBuff);
	strcpy(Fs.LongitudeLast, LongitudeBuff);

	//更新RTC
	if (!Flag.HaveSynRtc)
	{
		TIME_UpdateRTC();
	}

}
//判断角度变化大于50度。角度值是闭合圆形，要计算两个角度值大小两个差值，两个差值均大于60度才返回ture
unsigned char DegreeHandle(void)
{
	char tmp[5] = {0};
	char *p1 = NULL;
	unsigned short degreeTmp = 0;
	unsigned char r1 = 0, r2 = 0;

	p1 = strchr(DegreesBuff, '.');
	if (p1 != NULL && (p1 - DegreesBuff) < 4)
	{
		strncpy(tmp, DegreesBuff, (p1 - DegreesBuff));
		degreeTmp = Usr_Atoi(tmp);
	}
	else
	{
		return 0;
	}

	if (DegreeValue >= degreeTmp)
	{
		if ((DegreeValue - degreeTmp) >= 50)
		{
			r1 = 1; //计算第一个差值
		}

		if ((degreeTmp + 360 - DegreeValue) >= 50)
		{
			r2 = 1; //计算第二个差值
		}
	}
	else
	{
		if ((degreeTmp - DegreeValue) >= 50)
		{
			r1 = 1; //计算第一个差值
		}

		if ((DegreeValue + 360 - degreeTmp) >= 50)
		{
			r2 = 1; //计算第二个差值
		}
	}

	if (r1 && r2)
	{
		if (HaveDegreeCnt++ > 2)
		{
			DegreeValue = degreeTmp;
			HaveDegreeCnt = 0;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		HaveDegreeCnt = 0;
		return 0;
	}
}
//用于判断疑似堵车时位移是否大于200米，如果大于200米，返回true
unsigned char MoveHandle(void) //20140724_1
{
	//计算两坐标点之间的距离，单位为米
	float MlatTemp, MlonTemp, r = 0.0, T, sqrt_in, sqrt_root;
	unsigned int MI = 0;

	if (!LatitudeBuff[0])
		return 0;

	MlatTemp = GPS_GetDegreeFrmStr(LatitudeBuff, 0);
	MlonTemp = GPS_GetDegreeFrmStr(LongitudeBuff, 1);

	T = MlatTemp * 3.1415926 / 180;
	r = 6371.0 * cos(T);

	if (MoveData.LatSetted > MlatTemp)
	{
		sqrt_in = MoveData.LatSetted - MlatTemp;
	}
	else
	{
		sqrt_in = MlatTemp - MoveData.LatSetted;
	}

	sqrt_in *= 3.1415926;
	sqrt_in /= 180.0;
	sqrt_in *= 6371.0;
	sqrt_in *= 1000.0;
	MI = (unsigned int)sqrt_in;

	if (MoveData.LonSetted > MlonTemp)
	{
		sqrt_root = MoveData.LonSetted - MlonTemp;
	}
	else
	{
		sqrt_root = MlonTemp - MoveData.LonSetted;
	}
	sqrt_root *= 3.1415926;
	sqrt_root /= 180.0;
	//	sqrt_root/=60.0;
	sqrt_root *= r;
	sqrt_root *= 1000.0;
	MI = (unsigned int)sqrt_root;

	sqrt_root = sqrt_root * sqrt_root + sqrt_in * sqrt_in;

	sqrt_in = sqrt(sqrt_root);

	MI = (unsigned int)sqrt_in;
	//UART_debug("\r\n MI:%d\r\n",MI);
	//判断是否超出规定距离
	if (MI >= 200)
	{
		if (++HaveMoveCnt > 2)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		HaveMoveCnt = 0;
		return 0;
	}
}

void GPS_GetGpggaInfo(char *pSrc) //20141030_2
{
	char *p1 = NULL, *p2 = NULL;
	unsigned char i;

	//$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04(卫星数),24.4,19.7(海拔),M,,,,0000*1F
	if (((p1 = strstr(pSrc, "GPGGA")) != NULL) || ((p1 = strstr(pSrc, "GNGGA")) != NULL))
	{
		for (i = 0; i < 7; i++)
		{
			p2 = strchr(p1, ',');
			p1 = p2 + 1;
		}

		CurSateCnt = Usr_Atoi(p1); //保存正在使用的卫星数

		for(i=0;i<2;i++)
		{
			p2=strchr(p1,',');	
			p1=p2+1;
		}

//		CurElevation=atof(p1);	//保存海拔
		CurElevation=100;
	}
}

//没有定到位，也更新日期时间，但不存flash
void GPS_CopyTimeAndDate(void)
{
	if (!DateBuff[0] || !TimeBuff[0])
		return;

	//DateBuff TimeBuff保存原始时间数据不变
	strcpy(HandDBuf, DateBuff);
	strcpy(HandTBuf, TimeBuff);

	make_time();

	HandDBufTQ[0] = HandDBuf[4];
	HandDBufTQ[1] = HandDBuf[5];
	HandDBufTQ[2] = HandDBuf[2];
	HandDBufTQ[3] = HandDBuf[3];
	HandDBufTQ[4] = HandDBuf[0];
	HandDBufTQ[5] = HandDBuf[1];

	//更新RTC
	if (!Flag.HaveSynRtc)
	{
		TIME_UpdateRTC();
		Flag.HaveSynRtc = 1;
	}
}
u16 WIRELESS_AgpsSendPacket(void)
{
	return 0;
}

void GPS_DataProcess(char *pSrc)
{
	static unsigned char validCnt, unValidCnt = 0;
	char *pGPS = NULL, *p1 = NULL;
	//	static unsigned char timeBuff[7]={0};

#if 0   
	if((pGPS=strstr(pSrc,"GPRMC"))!=NULL) //20140811_1
	{	
		RecGpsCnt=0;				//20150129_3
		GPS_UpdateData(pGPS);			
	}
	else
	{			
		return;
	}
#endif

	if ((pGPS = strstr(pSrc, "GNRMC")) != NULL || (pGPS = strstr(pSrc, "GPRMC")) != NULL) //20180125
	{
		RecGpsCnt = 0; //20150129_3
		GPS_UpdateData(pGPS);
	}
	else
	{
		return;
	}

#if 0
	if(0==strcmp(timeBuff,TimeBuff))
	{
		VALUE_BUFF='V';
		if(TimeBuff[3] && ++timeEorCnt>3)	//20150122_2
		{
			timeEorCnt=0;
			UART_debug("\r\nreset gps");
			Flag.NeedResetGps=1;
		}
	}
	else
	{
		strcpy(timeBuff,TimeBuff);
	}
#endif
	if ((VALUE_BUFF != 'A') && (VALUE_BUFF != 'V'))
		return;

	if (VALUE_BUFF == 'A')
	{

		//VALUE_BUFF='\0';
		if (++validCnt > GPS_VALID_CNT) //20151215_4
		{

			validCnt = GPS_VALID_CNT;
			//gps_stop();
			chang_someone_data(LongitudeBuff, 5);
			chang_someone_data(LatitudeBuff, 4);

			strcpy(LongitudeBuffAlarm, LongitudeBuff);
			strcpy(LatitudeBuffAlarm, LatitudeBuff);

			if (chkLatLon())
			{
				chane_speed();
				make_speed();
				make_degrees();
				unValidCnt = 0;
				if (LatitudeBuff[1] && LongitudeBuff[1])
				{
					if (LatitudeBuff[9] == 'N' || LatitudeBuff[9] == 'S')
					{
						if (LongitudeBuff[10] == 'E' || LongitudeBuff[10] == 'W')
						{
							Flag.HaveGPS = 1;
							NoGpsTime = 1; //有定位情况，重置NoGpsTime
							if (Flag.FristGetGps)
							{
								Flag.HaveSynRtc = 0;
								Flag.FristGetGps = 0;
							}
						}
					}
				}

				if (LatitudeBuff[9] != 'N' && LatitudeBuff[9] != 'S')
				{
					Flag.HaveGPS = 0;
				}
				if (LongitudeBuff[10] != 'E' && LongitudeBuff[10] != 'W')
				{
					Flag.HaveGPS = 0;
				}
				if (Flag.HaveGPS)
				{
					copyGpsdataToTmp();
					if (speedByte > 3)
					{
						MoveData.MoveFisrt = 0;
									   
						//角度变化判断	20140728_2
						//速度首次大于2km/h,保存当前角度值
						if (!Flag.SetDegreeData)
						{
							char tmp[5] = {0};
							p1 = strchr(DegreesBuff, '.');
							if (p1 != NULL && (p1 - DegreesBuff) < 4)
							{
								strncpy(tmp, DegreesBuff, (p1 - DegreesBuff));
								DegreeValue = Usr_Atoi(tmp);
								Flag.SetDegreeData = 1;
								HaveDegreeCnt = 0;
							}
						}
						else if (DegreeHandle())
						{
							Flag.OtherSendPosi = 1;
						}
					}
					else
					{
						Flag.SetDegreeData = 0;

						//位移判断	 20140724_1
						//速度首次减为5km/h以下，可能为堵车，保存当前经纬度
						if (!MoveData.MoveFisrt)
						{
							MoveData.MoveFisrt = 1;
							HaveMoveCnt = 0;
							MoveData.LatSetted = GPS_GetDegreeFrmStr(LatitudeBuff, 0);
							MoveData.LonSetted = GPS_GetDegreeFrmStr(LongitudeBuff, 1);
							//UART_debug("\r\n%s MoveData.LatSetted:%f,MoveData.LonSetted:%f\r\n",TimeBuff,MoveData.LatSetted,MoveData.LonSetted);
						}
						//保存过堵车经纬度后，判断位移是否超过200米
						else if (MoveHandle())
						{
							Flag.OtherSendPosi = 1;
							HaveMoveCnt = 0;
							//超过规定位移，更新基准点
							MoveData.LatSetted = GPS_GetDegreeFrmStr(LatitudeBuff, 0);
							MoveData.LonSetted = GPS_GetDegreeFrmStr(LongitudeBuff, 1);
							//UART_debug("\r\n (CAR MOVE!)%s MoveData.LatSetted:%f,MoveData.LonSetted:%f\r\n",TimeBuff,MoveData.LatSetted,MoveData.LonSetted);
						}
					}
				}
			}
			else //判断跳点超过30个，可能判断错误，重新判断
			{
				printf("\r\nThe locton data is jumper piont,abord!\r\n");
				if (speedByte > 60) //有较大速度的时候，需要计算丢失定位时车辆还在移动导致位置偏移
					NoGpsTime++;
				if (++unValidCnt > 15)
				{
					unValidCnt = 0;
					SchkLat = 0;
					SchkLon = 0;
				}
			}
		}
	}
	else
	{
		GPS_CopyTimeAndDate();
		validCnt = 0;
		Flag.HaveGPS = 0;
		LatitudeBuffAlarm[0] = '\0';
		LongitudeBuffAlarm[0] = '\0';
	}
}

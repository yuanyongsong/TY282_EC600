#ifndef USR_GPS_H
#define USR_GPS_H


#define GPS_VALID_CNT  2
#define TIME_GPS_MAX   120

#ifndef _MOVE_DATA
#define _MOVE_DATA
typedef struct
{
	float LatSetted;      		        //堵车时车辆缓慢移动中心位置纬度
	float LonSetted;      		        //堵车时车辆缓慢移动中心位置经度	
	unsigned char  MoveFisrt; 		    //速度首次减为5km/h以下，可能为堵车
}MOVE_DATA;
#endif

extern unsigned int SchkLat;
extern unsigned int SchkLon;


extern unsigned char VALUE_BUFF;
extern unsigned char  speedByte;    //保存速度值，单位km/h
extern unsigned char CurSateCnt;		   //最近一次定位的使用卫星数
extern unsigned char OpenGpsCnt;
extern unsigned char OpenGpsEorCnt;
extern unsigned char RecGpsCnt;
extern unsigned short  NoGpsTime;

extern unsigned char VehSta[];           //天琴协议里4字节车辆状态

extern char HandTBuf[];     //当前时区 bcd时间 hhmmss
extern char HandDBuf[];     //当前时区 bcd日期 YYMMDD
extern char HandDBufTQ[];   //当前时区 bcd日期 DDMMYY
extern char SpeedTmp[];     //字符串格式，保存速度值，单位km/h
extern char DegreesTmp[];
extern char GprsSpeed[];
extern char TimeBuff[];         //
extern char DateBuff[];         //
extern char LatitudeBuff[];    //raw ascii data 整数5位，小数4位，不足补0
extern char LongitudeBuff[];   //raw ascii data 整数4位，小数4位，不足补0
extern char LatitudeTmp[]; //整数4位，小数4位，不足补0,最后有'N'或'S'
extern char LongitudeTmp[];
extern char SpeedBuff[];        //raw ascii data 保留一位小数
extern char DegreesBuff[];      //raw ascii data 保留一位小数
extern char LatitudeBuffAlarm[];    //raw ascii data 整数5位，小数4位，不足补0,各种报警判断用
extern char LongitudeBuffAlarm[]; 
extern unsigned char AgpsErrCnt;
extern float		 CurElevation;
extern unsigned short NoGpsRestartCnt;  
extern char  LBS_Piont[];
extern unsigned char LBS_Num;
extern unsigned char WaitUbloxCnt;
extern unsigned char CantConectAgpsCnt;

void GPS_Init(void);
void GPS_DataProcess(char * pSrc);
void GPS_Handle(void);
float GPS_GetDegreeFrmStr(char *pSrc,unsigned char TYPE);
void Itoa(unsigned char src,char dst[]);
void GPS_GetGpggaInfo(char *pSrc);
void HexAtoI(char *src, unsigned long *dst); 
void BLS_DataProcess(char *pSrc);
unsigned char MinuteToDegree(char *DM);
void ChangeToUpper(char *pSrc, unsigned char maxLen);
#endif

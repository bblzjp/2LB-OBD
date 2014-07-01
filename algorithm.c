#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>

#include "algorithm.h" 
#define  DEBUG  1
/*******************************************************************/
//  函数： 算法函数集合
//  功能： 显示当前时间
/******************************************************************/
GPS_INFO  BD;
GPS_INFO  *GPS = &BD;

//将获取文本信息转换为double型
static double Get_Double_Number(char *s)
{
    char buf[128];
    int i;
    double rev;
    i=GetComma(1,s);
    strncpy(buf,s,i);
    buf[i]=0;
    rev=atof(buf);

    return rev;
}

//得到指定序号的逗号位置
int GetComma(int num,char *str)
{
    int i,j=0;
    int len=strlen(str);
    for(i=0;i<len;i++)
    {
        if(str[i]==',')
        {
             j++;
        }

        if(j==num)
            return i+1;
    }
    return 0;
}

//***************************************************
// 语法格式：void UTC2BTC(DATE_TIME *GPS)
// 实现功能：转化时间为北京时区的时间
// 参    数：存放时间的结构体
// 返 回 值：无
//***************************************************

static void UTC2BTC(DATE_TIME *date_time)
{
	//***************************************************
	date_time->second ++;  
	if(date_time->second > 59)
	{
		date_time->second = 0;
		date_time->minute ++;
		if(date_time->minute > 59)
		{
			date_time->minute = 0;
			date_time->hour ++;
		}
	}	
	
//***************************************************
  date_time->hour = date_time->hour + 8;
	if(date_time->hour > 23)
	{
		date_time->hour -= 24;
		date_time->day += 1;
		if(date_time->month == 2 ||
		   		date_time->month == 4 ||
		   		date_time->month == 6 ||
		   		date_time->month == 9 ||
		   		date_time->month == 11 )
		{
			if(date_time->day > 30)
			{
		   		date_time->day = 1;
				date_time->month++;
			}
		}
		else
		{
			if(date_time->day > 31)
			{	
		   		date_time->day = 1;
				date_time->month ++;
			}
		}
		if(date_time->year % 4 == 0 )
		{
	   		if(date_time->day > 29 && date_time->month == 2)
			{		
	   			date_time->day = 1;
				date_time->month ++;
			}
		}
		else
		{
	   		if(date_time->day > 28 &&date_time->month == 2)
			{
	   			date_time->day = 1;
				date_time->month ++;
			}
		}
		if(date_time->month > 12)
		{
			date_time->month -= 12;
			date_time->year ++;
		}		
	}
}


//====================================================================//
// 语法格式: static float Str_To_Float(char *buf)
// 实现功能： 把一个字符串转化成浮点数
// 参    数：字符串
// 返 回 值：转化后单精度值
//====================================================================//
float Str_To_Float(char *buf)
{
	float rev = 0;
	float dat;
	int integer = 1;
	char *str = buf;
	int i;
	while(*str != '\0')
	{
		switch(*str)
		{
			case '0':
				dat = 0;
				break;
			case '1':
				dat = 1;
				break;
			case '2':
				dat = 2;
				break;		
			case '3':
				dat = 3;
				break;
			case '4':
				dat = 4;
				break;
			case '5':
				dat = 5;
				break;
			case '6':
				dat = 6;
				break;
			case '7':
				dat = 7;
				break;
			case '8':
				dat = 8;
				break;
			case '9':
				dat = 9;
				break;
			case '.':
				dat = '.';
				break;
		}
		if(dat == '.')
		{
			integer = 0;
			i = 1;
			str ++;
			continue;
		}
		if( integer == 1 )
		{
			rev = rev * 10 + dat;
		}
		else
		{
			rev = rev + dat / (10 * i);
			i = i * 10 ;
		}
		str ++;
	}
	return rev;

}

//====================================================================//
// 语法格式: static float Get_Float_Number(char *s)
// 实现功能： 把给定字符串第一个逗号之前的字符转化成单精度型
// 参    数：字符串
// 返 回 值：转化后单精度值
//====================================================================//
float Get_Float_Number(char *s)
{
	char buf[10];
	unsigned char i;
	float rev;
	i=GetComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev=Str_To_Float(buf);
	return rev;	
}


static void GPS_DisplayOne(void)
{
	char time[256] = {},location[256] = {};
	
	sprintf(time,"%4d年%02d月%02d日  %02d:%02d:%02d ", GPS->D.year, GPS->D.month, GPS->D.day, GPS->D.hour, GPS->D.minute, GPS->D.second);

	if ((GPS->NS == 'N') && (GPS->EW == 'E'))  //北纬，东经
		sprintf(location,"北纬：%2d° %2d＇%2d＂  东经：%2d° %2d＇%2d＂\n", GPS->latitude_Degree, GPS->latitude_Cent, GPS->latitude_Second, GPS->longitude_Degree, GPS->longitude_Cent, GPS->longitude_Second);
	else if ((GPS->NS == 'N') && (GPS->EW == 'W')) //北纬，西经
		sprintf(location,"北纬：%2d° %2d＇%2d＂  西经：%2d° %2d＇%2d＂\n", GPS->latitude_Degree, GPS->latitude_Cent, GPS->latitude_Second, GPS->longitude_Degree, GPS->longitude_Cent, GPS->longitude_Second);
	else if ((GPS->NS == 'S') && (GPS->EW == 'E')) //南纬，东经
		sprintf(location,"南纬：%2d° %2d＇%2d＂  东经：%2d° %2d＇%2d＂\n", GPS->latitude_Degree, GPS->latitude_Cent, GPS->latitude_Second, GPS->longitude_Degree, GPS->longitude_Cent, GPS->longitude_Second);
	else if ((GPS->NS == 'S') && (GPS->EW == 'W')) //南纬，西经
		sprintf(location,"南纬：%2d° %2d＇%2d＂  东经：%2d° %2d＇%2d＂\n", GPS->latitude_Degree, GPS->latitude_Cent, GPS->latitude_Second, GPS->longitude_Degree, GPS->longitude_Cent, GPS->longitude_Second);

	//信息发送到服务器
	gsm_send(2,time);
	gsm_send(2,location);
//	printf("%s %s\n",time,location);

}

void GPS_DisplayTwo(void)
{
	printf("速度：%5f km/h 航向：%5f 度 \n", GPS->speed, GPS->direction);
	printf("地面高度：%5f米  海拔高度：%5f米\n\n", GPS->height_ground, GPS->height_sea);
}

//***************************************************
//	语法格式：int gps_parse(char* buf)
//	实现功能：解析gps发出的数据
//	参数		：
//	返回值：
//***************************************************
int gps_parse(char* buf)
{
	float lati_cent_tmp, lati_second_tmp;
	float long_cent_tmp, long_second_tmp;
	float speed_tmp;
    int tmp;
   char c = buf[5];
    char status = buf[GetComma(2, buf)];
    
    if(c == 'C')
    {
    	
		if (status == 'A')  //如果数据有效，则分析
		{
			GPS -> NS       = buf[GetComma(4, buf)];
			GPS -> EW       = buf[GetComma(6, buf)];

			GPS->latitude   = Get_Double_Number(&buf[GetComma(3, buf)]);
			GPS->longitude  = Get_Double_Number(&buf[GetComma( 5, buf)]);

       	    GPS->latitude_Degree  = (int)GPS->latitude / 100;       //分离纬度
			lati_cent_tmp         = (GPS->latitude - GPS->latitude_Degree * 100);
			GPS->latitude_Cent    = (int)lati_cent_tmp;
			lati_second_tmp       = (lati_cent_tmp - GPS->latitude_Cent) * 60;
			GPS->latitude_Second  = (int)lati_second_tmp;

			GPS->longitude_Degree = (int)GPS->longitude / 100;	//分离经度
			long_cent_tmp         = (GPS->longitude - GPS->longitude_Degree * 100);
			GPS->longitude_Cent   = (int)long_cent_tmp;    
			long_second_tmp       = (long_cent_tmp - GPS->longitude_Cent) * 60;
			GPS->longitude_Second = (int)long_second_tmp;
		
			speed_tmp      = Get_Float_Number(&buf[GetComma(7, buf)]);    //速度(单位：海里/时)
			GPS->speed     = speed_tmp * 1.85;                           //1海里=1.85公里
			GPS->direction = Get_Float_Number(&buf[GetComma(8, buf)]); //角度	
			
			GPS->D.hour    = (buf[7] - '0') * 10 + (buf[8] - '0');		//时间
			GPS->D.minute  = (buf[9] - '0') * 10 + (buf[10] - '0');
			GPS->D.second  = (buf[11] - '0') * 10 + (buf[12] - '0');
			tmp = GetComma(9, buf);
			GPS->D.day     = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); //日期
			GPS->D.month   = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
			GPS->D.year    = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0')+2000;

			UTC2BTC(&GPS->D);
			GPS_DisplayOne();
		}
		
#if 0
		else if(c == 'V'){
			printf("正在定位...\r");
		}
#endif
    }
    
    if(c=='A')
    {
        //"$GNGGA"
        GPS->height_sea = Get_Float_Number(&buf[GetComma(9, buf)]);
		GPS->height_ground = Get_Float_Number(&buf[GetComma(11, buf)]);
		GPS_DisplayTwo();
    }
    	
 }


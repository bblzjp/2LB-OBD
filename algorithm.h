#ifndef  __ALGORITHM_
#define  __ALGORITHM_

typedef struct{
	int year;  
	int month; 
	int  day;
	int hour;
	int minute;
	int second;
}DATE_TIME;

typedef struct{
	double  high;
	double  latitude;  //经度
	double  longitude; //纬度
	int     latitude_Degree;	//度
	int		latitude_Cent;		//分
	int   	latitude_Second;    //秒
	int     longitude_Degree;	//度
	int		longitude_Cent;		//分
	int   	longitude_Second;   //秒
	int     satellite;
	float   speed;
	float	direction;
	float	height_ground;
	float	height_sea;
	unsigned char 	NS;
	unsigned char 	EW;
	DATE_TIME D;
}GPS_INFO;

int gps_parse(char* GPS_BUF);
#endif

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
	double  latitude;  //����
	double  longitude; //γ��
	int     latitude_Degree;	//��
	int		latitude_Cent;		//��
	int   	latitude_Second;    //��
	int     longitude_Degree;	//��
	int		longitude_Cent;		//��
	int   	longitude_Second;   //��
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

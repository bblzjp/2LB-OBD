#ifndef __GPSDATE_H
#define __GPSDATE_H


/***************************extern变量*****************************/

extern volatile char gps_flag;						 //GPS读取数据记录状态标识
extern char gps_count;								 //GPS字符接收个数计数值
extern char gps_rec[65];						 	 	 //存放GPS语句标识符
extern char gps_rmc_rec[65];					         //存放GPRMC语句字符
extern char gps_gsv_rec[15];					         //存放GPGSV语句字符
extern char gps_rmc_have; 						     //GPRMC语句完成标识
extern char gps_gsv_have; 						     //GPGSV语句完成标识
extern char gps_valid;								 //定位状态位 0：无效；1：有效
extern char sat_have_gps;							     //GPS定位卫星数量
extern char sat_have_bd;							     //北斗定位卫星数量
extern char gps_rmc[22];						         //存放经改编后的GPS位置信息及其他数据
extern int gps_fd;									//gps串口文件描述符
/********************************************************************/ 

void tel_gps(void);
void Gpgsv_date_pro(void);
inline void Gprmc_date_pro(void);
void Bdgsv_date_pro(void);
void simu_serial_gprmc(char* gps_check);
void simu_serial_gpgsv(char* gps_check);
void simu_serial_bdgsv(char* gps_check);
//void command_80(char* local,char type);
#endif

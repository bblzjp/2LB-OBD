#ifndef __GPSDATE_H
#define __GPSDATE_H


/***************************extern����*****************************/

extern volatile char gps_flag;						 //GPS��ȡ���ݼ�¼״̬��ʶ
extern char gps_count;								 //GPS�ַ����ո�������ֵ
extern char gps_rec[65];						 	 	 //���GPS����ʶ��
extern char gps_rmc_rec[65];					         //���GPRMC����ַ�
extern char gps_gsv_rec[15];					         //���GPGSV����ַ�
extern char gps_rmc_have; 						     //GPRMC�����ɱ�ʶ
extern char gps_gsv_have; 						     //GPGSV�����ɱ�ʶ
extern char gps_valid;								 //��λ״̬λ 0����Ч��1����Ч
extern char sat_have_gps;							     //GPS��λ��������
extern char sat_have_bd;							     //������λ��������
extern char gps_rmc[22];						         //��ž��ı���GPSλ����Ϣ����������
extern int gps_fd;									//gps�����ļ�������
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

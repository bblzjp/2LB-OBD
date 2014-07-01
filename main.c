#include <stdio.h>
#include <unistd.h>
#include "gpsdate.h"
#include "gprsproc.h"
#include "serial_gprs.h"
#include "gps.h"
#include "obd.h"
#define TD_DEV          "/dev/ttyS4"
#define	GPRS_DEV 	"/dev/ttyS3"
#define OBD_DEV 	"/dev/ttyS2"
#define DEV_SENSOR_PATH "/dev/sensor_dev"
extern int sht_fd ;
extern pthread_mutex_t g_mtx;
int Serial_Gprs_Init(void)
{
	gprs_fd = open(GPRS_DEV, O_RDWR | O_NOCTTY);
	if(gprs_fd<0){
		printf("Error: open serial port error.\n");
		return(-1);
	}
    set_tty_option(gprs_fd, 115200, 8, 'N', 1);
}

int gps_init(void){
    BDfd = open(TD_DEV,O_RDWR | O_NOCTTY);
	printf("fd is %d\n",BDfd);
    if (BDfd < 0)
    {
        printf("Cannot open gps device\n");
        close(BDfd);
		return -1;
    }
	tcflush(BDfd,TCIOFLUSH);
    set_tty_option(BDfd, 9600, 8, 'N', 1);
}
int Serial_Obd_Init(void)
{
	OBDfd = open(OBD_DEV, O_RDWR | O_NOCTTY| O_NDELAY);
	if(OBDfd<0){
		printf("Error: open obd  port error.\n");
		return(-1);
	}
    set_tty_option(OBDfd, 38400, 8, 'N', 1);
}

int sht10_open(void)
{
  sht_fd = open(DEV_SENSOR_PATH, O_RDWR);
	if (sht_fd == -1)
	{
	  perror("open");
	  return ;
	}
	printf("sht_fd is %d\n",sht_fd);
  
}
int main(int argc,char *argv[])
{
	pthread_mutex_init(&g_mtx,NULL);
	sim900a_power_on();
	td3020c_power_on(); 	
	printf("init!!\n");
	Serial_Gprs_Init();
	gprs_init(0);	
	send_jump();
	gsm_send(1,gprs_send);
	
	printf("gsm_send,ok!!!");
	
	Serial_Obd_Init();

	
	gps_init();

	sht10_open();
	gps_receive();
	pthread_mutex_destroy(&g_mtx);
	return 0;
}

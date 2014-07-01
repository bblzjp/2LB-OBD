#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <termios.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h>
#include "gps.h"
#include "gpsdate.h"
#include "gprsproc.h"
#include "usart.h"
#include "obd.h"
#include "pwctrl.h"

#define  DEBUG  1
static inline char receive_byte(void){
	char gps_date = '\0';

	if(read(BDfd,&gps_date,1) == -1)
		perror("Read Error:");

	return gps_date;
}

inline void gps_receive(void)
{
	char c,gps_type;
	int i =0,j=0;
	int timeout =30;
	int obdtime = 0;
	int sum = 0,gps_flag = 0;
	char buf[128] = "",buff[128] = "";
	char str[256] = "";
	//只接收$GNRMC
	while(1){
		if(sum > 255)
			sum = 0;
#if 1
		c = receive_byte();
		buf[sum++] = c;
#endif

		switch(gps_flag){
			case 0:
 				if(buf[0] == '$')
					gps_flag = 1;
				else
					sum = 0;
	 			break;
			case 1:
  	 			if (sum == 7)
				{
			 		if(!strncmp(buf,"$GNRMC,",7))
						gps_flag = 2,gps_type = 0;
					else
						gps_flag = 0,sum = 0,gps_type = 0;
				} 
 				break;
			case 2:
				buff[sum - 8] = c;
   		 		//接收完一行数据
				if(c == '\n') 
				{
   		 			sum = 0;
					gps_flag = 0;
					printf("%s",buff);
					printf("timeout=%d\n",timeout);
					printf("obdtime=%d\n",obdtime);

					if(timeout == TIMEOUT)
						{	
							pthread_mutex_lock(&mtx);
							thread_flag = 0;	
							pthread_mutex_unlock(&mtx);
					
							command_80(buff,&timeout);			

                                                }
					else if(buff[10] == 'V')
						{
							j++;
							printf("j=%d\n",j);	
							if(j > 350)
							{	j=0;
								td3020c_power_on();			
							}		
						}

					else if(obdtime > 150)
						{
							obdtime = 0;
						}
					else if (obdtime == (OBDTIME - 87))
						{
							command_B3();
							usleep(800);
							command_B4();
							usleep(800);
						 	command_B7();
						
						}
					else if (obdtime == (OBDTIME - 55))
						{
							command_B5();
							usleep(800);
							command_B6();
							usleep(800);
						 	command_B9();
						
						}
					else if (obdtime == (OBDTIME-3))
						{
							command_B0();
							usleep(800);
							command_B1();
							usleep(800);
						 	command_B2();
							usleep(800);
							command_B8();
							usleep(800);
							command_C0();
							usleep(800);
							command_C1();
							usleep(800);
							command_C2();
							usleep(800);
							command_C3();
							usleep(800);
							command_A7();
							obdtime = 0;
		
						}
					if(fcntl(gprs_fd,F_SETFL,FNDELAY) < 0)//NO delay
					{
						printf("fcntl fatil \n");
					}
					i = read(gprs_fd, str, 256);
					if(i < 0)					
					  {
					       printf("%d\n",__LINE__);
					       perror("read");		
					  }
					if(fcntl(gprs_fd,F_SETFL,0) < 0)
					{
						printf("fcntl fatil \n");
					}
					printf("%dstr=%s\n",__LINE__,str);
					memset(buf,0,sizeof(buf));
					memset(buff,0,sizeof(buff));
					memset(str,0,sizeof(str));					
					timeout++;		
					obdtime++;
  				} 
				break;
			default:
				break;
		}
		
	}
	
}

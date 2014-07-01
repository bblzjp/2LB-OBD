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

#include "obd.h"
#include "usart.h"

#if 0
unsigned char* read_obd(void) 
{ 
  struct obd_diagno_info *obd_info; 
  unsigned  char buf_rcv[256];
  memset(buf_rcv,0,256);
  int nread=0; int i;
  nread = read(OBDfd, buf_rcv, 256);  
  printf("read:   %s\n",buf_rcv);
  for(i=0;i<= nread;i++)
  {
    
    printf("buf[%d]=%c\n",i,buf_rcv[i]);
  }
  return buf_rcv;
   
} 
#endif

inline int obd_data(int obd_flag ,unsigned char* buf) 
{  
    pthread_mutex_lock(&g_mtx);
   char* buf_rcv[256];
   unsigned char buf_tmp[256]={0};
   int wread=0;
   int i;
   memset(buf_rcv,0,256);
   
    buf_rcv[0] = "BT+MIL\r\n";
    buf_rcv[1] = "BT+SPWR\r\n";//B0
    buf_rcv[2] = "BT+RDTC\r\n";//
    buf_rcv[3] = "BT+DATA.Load\r\n";//B1
    buf_rcv[4] = "BT+DATA.ECT\r\n";//B2
    buf_rcv[5] = "BT+DATA.RPM\r\n";//B3
    buf_rcv[6] = "BT+DATA.MAX_R\r\n";//B4
    buf_rcv[7] = "BT+DATA.VSS\r\n";//B5
    buf_rcv[8] = "BT+DATA.MAX_S\r\n";//B6

    buf_rcv[9] = "BT+DATA.BAD_H\r\n";//B7
    buf_rcv[10] = "BT+DATA.CAC_AFE\r\n";//B8
    buf_rcv[11] = "BT+DATA.WHP\r\n";//B9
    buf_rcv[12] = "BT+DATA.AD_Mil\r\n";//C0
    buf_rcv[13] = "BT+DATA.AD_FEH\r\n";//C1
    buf_rcv[14] = "BT+DATA.ICO2\r\n";//C2
    buf_rcv[15] = "BT+DATA.DriT\r\n";//C3
    switch(obd_flag)
    {
      case 0XB0:
			 i=1;
			 break;
	case 0XB1:
			i=3;
			break;
	case 0XB2:
			i=4;
			break;
	case 0XB3:
			i=5;
			break;
	case 0XB4:
			i=6;
			break;
	case 0XB5:
			i=7;
			break;
	case 0XB6:
			i=8;
			break;
	case 0XB7:
			i=9;
			break;
	case 0XB8:
			i=10;
			break;
	case 0XB9:
			i=11;
			break;
	case 0XC0:
			i=12;
			break;
	case 0XC1:
			i=13;
			break;
	case 0XC2:
			i=14;
			break;
	case 0XC3:
			i=15;
			break;
	default:
			break;
    }  
   	wread = write(OBDfd,buf_rcv[i], strlen(buf_rcv[i]));
	if(wread < 0)
	{
		perror("write");
		return -1;
	}
	printf("wread = %d wcv: %s\n", wread, buf_rcv[i]); 
	sleep(2);
	

  int nread=0; int j;
  nread = read(OBDfd, buf_tmp, 256);  
  printf("read:   %s\n",buf_tmp);
  for(j=0;j<= nread;j++)
  {
    
    printf("buf[%d]=%c\n",j,buf_tmp[j]);
  }
  strcpy(buf,buf_tmp);
  printf("%s\n",buf);
  pthread_mutex_unlock(&g_mtx);
} 
 


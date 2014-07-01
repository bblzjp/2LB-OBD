#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <time.h> 
#include <sys/times.h> 
#include <errno.h> 
#include <termios.h> 
#include <iconv.h> 
#include <unistd.h>
#include "usart.h" 
//#define TIMEOUT_SEC(buflen,baud) (buflen*20/baud+2) 
//#define G_NAME  "/dev/ttyS0" 
#define BUF_SIZE 1024 
 
 
//*****************************************************************************
//
//					延时函数封装
//
//*****************************************************************************
int Delay_ms(unsigned short nms){
	return (usleep((useconds_t)nms * 1000));		
}

//*****************************************************************************
//
//串口初始化
//*****************************************************************************
int set_tty_option(int fd, int nSpeed, int nBits, char nEvent, int nStop) 
{ 
 struct termios new_tio, old_tio; 
 if (tcgetattr(fd, &old_tio) != 0) //save the old parameter 
 {   perror("Setup Serial 1"); 
  	return -1; 
 } 
  
 bzero(&new_tio, sizeof(new_tio)); 
 new_tio.c_cflag |= CLOCAL | CREAD; //set local mode and enable recevie function 
 new_tio.c_cflag &= ~CSIZE;  //mask the character size bits 
 switch (nBits) 
 { 
  case 7:  new_tio.c_cflag |= CS7; break; //data: 7bits 
  case 8:  new_tio.c_cflag |= CS8; break; //data: 8bits 
  default: break; 
 } 
 switch (nEvent) 
 { 
  case 'O':  //奇校验 
   new_tio.c_cflag |= PARENB; 
   new_tio.c_cflag |= PARODD; 
   new_tio.c_iflag |= (INPCK | ISTRIP); 
   break; 
  case 'E':  //偶校验 
   new_tio.c_iflag |= (INPCK | ISTRIP); 
   new_tio.c_cflag |= PARENB; 
   new_tio.c_cflag &= ~PARODD; 
   break; 
  case 'N':  //无校验 
   new_tio.c_cflag &= ~PARENB; 
   break; 
 } 
 switch (nSpeed)  //set the Baudary 
 { 
  case 2400: 
   cfsetispeed(&new_tio, B2400); 
   cfsetospeed(&new_tio, B2400); 
   break; 
  case 4800: 
   cfsetispeed(&new_tio, B4800); 
   cfsetospeed(&new_tio, B4800); 
   break; 
  case 9600: 
   cfsetispeed(&new_tio, B9600); 
   cfsetospeed(&new_tio, B9600); 
   break; 
  case 38400: 
   cfsetispeed(&new_tio, B38400); 
   cfsetospeed(&new_tio, B38400); 
   break;
  case 115200:    
   cfsetispeed(&new_tio, B115200); 
   cfsetospeed(&new_tio, B115200); 
   break; 
  case 460800: 
   cfsetispeed(&new_tio, B460800); 
   cfsetospeed(&new_tio, B460800); 
   break; 
  default: 
   cfsetispeed(&new_tio, B9600); 
   cfsetospeed(&new_tio, B9600); 
   break; 
 } 
 //set the one bit stop 
 if (nStop == 1)  
 { 
  new_tio.c_cflag &= ~CSTOPB; 
 } 
 else if (nStop == 2) 
 { 
  new_tio.c_cflag |= CSTOPB; 
 } 
 
 new_tio.c_cc[VTIME] = 1; 
 new_tio.c_cc[VMIN] = 1; 
 tcflush(fd, TCIFLUSH);  //refresh received data buf don't read them 
 if (tcsetattr(fd, TCSANOW, &new_tio) != 0) 
 { 
  perror("serial port set error."); 
  return -1; 
 } 
 printf("serial port set done!\n"); 
 return 0; 
} 

//*****************************************************************************

//读取串口
//*****************************************************************************
void read_message(int tty_fd,char *message_buf) 
{ 
 char buf_rcv[BUF_SIZE] = {0}; 
 int nread=0; 
 
 while(nread == 0)//直到串口返回数据视为读取数据成功 
{ 
   nread = read(tty_fd, buf_rcv, BUF_SIZE); 
   sleep(1); 
} 
  printf("nread = %d,rcv_message: %s\n", nread, buf_rcv); 
   
  strcpy(message_buf,buf_rcv); 
} 
//*****************************************************************************

//发送命令
//*****************************************************************************
int write_cmd(int tty_fd,const char *buff) 
{ 
   char buf_rcv[BUF_SIZE] = {0}; 
   int nread=0; 
   char OK[]="OK"; 
   
   printf("Write cmd %s\n",buff); 
 while(nread == 0)//直到串口返回数据视为发送指令成功 
    { 
   write(tty_fd, buff, strlen(buff));//发送指令 
   sleep(1); 
   nread = read(tty_fd, buf_rcv, BUF_SIZE);//读取返回值 
   sleep(1); 
   }  
   printf("nread = %d,CMD_CMGF rcv: %s\n", nread, buf_rcv); 
 
   if(!strcmp(buf_rcv,OK)) 
   			return 0;//若返回值包含OK则指令设置成功，否则设置失败 
   else 
   			return -1; 
  
} 

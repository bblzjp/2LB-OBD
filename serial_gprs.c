//#include "serial_gps.h"
#include "gprsproc.h"
#include "usart.h"

#include <pthread.h>
#include <stdio.h>              // printf
#include <fcntl.h>              // open
#include <string.h>             // bzero
#include <stdlib.h>             // exit
#include <sys/times.h>          // times
#include <sys/types.h>          // pid_t
#include <termios.h>		//termios, tcgetattr(), tcsetattr()
#include <unistd.h>
#include <sys/ioctl.h>          // ioctl

int Gprs_Port_Sendc(char data)
{
	size_t len = 0;
	size_t datalen = sizeof (data);
	len = write(gprs_fd, &data, datalen);
	if(len<0 )
	{
		perror("write");
		return -1;
	}
}		

/*
	函数名：int Check_CMD(char *gsm)
	功能：  接收gprs串口命令（sim900a命令返回值，网络命令）
	参数：  char* gsm(可以没有)
*/
int Check_CMD(int len,char *gsm){	
	int g0=0,rdatalen;   

#if 0
	fd_set  fs_read;	
	struct timeval tv_timeout;       // rdatalen = strlen(Recv)*sizeof(Recv[0]);
	FD_ZERO(&fs_read);
	FD_SET(gprs_fd, &fs_read);
	tv_timeout.tv_sec = 30; //TIMEOUT_SEC(rdatalen,115200);	
	tv_timeout.tv_usec =0;// TIMEOUT_USEC;
	int fs_sel = select(gprs_fd+1, &fs_read, NULL, NULL, &tv_timeout);
#endif
	//if(1)
	{		
		gsm_count = read(gprs_fd, gsm, 256);
		char* retcmd = gsm;
		int i =0;
#if 0 	
		for(i =0;i< strlen(gsm);i++)
		{
			printf("gsm[%d]=%c\n",i,gsm[i]);
		}
#endif
		if(strlen(retcmd) >= 7)
		{
			while(*(retcmd + 6)){
				if(!strncmp(retcmd,"CIPSEOK",7))
				{
					printf("****bbl**************0k**********************\n");
					return 1;
				}
				retcmd++;
			}
		}

		retcmd = gsm;
		if(strlen(retcmd) >= 6)
		{
			while(*(retcmd + 5)){
				if(!strncmp(retcmd,"OPSEND",6))
					return 1;
				retcmd++;
			}
		}

		printf("func2 %s ,gsm %s\n",__func__,gsm);
		if(gsm_count == -1)
		{
			perror("read");
		}
		g0=len+2;	
	}


	if((gsm[gsm_count-1]==0x0a)&&(gsm[gsm_count-2]==0x0d))
	{	 
		if(gsm[g0]=='+')
		{
			if((gsm[g0+1]=='C')&&(gsm[g0+2]=='S')&&(gsm[g0+3]=='Q')&&(gsm[g0+4]==':'))
			{      
				AT_CSQ =(gsm[g0+6]-0x30)*10;
				AT_CSQ=AT_CSQ+(gsm[g0+7]-0x30);
				if(AT_CSQ>31)
					AT_CSQ=31;
				gsm_flag=3;
			}
			else if((gsm[g0+1]=='C')&&(gsm[g0+2]=='G')&&(gsm[g0+3]=='R')&&(gsm[g0+4]=='E')&&(gsm[g0+5]=='G')&&(gsm[g0+6]==':'))
			{   
				AT_STATUS=gsm[g0+10]-0x30;gsm_flag=4;
			}
			else if((gsm[g0+1]=='C')&&(gsm[g0+2]=='G')&&(gsm[g0+3]=='A')&&(gsm[g0+4]=='T')&&(gsm[g0+5]=='T')&&(gsm[g0+6]==':'))
			{
				AT_STATUS=gsm[g0+8]-0x30;
				gsm_flag=9;
			}
			else if((gsm[g0+1]=='C')&&(gsm[g0+2]=='M')&&(gsm[g0+3]=='E'))
			{
				gsm_flag=CME;
			}
			else if((gsm[gsm_count-4]=='O')&&(gsm[gsm_count-3]=='K'))
			{
			}
			}
			else if((gsm[g0]=='O')&&(gsm[g0+1]=='K'))
			{ 
				gsm_flag=OK;
				if((gsm[gsm_count-12]=='I')&&(gsm[gsm_count-11]=='P')&&(gsm[gsm_count-10]==0X20)&&(gsm[gsm_count-9]=='I')&&(gsm[gsm_count-8]=='N')&&(gsm[gsm_count-7]=='I')&&(gsm[gsm_count-6]=='T')&&(gsm[gsm_count-5]=='I')&&(gsm[gsm_count-4]=='A')&&(gsm[gsm_count-3]=='L'))
				  { 
				    gsm_flag=10;
				  }
				 
				if((gsm[gsm_count-14]=='C')&&(gsm[gsm_count-13]=='O')&&(gsm[gsm_count-12]=='N')&&(gsm[gsm_count-11]=='N')&&(gsm[gsm_count-10]=='E')&&(gsm[gsm_count-9]=='C')&&(gsm[gsm_count-8]=='T')&&(gsm[gsm_count-7]==0x20)&&(gsm[gsm_count-6]=='F')&&(gsm[gsm_count-5]=='A')&&(gsm[gsm_count-4]=='I')&&(gsm[gsm_count-3]=='L'))
				  { 
				      gsm_flag=CONNECT_FAIL;
				  } 
			}
			else if((gsm[g0+0]=='E')&&(gsm[g0+1]=='R')&&(gsm[g0+2]=='R')&&(gsm[g0+3]=='O')&&(gsm[g0+4]=='R'))
			{ 
				gsm_flag=ERROR;
			}
			
			
			else if((gsm[gsm_count-9]=='S')&&(gsm[gsm_count-8]=='E')&&(gsm[gsm_count-7]=='N')&&(gsm[gsm_count-6]=='D')&&(gsm[g0+5]==0x20)&&(gsm[gsm_count-4]=='O')&&(gsm[gsm_count-3]=='K'))
			{
				gsm_flag=6;
			}
			else if((gsm[gsm_count-11]=='S')&&(gsm[gsm_count-10]=='E')&&(gsm[gsm_count-9]=='N')&&(gsm[gsm_count-8]=='D')&&(gsm[gsm_count-7]==0x20)&&(gsm[gsm_count-6]=='F')&&(gsm[gsm_count-5]=='A')&&(gsm[gsm_count-4]=='I')&&(gsm[gsm_count-3]=='L'))
			{ 
				gsm_flag=SEND_FAIL;
			}
			else if((gsm[gsm_count-10]=='C')&&(gsm[gsm_count-9]=='L')&&(gsm[gsm_count-8]=='O')&&(gsm[gsm_count-7]=='S')&&(gsm[gsm_count-6]=='E')&&(gsm[gsm_count-5]==0x20)&&(gsm[gsm_count-4]=='O')&&(gsm[gsm_count-3]=='K'))
			{ 
				gsm_flag=7;//close_ok
			}
			else if((gsm[gsm_count-9]=='S')&&(gsm[gsm_count-8]=='H')&&(gsm[gsm_count-7]=='U')&&(gsm[gsm_count-6]=='T')&&(gsm[gsm_count-5]==0x20)&&(gsm[gsm_count-4]=='O')&&(gsm[gsm_count-3]=='K'))
			{ 
				gsm_flag=8;//shut_ok
			}
			else if(gsm[28]=='O'&&gsm[29]=='K')
			{
			  gsm_flag = OK;//激活移动场景
			}
			
			else if(gsm[gsm_count-3]=='>')
			{
			  
			  //printf("dasdsadd>>>>start11\n");
			}
			else if(gsm[g0]==0x3e)
			{ 
				gsm_flag=12;
			}	
	}		
	return 1;
}

/*
	函数名：int Gprs_Port_Send_Recv(char *Send,char *gsm)
	功能：  发送命令
	参数：  第一个为 命令，第二个接收返回值（可以省略）；
*/
int Gprs_Port_Send_Recv(char *Send,char *gsm)
{
	int  fs_sel,wdatalen;	
	int len = 0;       
	wdatalen = strlen(Send)*sizeof(Send[0]);	
	len = write(gprs_fd, Send, wdatalen);	//实际写入的长度       
	//printf("func1 %s ,gsm %s\n",__func__,gsm);
	if (len != wdatalen)      
	{           
		return (-1);   
	} 

	return Check_CMD(len,gsm);	 		
}




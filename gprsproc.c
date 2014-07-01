#include <unistd.h>
#include "gpsdate.h"
#include "gprsproc.h"
#include "serial_gprs.h"
#include "obd.h"
 pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
 pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
/**************************************************/
char 		 fip[4]={0x80,0x00,0x00,0x0F};			  	 //存放伪IP 
char* 		 udp_start="AT+CIPSTART=\"UDP\",\"042.121.125.207\",\"8686\"\r";
static u8    Alarm[2]={0};							 //记录报警状态，2字节
static u8    silent_flag=0;						 //静默标识
static u8    ex_speed=0;							 //超速报警标识
static u16    ACC_OFF=0;
static volatile u8    send_mode=0;							 //发送模式，0为定时报，上电默认；1为距离报
static u16   distance_thread;                     	 //距离阈值
static volatile u16 hhm_thread=60;							 //定时计数阈值，默认为60S发送一次
static u16    stop_bus=0; 							 //停车报警标识
int sht_fd = 0;
float mail =0;

int thread_flag = 1;
int rgprs_flag =0;
//*************************************************************************/
//	函数名GPRS模块初始化	
//	参数：    无参数	
//	功能：    根据GPRS初始化的不同阶段，分别执行各自的操作，共9步	
//*************************************************************************/

void gprs_init(char init_status)
{
	short t;
	char gsm_status,n,rs,rsn;
	char gprs_init_ok=0;
	char i;


	gsm_status=init_status;
	n=0;
	rs=0;
	rsn=0;
	while(gprs_init_ok==0)
	{
		switch(gsm_status)
		{
			case 0:
			
printf("AT,ok!!!\n");
			 usleep(10000);
			 t=0;
			 gsm_flag=0;

#if 0 
			 char ch[256] = {};
			 char *str = "AT+CIPSHUT\r";
			 write(gprs_fd,str,strlen(str));
			 sleep(1);
			 read(gprs_fd,ch,256);
			 printf("%s , %d,gsm_flag=%d\n",ch,gprs_fd,gsm_flag);
#endif
			sleep(3);			
			 Gprs_Port_Send_Recv("AT\r",gsm);//自适应默认波特率	  ,输入“AT”后，会回复”RDY“ 信号

			 while((gsm_flag==0)&&(t<1000))	              //等待 
				{
					usleep(1000);
					t++;
				}

				if(t>1000||gsm_flag!=OK)                     //不是OK，再次执行该指令
				{
					if(n++> ERRNUM )
					{
						rs=1;
						break;
					}
					gsm_status=0;
				}
				else
				{
					gsm_status=1;							  //USART3_Puts("AT ok\r\n");
					n=0;
				} 											  //OK，执行下一步指令

				break;
			case 1:

printf("ATE1,ok!!!!\n");
			
			 usleep(10000);
			 t=0;
			 gsm_flag=0;
			 Gprs_Port_Send_Recv("ATE1\r",gsm);                           //打开回显功能
			 while((gsm_flag==0)&&(t<1000))	              //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>1000)
				{
					gsm_status=0;  
					n=0;                       
				}
				else if(gsm_flag!=OK)                        //不是OK，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=1;
				}
				else
				{
					gsm_status=2;							 //USART3_Puts("ATE1 ok\r\n");
					n=0;
				}                             				 //OK，执行下一步指令

				break;
			case 2: 

printf("AT+CSQ,ok!!!!!\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			AT_CSQ=0;
			Gprs_Port_Send_Recv("AT+CSQ\r",gsm);                      //查询GPRS信号质量
			while((gsm_flag==0)&&(t<1000))	              //等待 
				{
					usleep(1000);
					t++;

				}
				if(t>1000||((gsm_flag!=CSQ)&&(gsm_flag!=OK))) //不是OK和+CSQ，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=2;
				}
				else if(gsm_flag==CSQ)                         //回显+CSQ
				{ 
					if(AT_CSQ<6)                       		   //信号过低，从AT开始
						gsm_status=0;
					else
						gsm_status=3;                          //正常信号，执行下一步指令
					n=0;
				}
				else if(gsm_flag==OK)                     
				{
					gsm_status=3;							   //USART3_Puts("AT+CSQ ok\r\n"); 
					n=0;
				}                                               //OK，执行下一步指令

				break;
			case 3: 

printf("AT+CGREG?,ok!!!!!\n");
		
			gsm_flag=0;	
			usleep(10000);
			t=0;
			gsm_flag=0;
			AT_STATUS=0;
			Gprs_Port_Send_Recv("AT+CGREG?\r",gsm);                         //是否已注册GPRS网络
			while((gsm_flag==0)&&(t<1000))	                    //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>1000||((gsm_flag!=CGREG)&&(gsm_flag!=OK))) //不是OK和+CGREG，再次执行该指令 
				{
					if(t>10000)
						printf("AT+CGREG=1,ok!!!!!\n");

					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=3;
				}
				else if(gsm_flag==CGREG)                         //回显+CGREG
				{
					if(AT_STATUS!=1)                 			 //没注册GPRS，重AT开始
					{  
						Gprs_Port_Send_Recv("AT+CGREG=1\r",gsm); 			 //启用网络注册非请求结果码
						sleep(3);
						gsm_status=3;             				 //再次查询网络注册
					}
					else if(AT_STATUS==1)          
					{ 	//printf("zu cebbllllll\n");
						gsm_status=4;							 //USART3_Puts("AT+CGREG+STATUS ok\r\n"); 
						n=0;
					}              								 //已注册，执行下一步指令
				}
				else if(gsm_flag==OK)                     
				{ 	//printf("=1 bbllllll\n");
					gsm_status=4;
					n=0;
				}                         						//OK，执行下一步指令

				break;
			case 4: 

printf("AT+CGATT?,ok!!!!\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			AT_STATUS=0;
			Gprs_Port_Send_Recv("AT+CGATT?\r",gsm);                        //模块是否附着GPRS网络
			while((gsm_flag==0)&&(t<1000))	                   //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>1000||((gsm_flag!=CGATT)&&(gsm_flag!=OK)))        //不是OK和+CGATT，再次执行该指令 
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=4;
				}
				else if(gsm_flag==CGATT)                 		//回显+CGATT
				{
					if(AT_STATUS!=1)
					{
					Gprs_Port_Send_Recv("AT+CGATT=1\r",gsm); 	 		//执行附着GPRS网络
						sleep(3);
						gsm_status=4;              		 		//再次查询GPRS网络附着
						
					}
					else if(AT_STATUS==1)           	 		//已付着GPRS网络，执行下一步指令
					{
						gsm_status=6;
						n=0;
																//USART3_Puts("AT+CGATT+STATUS ok\r\n");
					} 
					
				}
				else if(gsm_flag==OK)                  			 //OK，执行下一步指令
				{ 	
					gsm_status=6;
					n=0;
																//USART3_Puts("AT+CGATT ok\r\n");
				}

				break;
	//不执行case 5；
			case 5: 

printf("AT+CIPMUX=0,ok!!!\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			Gprs_Port_Send_Recv("AT+CIPMUX=0\r",gsm); 						//启动ip连接等于0为单路连接
			while((gsm_flag==0)&&(t<1000))	        			//等待 
				{
					usleep(1000);
					t++;
				}
				if(t>10000||gsm_flag==ERROR)            		//是ERROR，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}					
					gsm_status=5;
				}
				else if(gsm_flag==OK)						  	//成功			  
				{  	//printf("CIPMUXlll=ok\n");
					gsm_status=6; 
					n=0;
																//USART3_Puts("AT+CIPMUX 0 ok\r\n");
				}                        

				break;	 
			case 6: 

printf("AT+CIPMODE=0\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			Gprs_Port_Send_Recv("AT+CIPMODE=0\r",gsm); 			 			//选择tcp ip应用模式
				while((gsm_flag==0)&&(t<1000))	         		//等待 
				{
					usleep(1000);
					t++;
				}
				if(t>10000||gsm_flag==ERROR)              		//是ERROR，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=6;
				}
				else if(gsm_flag==OK)							//成功	  
				{
					gsm_status=7; 
					n=0;										//USART3_Puts("AT+CIPMODE 0 ok\r\n");
				}  
				break;	     
			case 7: 

printf("AT+CIPHEAD=1\r\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;		
			Gprs_Port_Send_Recv("AT+CIPHEAD=1\r",gsm);              	 //显示接收到的数据包的IP头
			while((gsm_flag==0)&&(t<1000))	            	 //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>10000||gsm_flag==ERROR)                 //是ERROR，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=7;
				}
				else if(gsm_flag==OK)			  
				{  
					gsm_status=8; 
					n=0;
				}                         					//是OK，初始化完毕
				break;	
			case 8: 

printf("AT+CIPSTATUS,ok!!!!!\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			Gprs_Port_Send_Recv("AT+CIPSTATUS\r",gsm);              	//查询当前的连接状态
			//printf("gsm_falg=%d\n",gsm_flag);
				while((gsm_flag==0)&&(t<1000))	            //等待 
				{
					usleep(1000);
					t++;

				}
				if(t>10000||gsm_flag!=1)					//是ERROR，再次执
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=0; 
				}
				
				
				
				if(gsm_flag!=10)
				{
				 printf("waiting!!\n");
				 gsm_status=8;
				}
				else
				{
				  gsm_status=9;
				  n=0;
				}											  //USART3_Puts("AT+CIPHEAD=1\r\n");
					    									  //OK，执行下一步指令 

				break;	       
			case 9: 

printf("AT+CSTT=\"CMNET,ok!!!!\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			Gprs_Port_Send_Recv("AT+CGDCONT=1,\"IP\",\"CMNET\"\r",gsm);//设置GPRS接入点已经接入到gprs网络中

			while((gsm_flag==0)&&(t<1000))	                 //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>10000||gsm_flag!=OK)                     //不是OK，再次执行该指令
				{
					if(n++>ERRNUM)
					{
				 		rs=1;
						break;
					}
				 	gsm_status=9;
				} 
			 if(gsm_flag==OK)                         //OK，执行下一步指令
 				{ 
 					gprs_init_ok = 1;
					gsm_status=10;
					n=0;
				}
				break;
				//gprs_init_ok = 1;不执行后面
			case 10: 

printf("AT+CIICR,ok!!!!!\n");
			
			usleep(10000);
			t=0;
			gsm_flag=0;
			
			Gprs_Port_Send_Recv("AT+CSTT?\r",gsm);//查询连接点
			
			Gprs_Port_Send_Recv("AT+CSTT\r",gsm);
			
			Gprs_Port_Send_Recv("AT+CIPSTATUS\r",gsm);
			
			Gprs_Port_Send_Recv("AT+CIICR\r",gsm); //激活移动场景
		
			while((gsm_flag==0)&&(t<1000))	              //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>1000||gsm_flag!=OK)                     //不是OK，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=10;
				}
				else if(gsm_flag==OK)		   
				{ 
					gsm_status=11;
					n=0;
															  //USART3_Puts("AT+CIICR ok\r\n");
				}   										  //OK，执行下一步指令
				break;
			case 11: 

printf("AT+CIFSR,ok!!!\n");
printf("gprs_init ok!!!!!\n");
	
			rs=0;
			gsm_flag=0;	
			usleep(10000);
			t=0;
			gsm_flag=0;
			Gprs_Port_Send_Recv("AT+CIFSR\r",gsm);                   	  //获取本地IP地址
				while((gsm_flag==0)&&(t<1000))	              //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>10000||gsm_flag==ERROR)                  //是ERROR，再次执行该指令
				{
					if(n++>ERRNUM)
					{
						rs=1;
						break;
					}
					gsm_status=11;
				}
				else 
				{  
					gprs_init_ok=1;
					n=0;
					rs=0;
															  //USART3_Puts("AT+CIFSR ok\r\n"); 
				} 											  //不是ERROR，执行下一步指令
				break;

			default:break;
		}
		if(rs==1)
		{
			rs=0;
			if(rsn>1)
			{
				rsn=0;

printf("FAIL ,reboot..........!!!!\n");

																//	return;
			printf("reboot\n"); 									//重启整个板子
			}
			rsn++;
			gsm_status=0;
			sleep(2); 				 							//暂停2s
		}
	}
 }

void gsm_init(void)
{ 
	short t;
	char gsm_init_ok=0;
	char gsm_status=0;

printf("->gsm_init\n");


	while(gsm_init_ok==0)
	{
		switch(gsm_status)
		{
			case 0: 
				usleep(10000);
				t=0;
				gsm_flag=0;
				Gprs_Port_Send_Recv("AT+CNMI=2,2,,1\r",gsm);             		//短信不做存储，直接从串口输出        
				while((gsm_flag==0)&&(t<1000))	             
				{
					usleep(1000);
					t++;
				}
				if(t>1000||gsm_flag==ERROR)                        //是ERROR，再次执行该指令
				{
					gsm_status=0;
				}
				else 
				{ 
					gsm_status=1;
printf("AT+CNMI=2,2,,1,ok!!!message!!\n");
 
				} 													//不是ERROR，执行下一步指令
				break;	 
			case 1: 
				usleep(10000);
				t=0;
				gsm_flag=0;
				Gprs_Port_Send_Recv("AT+CMGF=0\r",gsm);                         //PDU模式
				while((gsm_flag==0)&&(t<1000))	              	    //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>1000||gsm_flag==ERROR)                        //是ERROR，再次执行该指令
				{
					gsm_status=1;
				}
				else 
				{   
					gsm_status=2; 
printf("AT+CMGF,ok!!!udp,ok!!\n");
				} 													//不是ERROR，执行下一步指令
				break;	
			case 2: 
				usleep(10000);
				t=0;
				gsm_flag=0;
				Gprs_Port_Send_Recv("AT+CLIP=1\r",gsm);                         //来电显示 
				while((gsm_flag==0)&&(t<1000))	                    //等待 
				{
					usleep(1000);
					t++;
				}
				if(t>1000||gsm_flag==ERROR)                        //是ERROR，再次执行该指令
				{
					gsm_status=2;
				}
				else 
				{   
					gsm_init_ok=1; 
				} 													//不是ERROR，执行下一步指令
				break;	
			default: break;		 	 	
		}
	}
	/*Gprs_Port_Send_Recv("AT+CHFA=0\r",gsm);								//切换音频通道为主通道(0)
	usleep(10000);
	printf("AT+CHFA=0,ok!!!!\n");
	Gprs_Port_Send_Recv("AT+CLVL=60\r",gsm);								    // 接收器音量级别
	usleep(10000);
	printf("AT+CLVL=60,ok!!!!\n");
	Gprs_Port_Send_Recv("AT+CMIC=0,6\r",gsm);									//改变麦克风增益等级主通道(0)+9db
	usleep(10000);
	printf("AT+CMIC=0,6,ok!!!!\n");
	Gprs_Port_Send_Recv("AT+SIDET=0,0\r",gsm);									//改变麦克风增益等级主通道0，增益0
	usleep(10000);
	printf("AT+SIDET=0,0,ok!!!!\n");
	Gprs_Port_Send_Recv("AT+ECHO=0,7,4\r",gsm);									//回音消减控制主通道回音抑制等级
	usleep(10000);
	printf("AT+ECHO=0,7,4,ok!!!!\n");*/

	printf("<-\n");

 }

/*函数名：GPRS数据发送*/
inline void gsm_send(char send_status,char* Data)
{ 
	int t;
       int j=0;
	char str[350];
	char gsm_status;
	char gsm_send_ok=0;
	char i,n;
	n=0;
	char tmp = 0;
	int count=0;
printf("->gsm_send\n");

	gsm_status=send_status;
	while(gsm_send_ok==0)
	{
		switch(gsm_status)
		{
			case 0:   
				usleep(10000); 
				t=0;
				gsm_flag=0;
				Gprs_Port_Send_Recv("AT+CIPCLOSE\r",gsm);                      //关闭TCP连接
				while((gsm_flag==0)&&(t<1000))	                   //等待 
				{
					usleep(1000);
					t++; 
				}
				if(t>1000)                                        //时间溢出，重新执行
					gsm_status=0;
				else if(gsm_flag==CLOSE_OK)                        //CLOSE OK
				{ 
 					gsm_status=1;
					printf("CLOSE OK ok\r\n");										//正常发送完毕，跳出循环
				} 
				else if(gsm_flag==ERROR)							//error,重新执行
				{
					gsm_status=3; 
				}
				break;			 
			case 1:
				usleep(10000);
				t=0;
				gsm_flag=0;
				Gprs_Port_Send_Recv(udp_start,gsm);
				sleep(3);
				memset(str,0,350);//初始化
				int rlen = read(gprs_fd,str,350);
				printf("func=%s\nstr=%s\n",__FUNCTION__,str);
				
				
				
	 if (str[46] == 'A'&&(str[47]) == 'L'&&(str[48]) == 'R'&&(str[49])== 'E' &&(str[50]) == 'A'&&(str[51]) == 'D' &&(str[52]) == 'Y'&&(str[54]) == 'C'&&(str[55]) == 'O'&&(str[56]) == 'N'&&(str[57]) == 'N'&&(str[58]) == 'E'&&(str[59]) == 'C'&&(str[60]) == 'T')
					{
		 				printf("connect ok!!\n");
						gsm_status = 2;
						
					}
	else if (str[43] == 'C'&&(str[44]) == 'O'&&(str[45]) == 'N'&&(str[46])== 'N' &&(str[47]) == 'E'&&(str[48]) == 'C' &&(str[49]) == 'T'&&(str[51]) == 'O'&&(str[52]) == 'K')
					{
		 				printf("connect ok!!\n");
						gsm_status = 2;
						
					}
					else
					{
						if(count >50000)
						{
						 sim900a_power_on();
						 	
						}
						printf("connect bad!\n");
						gsm_status = 1;
						count++;
						
						
					}
				
				
				
				//printf("gsm_status = %d\n",gsm_status);
				break;	
			case 2: 
				
				Gprs_Port_Send_Recv("AT+CIPSEND\r",gsm);                   //发送不等长数据  //出现大于号,发送数据
				usleep(100000);	
				t=0;gsm_flag=0;
				t =Data[3]*256+Data[4];       				  	//取包长 
				t=t+5;
				for(i=0;i<t;i++) 
				{	
					Gprs_Port_Sendc(Data[i]);//发送数据
				}
			    
				usleep(1000000);
				char tmp = 0x1A;			//发送ctrl+z发送
				if(Gprs_Port_Send_Recv(&tmp,gsm) == 1);
				{
 
				}
				usleep(1000000);//可能延时不够

				printf("func=%s\ngsm=%s\n%d\n",__FUNCTION__,gsm,__LINE__);
				memset(str,0,350);//初始化
				 rlen = read(gprs_fd,str,350);
				printf("func=%s\nstr=%s\n",__FUNCTION__,str);
				
				for(t =0;t< strlen(str);t++)
				{
					printf("str[%d]=%c\n",t,str[t]);
				}
				if(rlen <= 2)
				{
 			 		perror("read");
					return;
				}
		
				char* retcmd = str;
				while(*(retcmd + 1)){
			 		if(*retcmd == 'O' && *(retcmd + 1) == 'K')
					{
		 				printf("*****1*send ok  %d*****\n",__LINE__);
						gsm_send_ok = 1;
						j=0;
						return ;
					}
					retcmd++;
				}
				char* retcmd1 = gsm;
				while(*(retcmd1 + 1)){
			 		if(*retcmd1 == 'O' && *(retcmd1 + 1) == 'K')
					{
						char tmp = 0x1A;			//发送ctrl+z发送
						if(Gprs_Port_Send_Recv(&tmp,gsm) == 1);
							{
 
							}
						usleep(1000000);//可能延时不够
		 				printf("*****2*send ok  %d*****\n",__LINE__);
						gsm_send_ok = 1;
						j=0;
						return ;
					}
					retcmd1++;
				}
				//if(j>20)
				if(j>5)
				{
					//system("reboot");//当不返回send ok 时可以选择重新启动系统
					//数据已经发送出去只是所发出去的数据不是以0x0d结尾的
					j=0;
					gsm_send_ok = 1;
					printf("*****3*send ok  %d*****\n",__LINE__);
					return ;			
				}
				j++;
				printf("j=%d\n",j);
				break;

				
				
			default:break;
		}
		if(n>ERRNUM)				   				
		{

		}
	} 
      printf("<-\n");
 } 

void send_jump(void)
{
	int xor,i;

	for(i=0;i<60;i++)//初始化60字节
	{
	  gprs_send[i]=0;
	}													//设置包头，发往服务器指令0X21(心跳包)，包长：指令0X21共6字节
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0x21;
	gprs_send[3]=0x00;
	gprs_send[4]=0x06;
														//设置伪IP
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];

	
	xor=0;
	for(i=0;i<9;i++)				//校验
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[9]=xor;      //设置校验码
	gprs_send[10]=0x0d;    //设置包尾码
}

void command_A7(void)
{	u8 xor;
	u8 i;
	char buf[5]={0};
	int rlong=0;
	int temp,humi;
	rlong=read(sht_fd,buf,4);
	if(rlong<0)
	{
	  perror("read");
//test***************//
#if 0
	  return;
#endif
	}
	printf("iֵ%d\n",rlong);
	temp=(buf[0]<<8)+buf[1];
	humi=(buf[2]<<8)+buf[3];
	printf("orignal data\ttemp:%d\thumi:%d\n",temp,humi);
	const float C1=-4.0;			
	const float C2=+0.0405;			
	const float C3=-0.0000028;		
	const float T1=+0.01;			
	const float T2=+0.00008;
	//************************test********
#if 1	
	humi=88;
	temp=21;
#endif			
	float rh=humi;			
	float t=temp;		
	float rh_lin;		
	float rh_true;					
	float t_C;						
	t_C=t*0.01 - 40;				
	rh_lin=C3*rh*rh + C2*rh + C1;	
	rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;	
	if(rh_true>100)rh_true=100;		
	if(rh_true<0.1)rh_true=0.1;		
	printf("converted\ttemp:%.2f\thumi:%.2f\n",t_C,rh_true);
	char tmp1[2]={0};
	char tmp2[2]={0};
	
	tmp1[0]=rh_true;
	tmp1[1]=((rh_true-tmp1[0])*100+1);
	
	tmp2[0]=t_C;
	tmp2[1]=((t_C-tmp2[0])*100+1);
   	//设置包头，发往服务器指令0XA7，包长：指令0X94共30字节
	printf("%d\n",tmp1[0]);
	printf("%d\n",tmp1[1]);
	printf("-------1-------\n");
	printf("%d\n",tmp2[0]);
	printf("%d\n",tmp2[1]);
	gprs_send[0]=0x24;gprs_send[1]=0x24;gprs_send[2]=0x94;gprs_send[3]=0x00;gprs_send[4]=0x1E;
	//设置伪IP
	gprs_send[5]=fip[0];gprs_send[6]=fip[1];gprs_send[7]=fip[2];gprs_send[8]=fip[3];
	//temperature&humidity parameter 参数	小数
	gprs_send[9]=0x00;
	gprs_send[10]=tmp1[0];	  
	gprs_send[11]=tmp1[1];	  
	if(t_C >0)	   
	 gprs_send[12]=0x00;
	else
	 gprs_send[12]=0x01;
	gprs_send[13]=tmp2[0];	 
	gprs_send[14]=tmp2[1]; 	  
	for(i=0;i<18;i++)
	{
	 gprs_send[15+i]=0x00;
	}
	xor=0;
	for(i=0;i<33;i++) 
	 { xor=xor^gprs_send[i];} 
	gprs_send[33]=xor;      //设置校验码
	gprs_send[34]=0x0D;     //设置包尾码
	printf("-------2-------\n");
	gsm_send(2,gprs_send);
	
	printf("------3--------\n");
	printf("--------------\n");
	printf("%d\n",gprs_send[10]);
	printf("%d\n",gprs_send[11]);
	printf("%d\n",gprs_send[13]);
	printf("%d\n",gprs_send[14]);
}

void command_B0(void)
{
 
	u8 xor;
	u8 i;

	char h_spwr = 0;
	char l_spwr = 0;
	char tmp[2]={0};
	unsigned char buff[128]={0};

	obd_data(0xB0,buff);
	
	sprintf(tmp,"%c%c",buff[5],buff[6]);
	h_spwr=(char)atoi(tmp);
	printf("h_spwr=%d\n",h_spwr);

	memset(tmp,0,2);

	sprintf(tmp,"%c%c",buff[8],buff[9]);
    l_spwr=(char)atoi(tmp);
	printf("l_spwr=%d\n",l_spwr);

	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB0;
	gprs_send[3]=0x00;
	gprs_send[4]=0x08;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	gprs_send[9]=h_spwr;
	gprs_send[10]=l_spwr;
	
		 
	xor=0;
	for(i=0;i<11;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[11]=xor;      //设置校验码
	gprs_send[12]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B1(void)
{
 
	u8 xor;
	u8 i;
	int j=0;
	float load = 0;
	unsigned char tmp1[2]={0};
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xB1,buff);
	
	for(i=5;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
   printf("tmp=%s\n",tmp);
   load=atof(tmp);
   printf("load=%f\n",load);
   tmp1[0]=load;
   tmp1[1]=((load-tmp1[0])*100+1);
   printf("tmp1[0]=%d\ntmp[1]=%d\n",tmp1[0],tmp1[1]);
	
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB1;
	gprs_send[3]=0x00;
	gprs_send[4]=0x08;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	gprs_send[9]=tmp1[0];
	gprs_send[10]=tmp1[1];
	
		 
	xor=0;
	for(i=0;i<11;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[11]=xor;      //设置校验码
	gprs_send[12]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B2(void)
{
 
	u8 xor;
	u8 i;
	
	int j=0;
	unsigned char ect = 0;
	char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xB2,buff);
	
	for(i=5;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	printf("tmp=%s\n",tmp);
	ect=(unsigned char)atoi(tmp);
	printf("ect=%d\n",ect);
	memset(tmp,0,128);


	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB2;
	gprs_send[3]=0x00;
	gprs_send[4]=0x08;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	if(ect >= 0)
	{
		gprs_send[9]=0;
	}
	else
	{
		gprs_send[9]=1;
	}
	gprs_send[10]=ect;
		 
	xor=0;
	for(i=0;i<11;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[11]=xor;      //设置校验码
	gprs_send[12]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B3(void)
{
 
	u8 xor;
	u8 i;
	int rpm = 0;
	char tmp[5]={0};
	unsigned char buff[128]={0};
	obd_data(0xB3,buff);

	sprintf(tmp,"%c%c%c%c%c%c",buff[5],buff[6],buff[7],buff[8],buff[9]);
	rpm =atoi(tmp);
	printf("rpm=%d\n",rpm);

	memset(tmp,0,5);

	
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB3;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	
	gprs_send[9]=(rpm &0xFF);
	gprs_send[10]=((rpm >> 8)&0xFF);
	gprs_send[11]=(((rpm >> 8)>>8)&0xFF);
	gprs_send[12]=(((rpm >> 8)>>8)>>8);
	
		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B4(void)
{
 
	u8 xor;
	u8 i;
	int max_r =0;
	char tmp[5]={0};
	unsigned char buff[128]={0};
	obd_data(0xB4,buff);
	
	sprintf(tmp,"%c%c%c%c%c",buff[7],buff[8],buff[9],buff[10],buff[11]);
	max_r = atoi(tmp);
	printf("max_r=%d\n",max_r);

	memset(tmp,0,5);

	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xC4;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	gprs_send[9]=(max_r &0xFF);
	gprs_send[10]=((max_r >> 8)&0xFF);
	gprs_send[11]=(((max_r >> 8)>>8)&0xFF);
	gprs_send[12]=(((max_r >> 8)>>8)>>8);
	
		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B5(void)
{
 
	u8 xor;
	u8 i;
	short vss = 0;
	unsigned char tmp[3]={0};
	unsigned char buff[128]={0};
	obd_data(0xB5,buff);

	memset(tmp,0,3);

	sprintf(tmp,"%c%c%c",buff[5],buff[6],buff[7]);
	vss=(short)atoi(tmp);
	printf("vss=%d\n",vss);

	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB5;
	gprs_send[3]=0x00;
	gprs_send[4]=0x08;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];

	gprs_send[9]=(vss&0xFF);
	gprs_send[10]=((vss>>8)&0xFF);
	
		 
	xor=0;
	for(i=0;i<10;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[11]=xor;      //设置校验码
	gprs_send[12]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);
	

}

void command_B6(void)
{
 
	u8 xor;
	u8 i;
	short max_s = 0;
    unsigned char tmp[3]={0};
	unsigned char buff[128]={0};
	obd_data(0xB6,buff);
 
	memset(tmp,0,3); 
    sprintf(tmp,"%c%c%c",buff[7],buff[8],buff[9]);
    max_s=(short)atoi(tmp);
    printf("max_s=%d\n",max_s);
	
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB6;
	gprs_send[3]=0x00;
	gprs_send[4]=0x08;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	gprs_send[9]=(max_s & 0xFF);
    gprs_send[10]=((max_s>>8)&0xFF);

	
		 
	xor=0;
	for(i=0;i<11;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[11]=xor;      //设置校验码
	gprs_send[12]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B7(void)
{
 
	u8 xor;
	u8 i;
    unsigned char bad_h = 0;
    unsigned char tmp[2]={0};

	unsigned char buff[128]={0};
	obd_data(0xB7,buff);

    memset(tmp,0,2);
    sprintf(tmp,"%c%c",buff[7],buff[8]);
    bad_h=(unsigned char)atoi(tmp);
    printf("bad_h=%d\n",bad_h);
	
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB7;
	gprs_send[3]=0x00;
	gprs_send[4]=0x07;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	
	gprs_send[9]=bad_h;

		 
	xor=0;
	for(i=0;i<10;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[10]=xor;      //设置校验码
	gprs_send[11]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B8(void)
{
 
	u8 xor;
	u8 i;
	int j=0;
	float cac_afe =0;
	short tmp1[2]={0};
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xB8,buff);
	for(i=9;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	  printf("tmp=%s\n",tmp);
	cac_afe=atof(tmp);
	printf("cac_afe=%f\n",cac_afe);
	tmp1[0]=cac_afe;
	tmp1[1]=((cac_afe-tmp1[0])*100+1);
	printf("tmp1[0]=%d\ntmp[1]=%d",tmp1[0],tmp1[1]);

	
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB8;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	 gprs_send[9]=(tmp1[0] &0xFF);
	 gprs_send[10]=((tmp1[0] >> 8)&0xFF);

	 gprs_send[11]=(tmp1[1] &0xFF);
	 gprs_send[12]=((tmp1[1] >> 8)&0xFF);

	
		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_B9(void)
{
 
	u8 xor;
	u8 i;
	short tmp1[2]={0};
	float whp =0;
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xB9,buff);
	int j =0;
	for(i=5;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	  printf("tmp=%s\n",tmp);
      whp=(float)atof(tmp);
      printf("whp=%f\n",whp);
	  tmp1[0]=whp;
      tmp1[1]=((whp-tmp1[0])*100+1);
	  printf("tmp1[0]=%d\ntmp[1]=%d",tmp1[0],tmp1[1]);

	
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xB9;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	gprs_send[9]=(tmp1[0] &0xFF);
	gprs_send[10]=((tmp1[0] >> 8)&0xFF);

	gprs_send[11]=(tmp1[1] &0xFF);
	gprs_send[12]=((tmp1[1] >> 8)&0xFF);
	
	
		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_C0(void)
{
 
	u8 xor;
	u8 i;
	
	int tmp1[2]={0};
	 mail =0;
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xC0,buff);
	
	int j =0;
	for(i=8;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	  printf("tmp=%s\n",tmp);
      mail=(float)atof(tmp);
      printf("mail=%f\n",mail);
	  tmp1[0]=mail;
      tmp1[1]=((mail-tmp1[0])*100+1);
	  printf("tmp1[0]=%d\ntmp[1]=%d",tmp1[0],tmp1[1]);
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xC0;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	 gprs_send[9]=(tmp1[0] &0xFF);
	 gprs_send[10]=((tmp1[0] >> 8)&0xFF);

	 gprs_send[11]=(((tmp1[0]>>8)>>8) &0xFF);//low 24 bit
	 gprs_send[12]=((tmp1[1])&0xFF);// only high 8 bit
	

		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}

void command_C1(void)
{
 
	u8 xor;
	u8 i;
	
	int tmp1[2]={0};
	float ad_feh =0;
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xC1,buff);
	
	int j =0;
	for(i=8;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	  printf("tmp=%s\n",tmp);
      ad_feh=(float)atof(tmp);
      printf("ad_feh=%f\n",ad_feh);
	  tmp1[0]=ad_feh;
      tmp1[1]=((ad_feh-tmp1[0])*100+1);
	  printf("tmp1[0]=%d\ntmp[1]=%d",tmp1[0],tmp1[1]);
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xC1;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	 gprs_send[9]=(tmp1[0] &0xFF);
	 gprs_send[10]=((tmp1[0] >> 8)&0xFF);

	 gprs_send[11]=(((tmp1[0]>>8)>>8) &0xFF);//low 24 bit
	 gprs_send[12]=((tmp1[1])&0xFF);// only high 8 bit
	

		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

}
	
void command_C2(void)
{
 
	u8 xor;
	u8 i;
	
	int tmp1[2]={0};
	float ico2 =0;
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xC2,buff);
	int j =0;
	for(i=6;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	  printf("tmp=%s\n",tmp);
      ico2=(float)atof(tmp);
      printf("ico2=%f\n",ico2);
     printf("mail=%f\n",mail);
	if(mail != 0.000000)
	{
	  ico2=((int) ico2)/((int)mail)*0.1;
	printf("ico2 mail\n");
	}
      printf("ico2=%f\n",ico2);
	  tmp1[0]=ico2;
      tmp1[1]=((ico2-tmp1[0])*10);
	  printf("tmp1[0]=%d\ntmp[1]=%d",tmp1[0],tmp1[1]);
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xC2;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	gprs_send[9]=(tmp1[0] &0xFF);
	gprs_send[10]=((tmp1[0] >> 8)&0xFF);

	gprs_send[11]=(((tmp1[0]>>8)>>8) &0xFF);//low 24 bit zhengshuwei
	 gprs_send[12]=((tmp1[1])&0xFF);// only high 8 bit xiaoshuwei 
	

		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);

	
}

void command_C3(void)
{
 
	u8 xor;
	u8 i;
	
	int tmp1[2]={0};
	float dir_time =0;
	unsigned char tmp[128]={0};
	unsigned char buff[128]={0};
	obd_data(0xC3,buff);
	
	int j =0;
	for(i=6;i<strlen(buff);i++,j++)
	   {
	     sprintf(&tmp[j],"%c",buff[i]);
		 
		}
	  printf("tmp=%s\n",tmp);
      dir_time=(float)atof(tmp);
      printf("dir_time=%f\n",dir_time);
	  tmp1[0]=dir_time;
      tmp1[1]=((dir_time-tmp1[0])*100+1);
	  printf("tmp1[0]=%d\ntmp[1]=%d",tmp1[0],tmp1[1]);
	gprs_send[0]=0x24;
	gprs_send[1]=0x24;
	gprs_send[2]=0xC3;
	gprs_send[3]=0x00;
	gprs_send[4]=0x0A;
	
	gprs_send[5]=fip[0];
	gprs_send[6]=fip[1];
	gprs_send[7]=fip[2];
	gprs_send[8]=fip[3];
	
	 gprs_send[9]=(tmp1[0] &0xFF);
	 gprs_send[10]=((tmp1[0] >> 8)&0xFF);

	 gprs_send[11]=(((tmp1[0]>>8)>>8) &0xFF);//low 24 bit
	 gprs_send[12]=((tmp1[1])&0xFF);// only high 8 bit
	

		 
	xor=0;
	for(i=0;i<13;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[13]=xor;      //设置校验码
	gprs_send[14]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);
}
 /////////////////////////////////////////////////////////////////////////////////////
//函数名：终端向中心发送位置信息												  //
//参数：  无参数																  //
//功能：  取出GPS位置信息，GPRS信号强度，内部温控器值送GPRS_SEND数组准备发送 	  //
//		   一共35个字节，没用到的字节暂设为0    								   //
//																				   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
//char thread_flag;


void command_80(char* local,int* type)
{  
	u8 xor;
	u8 i;
	strcpy(gps_rmc_rec,local);
	
#if 0	
	if(*type != TIMEOUT)
		return ;
	pthread_mutex_lock(&mtx);
	thread_flag--;	
	pthread_mutex_unlock(&mtx);
#endif
	rgprs_flag = 0;
	Gprmc_date_pro();//将接收数据进行压缩BCD编码

	//设置包头，发往服务器指令0X81，包长：指令0X81共35字节

	gprs_send[0]=0x24;gprs_send[1]=0x24;gprs_send[2]=0x80;gprs_send[3]=0x00;gprs_send[4]=0x23;
	//设置伪IP
	gprs_send[5]=fip[0];gprs_send[6]=fip[1];gprs_send[7]=fip[2];gprs_send[8]=fip[3];
	//设置从GPRMC,GPGSV获取的相关位置数据
	for(i=0;i<19;i++)
	{
		gprs_send[9+i]=gps_rmc[i];
	}

	//车钥开关数据，实际包括GSM信号强度
	gprs_send[28]=0x00;//0x05;
	//	temp=gps_rmc[19];
	gprs_send[29]=gps_rmc[19];//77;//temp;//gps_rmc[19];//0x80;//gps_rmc[19];//0x80;
	gprs_send[30]=0x00;
	gprs_send[31]=0x00;
	gprs_send[32]=0x00;//0x0d;
	gprs_send[33]=0x00;//0x44;
	gprs_send[34]=0x00;//gps_rmc[20];
	gprs_send[35]=0x00;//gps_rmc[21];
	gprs_send[36]=0x00;
	gprs_send[37]=0x00;	

	//位置数据共30字节，其余暂设置为0  
	xor=0;
	for(i=0;i<38;i++) 
	{ 
		xor=xor^gprs_send[i];
	} 
	gprs_send[38]=xor;      //设置校验码
	gprs_send[39]=0x0D;     //设置包尾码
	
	printf("%s,%d\n",__func__,__LINE__);
	gsm_send(2,gprs_send);
#if 1
	pthread_mutex_lock(&mtx);
	thread_flag++; 
	pthread_mutex_unlock(&mtx);
	pthread_cond_signal(&cond);
#endif
	rgprs_flag = 1;
	
	printf("thread_flag=%d",thread_flag);
	*type = 0;

}
/////////////////////////////////////////////////////////////////////////////////////
//函数名：终端向中心发送点名位置信息												  //
//参数：  无参数																  //
//功能：  取出GPS位置信息，GPRS信号强度，内部温控器值送GPRS_SEND数组准备发送 	  //
//		   一共36个字节，没用到的字节暂设为0    								   //
//																				   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
 void command_81(void)
{ 
	u8 xor,i;


	Gprmc_date_pro();//将接收数据进行压缩BCD编码
	    	 		
	//设置包头，发往服务器指令0X81，包长：指令0X81共35字节

	gprs_send[0]=0x24;gprs_send[1]=0x24;gprs_send[2]=0x81;gprs_send[3]=0x00;gprs_send[4]=0x23;
	//设置伪IP
	gprs_send[5]=fip[0];gprs_send[6]=fip[1];gprs_send[7]=fip[2];gprs_send[8]=fip[3];
	//设置从GPRMC,GPGSV获取的相关位置数据
	for(i=0;i<19;i++)
	{
		gprs_send[9+i]=gps_rmc[i];
	}

	//赋值远程开关功能
	gprs_send[28]=0x00; //remote_switch;
	gprs_send[29]=gps_rmc[19];//0x80;
	gprs_send[30]=0x00;
	gprs_send[31]=0x00;
	gprs_send[32]=0x00;//0x0d;
	gprs_send[33]=0x00;//0x44;
	gprs_send[34]=0x00;//gps_rmc[20];
	gprs_send[35]=0x00;//gps_rmc[21];
	gprs_send[36]=0x00;
	gprs_send[37]=0x00;	
	//位置数据共30字节，其余暂设置为0  
	xor=0;
	for(i=0;i<38;i++) 
		xor=xor^gprs_send[i]; 
	gprs_send[38]=xor;      //设置校验码
	gprs_send[39]=0x0d;     //设置包尾码

	printf("********************************\n");
//	printf("func %s , thread_flag %d\n",__func__,thread_flag);
	
	gsm_send(2,gprs_send);

}

#if 1
/////////////////////////////////////////////////////////////////////////////////////
//函数名：终端向中心发送车载设备工作状态										  //
//参数：  无参数																  //
//功能：  取出有关信息送GPRS_SEND数组准备发送 	                                   //
//		   一共30个字节，没用到的字节暂设为0    								   //
//																				   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
 void command_83(void)
 {	
	u8 xor,i;
	gps_rmc_have=0; gps_gsv_have=0;
	
	printf("%s,%s\n",__FILE__,__func__);
	
	Gprmc_date_pro();		           //将接收数据进行压缩BCD编码

	//设置包头，发往服务器指令0X83(车载设备状态)，包长：指令0X83共30字节
	gprs_send[0]=0x24;gprs_send[1]=0x24;gprs_send[2]=0x83;gprs_send[3]=0x00;gprs_send[4]=0x1E;
	//设置伪IP
	gprs_send[5]=fip[0];gprs_send[6]=fip[1];gprs_send[7]=fip[2];gprs_send[8]=fip[3];
	for(i=0;i<6;i++)
	{
		gprs_send[9+i]=gps_rmc[i];
	}		 //填入采样时间6字节
	gprs_send[15]=Alarm[0];				 //填入报警状态
	gprs_send[16]=Alarm[1];
	gprs_send[17]=gps_valid;				 //填入定位
	if(send_mode==0)						 //填入采样类型，采样值
	{ 
		gprs_send[18]=1;
		gprs_send[19]=hhm_thread>>8;
		gprs_send[20]=hhm_thread&0x00ff;
	}
	else
	{ 
		gprs_send[18]=0;
		gprs_send[19]=distance_thread>>8;
		gprs_send[20]=distance_thread&0x00ff;
	}
	if(silent_flag==0)					//填入发送方式
		gprs_send[21]=1;
	else
		gprs_send[21]=2;
	
	if(stop_bus==0)				   	    //填入停车设置状态
		gprs_send[22]=0;
	else
		gprs_send[22]=stop_bus/60;
	
	if(ex_speed==0)				   	    //填入超速设置状态
		gprs_send[23]=0;
	else
		gprs_send[23]=ex_speed;

	for(i=0;i<5;i++)
	{
		gprs_send[24+i]=0;
	}		         //中间5字节暂为0

	gprs_send[29]=ACC_OFF>>8;			 //采样值（ACC关）
	gprs_send[30]=ACC_OFF&0x00ff;

	for(i=0;i<2;i++)
	{
		gprs_send[31+i]=0;
	}		         //最后2字节暂为0
	xor=0;
	for(i=0;i<33;i++) 
		xor=xor^gprs_send[i]; 
	gprs_send[33]=xor;                     //设置校验码
	gprs_send[34]=0x0d;                    //设置包尾码
	
	gsm_send(2,gprs_send);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
//函数名：终端向中心发送车载设备应答数据										  //
//参数：  主信令（M_ID）,子信令（S_ID）,成功与否（SUC-flag						  //
//功能：  表示终端收到中心的指令，但因实际的情况，有些功能暂无法完成 ，            //
//		  故 只应答不动作  								                           //
//																				   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
void command_85(u8 m_id,u8 s_id,u8 suc_flag)
{  
	u8 xor,i;

	//设置包头，发往服务器指令0X85(车载设备应答数据)，包长：指令0X85共11字节
	gprs_send[0]=0x24;gprs_send[1]=0x24;gprs_send[2]=0x85;gprs_send[3]=0x00;gprs_send[4]=0x0B;
	//设置伪IP
	gprs_send[5]=fip[0];gprs_send[6]=fip[1];gprs_send[7]=fip[2];gprs_send[8]=fip[3];
	//设置回应数据:主信令，子信令，成功为1否则为0，后两字节保留为0
	gprs_send[9]=m_id;gprs_send[10]=s_id;gprs_send[11]=suc_flag;gprs_send[12]=0;gprs_send[13]=0;
	xor=0;
	for(i=0;i<14;i++) 
		xor=xor^gprs_send[i]; 
	gprs_send[14]=xor;                     //设置校验码
	gprs_send[15]=0x0d;                    //设置包尾码
	
	printf("file %s,func %s ,line %d,",__FILE__,__func__,__LINE__);
	printf("gprs_send %s\n",gprs_send);	
	gsm_send(2,gprs_send);
}

/////////////////////////////////////////////////////////////////////////////////////
//函数名：接收中心要求的检查软件版本号信息			                              //
//参数：  无参数					                                              //
//功能：  按协议填入有关数值                                                      //
//		                                                                           //
//																                   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
void command_86(void)
{
	u8 xor,i;

	Gprmc_date_pro();		        //将接收数据进行压缩BCD编码

	//设置包头，发往服务器指令0X86，包长：指令0X86共14字节
	gprs_send[0]=0x24;gprs_send[1]=0x24;gprs_send[2]=0x86;gprs_send[3]=0x00;gprs_send[4]=0x0E;
	//设置伪IP
	gprs_send[5]=fip[0];gprs_send[6]=fip[1];gprs_send[7]=fip[2];gprs_send[8]=fip[3];
	gprs_send[9]=0xd7;				       //默认型号
	gprs_send[10]=0x03;				       //默认软件版本
	
	for(i=0;i<4;i++)
		gprs_send[11+i]=gps_rmc[i];		 //填入采样时间4字节
	
	gprs_send[15]=0x3c;				         //其余暂为0
	gprs_send[16]=0x00;		         
	
	xor=0;
	for(i=0;i<17;i++) 
		xor=xor^gprs_send[i]; 
	gprs_send[17]=xor;                     //设置校验码
	gprs_send[18]=0x0d;                    //设置包尾码
			
	gsm_send(2,gprs_send);

 }

/////////////////////////////////////////////////////////////////////////////////////
//函数名：void* rgprs(void* p)												  //
//参数：  保留																  //
//功能：  接收gprs串口命令							   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
#if 0
void* rgprs(void* p){
	char command[256] = {};
	int i;
	pthread_mutex_lock(&mtx);
	while(thread_flag == 1)
	{
		printf("1rgprs,sleep!\n");
		pthread_cond_wait(&cond,&mtx);
		printf("2sleep\n");
//	}
	pthread_mutex_unlock(&mtx);
	printf("1233\n");
//	while(1)
//	{
		printf("1\tthread_flag=%d\n",thread_flag);
		printf("2\trgps_flag=%d\n",rgprs_flag);
		while(rgprs_flag)
		{
			int rlen = read(gprs_fd,command,256);
			sleep(1);
			printf("func %s, command %#X ,len %d\n",__func__,command,strlen(command));
			//fflush(stdout);	
			printf("rgprs->command[4]=%#X\n",command[4]);
			if(command[2]  == 0x24 && command[3] == 0x24)
			switch(command[4]){
					case 0x30: command_81();
						   break;
					case 0x65: printf("remote reboot 远程复位!!! !");
						   break;
					case 0x3d: command_86();
						   break;
					case 0x31: command_83();
						   break;
					case 0x68: command_A7(); //温湿度
						   break;
					
					default:
						   break;
				}
			}
	}
}

#endif 






















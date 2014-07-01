#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>             

#include "gpsdate.h"
#include "gprsproc.h"
//#include "serial_gps.h"

/**********************************************************/
volatile char gps_flag;
char gps_rec[65]={0};
char gps_count;
char gps_rmc_rec[65]={0};
char gps_gsv_rec[15]={0};
char gps_rmc_have=0;
char gps_gsv_have=0;
char gps_valid;
char sat_have_gps;
char sat_have_bd;
char gps_rmc[22]={0};

/////////////////////////////////////////////////////////////////////////////////////
//函数名：GPRMC语句数据处理			                                               //
//参数：  无参数					                                              //
//功能：  按每个字段的信息改编信息数据为协议要求的格式                             //
//		                                                                           //
//																                   //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
void Gprmc_date_pro(void)
{
  u8 i,j,k,r;
  u8 gps_temp[15]={0};
  u8 gps_bcd[10]={0};
  u8 temp;
  u8 dot_count;									          //记录”。“的位置
  u32 vec_temp;  								          //速度暂存值

	k=0;i=0;
	 do{ gps_temp[k++]=gps_rmc_rec[i++];                   //取得第1字段信息放入gps_temp[]，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                        //第1个逗号前面是“时时分分秒秒.秒秒秒（hhmmss.sss）”
	for(k=0,r=0,j=3;k<3;k++)                              //压缩BCD码，按“时分秒”排列（3，4，5）
	    {
		   temp=gps_temp[r]-0x30;
		   temp=(temp<<4)|(gps_temp[r+1]-0x30);
		   gps_rmc[j]=temp;
		   j++;
		   r+=2;
		   } 

	k=0;
	do{ gps_temp[k++]=gps_rmc_rec[i++];                    //取得第2字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                         //第2个逗号前面是“定位状态，A = 数据可用，V = 数据不可用”
    if(gps_temp[0]=='A')
	   gps_valid=1;
	else
	   gps_valid=0; 

    k=0;
	do{ gps_temp[k++]=gps_rmc_rec[i++];                    //取得第3字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                         //第3个逗号前面是“纬度，格式：度度分分.分分分分（ddmm.mmmm）”
	if(gps_temp[0]==',')			                  	   //若是没定位状态下，该字段无数据
	   {
	      gps_rmc[6]=0;gps_rmc[7]=0;gps_rmc[8]=0;gps_rmc[9]=0;
		}
	else{			 
	       gps_bcd[0]=0x30;
           for(k=0,r=1;gps_temp[k]!=',';k++) 
	          {
		        if(gps_temp[k]!='.')
		          { gps_bcd[r]=gps_temp[k];r++;}
		      } 
	       gps_bcd[r]=',';	 		   		                   //GPS_BCD[]结尾加‘，’号做结束符
	       for(k=0,r=0,j=6;gps_bcd[k]!=',';k++)                  //压缩BCD码，(6,7,8,9)
	          {
		       temp=gps_bcd[k]-0x30;
		       temp=(temp<<4)|(gps_bcd[k+1]-0x30);
		       gps_rmc[j]=temp;
		       j++;
			   r++;
			   if(r>=4)					                   //满4个字节跳出
			     break;
			   k++;
		      }	
	 	}	  	   

	k=0;
	do{ gps_temp[k++]=gps_rmc_rec[i++];                   //取得第4字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                        //第4个逗号前面是“北半球（N）或南半球（S）”
    if(gps_temp[0]=='S')
	  {	gps_rmc[6]=gps_rmc[6]|0x80;}                      //最高位加符号位

	k=0;
	do{ gps_temp[k++]=gps_rmc_rec[i++];                   //取得第5字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                        //第5个逗号前面是“经度，格式：度度度分分.分分分分”
    if(gps_temp[0]==',')			                  	//若是没定位状态下，该字段无数据
	   {
	     gps_rmc[10]=0;gps_rmc[11]=0;gps_rmc[12]=0;gps_rmc[13]=0;
		}
	else{				 
           for(k=0,r=0;gps_temp[k]!=',';k++)                     //去‘。’号重新排列入GPS_BCD[]
	          {
		        if(gps_temp[k]!='.')
		          { gps_bcd[r]=gps_temp[k];r++;}
		       }
	       gps_bcd[r]=',';	 		   	                      //GPS_BCD[]结尾加‘，’号做结束符
	       for(k=0,r=0,j=10;gps_bcd[k]!=',';k++)                //压缩BCD码，(10,11,12,13)
	         {
		       temp=gps_bcd[k]-0x30;
		       temp=(temp<<4)|(gps_bcd[k+1]-0x30);
		       gps_rmc[j]=temp;
		       j++;
			   r++;
			   if(r>=4)					                  //满4个字节跳出
			     break;
			   k++;
		     }
		  }
		 		      
  	 k=0;
	 do{ gps_temp[k++]=gps_rmc_rec[i++];                  //取得第6字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                        //第6个逗号前面是“经度区分，东（E）半球或西（W）半球）”
     if(gps_temp[0]=='W')
	  {	gps_rmc[10]=gps_rmc[10]|0x80;}                    //最高位加符号位
	  
	  k=0;
	 do{ gps_temp[k++]=gps_rmc_rec[i++];                  //取得第7字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                       //第7个逗号前面是“相对位移速度， 0.0 至 1851.8 knots”
	 if(gps_temp[0]==',')				                 //若是没定位状态下，该字段无数据
	   {
	     gps_rmc[14]=0;gps_rmc[15]=0;
		}
	 else
	    {  dot_count=0;
		   for(k=0,r=0;gps_temp[k]!=',';k++)            //去‘。’号重新排列入GPS_BCD[]并记录DOT点位
	          {
		         if(gps_temp[k]!='.')
		          { gps_bcd[r]=gps_temp[k]-0x30;r++;}
				 else
				  { dot_count=k; }
		       }
		     gps_bcd[r]=',';	 		   	        	//GPS_BCD[]结尾加‘，’号做结束符
			//将GPS中速度的单位由”海里“转换为”公里“
		   vec_temp=0;
		   switch(r)
		   {
			 case 5:vec_temp=gps_bcd[0]*10000+gps_bcd[1]*1000+gps_bcd[2]*100+gps_bcd[3]*10+gps_bcd[4];
			              break;
			 case 4:vec_temp=gps_bcd[0]*1000+gps_bcd[1]*100+gps_bcd[2]*10+gps_bcd[3];
				          break;
			 case 3:vec_temp=gps_bcd[0]*100+gps_bcd[1]*10+gps_bcd[2];
				          break;
			 case 2:vec_temp=gps_bcd[0]*10+gps_bcd[1];
				          break;
			 case 1:vec_temp=gps_bcd[0];
				          break;
			 default:break;
		   }
		    switch(k-dot_count-1)
		   {
			 case 2:vec_temp=(vec_temp*1852)/100000;
				          break;
			 case 1:vec_temp=(vec_temp*1852)/10000;
				          break;
			 default:break;
		   }
		   gps_bcd[0]=vec_temp/1000;
		   vec_temp=vec_temp-gps_bcd[0]*1000;
		   gps_bcd[1]=vec_temp/100;
		   vec_temp=vec_temp-gps_bcd[1]*100;
		   gps_bcd[2]=vec_temp/10;
		   gps_bcd[3]=vec_temp-gps_bcd[2]*10;

		   temp=gps_bcd[0];
		   gps_rmc[14]=(temp<<4)|gps_bcd[1];
		   temp=gps_bcd[2];
		    gps_rmc[15]=(temp<<4)|gps_bcd[3];
		 }	     

     k=0;
	 do{ gps_temp[k++]=gps_rmc_rec[i++];                    //取得第8字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                         //第8个逗号前面是“相对位移方向，000.0 至 359.9度”
	 if(gps_temp[0]==',')				                   //若是没定位状态下，该字段无数据
	   {
	     gps_rmc[16]=0;gps_rmc[17]=0;
		}
	 else
	    { 
	      for(k=0,r=0;gps_temp[k]!='.';k++)  
	        {
		      gps_bcd[r++]=gps_temp[k]-0x30;
	   	     }
	       gps_bcd[r]=',';	 		   		             //GPS_BCD[]结尾加‘，’号做结束符
	                                                     //压缩BCD码，(16,17)
	       gps_rmc[16]=gps_bcd[0];
	       temp=gps_bcd[1];
	       gps_rmc[17]=(temp<<4)|gps_bcd[2];
		}
     k=0;
	 do{ gps_temp[k++]=gps_rmc_rec[i++];                //取得第9字段信息，以逗号结尾
	     if(gps_rmc_rec[i-1]==',')
		    break;
		}while(1);                                     //第9个逗号前面是“日期，格式：日日月月年年（ddmmyy）”
	
	 for(k=0,r=0,j=2;gps_temp[k]!=',';k++)             //压缩BCD码，按“年月日”排列(0,1,2)
	    {
		       temp=gps_temp[k]-0x30;
		       temp=(temp<<4)|(gps_temp[k+1]-0x30);
		       gps_rmc[j]=temp;
		       j--;
			   r++;
			   if(r>=3)				                	//满3个字节跳出
			     break;
			   k++;
		 }
	 temp=gps_rmc[3]>>4;
	 temp=temp*10;
	 temp=temp+(gps_rmc[3]&0x0f)+8;

	 j=temp/10;
	 i=temp-j*10;
	 gps_rmc[3]=(j<<4)|i;

 }
#if 0
void tel_gps(void)
 { 	
 	char xor;
   	char i;
  
   	char gps_check[65]={0};
 	Serial_Gps_Init();
#if DEBUG_115kb
printf("->tel_gps\n");
#endif

    gps_rmc_have=0;
	gps_gsv_have=0;
    simu_serial_gprmc(gps_check);					//模拟串口接收GPS数据
    if(gps_rmc_have)			            //GPRMC数据帧接收完毕
       {
	    
        gps_rmc_have=0;
	    Gprmc_date_pro();
	   }									//将接收数据进行压缩BCD编码

	simu_serial_gpgsv(gps_check);	
     if(gps_gsv_have)			            //GPGSV数据帧接收完毕
       { 
        gps_gsv_have=0;
	    Gpgsv_date_pro();
	   }	
	   
	simu_serial_bdgsv(gps_check);	
     if(gps_gsv_have)			            //BDGSV数据帧接收完毕
       { 
        gps_gsv_have=0;
	 	Bdgsv_date_pro();
	 }		      		
       //设置包头，发往手柄指令0XA0，包长：26+2=28字节
	   gprs_send[0]=0x24;
	   gprs_send[1]=0xa0;
	   gprs_send[2]=0x1c;
       //设置从GPRMC,GPGSV获取的相关位置数据
       for(i=0;i<3;i++)
         {
         	gprs_send[3+i]=gps_rmc[i+3];
		 }	//时间，压缩BCD码
	   for(i=0;i<4;i++)
         {
         	gprs_send[6+i]=gps_rmc[i+10];
		 }	//经度，压缩BCD码
	   for(i=0;i<4;i++)
         {
         	gprs_send[10+i]=gps_rmc[i+6];
		}	//纬度，压缩BCD码
	   for(i=0;i<5;i++)
         {
         	gprs_send[14+i]=gps_rmc[i+14];
		 }	//速度，方位，定位状态
	   //类型状态,如要正常收发短信，须弄到GSM模式
		gprs_send[19]=0x13;
		//超速限制
		gprs_send[20]=0x00;
		//保留
		gprs_send[21]=0x00;
		gprs_send[22]=0x00;
		//网络信号
		gprs_send[23]=AT_CSQ;
		//ACC状态，不为0，已点火
		gprs_send[24]=0x00;
		//看车状态,不为0则已看车
		gprs_send[25]=0x00;//gps_rmc[20];
        for(i=0;i<3;i++)
         {
         	gprs_send[26+i]=gps_rmc[i];
		 }	//日期，压缩BCD码
	    xor=0;
       for(i=0;i<29;i++) 
         { 
        	 xor=xor^gprs_send[i];
		 } 
	  gprs_send[29]=xor;      //设置校验码
      gprs_send[30]=0x0a;     //设置包尾码

}
#endif
#if 0
void simu_serial_gprmc(char* gps_check)
{
  char gps_value;	
  char num,n;

#if DEBUG_115kb
printf("->simu_serial_gprmc\n");
#endif

  n=0;
  num=0;
  gps_flag=0;								  //开始GPS接收
  gps_count=0;
  while(1) 
   {   
	  Gps_Port_Recv(gps_check);				  //读取一字节从td的 缓冲区接收数据
#if DEBUG_115kb
printf("pgs_value=%d",gps_value);
#endif
   switch(gps_flag)						      //根据标志位，执行对应流程
    {
     case 0:
	 	if(gps_value=='$')			  		 //如果接收到“$”
    	    { 
    	    	gps_flag=1;
				gps_count=0;
				num=0;
				n=0;
			}  								 //跳转至检验语句流程
	    else
	        { 
	        	gps_flag=0;
				gps_count=0;
				num++;
			}	 							 //不是“$”字符，继续在第1步检验
	    break;
	  case 1:
	  	gps_check[gps_count]=gps_value;		 //将接收的字符放入检测语句缓冲区
	    gps_count+=1;				   		 //接收的字符个数加1
	    if(gps_count==6)			   		 //当收满6个字符
	     {							   		 //如果是“GPRMC”,跳转至GPRMC接收流程
	       if((gps_check[0]=='G')&&(gps_check[1]=='N')&&(gps_check[2]=='R')&&(gps_check[3]=='M')&&(gps_check[4]=='C')&&(gps_check[5]==','))
	            {   gps_flag=2; 
		   			gps_count=0;
				}
			else					  		  //其他语句不予接收，转回第1步
	            {   
					gps_flag=0;
					gps_count=0;
				}
		 }
	     break;
	  case 2:
	  	gps_rmc_rec[gps_count]=gps_value;	  //USART3_Putc(gps_value);
	    gps_count+=1; 
				                              //直到接收的最后两个字符是“0X0D”,“0X0A”代表一条语句接收完毕
	    if(gps_rmc_rec[gps_count]==0x0a)	  //&&(gps_rmc_rec[gps_count]==0x0a))//if(gps_count==60)
	        { 
				if(gps_rmc_rec[gps_count-1]==0x0D)	  //流程标志清0，GPRMC接收成功标志置1，字符个数清0
	         	gps_flag=0;
				gps_rmc_have=1;
				gps_count=0;
			}
		if(gps_count>250)
		    {
				gps_count=0;
				gps_flag=0;
			}
		break;
	    default:break;		 
        } 
	if(num>200)
	{
		num=0;
		n++;
		if(n>5)
		break;
	}
	if(gps_rmc_have==1)		//其中有一个语句接收完成即跳出循环
		   break; 
	}
}
#if 0
void simu_serial_gpgsv(char* gps_check)
{
  char gps_value;
  char num=0,n=0;

#if DEBUG_115kb
printf("->simu_serial_gpgsv");
#endif
	
  gps_flag=0;								 	    //开始GPS接收
  gps_count=0;
												     
  while(1) 
   {   
	  Gps_Port_Recv(gps_check);				   			 //读取一字节
#if DEBUG_115kb
printf("gps_value=%d",gps_value);
#endif
	    switch(gps_flag)						    //根据标志位，执行对应流程
        {
          case 0:
#if DEBUG_115kb
printf("0\n");
#endif
			if(gps_value=='$')			  			 //如果接收到“$”
			  	{ 
			  	  gps_flag=1; 
				  gps_count=0;
				  num=0;
				  n=0;
				}  									 //跳转至检验语句流程
	        else
	            {
	              gps_flag=0;
				  gps_count=0;
				  num++;
				}	  								 //不是“$”字符，继续在第1步检验
	        break;
	      case 1:
#if DEBUG_115kb
printf("1\n");
#endif			
				gps_check[gps_count]=gps_value;		 //将接收的字符放入检测语句缓冲区
	        	gps_count+=1;				   		 //接收的字符个数加1
	                if(gps_count==6)			  	 //当收满6个字符
	                {	                           	 //如果是“GPGSV”,跳转至GPGSV接收流程
	                   if((gps_check[0]=='G')&&(gps_check[1]=='P')&&(gps_check[2]=='G')&&(gps_check[3]=='S')&&(gps_check[4]=='V')&&(gps_check[5]==','))
	                    { 
	                    	gps_flag=2;
					   		gps_count=0; 
						}
						else					     //其他语句不予接收，转回第1步
	                    {   gps_flag=0;
							gps_count=0;
						}
	                }
	           break;
			   case 2:
#if DEBUG_115kb
printf("2\n");
#endif			
				   gps_gsv_rec[gps_count]=gps_value;
	               gps_count+=1; 
	              	if(gps_count>=15)                //只收取GPGSV语句前15个字符
	               		{  							 //流程标志清0，GPGSV接收成功标志置1，字符个数清0
	                  		gps_flag=0;
							gps_gsv_have=1;
							gps_count=0;
	               		} 
	               break;
	        
	       default:break;		 
        } 
		if(num>200)
		{
		   num=0;
		   n++;
		   if(n>4)break;
		}
		if(gps_gsv_have==1) 						//其中有一个语句接收完成即跳出循环
		   break; 
	 }
   													
}
void simu_serial_bdgsv(char* gps_check)
{
  char gps_value,num,n;

#if DEBUG_115kb
printf("->simu_serial_bdgsv\n");
#endif

       gps_flag=0;								   //开始GPS接收
	   gps_count=0;
	   num=0;
	   n=0;
												    
	while(1) 
   {   
		Gps_Port_Recv(gps_check);				  	   //读取一字节
#if DEBUG_115kb
printf("gps_value=%d",gps_value);
#endif

	switch(gps_flag)						       //根据标志位，执行对应流程
        {
            case 0:
				if(gps_value=='$')			  	   //如果接收到“$”
    	            { 
    	            	gps_flag=1; 
						gps_count=0;
						n=0;
						num=0;
					}  						       //跳转至检验语句流程
	                else
	                { 
	                	gps_flag=0;
						gps_count=0;
						num++;
					}	  							//不是“$”字符，继续在第1步检验
	                break;
	        case 1:
				gps_check[gps_count]=gps_value; 	//将接收的字符放入检测语句缓冲区
	               gps_count+=1;				    //接收的字符个数加1
	                if(gps_count==6)			    //当收满6个字符
	                {	                            //如果是“GPGSV”,跳转至GPGSV接收流程
	                   if((gps_check[0]=='B')&&(gps_check[1]=='D')&&(gps_check[2]=='G')&&(gps_check[3]=='S')&&(gps_check[4]=='V')&&(gps_check[5]==','))
	                    {   
	                   	 	gps_flag=2;
					gps_count=0;
			    }
			  else					 	 //其他语句不予接收，转回第1步
	                    {  
					gps_flag=0;
					gps_count=0;
			    }
	                }
	                break;

	        case 2:
			gps_gsv_rec[gps_count]=gps_value;
	               gps_count+=1; 
	              if(gps_count>=15)              	 //只收取GPGSV语句前15个字符
	               {  								 //流程标志清0，GPGSV接收成功标志置1，字符个数清0
	                  gps_flag=0;
					  gps_gsv_have=1;
					  gps_count=0;
	                } 
	               break;
	        
	       default:break;		 
        } 
		if(num>200)
		{
			num=0;
			n++;
			if(n>4)break;
		}
		if(gps_gsv_have==1) 						//其中有一个语句接收完成即跳出循环
		   break; 
	 }
												   
}
#endif
#endif 
/*
函数名：GPGSV语句数据处理			                                              
参数：  无参数					                                             
功能：  只取搜到的卫星数及定位的卫星数                                          //
*/
void Gpgsv_date_pro(void)
{
  char i,k;
  char gps_temp[15]={0};

#if DEBUG_115kb
printf("->Gpgsv_date_pro\n");
#endif

      k=0;i=0;
	 do{ gps_temp[k++]=gps_gsv_rec[i++];                 //取得第1字段信息，以逗号结尾
	     if(gps_gsv_rec[i-1]==',')
		    break;
		}while(1);                                      //第1个逗号前面是“天空中收到讯号的卫星总数”
	  k=0;
	 do{ gps_temp[k++]=gps_gsv_rec[i++];               //取得第2字段信息，以逗号结尾
	     if(gps_gsv_rec[i-1]==',')
		    break;
		}while(1);                                    //第2个逗号前面是“定位的卫星总数。”
	 for(k=0;gps_temp[k]!=',';k++)
	    {
		  	gps_temp[k]=gps_temp[k]-0x30;
		 }
	  k=0;
	 do{ gps_temp[k++]=gps_gsv_rec[i++];               //取得第3字段信息，以逗号结尾
	     if(gps_gsv_rec[i-1]==',')
		    break;
		}while(1);                                     //第3个逗号前面是“搜到的卫星总数。”
	 for(k=0;gps_temp[k]!=',';k++)
	    {
		  	gps_temp[k]=gps_temp[k]-0x30;
		 }
	  switch(k)										   //卫星数是两位数还是个位数
	  {
	    case 2:	sat_have_gps=gps_temp[0]*10+gps_temp[1];
		        break;
		case 1: sat_have_gps=gps_temp[0];
		        break; 
		 default:break;
	   }
 }

/*
函数名：BDGSV语句数据处理			                                               
参数：  无参数					                                              
功能：  只取搜到的卫星数及定位的卫星数                                         
*/
void Bdgsv_date_pro(void)
{
  char i,k;
  char gps_temp[15]={0};
  char temp3;	

#if DEBUG_115kb
printf("->Bdgsv_date_pro\n");
#endif


      k=0;i=0;
	 do{ gps_temp[k++]=gps_gsv_rec[i++];                 //取得第1字段信息，以逗号结尾
	     if(gps_gsv_rec[i-1]==',')
		    break;
		}while(1);                                       //第1个逗号前面是“天空中收到讯号的卫星总数”

	  k=0;
	 do{ gps_temp[k++]=gps_gsv_rec[i++];                 //取得第2字段信息，以逗号结尾
	     if(gps_gsv_rec[i-1]==',')
		    break;
		}while(1);                                       //第2个逗号前面是“定位的卫星总数。”

	  k=0;
	 do{ gps_temp[k++]=gps_gsv_rec[i++];                 //取得第3字段信息，以逗号结尾
	     if(gps_gsv_rec[i-1]==',')
		    break;
		}while(1);                                       //第3个逗号前面是“搜到的卫星总数。”
	 for(k=0;gps_temp[k]!=',';k++)
	    {
		  	gps_temp[k]=gps_temp[k]-0x30;
		 }
	  switch(k)										     //卫星数是两位数还是个位数
	  {
	    case 2:	sat_have_bd=gps_temp[0]*10+gps_temp[1];
		        break;
		case 1: sat_have_bd=gps_temp[0];
		        break; 
		 default:break;
	   }

	temp3=(gps_valid<<7)|(sat_have_bd<<4);
	temp3=temp3&0xf0;
	gps_rmc[18]=temp3|sat_have_bd;                       //定位数据


	temp3=AT_CSQ+10;		   							 //AT_CSQ
	if(temp3<32)
	   temp3=32;
	temp3=(temp3<<2);
	temp3=temp3&0xfc;
	temp3|=(sat_have_bd>>2);
	temp3=temp3&0xfe;
	gps_rmc[19]=temp3|0x01;                           	 //车钥开关：7--2：GSM信号强度；
														 //1：搜星数最后一位；
														 //0：车钥打开与否，设为1，打开

	#if DEBUG_115kb
	printf("<-\n");
	#endif

}
 

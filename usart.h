#ifndef  __USART_H
#define  __USART_H

#define GPRS_USART  "/dev/ttyS0"
#define TD3020C_USART "/dev/ttyS1"

#if 0
typedef  unsigned char u8;
typedef  unsigned short u16;
typedef  unsigned int u32;
#endif

int gprs_fd;
int set_tty_option(int fd, int nSpeed, int nBits, char nEvent, int nStop); 
void read_message(int tty_fd,char *message_buf);
int write_cmd(int tty_fd,const char *buff);

#endif //__USART_H

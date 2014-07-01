#ifndef __GPRSPROC_H
#define __GPRSPROC_H
#include <pthread.h>

#define ERRNUM 	     16
#define OK            1
#define ERROR         2
#define CSQ           3
#define CGREG         4
#define CME           2
#define CGATT         9
#define CONNECT_OK    5
#define CONNECT_FAIL  2
#define SEND_OK       6
#define SEND_FAIL     2
#define CLOSE_OK      7
#define SHUT_OK       8
#define STATE_IP_INI  10
#define STATE_CON_OK  11
#define SEND_START    12
#define IPD           13
#define NO_CARRIER    14
#define BUSY          15
#define TIMEOUT  60
#define OBDTIME  126

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
 
extern  pthread_mutex_t mtx;
extern  pthread_cond_t cond;
/************************extern********************************/
char		gsm[350];
char* 	udp_start;
char 	apa_tel[13]; 									
char 	ME_num[13];											
char 	AT_CSTT[16]; 									
char 	gprs_send[256];
int		gprs_fd;
short 			gsm_count;						
volatile char	gsm_flag;
volatile char	AT_STATUS;						
volatile char	AT_CSQ;
int sht_fd;
extern int thread_flag;
/********************************************************/

//void* rgprs(void*);
void gprs_init(char init_status);
void gsm_init(void);
inline void gsm_send(char send_status,char* Data);
void send_jump(void);
void command_A7(void);
void command_80(char* local,int* type);
void command_B0(void);
void command_B1(void);
void command_B2(void);
void command_B3(void);
void command_B4(void);
void command_B5(void);
void command_B6(void);
void command_B7(void);
void command_B8(void);
void command_B9(void);
void command_C0(void);
void command_C1(void);
void command_C2(void);
void command_C3(void);
#endif

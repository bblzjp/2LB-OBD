#ifndef __OBD_H
#define __OBD_H
#include <pthread.h>
pthread_mutex_t g_mtx;
int OBDfd;
int obd_data(int obd_flag,unsigned char* buf);
#endif

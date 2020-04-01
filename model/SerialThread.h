#ifndef MODEL_SERIAL_THREAD_H
#define MODEL_SERIAL_THREAD_H

#include "../lib/serial.h"
#include "../lib/acp/serial/main.h"
#include "Channel.h"

#define ST_TIMEOUT_MS 500

typedef struct sthread_st{
	int id;
	char serial_path[LINE_SIZE];
	int serial_rate;
	int state;
	Mutex rmutex;
	int fd;
	pthread_t thread;
	struct timespec cycle_duration;
	Mutex mutex;
	struct sthread_st *next;
} SerialThread;

DEC_LLIST(SerialThread)

extern void st_free ( SerialThread *item );
extern void stList_free ( SerialThreadLList *list );
extern void st_reset(SerialThread *item);
extern void st_control(SerialThread *item);
extern void *st_main ( void *arg );

#endif 

#ifndef MODEL_SERIAL_THREAD_STARTER_H
#define MODEL_SERIAL_THREAD_STARTER_H

#include <dirent.h>
#include "../lib/app.h"
#include "../lib/timef.h"
#include "../lib/serial.h"
#include "SlaveGetCommand.h"
#include "SerialThread.h"

//SerialThreadStarter creates SerialThread for each detected serial port
typedef struct {
	DIR *dfd;
	char *dir;
	char serial_pattern[LINE_SIZE];
	int serial_rate;
	char serial_config[LINE_SIZE];
	struct timespec thread_cd;
	
	int state;
	pthread_t thread;
} SerialThreadStarter;


extern int sts_init(SerialThreadStarter *item);
extern void sts_control(SerialThreadStarter *item);
extern void sts_free(SerialThreadStarter *item);
#endif 

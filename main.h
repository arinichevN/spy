#ifndef MAIN_H
#define MAIN_H

#include "lib/util.h"
#include "lib/app.h"
#include "lib/timef.h"
#include "lib/tsv.h"

#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/acp/cmd/main.h"
#include "lib/acp/tcp/main.h"
#include "lib/acp/tcp/server/common.h"
#include "lib/acp/tcp/server/parallel.h"
#include "lib/acp/serial/main.h"

#include "model/SlaveGetCommand.h"
#include "model/Channel.h"
#include "model/SerialThread.h"
#include "model/SerialThreadStarter.h"

extern Mutex serial_thread_list_mutex;
extern SerialThreadStarter serial_thread_starter;
extern SerialThreadLList serial_thread_list;
extern ChannelList channel_list;
extern Serverm server;


#define APP_NAME spy
#define APP_NAME_STR TOSTRING(APP_NAME)

#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/config/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./config/"
#endif
#define CONF_FILE_TYPE ".tsv"
#define CONFIG_FILE "" CONF_DIR "app" CONF_FILE_TYPE
#define CHANNELS_CONFIG_FILE "" CONF_DIR "channel/items" CONF_FILE_TYPE
#define CHANNELS_GET_DIR "" CONF_DIR "channel/command/get/"


extern int readSettings();
extern int initData();
extern int initApp();
extern void serverRun(int *state, int init_state);
extern void freeData();
extern void freeApp();
extern void exit_nicely();

#endif





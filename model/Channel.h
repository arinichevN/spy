#ifndef MODEL_CHANNEL_H
#define MODEL_CHANNEL_H

//#include "../lib/acp/cmd/main.h"
//#include "../lib/acp/serial/main.h"
//#include "../lib/util.h"
#include "../lib/app.h"
//#include "../lib/timef.h"
//#include "../lib/serial.h"
#include "../lib/tsv.h"
#include "SlaveGetCommand.h"

typedef struct {
   int id;
   SlaveGetCommandList gcmd_list;
} Channel;
DEC_LIST(Channel)

extern SlaveGetCommand *channel_getGetCmd(Channel *channel, const char *cmd);
extern void channelList_free (ChannelList *list);
extern int channel_initList ( ChannelList *list, const char *config_path, const char *get_dir, const char *file_type );
extern void channel_reset(Channel *item);

#endif 

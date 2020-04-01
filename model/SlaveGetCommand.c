#include "SlaveGetCommand.h"

void sgc_free(SlaveGetCommand *item){
	freeMutex ( &item->mutex );
}

int sgcList_init(SlaveGetCommandList *list, char *config_path ){
	RESET_LIST ( list )
	TSVresult *db = NULL;
    if ( !TSVinit ( &db, config_path ) ) {
        TSVclear ( db );
        return 1;
    }
    int nt = TSVntuples ( db );
    if(nt <= 0 ){
		TSVclear ( db );
		putsdo("no slave poll commands\n");
        return 1;
    }
    ALLOC_LIST ( list,nt );
    if ( list->max_length!=nt ) {
		FREE_LIST(list);
		TSVclear ( db );
        putsde ( "failed to allocate memory for channel poll list\n" );
        return 0;
    }
    for(int i = 0; i<nt; i++) {
		if(LL >= LML) break;
		char *cmd = TSVgetvalues(db, i, "cmd");
		if ( TSVnullreturned ( db ) ) {
			FREE_LIST(list);
			TSVclear ( db );
			putsde("null returned while reading channel_poll file 2\n");
			return 0;
		}
		strncpy(LIll.name, cmd, SLAVE_CMD_MAX_SIZE);
		if ( !initMutex ( &LIll.mutex ) ) {
			FREE_LIST ( list );
			TSVclear ( db );
			putsde ( "failed to initialize slave data mutex\n" );
			return 0;
		}
		LIll.result = -1;
		LL++;
		//printf("init poll LL++\n");
    }
    //printf("poll LL=%d\n",LL);
    TSVclear ( db );
    return 1;
}

void sgc_sendDataToClient (SlaveGetCommand *item, int tcp_fd ) {
	if(item->result == ACP_SUCCESS ){
		char q[ACP_BUF_MAX_LENGTH];
		lockMutex(&item->mutex);
		strncpy(q, item->data, ACP_BUF_MAX_LENGTH);
		unlockMutex(&item->mutex);
		acptcp_send(q, tcp_fd);
	}else{
		acptcp_send(ACP_EMPTY_PACK_STR, tcp_fd);
	}
}

void sgc_setData(SlaveGetCommand *item, const char *data, size_t data_len, int result){
	lockMutex(&item->mutex);
	strncpy(item->data, data, data_len);
	item->result = result;
	printdo("\tsaving result: %s, %d\n", item->data, item->result);
	unlockMutex(&item->mutex);
}

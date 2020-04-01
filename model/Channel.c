#include "Channel.h"

void channel_reset(Channel *item){
	FORLISTN(item->gcmd_list, i){
		SlaveGetCommand *gcmd = &item->gcmd_list.item[i];
		lockMutex(&gcmd->mutex);
		gcmd->result = 0;
		unlockMutex(&gcmd->mutex);
	}
}


int channel_check ( Channel *item ) {
    int success = 1;
    if ( 0 ) {
        printde ( "invalid: channel where id=%d\n", item->id );
        success = 0;
    }
    return success;
}

void channelList_free (ChannelList *list){
	FORLi{
        FORLISTN(LIi.gcmd_list, j){
			sgc_free(&LIi.gcmd_list.item[j]);
		}
		FREE_LIST(&LIi.gcmd_list);
	}
	FREE_LIST(list);
}

void channel_printInfo(ChannelList *list){
	FORLi{
		printdo("channel %d\n", LIi.id);
		FORLISTN(LIi.gcmd_list, j){
			printdo("\tcmd %s\n", LIi.gcmd_list.item[j].name);
		}
	}
}

int channel_initList ( ChannelList *list, const char *config_path, const char *get_dir, const char *file_type ) {
	RESET_LIST ( list )
    TSVresult *r = NULL;
    if ( !TSVinit ( &r, config_path ) ) {
        TSVclear ( r );
        return 1;
    }
    int n = TSVntuples ( r );
    if ( n <= 0 ) {
        TSVclear ( r );
        putsde ( "no data rows in file\n" );
        return 0;
    }
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
		TSVclear ( r );
        putsde ( "failed to allocate memory for channel list\n" );
        return 0;
    }
    for ( int i = 0; i < LML; i++ ) {
        LIi.id = TSVgetis ( r, i, "id" );
        char *get_file = TSVgetvalues ( r, i, "get" );
        if ( TSVnullreturned ( r ) ) {
            break;
        }
        char get_path[LINE_SIZE];
		get_path[0] = '\0';
		strncat(get_path, get_dir, LINE_SIZE - 1);
		strncat(get_path, get_file, LINE_SIZE - strlen(get_path) - 1);
		strncat(get_path, file_type, LINE_SIZE - strlen(get_path) - 1);
		printf("get path: %s\n", get_path);
        if(!sgcList_init(&LIi.gcmd_list, get_path)){
			TSVclear ( r );
            FREE_LIST ( list );
            return 0;
        }
        LL++;
    }
    TSVclear ( r );
    if ( list->length != list->max_length ) {
        printde ( "check file %s: list.length=%zu but %zu expected\n", config_path, list->length, list->max_length );
        FREE_LIST ( list );
        return 0;
    }
    FORLi{
		if ( !channel_check ( &LIi ) ) {
	        FREE_LIST ( list );
	        return 0;
	    }
	}
	channel_printInfo(list);
    return 1;
}



SlaveGetCommand *channel_getGetCmd(Channel *channel, const char *cmd){
	FORLISTN(channel->gcmd_list, j){
		SlaveGetCommand *item = &channel->gcmd_list.item[j];
		if(strncmp(cmd, item->name, SLAVE_CMD_MAX_SIZE) == 0){
		   return item;
		}
	}
    return NULL;
}

//void channelClientSendRawData (char *data, int tcp_fd,  Mutex *mutex ) {
	//char q[ACP_BUF_MAX_LENGTH];
	//lockMutex(mutex);
	//strncpy(q,data,ACP_BUF_MAX_LENGTH);
	//unlockMutex(mutex);
	//acptcp_send ( q, tcp_fd );
//}

//int channel_slaveGetRawData (int fd, Mutex *mutex, int channel_id, const char *cmd,  char *data, int len ) {
	//lockMutex(mutex);
	//int r = acpserial_sendChCmd (fd, channel_id, cmd);
	//if(r == ACP_ERROR_CONNECTION){
		//unlockMutex(mutex);
		//return r;
	//}
    //memset(data, 0, len * sizeof (*data));
    //SLEEP_BEFORE_READ_SLAVE
    //r = acpserial_readResponse(fd, data, len);
    //unlockMutex(mutex);
    //if(r != ACP_SUCCESS){
		//printde("\tcommunication error where channel_id=%d\n", channel_id);
		//return r;
	//}
    //r = acpserial_checkCRC(data);
    //if(r != ACP_SUCCESS){
		//return r;
	//}
	//acptcp_convertSerialPack(data);
	//return ACP_SUCCESS;
//}


//int channel_slaveSendRawData (Channel *channel, char *pack_str ) {
	//Mutex *mutex = &channel->thread->mutex;
	//int fd = channel->thread->fd;
	//lockMutex(mutex);
	//int r = acpserial_sendTcpPack(fd, pack_str);
	//unlockMutex(mutex);
	//return r;
//}

//void channel_clientSendRawData ( int channel_id, char *data, int tcp_fd,  Mutex *mutex ) {
	//char q[ACP_BUF_MAX_LENGTH];
	//lockMutex(mutex);
	//strncpy(q,data,ACP_BUF_MAX_LENGTH);
	//unlockMutex(mutex);
	//acptcp_send ( q, tcp_fd );
//}

//int channel_slaveToClient (Channel *channel, char *pack_str, int tcp_fd  ) {
	//Mutex *mutex = &channel->thread->mutex;
	//int fd = channel->thread->fd;
	//int channel_id = channel->id;
	////sending request to slave
	//lockMutex(mutex);
	//int r = acpserial_sendTcpPack(fd, pack_str);
	//if(r != ACP_SUCCESS){
		//unlockMutex(mutex);
		//return r;
	//}
	////reading slave response
	//size_t resp_len = ACP_BUF_MAX_LENGTH;
	//char response[resp_len];
    //memset(response, 0, resp_len);
    //SLEEP_BEFORE_READ_SLAVE
	//r = acpserial_readResponse(fd, response, resp_len);
	//if(r != ACP_SUCCESS){
		//printde("communication error where channel_id=%d\n", channel_id);
		//return r;
	//}
	//r = acpserial_checkCRC(response);
	//if(r != ACP_SUCCESS){
		//return r;
	//}
	//acptcp_convertSerialPack(response);
	////sending slave response to client
	//acptcp_send ( response, tcp_fd );
	//return ACP_SUCCESS;
//}

//int channel_slaveToClientText (Channel *channel, char *pack_str, int tcp_fd  ) {
	//Mutex *mutex = &channel->thread->mutex;
	//int fd = channel->thread->fd;
	////sending request to slave
	//lockMutex(mutex);
	//int r = acpserial_sendTcpPack(fd, pack_str);
	//if(r != ACP_SUCCESS){
		//unlockMutex(mutex);
		//return r;
	//}
	////from serial to inet
    //SLEEP_BEFORE_READ_SLAVE
	//if(!serial_canRead(fd, ACPSERIAL_TIMEOUT_MS)){
		//putsde("failed to read slave response\n");
		//unlockMutex(mutex);
		//return ACP_ERROR_NO_RESPONSE;
		////return ACP_ERROR_CONNECTION;
	//}
	//r = ACP_SUCCESS;
	//while(1){
		//char c;
		//ssize_t nr = read(fd, &c, 1);
		//if(nr < 1){
			//break;
		//}
		//ssize_t nw = write ( tcp_fd, &c, 1 );
		//if(nw < 1){
			//r = ACP_ERROR;
			//break;
		//}
	//}
	//unlockMutex(mutex);
	//return r;
//}



#define ADC ACP_DELIMITER_COLUMN_STR



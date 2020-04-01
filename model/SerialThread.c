#include "SerialThread.h"

extern ChannelList channel_list;

void st_free ( SerialThread *item ) {
//puts("stoping serial threads...");
//	STOP_THREAD(item->thread)
//	puts("serial threads done...");
	close(item->fd);
    freeMutex ( &item->mutex );
    free ( item );
}

void stList_free ( SerialThreadLList *list ) {
    SerialThread *item = list->top, *temp;
    while ( item != NULL ) {
        temp = item;
        item = item->next;
        st_free ( temp );
    }
    list->top = NULL;
    list->last = NULL;
    list->length = 0;
}

void st_reset(SerialThread *item){
	close(item->fd); 
	item->state = TERMINATED;
	printdo("SERIAL THREAD %d TERMINATED\n", item->id);
}

int st_checkPackId(const char *buf, size_t len, int id){
	int id1;
	int r = sscanf(buf, ADSS "%d" ADCS, &id1 );
	int nr = 1;
	if(r != nr){
		printde("failed to parse response (found:%d, expected:%d)\n", r, nr);
		return 0;
	}
	if(id1 != id) {
		printde("failed to check id (found:%d, expected:%d)\n", id1, id);
		return 0;
	}
	return 1;
}

int st_readResponse(int fd, char *buf, size_t len){
	if(!serial_canRead(fd, ACPSERIAL_TIMEOUT_MS)){
		putsde("failed to read slave response\n");
		return ACP_ERROR_NO_RESPONSE;
	}
	size_t n = serial_readUntil(fd, buf, len - 1, ACP_DELIMITER_END, 0);
    printdo("slave response: %s \tsize = %zu\n", buf, n);
	if(n == 0){
		struct stat stb;
		fstat(fd, &stb);
		if(stb.st_nlink <= 0){
			putsde("file not found\n");
			return ACP_ERROR_CONNECTION;
		}
		putsde("no response from slave\n");
		return ACP_ERROR_NO_RESPONSE;
	}
	
	if(strlen(buf) < ACP_PACK_MIN_LENGTH) return ACP_ERROR_SHORT_RESPONSE;
	return ACP_SUCCESS;
}

void st_control(SerialThread *item){
	switch(item->state){
		case RUN:{//printdo("SERIAL THREAD %d RUN\n", item->id);
			if(!serial_canRead(item->fd, ST_TIMEOUT_MS)){
				return;
			}
			size_t buf_len = ACP_BUF_MAX_LENGTH;
			char buf[buf_len];
		    memset(buf, 0, buf_len);
		    size_t n = serial_readFromTo(item->fd, buf, buf_len, ACP_DELIMITER_START, ACP_DELIMITER_END, 0); 
		    printdo("request to slave: %s \tsize = %zu\n", buf, n);
			if(n == 0){
				struct stat stb;
				fstat(item->fd, &stb);
				if(stb.st_nlink <= 0){
					putsde("file not found\n");
					//return ACP_ERROR_CONNECTION;
					st_reset(item);
					return;
				}
				putsde("no response from slave\n");
				//return ACP_ERROR_NO_RESPONSE;
				return;
			}
			
			if(n < ACP_PACK_MIN_LENGTH){
				//return ACP_ERROR_SHORT_RESPONSE;
				putsde("\tshort package\n");
				return;
			}
			//catching package=====================================================
			char cmd[ACP_CMD_MAX_LENGTH];
			int channel_id;
			int r = acpserial_extractSI(buf, buf_len, cmd, ACP_CMD_MAX_LENGTH, &channel_id);
			if(r != ACP_SUCCESS){
				//return r;
				putsde("\tfailed to parse package\n");
				return;
			}
			//searching command by channel_id and name
			printdo("\tcommand %s for channel_id=%d\n", cmd, channel_id);
			Channel *channel;
			LIST_GETBYID(channel, &channel_list, channel_id);
			if(channel == NULL) {putsde("\tchannel not found\n"); return;}
			SlaveGetCommand *gcmd = channel_getGetCmd(channel, cmd);
			if(gcmd == NULL) {putsde("\tcommand not found\n"); return;}
			//this is what we need, reading slave response:
			int dr = ACP_SUCCESS;
			size_t resp_len = ACP_BUF_MAX_LENGTH;
			char response[resp_len];
		    memset(response, 0, resp_len);
		    //NANOSLEEP(0,100000000);
		    putsdo("\treading response...\n");
			r = st_readResponse(item->fd, response, resp_len);
			if(r != ACP_SUCCESS){
				printde("\tcommunication error where channel_id=%d\n", channel_id);
				//return r;
				dr = r;
				goto savedata;
			}
			r = acpserial_checkCRC(response);
			if(r != ACP_SUCCESS){
				//return r;
				dr = r;
				goto savedata;
			}
			if(!st_checkPackId(response, resp_len, channel_id)){
				dr = ACP_ERROR_FORMAT;
				goto savedata;
			}
			acptcp_convertSerialPack(response);
			//saving result
			savedata:{
			int old_state;
	        if ( threadCancelDisable ( &old_state ) ) {
				sgc_setData(gcmd, response, resp_len, dr);
				threadSetCancelState ( old_state );
	        }
			}
			break;}
		case INIT:printf("SERIAL THREAD %d INIT\n", item->id);
		    item->state = RUN;
			break;
		case TERMINATED:
			break;
		default:
			printde("bad state where thread.id = %d\n", item->id);
			break;
	}
}

void st_cleanup_handler ( void *arg ) {
    SerialThread *item = arg;
    printf ( "cleaning up thread %s\n", item->serial_path );
}

void *st_main ( void *arg ) {
    SerialThread *item = arg;
    printdo ( "thread with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( st_cleanup_handler, item );
#endif
    while ( 1 ) {
      //  struct timespec t1 = getCurrentTime();
        //int old_state;
       // if ( threadCancelDisable ( &old_state ) ) {
            st_control(item);
           // threadSetCancelState ( old_state );
       // }
       // delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}

#include "SerialThreadStarter.h"

#define THREAD_STARTER_SLEEP_DELAY (struct timespec) {1,700}

extern Mutex serial_thread_list_mutex;
extern SerialThreadStarter serial_thread_starter;
extern SerialThreadLList serial_thread_list;
extern ChannelList channel_list;

int hasUnconnectedChannels(ChannelList *list){
	int out = 0;
	FORLi{
		FORLISTN(LIi.gcmd_list, j){
			SlaveGetCommand *gcmd = &LIi.gcmd_list.item[j];
			lockMutex(&gcmd->mutex);
			if(gcmd->result != ACP_SUCCESS){
				out++;
				unlockMutex(&gcmd->mutex);
				//printdo("not connected: %d\n", out);
				return out;
			}
			unlockMutex(&gcmd->mutex);
		}
	}
	//printdo("not connected: %d\n", out);
	return out;
}

int fileIsUsed(SerialThreadLList *list, char *path){
	FORLLi(SerialThread){
		if(item->state != TERMINATED && (strcmp(item->serial_path, path) == 0)){
			return 1;
		}
	}
	return 0;
}

SerialThread *getIdleThread(SerialThreadLList *list){
	FORLLi(SerialThread){
		if(item->state == TERMINATED){
			return item;
		}
	}
	return NULL;
}

int addThread ( SerialThread *item, SerialThreadLList *list, Mutex *list_mutex ) {
    if ( list->length >= INT_MAX ) {
        printde ( "can not load thread with id=%d - list length exceeded\n", item->id );
        return 0;
    }
    if ( list->top == NULL ) {
        lockMutex ( list_mutex );
        list->top = item;
        unlockMutex ( list_mutex );
    } else {
        lockMutex ( &list->last->mutex );
        list->last->next = item;
        unlockMutex ( &list->last->mutex );
    }
    list->last = item;
    list->length++;
    printdo ( "thread with id=%d loaded\n", item->id );
    return 1;
}

SerialThread * deleteThread ( SerialThread *item, SerialThreadLList *list, Mutex *list_mutex ) {
    SerialThread *prev = NULL;
    FOREACH_LLIST ( curr,list,SerialThread ) {
        if ( curr == item ) {
            if ( prev != NULL ) {
                lockMutex ( &prev->mutex );
                prev->next = curr->next;
                unlockMutex ( &prev->mutex );
            } else {//curr=top
                lockMutex ( list_mutex );
                list->top = curr->next;
                unlockMutex ( list_mutex );
            }
            if ( curr == list->last ) {
                list->last = prev;
            }
            list->length--;
            return curr;
        }
        prev = curr;
    }
    return NULL;
}

int startSerialThread(int fd, struct timespec cd, char *filename, int rate, ChannelList *rlist, SerialThreadLList *list, Mutex *list_mutex){
	static int id = 0;
	SerialThread *item = getIdleThread(list);
	if(item == NULL){//create new thread
		item = malloc ( sizeof * ( item ) );
	    if ( item == NULL ) {
	        putsde ( "failed to allocate memory for new thread\n" );
	        return 0;
	    }
	    memset ( item, 0, sizeof *item );
	    item->id = id;id++;
	    item->next = NULL;
	    strncpy(item->serial_path, filename, LINE_SIZE);
	    item->serial_rate = rate;
	    item->cycle_duration = cd;
	    item->fd = fd;
	    item->state = INIT;
		if ( !initMutex ( &item->mutex ) ) {
	        free ( item );
	        return 0;
	    }
	    if ( !addThread ( item, list, list_mutex ) ) {
	        freeMutex ( &item->mutex );
	        free ( item );
	        return 0;
	    }
		if ( !createMThread ( &item->thread, &st_main, item ) ) {
			freeMutex ( &item->mutex );
			free ( item );
			return 0;
		}
		putsdo("\tnew serial thread created\n");
	}else{//use old thread
		strncpy(item->serial_path, filename, LINE_SIZE);
	    item->serial_rate = rate;
	    item->cycle_duration = cd;
	    item->fd = fd;
	    item->state = INIT;
	    putsdo("\told serial thread started\n");
	}
	return 1;
    
}

void prepPort(int fd){
    NANOSLEEP(0,100000000);
}


void sts_control(SerialThreadStarter *item){
	switch(item->state){
		case SEARCH_NEED://putsdo("TS SEARCH_NEED\n");
			if(hasUnconnectedChannels(&channel_list) > 0){
				item->state = SEARCH_PATH;
			}
			break;
		case SEARCH_PATH:{//putsdo("TS SEARCH_PATH\n");
			item->state = SEARCH_NEED;
			struct dirent *dp;
			while ((dp = readdir(item->dfd)) != NULL)	{
				char *ret = strstr(dp->d_name, item->serial_pattern);	
				if(ret != NULL){		
					char filename[LINE_SIZE*2] ;
					snprintf( filename, sizeof filename, "%s/%s", item->dir, dp->d_name) ;
					printdo("TS port found: %s\n", filename);
					if(!fileIsUsed(&serial_thread_list, filename)){
						int fd;
						if (!serial_init(&fd, filename, item->serial_rate, item->serial_config)) {
					        printde("failed to initialize serial:%s at rate %d\n", filename, item->serial_rate);
					        continue;
					    }
					    printdo("\tTS starting thread for port: %s\n", filename);
                        prepPort(fd);
					    startSerialThread(fd, item->thread_cd, filename, item->serial_rate, &channel_list, &serial_thread_list, &serial_thread_list_mutex);
					}else{
						putsdo("TS we have already thread for this file\n");
					}
				}
			}
			rewinddir(item->dfd);
			}
			break;
		case INIT:puts("TS INIT");
			item->dir = "/dev";
			if ((item->dfd = opendir(item->dir)) == NULL) {
				printde("can't open %s\n", item->dir);
				return;
			}
			item->state = SEARCH_NEED;
			break;
		default:
			putsde("bad thread starter state\n");
			break;
	}
}

void sts_cleanup_handler ( void *arg ) {
    puts ( "cleaning up thread starter" );
}

void *sts_main ( void *arg ) {
    SerialThreadStarter *item = arg;
    printdo ( "threadStarter for serial_path: %s has been started\n", item->serial_pattern );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( sts_cleanup_handler, item );
#endif
    while ( 1 ) {
        int old_state;
        if ( threadCancelDisable ( &old_state ) ) {
            sts_control(item);
            threadSetCancelState ( old_state );
        }
        delayTsIdle(THREAD_STARTER_SLEEP_DELAY);
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}

int sts_init(SerialThreadStarter *item){
	item->dfd = NULL;
	item->dir = "/dev";
	item->state = INIT;
	if ( !createMThread ( &item->thread, &sts_main, item ) ) {
		free ( item );
		return 0;
	}
	return 1;
}

void sts_free(SerialThreadStarter *item){
	STOP_THREAD(item->thread);
	closedir(item->dfd);
	item->dfd = NULL;
	
}

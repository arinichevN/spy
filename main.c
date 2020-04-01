#include "main.h"

int app_state = APP_INIT;

int sock_port = -1;
int conn_num = 0;
Serverm server;

Mutex serial_thread_list_mutex = MUTEX_INITIALIZER;
SerialThreadStarter serial_thread_starter;
SerialThreadLList serial_thread_list = LLIST_INITIALIZER;
ChannelList channel_list = LIST_INITIALIZER;

#include "model/SlaveGetCommand.c"
#include "model/Channel.c"
#include "model/SerialThread.c"
#include "model/SerialThreadStarter.c"
#include "util.c"
#include "server.c"

int readSettings ( const char *data_path, int *port, struct timespec *cd, int *conn_num, int *serial_rate, char *serial_config, char *serial_pattern) {
    TSVresult *r = NULL;
    if ( !TSVinit ( &r, data_path ) ) {
        return 0;
    }
    int _port = TSVgetiById ( r, "id", "value", "port" );
    int _cd_s = TSVgetiById ( r, "id", "value", "cd_s" );
    int _cd_ns = TSVgetiById ( r, "id", "value", "cd_ns" );
    int _conn_num = TSVgetiById ( r, "id", "value", "conn_num_max" );
    int _serial_rate = TSVgetiById ( r, "id", "value", "serial_rate" );
    char *_serial_config = TSVgetvalueById ( r, "id", "value", "serial_config" );
    char *_serial_pattern = TSVgetvalueById ( r, "id", "value", "serial_pattern" );
    if ( TSVnullreturned ( r ) ) {
		TSVclear ( r );
        return 0;
    }
    *port = _port;
    cd->tv_sec = _cd_s;
    cd->tv_nsec = _cd_ns;
    *conn_num = _conn_num;
    *serial_rate = _serial_rate;
    strncpy(serial_config, _serial_config, LINE_SIZE);
    strncpy(serial_pattern, _serial_pattern, LINE_SIZE);
    TSVclear ( r );
    return 1;
}


int initApp() {
    if ( !readSettings ( CONFIG_FILE, &sock_port, &serial_thread_starter.thread_cd, &conn_num, &serial_thread_starter.serial_rate,  serial_thread_starter.serial_config, serial_thread_starter.serial_pattern) ) {
        putsde ( "failed to read settings\n" );
        return 0;
    }
    if ( !initMutex ( &serial_thread_list_mutex ) ) {
        putsde ( "failed to initialize serial_thread_list mutex\n" );
        return 0;
    }
    if ( !serverm_init(&server, sock_port, conn_num, serveRequest)){
		putsde ( "failed to initialize multythreaded server\n" );
        return 0;
	 }
    return 1;
}


int initData() {
    if ( !channel_initList ( &channel_list, CHANNELS_CONFIG_FILE, CHANNELS_GET_DIR, CONF_FILE_TYPE) ) {
        channelList_free( &channel_list );
        goto failed;
    }
    if ( !sts_init ( &serial_thread_starter ) ) {
		channelList_free ( &channel_list );
        goto failed;
    }
    return 1;
failed:
    return 0;
}

void freeData() {
	STOP_ALL_LLIST_THREADS(&serial_thread_list, SerialThread)
	channelList_free ( &channel_list );
	stList_free ( &serial_thread_list );
	sts_free(&serial_thread_starter);
}

void freeApp() {
    freeData();
    serverm_free(&server);
    freeMutex ( &serial_thread_list_mutex );
}

void exit_nicely ( ) {
    freeApp();
    putsdo ( "\nexiting now...\n" );
    exit ( EXIT_SUCCESS );
}

int main ( int argc, char** argv ) {
#ifdef MODE_FULL
    daemon ( 0, 0 );
#endif
    conSig ( &exit_nicely );
    while ( 1 ) {
       // printdo ( "%s(): %s\n", F, getAppState ( app_state ) );
        switch ( app_state ) {
        case APP_RUN:
            nanosleep ( &(struct timespec) {0,10000000}, NULL );
            break;
        case APP_INIT:
            if ( !initApp() ) {
                return ( EXIT_FAILURE );
            }
            app_state = APP_INIT_DATA;
            break;
        case APP_INIT_DATA:
            initData();
            app_state = APP_RUN;
            delayUsIdle ( 1000000 );
            break;
        case APP_STOP:
            freeData();
            app_state = APP_RUN;
            break;
        case APP_RESET:
            freeApp();
            delayUsIdle ( 1000000 );
            app_state = APP_INIT;
            break;
        case APP_EXIT:
            exit_nicely();
            break;
        default:
            freeApp();
            putsde ( "unknown application state\n" );
            return ( EXIT_FAILURE );
        }
    }
    freeApp();
    putsde ( "unexpected while break\n" );
    return ( EXIT_FAILURE );
}

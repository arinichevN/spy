extern int app_state;

int serveChannelCmd(int SERVER_FD, const char *SERVER_CMD, char *buf){
	//printdo("serving request: %s\n", buf);
	int channel_id;
	if ( !acp_packGetCellInt(buf, 1, &channel_id) ) {
		putsde("failed to get channel id\n");
		return 0;
	}
	//printdo("tcp: channel_id: %d\n", channel_id);
	Channel *channel = NULL;
	LIST_GETBYID(channel, &channel_list, channel_id)
	if(channel == NULL) return 0;
	SlaveGetCommand *gcmd = channel_getGetCmd(channel, SERVER_CMD);
	if(gcmd != NULL){
		//printdo("get cmd %s for channel %d data %s\n", SERVER_CMD, channel->id, gcmd->data);
		sgc_sendDataToClient(gcmd, SERVER_FD);
		goto success;
	}
	putsdo("request not served\n");
	return 0;
	success:
	//putsdo("request successfully served\n");
	return 1;
}

int serveAppCmd(int SERVER_FD, const char *SERVER_CMD, char *buf){
	if ( CMD_IS ( ACP_CMD_APP_PRINT ) ) {
        printData ( SERVER_FD );
        return 1;
    } else if ( CMD_IS ( ACP_CMD_APP_RESET ) ) {
		app_state = APP_RESET;
		return 1;
    } else {
		//putsdo("no app command matched\n");
	}
    return 0;
}

int serveRequest(int SERVER_FD, char *buf){
	char SERVER_CMD[ACP_CMD_MAX_LENGTH];
	memset(SERVER_CMD, 0, sizeof SERVER_CMD);
	if ( !acp_packGetCellStr(buf, 0, SERVER_CMD, ACP_CMD_MAX_LENGTH)) {
		putsde("failed to get command\n");
		return 0;
	}
	//printdo("tcp: command: %s\n", SERVER_CMD);
	if(serveChannelCmd(SERVER_FD, SERVER_CMD, buf)){
		return 1;
	}else if ( serveAppCmd(SERVER_FD, SERVER_CMD, buf) ) {
		return 1;
	}
	//printdo("tcp: unknown command: %s\n", SERVER_CMD);
	return 0;
}

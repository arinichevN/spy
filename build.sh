#!/bin/bash

APP=spy
APP_DBG=`printf "%s_dbg" "$APP"`
INST_DIR=/usr/sbin
CONF_DIR=/etc/controller
CONF_DIR_APP=$CONF_DIR/$APP

#DEBUG_PARAM="-Wall -pedantic"
DEBUG_PARAM="-Wall -pedantic -g"
MODE_DEBUG=-DMODE_DEBUG
MODE_FULL=-DMODE_FULL

NONE=-DNONEANDNOTHING

function move_bin {
	([ -d $INST_DIR ] || mkdir $INST_DIR) && \
	cp $APP $INST_DIR/$APP && \
	chmod a+x $INST_DIR/$APP && \
	chmod og-w $INST_DIR/$APP && \
	echo "Your $APP executable file: $INST_DIR/$APP";
}

function move_bin_dbg {
	([ -d $INST_DIR ] || mkdir $INST_DIR) && \
	cp $APP_DBG $INST_DIR/$APP_DBG && \
	chmod a+x $INST_DIR/$APP_DBG && \
	chmod og-w $INST_DIR/$APP_DBG && \
	echo "Your $APP executable file for debugging: $INST_DIR/$APP_DBG";
}

function move_conf {
	([ -d $CONF_DIR ] || mkdir $CONF_DIR) && \
	([ -d $CONF_DIR_APP ] || mkdir $CONF_DIR_APP) && \
	cp -r ./config $CONF_DIR_APP && \
	chmod -R a+rw $CONF_DIR_APP
	echo "Your $APP configuration files are here: $CONF_DIR_APP";
}

#your application will run on OS startup
function conf_autostart {
	cp -v init.sh /etc/init.d/$APP && \
	chmod 755 /etc/init.d/$APP && \
	update-rc.d -f $APP remove && \
	update-rc.d $APP defaults 30 && \
	echo "Autostart configured";
}

function build_lib {
	gcc $1  -c app.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c util.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c timef.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c tsv.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1	-c serial.c -D_REENTRANT $DEBUG_PARAM  && \
	cd acp && \
	gcc  -c main.c -D_REENTRANT $DEBUG_PARAM  && \
	cd serial && \
	gcc  -c main.c -D_REENTRANT $DEBUG_PARAM  && \
	cd ../tcp && \
	gcc	-c main.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc	-c server/parallel.c -D_REENTRANT $DEBUG_PARAM  && \
	
	cd ../ && \
	cd ../ && \
	echo "library: making archive..." && \
	rm -f libpac.a
	ar -crv libpac.a app.o util.o timef.o tsv.o serial.o  acp/main.o   acp/serial/main.o  acp/tcp/main.o  acp/tcp/parallel.o && echo "library: done"
}

function build_model {
	gcc $1  -c SlaveGetCommand.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c SerialThread.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c SerialThreadStarter.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c Channel.c -D_REENTRANT $DEBUG_PARAM  && \
	
	#echo "model: making archive..."
	echo "model: making archive..." 
	rm -f libmodel.a
	ar -crv libmodel.a SlaveGetCommand.o SerialThread.o  SerialThreadStarter.o Channel.o && echo "model: done"
}

#    1         2
#debug_mode bin_name
function build {
	cd lib && \
	build_lib $1 && \
	cd ../
	gcc -D_REENTRANT $1 $3 main.c -o  $2 $DEBUG_PARAM -lpthread -L./lib -lpac && echo "Application successfully compiled. Launch command: ./"$2
}

function full {
	DEBUG_PARAM=$NONE
	build $NONE $APP $MODE_FULL && \
	build $MODE_DEBUG $APP_DBG $MODE_FULL && \
	move_bin && move_bin_dbg && move_conf && conf_autostart
}
function full_nc {
	DEBUG_PARAM=$NONE
	build $NONE $APP $MODE_FULL && \
	build $MODE_DEBUG $APP_DBG $MODE_FULL  && \
	move_bin && move_bin_dbg
}
function part_debug {
	clear
	build $MODE_DEBUG $APP_DBG $NONE
}

function part {
	build $NONE $APP_DBG $NONE
}

function uninstall_nc {
	pkill $APP --signal 9
	pkill $APP_DBG --signal 9
	rm -f $INST_DIR/$APP
	rm -f $INST_DIR/$APP_DBG
}
function uninstall {
	uninstall_nc
	update-rc.d -f $APP remove
	rm -rf $CONF_DIR_APP
}

#test
function build_lib_st {
	gcc $1  -c app.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c timef.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c serial.c -D_REENTRANT $DEBUG_PARAM  && \
	echo "library: making archive..." && \
	rm -f libpacst.a
	ar -crv libpacst.a app.o timef.o serial.o && echo "libpacst: done" 
}
function build_st {
	cd lib && \
	build_lib_st $1 && \
	cd ../ 
	gcc -D_REENTRANT $1 $3 test/puart/main.c -o srl $DEBUG_PARAM -pthread -L./lib -lpacst && echo "Application successfully compiled. Launch command: sudo ./srl"
}
function puart_test {
	build_st $MODE_DEBUG $APP_DBG $NONE
}

function build_lib_a1 {
	gcc $1  -c app.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c timef.c -D_REENTRANT $DEBUG_PARAM  && \
	gcc $1  -c serial.c -D_REENTRANT $DEBUG_PARAM  && \
	cd acp && \
	gcc   -c main.c -D_REENTRANT $DEBUG_PARAM  && \
	cd serial && \
	gcc   -c main.c -D_REENTRANT $DEBUG_PARAM  && \
	cd ../ && \
	cd ../ && \
	echo "library: making archive..." && \
	rm -f libpacst.a
	ar -crv libpacst.a app.o timef.o serial.o acp/main.o acp/serial/main.o && echo "libpacst: done" 
}
function build_a1 {
	cd lib && \
	build_lib_a1 $1 && \
	cd ../ 
	gcc -D_REENTRANT $1 $3 test/acp1/main.c -o acp1 $DEBUG_PARAM -pthread -L./lib -lpacst && echo "Application successfully compiled. Launch command: sudo ./acp1"
}
function acp1_test {
	build_a1 $MODE_DEBUG $APP_DBG $NONE
}

f=$1
${f}

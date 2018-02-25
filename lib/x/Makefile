
CC=clang #gcc
CFLAGS=-g -Wall -ansi -pedantic 
LDFLAGS=-pthread

VPATH=.:protocols

SERV_TARGET=serv
SERV_DIR=server
SERV_OBJS=server.o \
	sock_serv.o \
	thread_pool.o \
	file.o \
	dlog.o \
	filetrans.o 

CLI_TARGET=cli
CLI_DIR=client
CLI_OBJS=client.o \
	 sock_cli.o \
	 file.o \
	 dlog.o \
	 filetrans.o
	

ALL: $(SERV_TARGET) $(CLI_TARGET)

$(SERV_TARGET): $(SERV_OBJS)
	$(CC) -o $(SERV_TARGET) $(SERV_OBJS) $(LDFLAGS) 
	mv $(SERV_TARGET) $(SERV_DIR)

$(CLI_TARGET): $(CLI_OBJS)
	$(CC) -o $(CLI_TARGET) $(CLI_OBJS) $(LDFLAGS) 
	mv $(CLI_TARGET) $(CLI_DIR)

%.o: %.c
	 $(CC) $(CFLAGS) -c $<

depend:
	makedepend -Y protocols/*[ch] *.[ch]
clean:
	rm -f $(CLI_OBJS) $(CLI_DIR)/$(CLI_TARGET) $(SERV_OBJS) $(SERV_DIR)/$(SERV_TARGET)


# DO NOT DELETE

protocols/filetrans.o: file.h debug.h dlog.h protocols/protocol_if.h
protocols/filetrans.o: protocols/filetrans.h
client.o: dlog.h debug.h protocols/protocol_if.h protocols/filetrans.h
client.o: sock_cli.h
file.o: dlog.h debug.h
server.o: dlog.h debug.h protocols/protocol_if.h protocols/filetrans.h
server.o: sock_serv.h
sock_cli.o: dlog.h protocols/protocol_if.h sock_cli.h debug.h file.h
sock_serv.o: dlog.h protocols/protocol_if.h sock_serv.h debug.h file.h
sock_serv.o: thread_pool.h
thread_pool.o: dlog.h debug.h thread_pool.h

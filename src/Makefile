CC			:= gcc
override CFLAGS	+= -Wall -std=c11 -D_DEFAULT_SOURCE -O8 -s
LFLAGS		:= -pthread
SRV1_OBJS 	:= ../build/tcp.o			\
		   	   ../build/main.o			\
		       ../build/queue.o			\
		   	   ../build/recv.o			\
		       ../build/send.o			\

SRV2_OBJS 	:= ../build/tcp.o			\
		   	   ../build/main.o			\
		       ../build/queue.o			\
		   	   ../build/recv.o			\
		       ../build/send.o			\

all: server1 server2 server3

server1:
	$(MAKE) CFLAGS="-DSRV1" ../bin/server1-LesRetardataires

server2:
	$(MAKE) CFLAGS="-DSRV2" ../bin/server2-LesRetardataires

server3:
	$(MAKE) CFLAGS="-DSRV1 -DMULTI" ../bin/server3-LesRetardataires

../bin/server1-LesRetardataires: ../build/main.o $(SRV1_OBJS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

../bin/server2-LesRetardataires: ../build/main.o $(SRV2_OBJS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

../bin/server3-LesRetardataires: ../build/main.o $(SRV1_OBJS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

../build/%.o: %.c build-dir
	@echo 'Compiling $<'
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean build-dir run server1 server2 server3
clean:
	@rm -f server client test log.txt
	@rm -R build

build-dir:
	@mkdir -p ../build

run: client server
	gnome-terminal -- bash -c "echo Server\ :; ./bin/server1 4545; exec bash"
	gnome-terminal -- bash -c "echo Client\ :; ./bin/client1 127.0.0.1 4545; exec bash"

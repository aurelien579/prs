CC		:= gcc
override CFLAGS	+= -Wall -std=c11 -D_DEFAULT_SOURCE -O8 -s
LFLAGS	:= -pthread
OBJECTS := build/tcp.o			\
		   build/main.o			\
		   build/queue.o		\
		   build/recv.o			\
		   build/send.o			\

all: server1 server3

server1:
	$(MAKE) bin/server1-LesRetardataires

server3:
	$(MAKE) CFLAGS="-DSRV3" bin/server3-LesRetardataires

bin/server1-LesRetardataires: build/main.o $(OBJECTS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

bin/server3-LesRetardataires: build/main.o $(OBJECTS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

build-objects: $(OBJECTS)

build/%.o: src/%.c build-dir
	@echo 'Compiling $<'
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean build-dir run server1 server2 server3
clean:
	@rm -f server client test log.txt
	@rm -R build

build-dir:
	@mkdir -p build

run: client server
	gnome-terminal -- bash -c "echo Server\ :; ./bin/server1 4545; exec bash"
	gnome-terminal -- bash -c "echo Client\ :; ./bin/client1 127.0.0.1 4545; exec bash"

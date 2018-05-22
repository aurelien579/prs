CC		:= gcc
override CFLAGS	+= -Wall -Wno-unused-function -std=c11 -D_DEFAULT_SOURCE
LFLAGS	:= -pthread
OBJECTS := build/utils.o 		\
		   build/tcp.o			\
		   build/log.o			\
		   build/clock.o		\
		   build/queue.o		\
		   build/recv.o

all: server1 server2 server3

server1:
	$(MAKE) CFLAGS="" bin/server1-LesRetardataires

server2:
	$(MAKE) CFLAGS="-DNO_FAST_RETRANSMIT" bin/server2-LesRetardataires

server3:
	$(MAKE) bin/server3-LesRetardataires

bin/server1-LesRetardataires: build/server1.o $(OBJECTS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

bin/server2-LesRetardataires: build/server1.o $(OBJECTS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

bin/server3-LesRetardataires: build/server3.o $(OBJECTS)
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

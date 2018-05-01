CC		:= gcc
CFLAGS	:= -Wall -Wno-unused-function -std=c11
LFLAGS	:=
OBJECTS := build/utils.o 		\
		   build/tcp.o			\
		   build/log.o

all: bin/server1

bin/server1: build/server1.o $(OBJECTS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

test: build/test.o $(OBJECTS)
	@echo ' Linking  $@'
	@$(CC) $(LFLAGS) $^ -o $@

build/%.o: src/%.c build-dir
	@echo 'Compiling $<'
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean build-dir run
clean:
	@rm -f server client test log.txt
	@rm -R build

build-dir:
	@mkdir -p build

run: client server
	gnome-terminal -- bash -c "echo Server\ :; ./bin/server1 4545; exec bash"
	gnome-terminal -- bash -c "echo Client\ :; ./bin/client1 127.0.0.1 4545; exec bash"
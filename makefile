
# NOTE: Change the SERVER to whcihever server you want to test
SERVER=server_threee.c
PORT = 54760
PROGRAMS = server client
S = -std=c99 -ggdb
T = -lpthread

both: $(PROGRAMS)

server: ${SERVER}
	gcc $S ${SERVER} $T -o server
}

client: client.c Timer.o
	gcc $S client.c $T Timer.o -o client

Timer.o : Timer.h Timer.c
	gcc $S Timer.c -c

test: both
	server $(PORT) &
	echo Starting client
	client `hostname` $(PORT)
	ls -lr Thread_*
	du

clean:
	/bin/rm -rf $(PROGRAMS) Thread_* Timer.o

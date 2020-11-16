CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -D_GNU_SOURCE
LDLIBS += -lrt -lpthread

default: server talker

server: server.o
talker: talker.o

clean:
	$(RM) *.o *.a *.so *.out core server talker



CFLAGS += -Wall -Wextra -Wpedantic -Wwrite-strings -Wvla -Winline -Wfloat-equal -Wstack-usage=512

server: LDLIBS += -lpthread
server: server.o Pool.o Queue.o Handle.o Mirrors.o

.DEFAULT: server

debug: CFLAGS += -g
debug: .DEFAULT

clean:
	$(RM) *.o server

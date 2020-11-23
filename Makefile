.PHONY: debug clean
CFLAGS += -Wall -Wextra -Wpedantic -Wwrite-strings -Wvla -Winline -Wfloat-equal -Wstack-usage=512
LDLIBS += -lpthread
OBJS := server.o Pool.o Queue.o Handle.o Mirrors.o

.DEFAULT: server

server: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

debug: CFLAGS += -g
debug: .DEFAULT

clean:
	$(RM) *.o server

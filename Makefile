SOURCES = ringbuf.c ringbuf_test.c
OBJS = $(SOURCES:.c=.o)
LIBS = 
CFLAGS = -Wall -g

all: ringbuf_test 

ringbuf_test: $(OBJS)
	$(CC) $(CFLAGS) -o ringbuf_test $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $ $<
clean:
	$(RM) $(OBJS) ringbuf_test 

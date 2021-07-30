
CFLAGS=-I../inc #-DUNIT_DEBUG=1
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=sys_msg.o watchdog.o logging.o deviceID.o

system.a: $(OBJS)
	ar rc system.a *.o


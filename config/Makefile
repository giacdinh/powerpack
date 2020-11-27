
CFLAGS=-I../inc -I../support/json/inc #-DUNIT_DEBUG=1
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=config.o parse_json.o

config.a: $(OBJS)
	ar rc config.a *.o

clean:
	rm *.o *.a


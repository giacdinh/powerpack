
CFLAGS=-I../inc -DUNIT_DEBUG=1
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=remotem.o broadcast_id.o deviceID.o

remotem.a: $(OBJS)
	ar rc remotem.a *.o

clean: 
	rm *.o *.a


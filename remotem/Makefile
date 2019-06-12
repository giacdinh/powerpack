
CFLAGS=-I../inc
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=remotem.o broadcast_id.o

remotem.a: $(OBJS)
	ar rc remotem.a *.o

clean: 
	rm *.o *.a



CFLAGS=-I../inc -DUNIT_DEBUG=1
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=controller.o ppgps.o cloudpost.o get_json_data.o

controller.a: $(OBJS)
	ar rc ctrl.a *.o


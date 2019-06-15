
CFLAGS=-I../inc
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=controller.o ppgps.o cloudpost.o deviceID.o

controller.a: $(OBJS)
	ar rc ctrl.a *.o


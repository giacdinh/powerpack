
CFLAGS=-I../inc #-DUNIT_DEBUG=1
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=controller.o ppgps.o cloudpost.o get_json_data.o gpio.o agps.o gps_nocoord.o simgps.o

controller.a: $(OBJS)
	ar rc ctrl.a *.o

clean:
	 rm *.o *.a


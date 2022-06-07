
CFLAGS= -I../inc #-DUNIT_DEBUG=1
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=controller.o cloudpost.o get_json_data.o gpio.o sim7x00.o arduPi.o GPS.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CFLAGS)

controller.a: $(OBJS)
	ar -cvq ctrl.a *.o

clean:
	 rm *.o *.a


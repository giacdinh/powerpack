
CFLAGS=-I../inc #-DUNIT_DEBUG
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

OBJS=swupgrade.o filedownload.o postversion.o get_json_data.o

controller.a: $(OBJS)
	ar rc su.a *.o

clean:
	rm *.o *.a


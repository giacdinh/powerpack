

all: subdirs main_app 
CFLAGS=-I./inc
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

SUBDIRS = remotem system ctrl

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

main_app: main.o
		gcc -o main_app main.o system/system.a \
				remotem/remotem.a ctrl/ctrl.a -lpthread -lcurl


clean:
	rm *.o main_app 

depclean:
	rm main_app main.o remotem/remotem.a system/system.a \
			ctrl/ctrl.a ctrl/*.o remotem/*.o system/*.o

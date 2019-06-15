

all: subdirs main_app 
CFLAGS=-I./inc
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

SUBDIRS = system ctrl #remotem

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

main_app: main.o
		gcc -o main_app main.o system/system.a \
				ctrl/ctrl.a -lpthread -lcurl -lssl -lcrypto

clean:
	rm *.o main_app 

depclean:
	rm main_app main.o system/system.a \
			ctrl/ctrl.a ctrl/*.o system/*.o

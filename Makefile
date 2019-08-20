all: timestamp subdirs main_app 
CFLAGS=-I./inc
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

SUBDIRS = system ctrl remotem config

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

main_app: main.o
		gcc -o main_app main.o system/system.a remotem/remotem.a \
				ctrl/ctrl.a config/config.a -ljson-c -lpthread -lcurl -lssl -lcrypto
		rm build.h

timestamp:
	echo "#define BUILDTIME \"commit:"`git show |grep commit | cut -c 40-47`"\" \" `date`\" " > build.h

clean:
	rm *.o main_app 

depclean:
	rm main_app main.o system/system.a remotem/remotem.a remotem/*.o \
			ctrl/ctrl.a ctrl/*.o config/config.a config/*.o system/*.o

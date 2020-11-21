all: timestamp subdirs powerpack 
CFLAGS=-I./inc
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

SUBDIRS = system ctrl remotem config su

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

powerpack: main.o
		gcc -o powerpack main.o system/system.a remotem/remotem.a ctrl/ctrl.a config/config.a \
				su/su.a /usr/local/lib/libjson-c.a -lpthread -lcurl -lssl -lcrypto -lwiringPi
		rm build.h

timestamp:
	echo "#define BUILDTIME \"commit:"`git show |grep commit | cut -c 40-47`"\" \" `date`\" " > build.h

clean:
	rm *.o powerpack

depclean:
	rm powerpack main.o system/system.a remotem/remotem.a su/su.a remotem/*.o \
			ctrl/ctrl.a ctrl/*.o config/config.a config/*.o system/*.o su/*.o

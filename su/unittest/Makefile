
all: fw 

fw:
	gcc -o fw postversion.c ../../system/get_json_data.c ../../system/deviceID.c \
	-lcurl -I../../inc ../../system/logging.o ../../config/parse_json.o \
	../../bin/libjson-c.a -lssl -lcrypto

clean:
	rm fw 



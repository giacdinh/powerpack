
all: unittest deviceID gps cloudpost raspihat

unittest:
	gcc -o unittest unittest.c -I../../inc

cloudpost:
	gcc -o cloudpost cloudpost.c ../get_json_data.c -lcurl -I../../inc \
		../../system/logging.o ../../config/parse_json.o ../../bin/libjson-c.a

deviceID:
	gcc -o deviceID deviceID.c -lssl -lcrypto 

gps:
	gcc -o gps ppgps.c -I../../inc ../../system/logging.o

raspihat:
	gcc -o raspihat emnify.c -lcurl

hatgps:
	gcc -o hatgps hatgps.c -I../../inc ../../system/logging.o

clean:
	rm cloudpost deviceID unittest gps raspihat hatgps



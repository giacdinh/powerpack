
all: unittest deviceID gps cloudpost raspihat getpin setpin

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
	gcc -o hatgps hatgps.c -I../../inc

hattest:
	gcc -o hattest hattest.c -I../../inc ../../system/logging.o

getpin:
	gcc -o getpin getpin.c -lwiringPi 

setpin:
	gcc -o setpin setpin.c -lwiringPi 
loadepo:
	gcc -o loadepo loadepo.c ../agps.c ../ppgps.c -I../../inc

clean:
	rm cloudpost deviceID unittest gps raspihat hatgps getpin setpin



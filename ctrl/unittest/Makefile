
all: clean unittest deviceID

unittest:
	gcc -o unittest unittest.c -I../../inc

cloudpost:
	gcc -o cloudpost cloudpost.c -lcurl

deviceID:
	gcc -o deviceID deviceID.c -lssl -lcrypto 

clean:
	rm unittest

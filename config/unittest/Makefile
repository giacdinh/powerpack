
CFLAGS=-I../../inc -I../../support/json/inc ../../support/json/libs/libjson-c.a
all: json

json:
	gcc -o json json.c ../parse_json.c $(CFLAGS)

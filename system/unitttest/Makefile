
all: test_json

test_json:
	gcc -g -ggdb -I../../inc -o test_json test_json.c ../get_json_data.c  \
			../../system/logging.o ../../config/parse_json.o ../../bin/libjson-c.a

clean:
	rm test_json

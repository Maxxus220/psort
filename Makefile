
build: psort.c
	gcc psort.c -o psort.o -Wall -Werror -pthread -g

test: test.c
	gcc test.c -o test -Wall -Werror -pthread -g -O
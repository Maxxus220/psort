
build: psort.c psort.h
	gcc psort.c -o psort -Wall -Werror -pthread -O

test: test.o psort.o
	gcc test.o psort.o -o test -Wall -Werror -pthread -g -O

test.o: test.c psort.h
	gcc test.c -c -Wall -O

psort.o: psort.c psort.h
	gcc psort.c -c -Wall -O
all:
	gcc -Wall -c sorted-list.c  indexer.c
	ar rcs libsl.a sorted-list.o
	gcc -o indexer -L. -lsl indexer.c
	gcc -Wall -g -o search cache.c
	gcc -Wall -o oldsearch search.c
clean:
	rm -f sl indexer.o libsl.a sorted-list.o
	rm search
	rm oldsearch
	rm indexer

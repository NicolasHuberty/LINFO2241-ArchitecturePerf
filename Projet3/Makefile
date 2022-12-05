CC = gcc
CFLAGS += -std=gnu99 -Wall -Werror -Wextra -pthread -lm -g  -mno-sse2 -mno-avx -mno-avx2 -mno-avx512f -fno-unroll-loops -fno-tree-vectorize -O2 -mavx 

clean:
	rm -f client
	rm -f server-float
	rm -f server-float-avx
	rm -f results8.txt
	rm -f resultssimd8.txt
	rm -f results128.txt
	rm -f resultssimd128.txt
client:
	$(CC) -o client client.c $(CFLAGS)
	chmod +x client
server-float:
	$(CC) -o server-float server.c $(CFLAGS)

server-float-avx:
	$(CC) -o server-float-avx server.c $(CFLAGS) -DOPTIM

all: 
	make clean
	make client
	make server-float
	make server-float-avx
	
graph:
	make all
	./server-float -p 8002 &
	./client localhost:8002 -k 8
	./client localhost:8002 -k 128
	rm client
	$(CC) -o client client.c $(CFLAGS) -DOPTIM
	./server-float-avx -p 8003 &
	./client localhost:8003 -k 8
	./client localhost:8003 -k 128
	fuser -f -k 8002/tcp 8003/tcp
	python3 graph.py

tar:
	tar -zcvf archive.tar.gz *


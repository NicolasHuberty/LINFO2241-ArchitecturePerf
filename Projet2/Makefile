CC = gcc
CFLAGS += -std=gnu99 -Wall -Werror -Wextra -pthread -lm -g  -mno-sse2 -mno-avx -mno-avx2 -mno-avx512f -fno-unroll-loops -fno-tree-vectorize -O2

kill:
	fuser -v -k 8086/tcp
clean:
	rm -f client
	rm -f server
server:
	$(CC) -o server server.c $(CFLAGS)
	chmod +x server
server-optim:
	$(CC) -o server-optim server.c $(CFLAGS)
client:
	$(CC) -o client client.c $(CFLAGS)
	chmod +x client
tar:
	tar -zcvf archive.tar.gz *

test:
	make clean
	make server
	make client
run:
	make clean
	make server
	make client
	./server2 &
	./client
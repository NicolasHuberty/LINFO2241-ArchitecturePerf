test:
	make kill
	gcc -o server2 server.c -lm
	gcc -o client client.c -lm -g
	./server2 &
	./client
kill:
	kill -9 $(lsof -i:8080)
	pkill server
run:
	fuser -k 8080/tcp
	./server 
	python3 testPerformance.py
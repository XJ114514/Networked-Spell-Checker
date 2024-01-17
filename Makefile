server: serverFile/dictionary.txt serverFile/log.txt src/server.c src/connectionBuffer.c src/dictionary.c src/logQueue.c src/threadFn.c
	gcc -o server src/server.c -Wall -Werror

client: server src/client.c
	gcc -o client src/client.c -Wall -Werror

clean: server
	rm server client



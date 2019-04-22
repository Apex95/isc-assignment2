all:
	gcc -Wall server/server.c -o server_app
	gcc -Wall client/client.c -o client_app

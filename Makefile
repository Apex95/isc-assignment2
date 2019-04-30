all:
	gcc -Wall server/server.c server/license.c -o server_app
	gcc -Wall client/client.c -o client_app

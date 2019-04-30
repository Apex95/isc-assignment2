all:
	gcc -Wall -no-pie server/server.c server/license.c -o server_app
	gcc -Wall -no-pie client/client.c -o client_app

clean:
	rm server_app
	rm client_app 

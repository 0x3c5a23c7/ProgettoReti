CC 		= gcc
CFLAGS 	= -Wall -o
S_SRC 	= src/server_b.c 
C_SRC 	= src/client.c

server: $(S_SRC) 
	$(CC) $(CFLAGS) server $(S_SRC)

server_clean:
	rm -f server

client: $(C_SRC) 
	$(CC) $(CFLAGS) client $(C_SRC)

client_clean:
	rm -f client

all: $(S_SRC) $(C_SRC)
	$(CC) $(CFLAGS) server $(S_SRC)
	$(CC) $(CFLAGS) client $(C_SRC)

clean: 
	rm -f server client

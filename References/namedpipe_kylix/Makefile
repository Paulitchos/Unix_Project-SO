SERVER = dserv

CLIENT = dcli

C = gcc

CFLAGS = -Wall -Werror -Wextra

server:
	@$(C) $(CFLAGS) $(SERVER).c -o /tmp/$(SERVER) && /tmp/$(SERVER)

client:
	@$(C) $(CFLAGS) $(CLIENT).c -o /tmp/$(CLIENT) && /tmp/$(CLIENT)

#all:
#	@$(C) $(CFLAGS) $(CLIENT).c -o $(CLIENT)
#	@$(C) $(CFLAGS) $(SERVER).c -o $(SERVER)
clean:
	@rm $(CLIENT) $(SERVER)

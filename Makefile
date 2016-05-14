CC=gcc

CFLAGS= -O2

make: $(OBJS)
		  $(CC) $(CFLAGS) -o servidor servidor.c
		  $(CC) $(CFLAGS) -o cliente cliente.c

clean:
	-@rm servidor
	-@rm cliente

CC=gcc

CFLAGS= -O2

make:
		  $(CC) $(CFLAGS) -o sobusrv servidor.c estrutura.c
		  $(CC) $(CFLAGS) -o sobucli cliente.c estrutura.c

run:
	$(CC) $(CFLAGS) -o sobusrv servidor.c estrutura.c
	$(CC) $(CFLAGS) -o sobucli cliente.c estrutura.c
	./sobusrv

clean:
	-@rm sobusrv
	-@rm sobucli

kill:
	pkill -f ./sobusrv

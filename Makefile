CC=gcc

CFLAGS= -O2

make: $(OBJS)
		  $(CC) $(CFLAGS) -o sobusrv servidor.c
		  $(CC) $(CFLAGS) -o sobucli cliente.c

make run: $(OBJS)
	$(CC) $(CFLAGS) -o sobusrv servidor.c
	$(CC) $(CFLAGS) -o sobucli cliente.c
	./sobusrv

clean:
	-@rm sobusrv
	-@rm sobucli

make exit:
	pkill -f ./sobusrv

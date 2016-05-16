CC=gcc

CFLAGS= -O2

make: $(OBJS)
		  $(CC) $(CFLAGS) -o sobusrv servidor.c
		  $(CC) $(CFLAGS) -o sobucli cliente.c

run: $(OBJS)
	$(CC) $(CFLAGS) -o sobusrv servidor.c
	$(CC) $(CFLAGS) -o sobucli cliente.c
	./sobusrv

clean:
	-@rm sobusrv
	-@rm sobucli

exit:
	pkill -f ./sobusrv

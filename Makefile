CC=gcc

CFLAGS= -O2

make: $(OBJS)
		  $(CC) $(CFLAGS) -o sobusrv servidor.c
		  $(CC) $(CFLAGS) -o sobucli cliente.c

clean:
	-@rm sobusrv
	-@rm sobucli

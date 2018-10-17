FLAGS = -Wall -std=gnu99 -g

all: rpsls_client rpsls_server guess

rpsls_client: rpsls_client.o
	gcc ${FLAGS} -o $@ $^

rpsls_server: rpsls_server.o
	gcc ${FLAGS} -o $@ $^

guess: guess.o
	gcc ${FLAGS} -o $@ $^

rpsls_server.o: rpsls_server.c rpsls.h
	gcc ${FLAGS} -c $<

rpsls_client.o: rpsls_client.c rpsls.h
	gcc ${FLAGS} -c $<

%.o: %.c
	gcc ${FLAGS} -c $<

clean:
	rm *.o 

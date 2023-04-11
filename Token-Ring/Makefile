all: token_starter token_philosopher

token_starter: token_starter.c
	gcc -std=c99 -o token_starter token_starter.c

token_philosopher: token_philosopher.c token_philosopher.h
	gcc -std=c99 -o token_philosopher token_philosopher.c

clean:
	rm -f token_philosopher
	rm -f token_starter
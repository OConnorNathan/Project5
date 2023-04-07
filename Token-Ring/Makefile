all: Philosopher Starter

Philosopher: Philosopher.c Shared.h
	cc Philosopher.c -o philosopher -lpthread

Starter: starter.c philosopher
	cc starter.c -o starter
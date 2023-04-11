all: Controller Philosopher Starter
Controller: Controller.c Shared.h
	cc -std=c99 Controller.c -o controller -lpthread
	
Philosopher: Philosopher.c Shared.h
	cc -std=c99 Philosopher.c -o philosopher -lpthread

Starter: Starter.c Shared.h
	cc -std=c99 Starter.c -o starter
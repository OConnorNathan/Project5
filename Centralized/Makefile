all: Controller Philosopher
Controller: Controller.c Shared.h
	cc Controller.c -o controller -lpthread
	
Philosopher: Philosopher.c Shared.h
	cc Philosopher.c -o philosopher -lpthread
	
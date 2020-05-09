 
lift_sim_A : Lift.o
	gcc -Wall -g Lift.o -o lift_sim_A -lpthread

Lift.o : Lift.c Lift.h
	gcc -c Lift.c -Wall -g -lpthread

clean : 
	rm -f lift_sim_A Lift.o

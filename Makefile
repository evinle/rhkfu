
lift_sim_B : Lift2.o
	gcc -Wall -g Lift2.o -o lift_sim_B -lpthread
 
Lift2.o : Lift2.c
	gcc -c Lift2.c -Wall -g -lpthread

clean :
	rm -f lift_sim_B Lift2.o

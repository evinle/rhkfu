lift_sim_A readme

Date Created: 18/05/2020

Date Last Modified: 18/05/2020

Files:  Lift.c 
        Lift.h 
        Makefile
        sim_input

How to compile: 
    use the 'make' command when the all files are present in the same directory
OR
    compile the object file Lift.o using
        gcc -c Lift.c -lpthread 
    compile the executable lift_sim_A using
        gcc Lift.o -o lift_sim_A -lpthread 
   
How to run:
    Enter into the command line: 

    ./lift_sim_A m t
  
    where:
        m is the BUFFER SIZE, m >= 1
        t is the SLEEP TIME, t >= 0

    both conditions need to be met for the program to run

    

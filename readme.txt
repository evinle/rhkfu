lift_sim_B readme

Date Created: 18/05/2020

Date Last Modified: 18/05/2020

Files:  Lift2.c 
        Lift2.h 
        Makefile
        sim_input

How to compile: 
    use the 'make' command when the all files are present in the same directory
OR
    compile the object file Lift.o using
        gcc -c Lift2.c -lpthread 
    compile the executable lift_sim_B using
        gcc Lift2.o -o lift_sim_B -lpthread 
   
How to run:
    Enter into the command line: 

    ./lift_sim_B m t
  
    where:
        m is the BUFFER SIZE, m >= 1
        t is the SLEEP TIME, t >= 0

    both conditions need to be met for the program to run

    

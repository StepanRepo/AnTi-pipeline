SHELL := /bin/bash

errors = -Wall -Wextra -pedantic
mem = -fbounds-check -g
vectorization = -O3 -mavx2 -fopenmp -ffast-math #-fopt-info-vec-missed 

main: .o/configuration.o .o/etalon_profile.o .o/custom_time.o .o/custom_math.o .o/session_info.o .o/raw_profile.o .o/frequency_response.o .o/int_profile.o .o/main.o
	g++ $(errors) $(mem) $(vectorization) $^ -I./lib -L./lib  -o $@

.o/%.o: src/%.cpp
	g++ $(errors) $(mem) $(vectorization) -c $^ -I./lib -L./lib -o $@

run: main
	clear
	time ./main

del: 
	rm .o/* main

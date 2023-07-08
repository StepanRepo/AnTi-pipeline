SHELL := /bin/bash
.PHONY: run del

errors = -Wall -Wextra -pedantic
mem = -fbounds-check -g
opt = -O2  -fopenmp


msg = lib/massages.h

files = $(wildcard src/*.cpp)
obj = $(files:src/%.cpp=.o/%.o)



compile_current = g++ $(errors) $(mem) $(opt) -c $< -o $@

#main: .o/configuration.o .o/etalon_profile.o .o/custom_time.o .o/custom_math.o .o/session_info.o .o/raw_profile.o .o/frequency_response.o .o/int_profile.o .o/main.o
#	g++ $(errors) $(mem) $(opt) $^ -I./lib -L./lib  -o $@
main: $(msg) $(obj) #.o/raw_profile.o .o/int_profile.o .o/etalon_profile.o .o/frequency_response.o .o/configuration.o 
	g++ $(errors) $(mem) $(opt) $^ -o $@

# Compile object files
.o/raw_profile.o: src/raw_profile.cpp lib/raw_profile.h $(msg) .o/session_info.o  .o/configuration.o
	$(compile_current)
	echo $(obj)

.o/int_profile.o: src/int_profile.cpp lib/int_profile.h $(msg) .o/session_info.o .o/raw_profile.o .o/custom_math.o .o/configuration.o
	$(compile_current)

.o/etalon_profile.o: src/etalon_profile.cpp lib/etalon_profile.h $(msg) .o/int_profile.o .o/custom_math.o .o/configuration.o
	$(compile_current)

.o/frequency_response.o: src/frequency_response.cpp lib/frequency_response.h $(msg) .o/int_profile.o  .o/raw_profile.o .o/custom_math.o .o/configuration.o
	$(compile_current)

.o/session_info.o: src/session_info.cpp lib/session_info.h $(msg) .o/configuration.o .o/custom_time.o
	$(compile_current)

.o/%.o: src/%.cpp lib/%.h $(msg)
	$(compile_current)

.o/%.o: src/%.cpp $(msg)
	$(compile_current)

# Some run commands
run: main
	clear
	time ./main

del: 
	rm .o/* main

errors = -Wall -Wextra -pedantic
mem = -fbounds-check -g

main: .o/etalon_profile.o .o/custom_time.o .o/custom_math.o .o/session_info.o .o/raw_profile.o .o/frequency_response.o .o/int_profile.o .o/main.o
	g++ $(errors) $(mem) -O3 $^ -I./lib -L./lib  -o $@

.o/%.o: src/%.cpp
	g++ $(errors) $(mem) -O3 -c $^ -I./lib -L./lib -o $@

run: main
	clear
	./main

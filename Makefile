errors = -Wall -Wextra -pedantic

main: .o/custom_time.o .o/session_info.o .o/raw_profile.o .o/int_profile.o .o/main.o
	g++ $(errors) -O3 $^ -I./lib -L./lib  -o $@

.o/%.o: src/%.cpp
	g++ $(errors) -O3 -c $^ -I./lib -L./lib -o $@

run: main
	clear
	./main

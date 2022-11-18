
main: .o/raw_profile.o .o/int_profile.o .o/main.o
	g++ -O3 $^ -I./lib -L./lib  -o $@

.o/%.o: src/%.cpp
	g++ -O3 -c $^ -I./lib -L./lib -o $@

run: main
	clear
	./main

routing : simulation.cpp utility.cpp utility.h
	g++ utility.cpp simulation.cpp -o routing.o
clean :
	rm *.o *.txt
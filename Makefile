routing : simulation.cpp utility.cpp utility.h
	g++-11 utility.cpp simulation.cpp -o routing
clean :
	rm *.o *.txt
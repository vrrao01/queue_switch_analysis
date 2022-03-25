routing : simulation.cpp utility.cpp utility.h
	@g++-11 utility.cpp simulation.cpp -o routing
	@echo "TO RUN THE SIMULATION:\n./routing -N <number_of_port> -p <packet_gen_prob> -q <INQ/KOUQ/iSLIP> -K <knockout> -o <output_file> -T <timeslots>"
clean :
	-rm *.txt routing
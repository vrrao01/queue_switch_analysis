# Scheduling Techniques in a Packet Switch
This repository contains a simulation for three different packet scheduling techniques in a buffered packet switch. The scheduling techniques implemented are:
- INQ: For  each  packet  generated,  if  there  is  no  contention  for  its desired  output  port,  it  is  selected  for  transmission  and  placed  in the corresponding output port’s buffer.  For packets contending for the  same  output  port,  one  of  the  packets  is  randomly  selected  for transmission  and  placed  in  the  corresponding  output  port’s  buffer;the other packets are queued at the corresponding input port.
- KOUQ: A maximum of K packets (per output port) that arrive in a given slot are queued (based on packet arrival time) at the corresponding output port.  If two or more packets have the same arrival time, the packets can be queued in any order. If more than K packets arrive in a slot for a particular output port, then K packets are randomly selected for buffering, and the remaining packets are dropped. The default value is K= 0.6N, where N is the number of input/output ports.
- iSLIP: An improvement to the basic round robin scheduling and parallel iterative matching scheduling that uses virtual output queueing. The scheduling algorithm has been taken from [this paper](https://ieeexplore.ieee.org/document/769767). 

Note: Refer to **Report.pdf** for a technical report. 

## About the simulation
The simulation runs for a given number of timeslots. In each slot, a packet may be generated at an input port independently with an entered probability called the packeted generation probability. Then, the chosen scheduling algorithm, schedules packets queued at the input port for transfer to output port and subsequent transmission out of the switch. 

## Steps to run
1. Clone this repository.
2. Navigate into the repository folder and run `make routing`.
3. In order to run the simulation run: 
```
./routing -N <number_of_port> -p <packet_gen_prob> -q <INQ/KOUQ/iSLIP> -K <knockout> -o <output_file> -T <timeslots> -L <number_of_islip_iterations>
```
  Note: p, K are floating point numbers between 0 and 1. K is the knockout expressed as a fraction of the total number of ports. 
  

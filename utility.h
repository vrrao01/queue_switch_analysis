#ifndef UTILITY_H
#define UTILITY_H

#include <string>
using namespace std;

// Variables defined in simulation.cpp
extern unsigned int nPort;      // Number of input and output ports
extern unsigned int bufferSize; // Buffer size
extern double packGenProb;      // Packet generation probability
extern string qSchedule;        // Queue scheduling technique
extern double knockout;         // knockout as fraction of total number of ports
extern string outputFileName;   // Output file name
extern unsigned int timeSlots;  // Max number of time slots

typedef struct packet
{
    int sourcePort;
    int destPort;
    unsigned int genTime; // Packet generation timeslot
    packet(int s, int d, unsigned int g) : sourcePort(s), destPort(d), genTime(g) {}

} packet;

/**
 * @brief Parses the command line input given and appropriates modifies global variables
 *
 * @param argc Number of command line arguments
 * @param argv List of provided arguments
 */
void parseCommandLineArgs(int argc, char *argv[]);

#endif
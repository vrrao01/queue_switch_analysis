#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>
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
    unsigned int packetID;
    unsigned int genTime; // Packet generation timeslot
    packet(unsigned int pid, int s, int d, unsigned int g) : sourcePort(s), destPort(d), packetID(pid), genTime(g) {}

} packet;

/**
 * @brief Parses the command line input given and appropriates modifies global variables
 *
 * @param argc Number of command line arguments
 * @param argv List of provided arguments
 */
void parseCommandLineArgs(int argc, char *argv[]);

/**
 * @brief Removes a packet that has been scheduled for transmission from its respective input port buffer
 *
 * @param p A packet chosen for transmission
 */
void removeFromInputBuffer(packet p);

/**
 * @brief Calculates the average of a vector of unsigned int
 *
 * @param arr Input vector
 * @return unsigned long long Average of all values in the vector
 */
double calculateAverage(const vector<unsigned int> &arr);

/**
 * @brief In case of iSLIP, this function will fill virtual output queues
 *
 */
void fillVOQ();

extern vector<vector<packet>> inputBuffer; // Input port buffers
extern vector<vector<vector<packet>>> voq; // Virtual output queues for input ports

#endif
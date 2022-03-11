#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "utility.h"

using namespace std;

// Variables representing the state of the switch and input parameters
unsigned int nPort = 8;                    // Number of input and output ports
unsigned int bufferSize = 4;               // Buffer size
double packGenProb = 0.5;                  // Packet generation probability
string qSchedule = "INQ";                  // Queue scheduling technique
double knockout = 0.6;                     // knockout as fraction of total number of ports
string outputFileName = "sim_results.txt"; // Output file name
unsigned int timeSlots = 10000;            // Max number of time slots
vector<vector<packet>> inputBuffer;        // Represents input port buffer
vector<vector<packet>> outputBuffer;       // Represents output port buffer
vector<unsigned int> packetGenTime;        // Stores the next generation time for a packet at each port

void init();                             // Performs some required initialisation tasks
void generateTraffic(unsigned int time); // Executes Phase 1 - Traffic Generation

int main(int argc, char *argv[])
{
    parseCommandLineArgs(argc, argv);
    cout << "nPort = " << nPort << endl;
    cout << "bufferSize = " << bufferSize << endl;
    cout << "packGenProb = " << packGenProb << endl;
    cout << "knockout = " << knockout << endl;
    cout << "outputFileName = " << outputFileName << endl;
    cout << "timeSlots = " << timeSlots << endl;
    init();
    for (unsigned int time = 0; time < timeSlots; time++)
    {
        generateTraffic(time);
    }
}

void init()
{
    packetGenTime.resize(nPort);
    inputBuffer.resize(nPort);
    outputBuffer.resize(nPort);
    // Exponential distribution for interarrival times
    int seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    std::exponential_distribution<double> expDist(packGenProb);

    // Sets the generation time for the first packet
    for (int i = 0; i < nPort; i++)
    {
        packetGenTime[i] = static_cast<unsigned int>(expDist(generator));
    }
}

void generateTraffic(unsigned int time)
{
    // Exponential distribution for interarrival times
    int seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    std::exponential_distribution<double> expDist(packGenProb);
    // Uniform distribution for output port selection of a generated packet
    uniform_int_distribution<int> uniDist(0, nPort);

    for (int i = 0; i < nPort; i++)
    {
        if (time == packetGenTime[i])
        {
            // Create packet for port i
            inputBuffer[i].emplace_back(i, uniDist(generator), time);
            // Find time of next packet generation
            packetGenTime[i] = time + static_cast<unsigned int>(expDist(generator)) + 1;
            // cout << time << " : Generated packet at input port " << i << " destined to output port " << inputBuffer[i].back().destPort << " Next gen at = " << packetGenTime[i] << endl;
        }
    }
}
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
unsigned int packetID = 0;                 // Next available packet ID

void init();                             // Performs some required initialisation tasks
unsigned int getNextPID();               // Returns the next available Packet ID and increments the counter
void generateTraffic(unsigned int time); // Executes Phase 1 - Traffic Generation
void schedulePackets(unsigned int time); // Executes Phase 2 - Packet Scheduling

void printInputDebug()
{
    for (int i = 0; i < nPort; i++)
    {
        cout << "Input Port " << i << " ";
        for (int j = 0; j < inputBuffer[i].size(); j++)
        {
            cout << "|" << inputBuffer[i][j].packetID << " " << inputBuffer[i][j].destPort << "| ";
        }
        cout << endl;
    }
}

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
        schedulePackets(time);
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

unsigned int getNextPID()
{
    unsigned int pid = packetID;
    packetID++;
    return pid;
}

void generateTraffic(unsigned int time)
{
    // Exponential distribution for interarrival times
    int seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    std::exponential_distribution<double> expDist(packGenProb);

    // Uniform distribution for output port selection of a generated packet
    uniform_int_distribution<int> uniDist(0, nPort - 1);

    for (int i = 0; i < nPort; i++)
    {
        if (time == packetGenTime[i])
        {
            // Create packet for port i
            inputBuffer[i].emplace_back(getNextPID(), i, uniDist(generator), time);

            // Find time of next packet generation
            packetGenTime[i] = time + static_cast<unsigned int>(expDist(generator)) + 1;

            cout << time << " : Generated packet " << inputBuffer[i].back().packetID << " at input port " << i << " destined to output port " << inputBuffer[i].back().destPort << " Next gen at = " << packetGenTime[i] << endl;
        }
    }
}

void schedulePackets(unsigned int time)
{
    if (qSchedule == "INQ")
    {
        vector<vector<packet>> inputQueue = inputBuffer;
        cout << "TIME SCHEDULE = " << time << endl;
        printInputDebug();
        // Find packets destined to each output port
        for (int outP = 0; outP < nPort; outP++)
        {
            vector<packet> packetsToOutput;
            packetsToOutput.reserve(nPort);
            for (int i = 0; i < nPort; i++)
            {
                if (!inputQueue[i].empty() && inputQueue[i].front().destPort == outP)
                {
                    packetsToOutput.push_back(inputQueue[i].front());
                }
            }
            // If single packet destined to port, store in output port buffer
            if (packetsToOutput.size() == 1)
            {
                packet pckt = packetsToOutput.front();
                outputBuffer[outP].push_back(pckt);
                removeFromInputBuffer(pckt);
                // cout << time << " : Selected packet generated at t = " << pckt.genTime << " to output port " << pckt.destPort << endl;
            }
            else if (packetsToOutput.size() > 1)
            {
                // Randomly select one of the packets destined to same output port
                int randIndex = rand() % packetsToOutput.size();
                packet pckt = packetsToOutput[randIndex];
                outputBuffer[outP].push_back(pckt);
                removeFromInputBuffer(pckt);
                cout << time << " : Selected packet " << pckt.packetID << " generated at t = " << pckt.genTime << " to output port " << pckt.destPort << endl;
            }
        }
        printInputDebug();
    }
}

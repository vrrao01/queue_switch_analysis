#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
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
vector<unsigned int> packetGen;        // Stores the next generation time for a packet at each port
unsigned int packetID = 0;                 // Next available packet ID
unsigned int totalPacketsGenerated = 0;
unsigned int totalPacketsDropped = 0;
unsigned int totalPacketsProcessed = 0;

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

void removeFromInputBuffer(packet p)
{
    extern vector<vector<packet>> inputBuffer;
    vector<packet>::iterator itr = inputBuffer[p.sourcePort].begin();
    while (itr != inputBuffer[p.sourcePort].end())
    {
        if (itr->packetID == p.packetID)
        {
            inputBuffer[p.sourcePort].erase(itr);
            break;
        }
        itr++;
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
    cout << "totalPacketsGenerated: " << totalPacketsGenerated << " totalPacketsDropped: " << totalPacketsDropped << " totalPacketsProcessed: " << totalPacketsProcessed << '\n';
}

void init()
{
    packetGen.resize(nPort);
    inputBuffer.resize(nPort);
    outputBuffer.resize(nPort);

    // Exponential distribution for interarrival times
    int seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    std::exponential_distribution<double> expDist(packGenProb);

    // Sets the generation time for the first packet
    for (int i = 0; i < nPort; i++)
    {
        packetGen[i] = rand() % 2;
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
        if (packetGen[i])
        {
            // Create packet for port i
            if (inputBuffer[i].size() < bufferSize) {
                inputBuffer[i].emplace_back(getNextPID(), i, uniDist(generator), time);
                ++totalPacketsGenerated;
                cout << "At time " << time << " : generated packet " << inputBuffer[i].back().packetID << " at input port " << i << " destined to output port " << inputBuffer[i].back().destPort << endl;
            } else {
                ++totalPacketsDropped;
                ++totalPacketsGenerated;
                cout << "At time " << time << " : generated packet " << getNextPID() << " dropped at input port " << i << " destined to output port " << uniDist(generator) << endl;;
            }
        }
        // Find time of next packet generation
        packetGen[i] = rand() % 2;
    }
}

void schedulePackets(unsigned int time)
{
    if (qSchedule == "INQ")
    {
        vector<vector<packet>> inputQueue = inputBuffer;
        cout << "TIME SCHEDULE = " << time << endl;
        // printInputDebug();
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
                ++totalPacketsProcessed;
                cout << time << " : Selected packet " << pckt.packetID << " generated at t = " << pckt.genTime << ", pushed to output port " << pckt.destPort << endl;
            }
            else if (packetsToOutput.size() > 1)
            {
                // Randomly select one of the packets destined to same output port
                int randIndex = rand() % packetsToOutput.size();
                packet pckt = packetsToOutput[randIndex];
                outputBuffer[outP].push_back(pckt);
                removeFromInputBuffer(pckt);
                ++totalPacketsProcessed;
                cout << time << " : Selected packet " << pckt.packetID << " generated at t = " << pckt.genTime << ", pushed to output port " << pckt.destPort << endl;
            }
        }
        // printInputDebug();
    }
    else if (qSchedule == "KOUQ")
    {
        vector<vector<packet>> inputQueue = inputBuffer;
        cout << "TIME SCHEDULE = " << time << endl;
        // printInputDebug();
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
            int Z = knockout * 1.0 * nPort;
            // If single packet destined to port, store in output port buffer
            if (packetsToOutput.size() <= Z)
            {
                // sort the packets according to arrival time
                sort(packetsToOutput.begin(), packetsToOutput.end(), [](auto const & left, auto const & right) {
                    return left.genTime < right.genTime;
                });
                for (packet pckt : packetsToOutput) {
                    outputBuffer[outP].push_back(pckt);
                    removeFromInputBuffer(pckt);
                    ++totalPacketsProcessed;
                    cout << time << " : Selected packet " << pckt.packetID << " generated at t = " << pckt.genTime << ", pushed to output port " << pckt.destPort << endl;
                }
            }
            else
            {
                int X = min((int) packetsToOutput.size(), Z);
                // Randomly select Z packets destined to same output port
                while (X--) {
                    int randIndex = rand() % (X + 1);
                    packet pckt = packetsToOutput[randIndex];
                    outputBuffer[outP].push_back(pckt);
                    removeFromInputBuffer(pckt);
                    ++totalPacketsProcessed;
                    cout << time << " : Selected packet " << pckt.packetID << " generated at t = " << pckt.genTime << ", pushed to output port " << pckt.destPort << endl;
                }
            }
        }
        // printInputDebug();
    }
}

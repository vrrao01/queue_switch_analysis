#include <iostream>
#include <fstream>
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
unsigned int iSLIPiters = 2;               // Number of iSLIP iterations
vector<vector<packet>> inputBuffer;        // Represents input port buffer
vector<vector<vector<packet>>> voq;        // Virtual output queues for input ports
vector<vector<packet>> outputBuffer;       // Represents output port buffer
vector<int> opArbiter;                     // Output port arbiter for iSLIP
vector<int> ipArbiter;                     // Input port arbiter for iSLIP
vector<unsigned int> packetGen;            // Stores the next generation time for a packet at each port
unsigned int packetID = 0;                 // Next available packet ID
unsigned int totalPacketsGenerated = 0;    // Counter for total packets generated during simulation
unsigned int totalPacketsDropped = 0;      // Number of packets dropped due to buffer overflow or KOUQ
unsigned int totalPacketsTransmitted = 0;  // Number of packets transmitted
vector<unsigned int> packetDelays;         // Stores delay of each packet for calculations
vector<unsigned int> kouqDropProb;         // Stores KOUQ Drop Probability
ofstream fout;                             // Simulation results output

void init();                             // Performs some required initialisation tasks
unsigned int getNextPID();               // Returns the next available Packet ID and increments the counter
void generateTraffic(unsigned int time); // Executes Phase 1 - Traffic Generation
void schedulePackets(unsigned int time); // Executes Phase 2 - Packet Scheduling
void transmitPackets(unsigned int time); // Executes Phase 3 - Packet Transmission

int main(int argc, char *argv[])
{
    parseCommandLineArgs(argc, argv);
    cout << "Starting simulation with following parameters:" << endl;
    cout << "Queue type = " << qSchedule << endl;
    cout << "Number of ports = " << nPort << endl;
    cout << "Buffer Size = " << bufferSize << endl;
    cout << "Packet Generation Probability  = " << packGenProb << endl;
    cout << "Number of timeslots = " << timeSlots << endl;
    if (qSchedule == "KOUQ")
    {
        cout << "knockout = " << knockout << endl;
    }
    cout << "Output File Name = " << outputFileName << endl;
    // Perform initialisation
    init();
    // Simulate time slots
    for (unsigned int time = 0; time < timeSlots; time++)
    {
        generateTraffic(time);
        schedulePackets(time);
        transmitPackets(time);
    }
    totalPacketsDropped = totalPacketsGenerated - totalPacketsTransmitted;
    writeOutput();
    string line(50, '-');
    line.replace(23, 5, "STATS");
    cout << line << endl;
    cout << "Total Packets Generated: " << totalPacketsGenerated << endl
         << "Total Packets Dropped: " << totalPacketsDropped << endl
         << "Total Packets Transmitted: " << totalPacketsTransmitted << '\n';
}

void init()
{
    int seed = chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    packetGen.resize(nPort);
    inputBuffer.resize(nPort);
    outputBuffer.resize(nPort);

    if (qSchedule == "iSLIP")
    {
        voq.resize(nPort, vector<vector<packet>>(nPort));
        ipArbiter.resize(nPort, 0);
        opArbiter.resize(nPort, 0);
    }

    // Sets the generation time for the first packet
    for (int i = 0; i < nPort; i++)
    {
        int random = (100.0 * rand() / (RAND_MAX + 1.0)) + 1;
        if (random < 100.0 * packGenProb)
            packetGen[i] = 1;
        else
            packetGen[i] = 0;
    }
    fout.open(outputFileName);
}

unsigned int getNextPID()
{
    unsigned int pid = packetID;
    packetID++;
    return pid;
}

void generateTraffic(unsigned int time)
{
    for (int i = 0; i < nPort; i++)
    {
        if (packetGen[i])
        {
            // Create packet for port i
            if (inputBuffer[i].size() < bufferSize)
            {
                inputBuffer[i].emplace_back(getNextPID(), i, rand() % nPort, time);
                ++totalPacketsGenerated;
            }
            else // Drop packet since input buffer is full
            {
                ++totalPacketsGenerated;
            }
        }
        // Find time of next packet generation
        int random = (100.0 * rand() / (RAND_MAX + 1.0)) + 1;
        if (random < 100.0 * packGenProb)
            packetGen[i] = 1;
        else
            packetGen[i] = 0;
    }
    if (qSchedule == "iSLIP")
    {
        fillVOQ();
    }
}

void schedulePackets(unsigned int time)
{
    if (qSchedule == "INQ")
    {
        vector<vector<packet>> inputQueue = inputBuffer;
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
            }
            else if (packetsToOutput.size() > 1)
            {
                // Randomly select one of the packets destined to same output port
                int randIndex = rand() % packetsToOutput.size();
                packet pckt = packetsToOutput[randIndex];
                outputBuffer[outP].push_back(pckt);
                removeFromInputBuffer(pckt);
            }
        }
    }
    else if (qSchedule == "KOUQ")
    {
        int kouqDropCount = 0; // Number of ports that received more than K packets
        vector<vector<packet>> inputQueue = inputBuffer;
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
            int K = knockout * 1.0 * nPort;
            // If less than or equal to K (knockout value) packets destined to output port, directly queue them at output port
            if (packetsToOutput.size() <= K)
            {
                // Sort the packets according to arrival time
                sort(packetsToOutput.begin(), packetsToOutput.end(), [](auto const &left, auto const &right)
                     { return left.genTime < right.genTime; });
                for (packet pckt : packetsToOutput)
                {
                    if (outputBuffer[outP].size() < bufferSize)
                    {
                        outputBuffer[outP].push_back(pckt);
                    }
                    else
                    {
                        // cout << time << " : NO SPACE IN BUFFER" << endl;
                    }
                    removeFromInputBuffer(pckt);
                }
            }
            else // More than K (knockout) value packets destined to output port
            {
                kouqDropCount++;
                int X = K;
                // Randomly select K packets destined to same output port
                while (X--)
                {
                    int randIndex = rand() % (packetsToOutput.size());
                    packet pckt = packetsToOutput[randIndex];
                    if (outputBuffer[outP].size() < bufferSize)
                    {
                        outputBuffer[outP].push_back(pckt);
                    }
                    removeFromInputBuffer(pckt);
                    packetsToOutput.erase(packetsToOutput.begin() + randIndex);
                }
                // Sort packets in output queue according to arrival time.
                sort(outputBuffer[outP].begin(), outputBuffer[outP].end(), [](auto const &left, auto const &right)
                     { return left.genTime < right.genTime; });
                // Drop remaining packets that were not randomly selected
                for (packet pckt : packetsToOutput)
                {
                    removeFromInputBuffer(pckt);
                    totalPacketsDropped++;
                }
            }
        }
        kouqDropProb.push_back(kouqDropCount);
    }
    else if (qSchedule == "iSLIP")
    {
        vector<bool> matchedIP(nPort, false); // True if particular input port is matched
        vector<bool> matchedOP(nPort, false); // True if particular output port  is matched

        for (int iter = 0; iter < iSLIPiters; iter++)
        {
            vector<vector<int>> requests(nPort); // requests[i] stores requests sent by input ports to output port i
            vector<vector<int>> grants(nPort);   // grants[i] stores all grants received by input port i
            // Step 1: Request
            for (int i = 0; i < nPort; i++)
            {
                for (int outP = 0; outP < nPort; outP++)
                {
                    // If input port is unmatched and has a packet to output port, then send requests
                    if (!matchedIP[i] && voq[i][outP].size())
                    {
                        requests[outP].push_back(i);
                    }
                }
            }
            // Step 2: Grant
            for (int outP = 0; outP < nPort; outP++)
            {
                if (!matchedOP[outP] && requests[outP].size())
                {
                    int grantedInputPort = opArbiter[outP];
                    // Grant input port than appears next in round robin fashion in arbiter
                    while (find(requests[outP].begin(), requests[outP].end(), grantedInputPort) == requests[outP].end())
                    {
                        grantedInputPort = (grantedInputPort + 1) % nPort;
                    }
                    grants[grantedInputPort].push_back(outP);
                }
            }
            // Step 3: Accept
            for (int i = 0; i < nPort; i++)
            {
                if (!matchedIP[i] && grants[i].size())
                {
                    int acceptedOutputPort = ipArbiter[i];
                    // Find grant to accept by choosing output port that comes next in round robin fashion
                    while (find(grants[i].begin(), grants[i].end(), acceptedOutputPort) == grants[i].end())
                    {
                        acceptedOutputPort = (acceptedOutputPort + 1) % nPort;
                    }
                    if (iter == 0) // Only update arbiters if it is first iteration
                    {
                        // Increase input arbiter to point to one location beyond accepted output port
                        ipArbiter[i] = (acceptedOutputPort + 1) % nPort;
                        // Increase output arbiter to point to one lcoation beyond input port that accepted request
                        opArbiter[acceptedOutputPort] = (i + 1) % nPort;
                    }
                    // Send packet to output buffer
                    packet pckt = voq[i][acceptedOutputPort].front();
                    if (outputBuffer[acceptedOutputPort].size() < bufferSize)
                    {
                        outputBuffer[acceptedOutputPort].push_back(pckt);
                    }
                    voq[i][acceptedOutputPort].erase(voq[i][acceptedOutputPort].begin());
                    matchedIP[i] = true;
                    matchedOP[acceptedOutputPort] = true;
                }
            }
            requests.clear();
            grants.clear();
        }
    }
}

void transmitPackets(unsigned int time)
{
    for (int outP = 0; outP < nPort; outP++)
    {
        // Transmit the packet at the head of each output buffer
        if (outputBuffer[outP].size())
        {
            packet pckt = outputBuffer[outP][0];
            // Remove packet from output buffer
            outputBuffer[outP].erase(outputBuffer[outP].begin());
            // Store the delay
            unsigned int pcktDelay = time - pckt.genTime + 1; // Add 1 to since it takes one time slot to transmit
            packetDelays.push_back(pcktDelay);
            totalPacketsTransmitted++;
        }
    }
}
#include "utility.h"
#include <cctype>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>

void parseCommandLineArgs(int argc, char *argv[])
{
    char *ptr;
    for (int i = 1; i < argc; i += 2)
    {
        char option = argv[i][1];
        switch (toupper(option))
        {
        case 'N':
            nPort = strtoul(argv[i + 1], &ptr, 10);
            cout << nPort << '\n';
            break;
        case 'B':
            bufferSize = strtoul(argv[i + 1], &ptr, 10);
            cout << bufferSize << '\n';
            break;
        case 'P':
            packGenProb = strtod(argv[i + 1], &ptr);
            cout << packGenProb << '\n';
            break;
        case 'Q':
            qSchedule = argv[i + 1];

            if (qSchedule != "KOUQ" && qSchedule != "INQ" && qSchedule != "iSLIP")
            {
                cout << "Queue type must be INQ / KOUQ / iSLIP. (CASE SENSITIVE)" << endl;
                exit(0);
            }
            cout << qSchedule << '\n';
            break;
        case 'K':
            knockout = strtod(argv[i + 1], &ptr);
            break;
        case 'O':
            outputFileName = argv[i + 1];
            break;
        case 'T':
            timeSlots = strtoul(argv[i + 1], &ptr, 10);
            break;
        case 'L':
            iSLIPiters = atoi(argv[i + 1]);
            break;
        default:
            cout << "Received invalid argument" << endl;
            exit(0);
        }
    }
}

void removeFromInputBuffer(packet p)
{
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

double calculateAverage(const vector<unsigned int> &arr)
{
    int length = arr.size();
    unsigned long long sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += arr[i];
    }
    double avg = static_cast<double>(sum) / length;
    return avg;
}

void fillVOQ()
{
    for (int i = 0; i < nPort; i++)
    {
        for (packet pckt : inputBuffer[i])
        {
            int dest = pckt.destPort;
            if (voq[i][dest].size() < bufferSize)
            {
                voq[i][dest].push_back(pckt);
            }
        }
        inputBuffer[i].clear();
    }
}

double calculateStdDev(const vector<unsigned int> &arr, double avg)
{
    double sqSum = 0;
    int length = arr.size();
    for (int i = 0; i < length; i++)
    {
        sqSum += arr[i] * arr[i];
    }
    double squareMean = sqSum / length;
    double stdDev = squareMean - avg * avg;
    stdDev = sqrt(stdDev);
    return stdDev;
}

void writeOutput()
{
    extern ofstream fout;
    extern vector<unsigned int> packetDelays; // Stores delay of each packet for calculations
    extern vector<unsigned int> kouqDropProb; // Stores KOUQ Drop Probability
    extern int totalPacketsTransmitted;
    double avgDelay = calculateAverage(packetDelays);
    fout << setw(5) << left << "N" << setw(10) << left << "p" << setw(15) << left << "Queue type" << setw(15) << left << "Avg. PD" << setw(18) << left << "Std Dev of PD" << setw(25) << left << "Avg link utilisation";
    if (qSchedule == "KOUQ")
    {
        fout << setw(15) << left << "KOUQ Drop Prob" << endl;
    }
    else
    {
        fout << endl;
    }
    fout << setw(5) << left << nPort << setw(10) << left << packGenProb << setw(15) << left << qSchedule << setw(15) << left << avgDelay << setw(18) << left << calculateStdDev(packetDelays, avgDelay) << setw(25) << left << (double)totalPacketsTransmitted / (nPort * (timeSlots - 1));
    if (qSchedule == "KOUQ")
    {
        double avg = calculateAverage(kouqDropProb);
        avg = avg / nPort;
        fout << setw(15) << left << avg << endl;
    }
}
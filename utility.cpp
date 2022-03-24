#include "utility.h"
#include <cctype>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <iostream>

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
            voq[i][dest].push_back(pckt);
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
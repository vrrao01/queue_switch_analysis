#include "utility.h"
#include <cctype>
#include <cstdlib>
#include <vector>
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
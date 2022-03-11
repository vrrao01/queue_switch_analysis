#include "utility.h"
#include <cctype>
#include <cstdlib>
#include <iostream>

// ./routing −N switchportcount −B buffersize −p packetgenprob
// −queue IN Q|KOU Q|ISLIP −K  knockout −out−outputf ile −T maxtimeslots
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
            break;
        case 'B':
            bufferSize = strtoul(argv[i + 1], &ptr, 10);
            break;
        case 'P':
            packGenProb = strtod(argv[i + 1], &ptr);
            break;
        case 'Q':
            qSchedule = argv[i + 1];
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
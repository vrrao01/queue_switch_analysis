#include <iostream>
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

int main(int argc, char *argv[])
{
    parseCommandLineArgs(argc, argv);
    cout << "nPort = " << nPort << endl;
    cout << "bufferSize = " << bufferSize << endl;
    cout << "packGenProb = " << packGenProb << endl;
    cout << "knockout = " << knockout << endl;
    cout << "outputFileName = " << outputFileName << endl;
    cout << "timeSlots = " << timeSlots << endl;
}
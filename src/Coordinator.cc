// INCLUDES
#include "Coordinator.h"

// DEFINITIONS
Define_Module(Coordinator);

// INITIALIZATION
void Coordinator::initialize()
{
    // create a new file handler with the coordinator file path
    fileHandler = new FileHandler("../data/input/coordinator.txt");
    // reset outputs
    fileHandler->resetOutput();
    fileHandler->resetReceiverOutput();
    // read the coordinator data from the file
    std::pair<int, int> coordinatorData = fileHandler->readCoordinatorData();
    nodeNumber = coordinatorData.first;    // get the node number
    startTime = coordinatorData.second;    // get the start time
    cMessage *msg = new cMessage("start"); // create a new message with name "start"
    // send the message to the first node in the network after the start time at the gate "gout"
    // [gate0: Node0, gate1: Node1]
    sendDelayed(msg, startTime, "gout", nodeNumber);
}

void Coordinator::handleMessage(cMessage *msg)
{
    // EMPTY
    // Coordinator does not receive any messages
}

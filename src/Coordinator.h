/*
 * Coordinator class
 * Coordinator is the first node in the network
 * It sends a message to the first node in the network
 * It reads the input file and gets the node number and start time
 * It sends a message to the first node in the network after the start time
 */

#ifndef __FALL2023_PROJECT_COORDINATOR_H_
#define __FALL2023_PROJECT_COORDINATOR_H_

// INCLUDES
#include <omnetpp.h>
#include "FileHandler.h"

// namespace
using namespace omnetpp;

// ======================
// Coordinator class
// ======================
class Coordinator : public cSimpleModule
{
private:
  FileHandler *fileHandler; // file handler
  int nodeNumber;           // node number
  int startTime;            // start time

protected:
  virtual void initialize() override;                 // initialization
  virtual void handleMessage(cMessage *msg) override; // handle message
};

#endif

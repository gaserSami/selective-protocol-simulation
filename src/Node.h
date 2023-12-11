/*
 * Node.h
 */
#ifndef __FALL2023_PROJECT_NODE_H_
#define __FALL2023_PROJECT_NODE_H_

// includes and namespaces omitted
#include <omnetpp.h>
#include <unordered_map>
#include <bitset>
#include <iostream>
#include "FileHandler.h"
#include "Frame_m.h"

using namespace omnetpp;

// ===========================================================================
// ================================ NODE =====================================
// ===========================================================================
class Node : public cSimpleModule
{
private:
  // parameters
  double processingTime;    // processing time
  double timeout;           // timeout
  double transmissionDelay; // transmission delay
  double errorDelay;        // error delay
  double duplicationDelay;  // duplication delay
  int senderWindowSize;     // sender window size
  int receiverWindowSize;   // receiver window size

  // sender variables
  int expectedAck;                                         // expected ack number (sender lower bound)
  int nextFrameToSend;                                     // next frame to send (sender upper bound + 1)
  int nBuffered;                                           // how many packets currently buffered
  int senderMaxSeq;                                        // max seq number (2^k - 1)
  bool *arrivedOut;                                        // arrived packets in sender
  std::string *packetsOut;                                 // packets to send
  std::string *timers;                                     // the scheduled timers at indices of the seq numbers
  bool isFull;                                             // is the buffer full indicated the sender can't get more packets from the network layer
  bool noMorePackets;                                      // are there more packets in the network layer
  std::unordered_map<std::string, cMessage *> timeOutMsgs; // Map to store cMessage pointers

  // receiver variables
  int expectedFrame;      // expected frame number (receiver lower bound)
  int recUpperBound;      // receiver upper bound
  int receiverMaxSeq;     // max seq number (2^k - 1)
  std::string *packetsIn; // received packets
  bool *arrivedIn;        // arrived packets in receiver

  // other variables
  FileHandler *fileHandler;                             // file handler object
  int counter;                                          // counter to remove confusion in the logs used for self messages and timers
  std::unordered_map<std::string, cMessage *> selfMsgs; // Map to store cMessage pointers

protected:
  virtual void initialize() override;                                                                // initialize method
  virtual void handleMessage(cMessage *msg) override;                                                // handle message method
  void sendFrame(int dataSeqNr, int type, std::string payload, int ackSeqNr, std::bitset<4> prefix); // send frame method
  void receiveACK(Frame *frame);                                                                     // receive ACK method
  void receiveNACK(Frame *frame);                                                                    // receive NACK method
  void recieveData(Frame *frame);                                                                    // receive data method
  bool between(int a, int b, int c);                                                                 // between method
  void processNextPacket();                                                                          // process next packet method
  void startTimer(int seqNr);                                                                        // start timer method
  void stopTimer(int seqNr);                                                                         // stop timer method
  void printInfo();                                                                                  // just for debugging
};

#endif

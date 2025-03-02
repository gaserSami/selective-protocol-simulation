/*
 * Node.cc
 */
#include "Node.h"

// register the module
Define_Module(Node);

// print the debugging info
void Node::printInfo()
{
    std::cout << "Sender Debugging Info:" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "senderWindowSize: " << senderWindowSize << std::endl;
    std::cout << "expectedAck (sender lower bound): " << expectedAck << std::endl;
    std::cout << "nextFrameToSend (sender upper bound): " << nextFrameToSend << std::endl;
    std::cout << "nBuffered: " << nBuffered << std::endl;
    std::cout << "senderMaxSeq: " << senderMaxSeq << std::endl;
    std::cout << "isFull: " << isFull << std::endl;
    std::cout << "======================================" << std::endl;

    std::cout << "Receiver Debugging Info:" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "receiverWindowSize: " << receiverWindowSize << std::endl;
    std::cout << "expectedFrame (recv lower bound): " << expectedFrame << std::endl;
    std::cout << "recUpperBound (recv upper bound): " << recUpperBound << std::endl;
    std::cout << "receiverMaxSeq: " << receiverMaxSeq << std::endl;
    std::cout << "arrivedIn:"
              << "[" << arrivedIn[0] << "]"
              << "[" << arrivedIn[1] << "]"
              << "[" << arrivedIn[2] << "]"
              << "[" << arrivedIn[3] << "]" << std::endl;
    std::cout << "======================================" << std::endl;
}

// send frame
void Node::sendFrame(int dataSeqNr, int type, std::string payload, int ackSeqNr, std::bitset<4> prefix)
{
    // for better readabiltiy
    bool delayError = prefix[0];                           // delay bit
    bool duplicateError = prefix[1];                       // duplicate bit
    bool lostError = prefix[2];                            // lost bit
    bool modficationError = prefix[3];                     // modification bit
    int modfiedBit = -1;                                   // the bit to be modified
    if (modficationError)                                  // if the frame is modified, choose a random bit to be modified
        modfiedBit = uniform(0, (payload.size() - 1) * 8); // choose a random bit to be modified

    Frame *frame = new Frame("");                                                                  // create a new frame
    frame->setFrameInfo(dataSeqNr, type, ackSeqNr, payload.c_str(), modficationError, modfiedBit); // set the frame info

    // printing the frame info
     if(type == 0)
     {
        std::string info = "At time = [" + std::string((simTime() + processingTime).str()) + "], " + std::string(getName()) + "[sent] frame with seq_num =" + std::to_string(dataSeqNr) + " and payload = [" + frame->getPayload() + "] and trailer = [" + std::string(frame->getTrailer().to_string()) + "], Modified[" + (modficationError ? std::to_string(modfiedBit) : "-1") + "], Lost[" + (lostError ? "Yes" : "No") + "], Duplicate[" + (duplicateError ? "1" : "0") + "], Delay[" + (delayError ? std::to_string(errorDelay) : "0") + "]";
        EV << info << endl;
        fileHandler->writeInOutput(info);
     }

    // If the frame is lost, don't send it and schedule a self message to continue sending frames
    if (lostError)
    {
        if (duplicateError && type == 0) // if the frame is duplicated, print the info
        {
            // printing the frame info
            std::string duplicateInfo = "At time = [" + std::string((simTime() + processingTime).str()) + "], " + std::string(getName()) + "[sent] frame with seq_num =" + std::to_string(dataSeqNr) + " and payload = [" + frame->getPayload() + "] and trailer = [" + std::string(frame->getTrailer().to_string()) + "], Modified[" + (modficationError ? std::to_string(modfiedBit) : "-1") + "], Lost[" + (lostError ? "Yes" : "No") + "], Duplicate[" + "2" + "], Delay[" + (delayError ? std::to_string(errorDelay) : "0") + "]";
            EV << duplicateInfo << endl;
            fileHandler->writeInOutput(duplicateInfo); // write the info in the output file
        }
        return;
    };

    // calculate the total delay
    double totalDelay = processingTime + transmissionDelay + ((delayError) ? errorDelay : 0); // Total delay = PT + TD + ED * (delay bit)
    // If the frame is not lost, send it after the total delay
    sendDelayed(frame, totalDelay, "gout", 0); // send at gate 0 (the peer node) after totalDelay

    if (duplicateError) // if the frame is duplicated, send a duplicate frame
    {
        Frame *duplicateFrame = frame->dup();                                  // duplicate the frame to send it again
        sendDelayed(duplicateFrame, totalDelay + duplicationDelay, "gout", 0); // send a duplicate frame after totalDelay + DD
        // printing the frame info
        if(type == 0)
        {
        std::string duplicateInfo = "At time = [" + std::string((simTime() + processingTime).str()) + "], " + std::string(getName()) + "[sent] frame with seq_num =" + std::to_string(dataSeqNr) + " and payload = [" + frame->getPayload() + "] and trailer = [" + std::string(frame->getTrailer().to_string()) + "], Modified[" + (modficationError ? std::to_string(modfiedBit) : "-1") + "], Lost[" + (lostError ? "Yes" : "No") + "], Duplicate[" + "2" + "], Delay[" + (delayError ? std::to_string(errorDelay) : "0") + "]";
        EV << duplicateInfo << endl;
        fileHandler->writeInOutput(duplicateInfo);
        }
    }
}

// reieve postive ack
// handles the acks and slides the window
void Node::receiveACK(Frame *frame)
{

    // print the info
    std::string ACKinfo = "At time " + std::string((simTime()).str()) + ", " + std::string(getName()) + " [recived] " + "[ACK]" + " with number [" + std::string(std::to_string((frame->getAckSeqNr()) % (receiverMaxSeq + 1))) + "]";
    EV << ACKinfo << std::endl;
    fileHandler->writeInOutput(ACKinfo); // write the info in the output file

    std::cout << "================Rec ACK= 1=====================" << std::endl;
    std::cout<< "expectedAck: " << expectedAck << std::endl;
    std::cout<< "frame->getAckSeqNr(): " << frame->getAckSeqNr() << std::endl;
    std::cout<< "nextFrameToSend: " << nextFrameToSend << std::endl;
    std::cout << "======================================" << std::endl;

    while (between(expectedAck, ((frame->getAckSeqNr() + senderMaxSeq) % (senderMaxSeq + 1)), nextFrameToSend)) {
        // stop the timer
        std::cout << "Stoping timer with number: " << expectedAck << std::endl;
        stopTimer(expectedAck);
        // remove the frame from the buffer
        nBuffered--;
        // slide the window
        expectedAck = (expectedAck + 1) % (senderMaxSeq + 1); // slide the window
    }

    
    std::cout << "================Rec ACK= 2=====================" << std::endl;
    std::cout<< "expectedAck: " << expectedAck << std::endl;
    std::cout<< "frame->getAckSeqNr(): " << frame->getAckSeqNr() << std::endl;
    std::cout<< "nextFrameToSend: " << nextFrameToSend << std::endl;
    std::cout << "======================================" << std::endl;

    // if the buffer was full and now there is space so start checking the netwrork for new packets
    if (nBuffered < senderWindowSize && isFull) // was full and now there is space so start checking the netwrork for new packets
    {
        isFull = false;
    }

    // if there are no more packets to send
    if (noMorePackets)
    {
        if (expectedAck == nextFrameToSend)
        {
            // else end the simulation (sent all the packets and recieved all the acks)
            endSimulation();
        }
    }
}

// recieve negative ack
// handles the nacks and resends the frame
void Node::receiveNACK(Frame *frame)
{
    // print the info
    std::string NACKinfo = "At time " + std::string((simTime()).str()) + ", " + std::string(getName()) + " [recived] " + "[NACK]" + " with number [" + std::string(std::to_string((frame->getAckSeqNr()) % (receiverMaxSeq + 1))) + "]";
    EV << NACKinfo << std::endl;
    fileHandler->writeInOutput(NACKinfo); // write the info in the output file

    if (!between(expectedAck, (frame->getAckSeqNr()), nextFrameToSend)) // if the ack is not in the window
        return;
    
    // retransmit the frame
    std::string info = "At time [" + std::string(simTime().str()) + "], " + std::string(getName()) + ", Introducing channel error with code =[" + "0000" + "], and msg =" + packetsOut[(frame->getAckSeqNr() % senderWindowSize)] + "";
    EV << info << std::endl;
    fileHandler->writeInOutput(info);    

    // stop the timer
    stopTimer((frame->getAckSeqNr()));
    // send the frame again and restart the timer
    std::bitset<4> prefix("0000");
    sendFrame(frame->getAckSeqNr(), 0, packetsOut[(frame->getAckSeqNr() % senderWindowSize)], expectedFrame, prefix);
    startTimer((frame->getAckSeqNr()));
}

// recieve data
// handles the data and sends acks or nacks
void Node::recieveData(Frame *frame)
{
    bool duplicateError = arrivedIn[((frame->getDataSeqNr()) % receiverWindowSize)]; // is the frame duplicated
    bool lostError = false;                                                     // is the frame lost
    bool delayError = false;                                                    // is the frame delayed
    bool modficationError = !frame->checkCheckSum();                             // is the frame modified
    bool isTheExpectedFrame = (frame->getDataSeqNr() == expectedFrame); // is the frame the expected frame

    printInfo();

     // print the info
    std::string info = "At time = [" + std::string((simTime()).str()) + "], " + std::string(getName()) + "[received] frame with seq_num = " + std::to_string(frame->getDataSeqNr()) + " and payload = [" + frame->getPayload() + "] and trailer = [" + std::string(frame->getTrailer().to_string()) + "], Modified[" + (modficationError ? "1" : "-1") + "], Lost[" + (lostError ? "Yes" : "No") + "], Duplicate[" + (duplicateError ? "1" : "0") + "], Delay[" + (delayError ? "1" : "0") + "]";
    EV << info << std::endl;
    fileHandler->writeInOutput(info); // write the info in the output file

    if (!between(expectedFrame, (frame->getDataSeqNr()), recUpperBound) || duplicateError)
    {
        std::cout<< "didn't recieve" << std::endl;
        return;
    } // if the frame is not in the window     

    if(!isTheExpectedFrame && !nackSent) // if the frame is not the expected frame send a nack on the expected frame
    {
        // send negative ack
        std::bitset<4> prefix("0000"); // no error
        std::string NACKinfo = "At time [" + std::string((simTime() + processingTime).str()) + "], " + std::string(getName()) + " [Sending] " + "[NACK]" + " with number [" + std::to_string(expectedFrame) + "]";
        EV << NACKinfo << std::endl;
        fileHandler->writeInOutput(NACKinfo);

        // send the nack
        sendFrame(0, 2, "", expectedFrame, prefix);
        nackSent = true; // mark that a nack is sent
    }

    if (frame->checkCheckSum()) // this removes the framing also
    {
        // send postive ack
        std::bitset<4> prefix("0000"); // no error

        frame->removeFraming();                                                         // remove the framing
        // send the ack
      
        packetsIn[(frame->getDataSeqNr() % receiverWindowSize)] = frame->getPayload(); // save the payload
        arrivedIn[frame->getDataSeqNr() % receiverWindowSize] = true;                  // mark the frame as arrivedIn

        // slide the window
        while (arrivedIn[expectedFrame % receiverWindowSize])
        {
            fileHandler->writeInReceiverOutput(packetsIn[expectedFrame % receiverWindowSize]); // write the payload in the output file
            arrivedIn[expectedFrame % receiverWindowSize] = false;                             // remove the frame from the buffer
            expectedFrame = (expectedFrame + 1) % (receiverMaxSeq + 1);                        // slide the window
            recUpperBound = (recUpperBound + 1) % (receiverMaxSeq + 1);                        // slide the window
            nackSent = false;                                                                  // mark that a nack is not sent
        }

          if(isTheExpectedFrame)
          {
              std::string ACKinfo = "At time [" + std::string((simTime() + processingTime).str()) + "], " + std::string(getName()) + " [Sending] " + "[ACK]" + " with number [" + std::to_string((expectedFrame)) + "]";
              EV << ACKinfo << std::endl;
              fileHandler->writeInOutput(ACKinfo); // write the info in the output file
              sendFrame(0, 1, "", (expectedFrame), prefix);   // send the cumlative ack
          }
                             
    }
    else // the frame is corrupted
    {
        // send negative ack
        std::bitset<4> prefix("0000"); // no error
        std::string NACKinfo = "At time [" + std::string((simTime() + processingTime).str()) + "], " + std::string(getName()) + " [Sending] " + "[NACK]" + " with number [" + std::to_string(frame->getDataSeqNr()) + "]";
        EV << NACKinfo << std::endl;
        fileHandler->writeInOutput(NACKinfo);

        // send the nack
        sendFrame(0, 2, "", frame->getDataSeqNr(), prefix);
    }

}

// process the next packet
void Node::processNextPacket()
{
    // if the buffer is full don't get more packets
    if (isFull == true)
    {
        std::cout << "Buffer is full" << std::endl;
        return;
    }

    // get the next packet
    std::pair<std::string, std::string> packetData = fileHandler->getNextPacketData();
    if (packetData.first == "" && packetData.second == "") // no more packets
    {
        noMorePackets = true;
        return;
    };

    // get the message data
    std::bitset<4> prefix(packetData.first);
    std::string packet = packetData.second;
    // Add the required information to the string stream
    std::string info = "At time [" + std::string(simTime().str()) + "], " + std::string(getName()) + ", Introducing channel error with code =[" + std::string(prefix.to_string()) + "], and msg =" + packet + "";
    EV << info << std::endl;
    fileHandler->writeInOutput(info);

    // send the frame
    packetsOut[nextFrameToSend % senderWindowSize] = packet; // save the packet
    nBuffered++;                                             // increase the number of buffered packets
    if (nBuffered == senderWindowSize)                       // buffer is full so stop checking the network for new packets
        isFull = true;
    // set arrive of the sent frame to false
    sendFrame(nextFrameToSend, 0, packetsOut[nextFrameToSend % senderWindowSize], expectedFrame, prefix); // send the frame
    startTimer(nextFrameToSend);                                                                          // start the timer
    nextFrameToSend = (nextFrameToSend + 1) % (senderMaxSeq + 1);                                         // increase the next frame to send
}

// check if a number is between two numbers
bool Node::between(int a, int b, int c)
{
    return (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)));
}

// start the timer
void Node::startTimer(int seqNr)
{
    // Create a new self message
    std::string name = "timeOutMessage" + std::to_string(counter);   // self message for triggering timeout
    timeOutMsgs[name] = new cMessage(std::to_string(seqNr).c_str()); // self message for triggering timeout
    counter++;                                                       // increase the counter to remove confusion in the logs
    timers[seqNr] = name;                                            // save the timer name
    scheduleAt(simTime() + timeout, timeOutMsgs[name]);              // schedule the timer
}

// stop the timer
void Node::stopTimer(int seqNr)
{
    // std::cout << "Stop timer" << std::endl;
    // std::cout << "seqNr: " << seqNr << std::endl;
    // std::cout << sizeof(timers) << std::endl;

    //  if (seqNr < 0 || seqNr >= sizeof(timers)) {
    //    return; // seqNr is out of bounds
    //}

    std::string name = timers[seqNr]; // get the timer name
    if (name.size() == 0 || timeOutMsgs.find(name) == timeOutMsgs.end() || !(timeOutMsgs[name]->isScheduled()) || !strcmp((timeOutMsgs[name]->getName()), ""))
        return; // the timer is already cancelled or does not exist

    // cancel the timer
    cancelEvent(timeOutMsgs[name]); // Cancel the timer
    delete timeOutMsgs[name];       // Delete the timer
    timeOutMsgs.erase(name);        // Remove the timer from the map
    timers[seqNr] = "";             // remove the timer name
}

// initialize the node
void Node::initialize()
{
    // initialize the node
    (strcmp(getName(), "Node0") == 0) ? fileHandler = new FileHandler("../data/input/input0.txt") : fileHandler = new FileHandler("../data/input/input1.txt");
    senderWindowSize = par("WS").intValue();
    receiverWindowSize = par("WR").intValue();
    processingTime = par("PT").doubleValue();
    timeout = par("TO").doubleValue();
    transmissionDelay = par("TD").doubleValue();
    errorDelay = par("ED").doubleValue();
    duplicationDelay = par("DD").doubleValue();
    counter = 0;
    isFull = false;
    noMorePackets = false;

    // initialize sender variables
    senderWindowSize = par("WS").intValue();
    expectedAck = 0;                                // expected ack number (sender lower bound)
    nextFrameToSend = 0;                            // next frame to send (sender upper bound + 1)
    senderMaxSeq = senderWindowSize * 2 - 1;        // max seq number (2^k - 1)
    nBuffered = 0;                                  // how many output buffers currently used
    packetsOut = new std::string[senderWindowSize]; // sent packets
    timers = new std::string[senderMaxSeq + 1];     // timers for each packet
    for (int i = 0; i < senderMaxSeq + 1; i++)
        timers[i] = ""; // initialize timers

    // initialize receiver variables
    receiverWindowSize = par("WR").intValue();
    expectedFrame = 0;                               // expected frame number (receiver lower bound)
    recUpperBound = receiverWindowSize;              // receiver upper bound
    receiverMaxSeq = receiverWindowSize * 2 - 1;     // max seq number (2^k - 1)
    packetsIn = new std::string[receiverWindowSize]; // sent packets
    arrivedIn = new bool[receiverWindowSize];        // arrivedIn packets
    nackSent = false;                                // is a nack sent
    for (int i = 0; i < receiverWindowSize; i++)
        arrivedIn[i] = false; // initialize arrivedIn
}

void Node::handleMessage(cMessage *msg)
{
    // message from the coordinator to start the simulation
    if (strcmp(msg->getName(), "start") == 0)
    {
        scheduleAt(simTime(), new cMessage("selfMessage")); // schedule a self message to start sending frames
        return;                                             // return to avoid deleting the message
    }
    // after processing the frame get the next packet and send

    // sender role

    // main loop
    else if (msg->isSelfMessage() && strcmp(msg->getName(), "selfMessage") == 0)
    {
        // send self message
        std::string name = "selfMessage" + std::to_string(counter);
        selfMsgs[name] = new cMessage("selfMessage");
        counter++;
        scheduleAt(simTime() + processingTime, selfMsgs[name]); // schedule a self message to continue sending frames
        processNextPacket();
        return;
    }
    // end of main loop

    // timeout case
    else if (msg->isSelfMessage()) // timout message
    {
        // get the seq number
        int seqNr = std::stoi(msg->getName());
        // print the info
        std::string timeOutInfo = "Time out event at time " + simTime().str() + ", at " + std::string(getName()) + " for frame with seq_num=" + std::to_string(seqNr);
        EV << timeOutInfo << std::endl;
        fileHandler->writeInOutput(timeOutInfo);

        // retransmit the frame
        std::string info = "At time [" + std::string(simTime().str()) + "], " + std::string(getName()) + ", Introducing channel error with code =[" + "0000" + "], and msg =" + packetsOut[(seqNr % senderWindowSize)] + "";
        EV << info << std::endl;
        fileHandler->writeInOutput(info);  

        // resend the frame again
        std::bitset<4> prefix("0000");
        startTimer(seqNr);
        sendFrame(seqNr, 0, packetsOut[seqNr % senderWindowSize], (expectedAck % senderWindowSize), prefix);
    }
    // end timeout case
    // end sender role

    // reciever role
    else
    {
        Frame *frame = check_and_cast<Frame *>(msg); // cast the message to a frame
        switch (frame->getType())                    // check the type of the frame
        {
        case 0:                 // Data
            recieveData(frame); // receive data
            break;
        case 1:                // Ack
            receiveACK(frame); // receive ack
            break;
        case 2:                 // Nak
            receiveNACK(frame); // receive nack
            break;
        default:
            break;
        }
    }
    // end reciever role
}

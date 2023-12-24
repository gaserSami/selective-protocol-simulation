/*
 * File Handler class
 * handles file input and output
 * writes the receiver output to the receiver file
 * reads the coordinator data from the coordinator file
 * reads the next packet data from the input file
 * writes the packet data from the receiver file
 */

// INCLUDES
#include <fstream>
#include <string>

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

// ======================
// File Handler class
// ======================
class FileHandler
{
public:
    FileHandler(std::string filePath); // constructor
    virtual ~FileHandler();            // destructor

    // METHODS
    // reads line from the file and gets the 4-bits binary prefix and message from the line (packet information)
    std::pair<std::string, std::string> getNextPacketData();
    void writeInOutput(std::string content);         // appends to the output file
    void writeInReceiverOutput(std::string content); // appends to receiver file
    void writeInfile(std::string content, std::string filePath);           // appends to file
    std::pair<int, int> readCoordinatorData();       // reads coordinator data
    void resetFile(std::string filePath);            // resets the file
    void resetOutput();                              // resets the output file
    void resetReceiverOutput();                      // resets the receiver file
private:
    std::string filePath; // file name
    int currentLine;      // current line
};

#endif /* FILEHANDLER_H_ */

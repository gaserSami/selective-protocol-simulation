// Include files
#include <fstream>
#include <iostream>
#include "FileHandler.h"

// Constructor
FileHandler::FileHandler(std::string filePath)
{
    this->filePath = filePath; // set the file path
    currentLine = 0;           // set the current line to 0
}

// Destructor
FileHandler::~FileHandler()
{
    // EMPTY
}

// Read a line from the file and gets the 4-bits binary prefix and message
std::pair<std::string, std::string> FileHandler::getNextPacketData()
{
    std::ifstream inputFile(filePath); // open the file
    std::string line;                  // line
    int lineCount = 0;                 // line count

    if (inputFile.is_open())
    { // if the file is open
        while (getline(inputFile, line))
        { // get the line
            if (lineCount == currentLine)
            {                                                       // if the line count is equal to the current line
                currentLine++;                                      // increment the current line
                inputFile.close();                                  // close the file
                size_t pos = line.find(' ');                        // find the space
                return {line.substr(0, pos), line.substr(pos + 1)}; // return the prefix and message
            }
            lineCount++; // increment the line count
        }
        inputFile.close(); // close the file
    }
    else
    { // if the file is not open
        std::cout << "Unable to open file for reading.\n";
    }

    return {"", ""}; // return empty strings
}

void FileHandler::writeInfile(std::string content, std::string filePath)
{
    std::ofstream outputFile(filePath, std::ios::app); // open the file

    if (outputFile.is_open())
    {                                  // if the file is open
        outputFile << content << '\n'; // append the content
        outputFile.close();            // close the file
    }
    else
    {
        std::cout << "Unable to open file for writing.\n"; // if the file is not open
    }
}

// Append content to the output file
void FileHandler::writeInOutput(std::string content)
{
    writeInfile(content, "../data/output/output.txt"); // append the content
}

// Append content to the receiver file
void FileHandler::writeInReceiverOutput(std::string content)
{
    writeInfile(content, "../data/output/receiver.txt"); // append the content
}

// Read coordinator data
std::pair<int, int> FileHandler::readCoordinatorData()
{
    std::ifstream inputFile(filePath); // open the file
    int nodeNumber, startTime;         // node number and start time

    if (inputFile.is_open())
    {                                         // if the file is open
        inputFile >> nodeNumber >> startTime; // read the node number and start time
        inputFile.close();                    // close the file
        return {nodeNumber, startTime};       // return the node number and start time
    }
    else
    {
        std::cout << "Unable to open file for reading.\n"; // if the file is not open
    }

    return {-1, -1}; // return -1, -1
}

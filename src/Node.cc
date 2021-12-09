//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Node.h"
#include <vector>
#include <fstream>
#include <string>

Define_Module(Node);

void Node::initialize()
{
    // TODO - Generated method body
}

void Node::handleMessage(cMessage *msg)
{
    //this will initial if it is not Initialized
    if(!isInitialized)
    {
        Initial(msg);
    }

    // TODO - Generated method body

}


void Node::Initial(cMessage *msg)
{
     std:: string Message = msg->getName();
     std::vector <std::string> MessageSplit;
     std::string temp = "";
     //getting text file names and start if it has a start
     for(int i = 0 ; i < Message.size();i++)
     {
         //not space don't slice else slice
         if(Message[i] != ' ')
             temp+= Message[i];
         else
         {
             MessageSplit.push_back(temp);
             temp = "";
         }
     }
     //add last message
     MessageSplit.push_back(temp);
     ReadFromFile(MessageSplit[0]);
     isInitialized = true;

     //Greater than One means it contains more than the input file name
     if(MessageSplit.size() > 1)
     {
         //get last digit in the string as it represent the start time
         int Start = std::stoi(MessageSplit[MessageSplit.size()-1]);
         scheduleAt(simTime() + Start, new cMessage(""));
     }
}


void Node::ReadFromFile(std:: string FileName)
{
    std::string location = "D:/Downloads/Compressed/omnetpp-5.7-windows-x86_64/omnetpp-5.7/samples/Data-Link-Layer-Protocols-Simulation/inputs_samples/"+FileName;
    std::ifstream file (location);
    std::string str;
    while (std::getline(file, str))
    {
       // Process str
        MessageQueueEffect.push_back(str.substr(0,4));
        MessageQueue.push_back(str.substr(5));
    }
}

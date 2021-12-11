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
    if (!isInitialized)
    {
        Initial(msg);
    }

    // TODO - Generated method body
    MyMessage_Base *myMsg = (MyMessage_Base *)msg;
    //assume i have it
    std::string modtype = MessageQueueEffect[0];
    MessageQueueEffect.pop_back();
    ModifyMessage(modtype, myMsg);
}

void Node::Initial(cMessage *msg)
{
    std::string Message = msg->getName();
    std::vector<std::string> MessageSplit;
    std::string temp = "";
    //getting text file names and start if it has a start
    for (int i = 0; i < Message.size(); i++)
    {
        //not space don't slice else slice
        if (Message[i] != ' ')
            temp += Message[i];
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
    if (MessageSplit.size() > 1)
    {
        //get last digit in the string as it represent the start time
        int Start = std::stoi(MessageSplit[MessageSplit.size() - 1]);
        scheduleAt(simTime() + Start, new cMessage(""));
    }
}

void Node::ReadFromFile(std::string FileName)
{
    std::string location = "D:/Downloads/Compressed/omnetpp-5.7-windows-x86_64/omnetpp-5.7/samples/Data-Link-Layer-Protocols-Simulation/inputs_samples/" + FileName;
    std::ifstream file(location);
    std::string str;
    while (std::getline(file, str))
    {
        // Process str
        MessageQueueEffect.push_back(str.substr(0, 4));
        MessageQueue.push_back(str.substr(5));
    }
}
void Node::ModifyMessage(std::string modificationType, MyMessage_Base *msg)
{
    if (modificationType[0] == 1)
    {
        /*Modification: This is done to any randomly selected single bit on the payload
         after the byte stuffing. The modification could not happen to other message fields*/

        //1)get payload from message
        std::string payload = msg->getM_Payload();
        //2) define the vector of the bitsets
        std::vector<std::bitset<8>> myBits;
        //3)loop on the characters and append them to the vector
        for (int i = 0; i < payload.size(); i++)
        {
            std::bitset<8> charFromMsg(payload[i]);
            myBits.push_back(charFromMsg);
        }
        // 4) do the modification to single bit

        int ChosenWord = uniform(0, payload.size());
        int ChosenBit = uniform(0, 9);
        std::cout << "The Error will happen in word number " << ChosenWord << " in Bit Number " << ChosenBit << endl;
        myBits[ChosenWord][ChosenBit] = ~myBits[ChosenWord][ChosenBit];

        //5) loop on the vector convert every bitset to char
        std::string final = "";
        for (std::size_t i = 0; i < myBits.size(); i++)
        {
            final += (char)myBits[i].to_ulong();
        }
        //6) convert modified payload to cstring
        msg->setM_Payload(final.c_str());
        //7) send the modified message
        send((cMessage *)msg, "out");
    }
    else if (modificationType[1] == 1)
    {
        /* Duplicated: the whole message should be sent
         twice with a small difference in time (0.01s)*/

        // 1) send the first message
        send((cMessage *)msg, "out");
        // 2) send the second message with the same message after 0.01 second
        sendDelayed((cMessage *)msg, 0.01, "out");
    }
    else if (modificationType[2] == 1)
    {
        /* Delay: the whole message should be delayed 
        using the delay value from the .ini file. [dont use busy waiting].*/

        // 1)get delay time from omnetpp.ini
        sendDelayed((cMessage *)msg, par("delay"), "out");
    }
    else if (modificationType[3] == 1)
    {
        /*Loss: the whole message should not be sent using the send function,
         but it should be included in the log file and the system calculations*/

        // 1) No send but must write in log file
    }
}

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

#include "coordinator.h"
#include <fstream>
#include <string>
#include <iostream>


Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    //Send A self message to start the project
    scheduleAt(simTime(), new cMessage(""));
}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body

    //File locations should change depending on absolute path of the text file
    
    std::string location = "E:/5th Semester CCE/Computer Networks/Project/Project Code/Data-Link-Layer-Protocols-Simulation/inputs_samples/coordinator.txt";
    std::ifstream file (location);
    std::string str; 

    std::string names [6] = {"pair01.txt","pair01.txt","pair23.txt","pair23.txt","pair45.txt","pair45.txt"};
    int i = 0;
    while (std::getline(file, str))
    {
        // Process str
        //get node number to give it a message of its informations
        std::string Gate = "out" + str.substr(0,1);
        std::string Rest_Message = str.substr(2,str.size()) + " " + names[i++];
        cMessage* newMsg = new cMessage();
        newMsg->setName(Rest_Message.c_str());
        send(newMsg,Gate.c_str());
    }


}



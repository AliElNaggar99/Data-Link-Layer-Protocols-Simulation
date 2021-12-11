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

#ifndef __DATA_LINK_LAYER_PROTOCOLS_SIMULATION_NODE_H_
#define __DATA_LINK_LAYER_PROTOCOLS_SIMULATION_NODE_H_

#include <omnetpp.h>
#include "MyMessage_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  //MessageQueueEffect  possibility of [Modification, Loss, Duplication, Delay]
  std::vector<std::string> MessageQueueEffect;
  std::vector<std::string> MessageQueue;
  int CurrentMsg = 0;
  bool isInitialized = false;

protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  void Initial(cMessage *msg);
  void ReadFromFile(std::string FileName);
  void ModifyMessage(std::string modificationType, MyMessage_Base *msg);
};

#endif

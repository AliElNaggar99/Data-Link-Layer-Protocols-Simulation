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
#include "utilities.h"

Define_Module(Node);


std::ofstream MyFile ("pair01.txt");

void Node::initialize()
{
    // TODO - Generated method body

}

void Node::handleMessage(cMessage *msg)
{
    //MyFile.open("pair01.txt");
    //this will initial if it is not Initialized
    if (!isInitialized)
    {
        Initial(msg);
        return;
    }
    //Cancel and Delete don't forget
    //To wake Up from Coordinator
    MyMessage_Base* MsgRecived = (MyMessage_Base*) msg;
    if(MsgRecived->getM_Type() == Self_Message)
    {
        //Start sending
        //cancelAndDelete(msg);
        SendMsg();
    }
    //Ana Receiver
    else if(MsgRecived->getM_Type() == DATA)
    {
        //Check on SeqNum that I should recievce
        if(MsgRecived->getSeq_Num() == CurrentSeqNum)
        {
            ReceiveData(msg,MsgRecived);
            CurrentSeqNum++;
        }
        //If not same SeqNum, it means this message is duplicated and should be discard and log what happend
        else
        {
          ///Logs
            MyFile<<" - "<<getName()<<" Received Message with id = "<< to_string(MsgRecived->getMessageId()) <<" and Content = "<<MsgRecived->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<" and It is duplicated, so it will be discard"<<endl;
            //MyFile.close();
            MyMessage_Base* ACKMsg = new MyMessage_Base();
            ACKMsg->setM_Type(ACK);
            sendDelayed((cMessage *)ACKMsg, getParentModule()->par("delay").intValue(), "out");
        }
    }
    else if(MsgRecived->getM_Type() == ACK || MsgRecived->getM_Type() == NACK)
    {
        //Ack Rececived
        //Get new msg and send
        CurrentMsg++;
        //cancelAndDelete(msg);
        if(CurrentMsg == MessageQueue.size()-1)
        {
            ///////End Simulation
            std::cout<<"Finished"<<std::endl;
            return;
        }
        SendMsg();
    }
    // TODO - Generated method body


}

void Node::ReceiveData(cMessage *msg,MyMessage_Base* MsgRecived)
{
    std::string MsgPayLoad = MsgRecived->getM_Payload();
    int reminder = computeCrcAtReciever(MsgPayLoad,MsgRecived->getTrailer());
    std::string MsgPayLoadDeframe = DeFrame(MsgPayLoad);
    //cancelAndDelete(msg);
    if(reminder == 0)
    {
        //No error Happened log and Send ACK
        MyFile<<" - "<<getName()<<" Received Message with id = "<< to_string(MsgRecived->getMessageId()) <<" and Content = "<<MsgRecived->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<" without Modification"<<endl;
        //MyFile.close();
        MyMessage_Base* ACKMsg = new MyMessage_Base();
        ACKMsg->setM_Type(ACK);
        sendDelayed((cMessage *)ACKMsg, getParentModule()->par("delay").intValue(), "out");
    }
    else
    {
        //Error Happpened log and Send NACK
        //No error Happened log and Send ACK
        MyFile<<" - "<<getName()<<" Received Message with id = "<< to_string(MsgRecived->getMessageId()) <<" and Content = "<<MsgRecived->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<" with Modification"<<endl;
       // MyFile.close();
        MyMessage_Base* ACKMsg = new MyMessage_Base();
        ACKMsg->setM_Type(NACK);
        sendDelayed((cMessage *)ACKMsg, getParentModule()->par("delay").intValue(), "out");
    }
}

void Node::SendMsg()
{
      //assume i have it
      MyMessage_Base* myMsg = new MyMessage_Base();
      std::string modtype = MessageQueueEffect[CurrentMsg];
      std::string MessageAfterFraming = Frame(MessageQueue[CurrentMsg]);
      char CRC = computeCrcAtSender(MessageAfterFraming);
      myMsg->setM_Type(DATA);
      myMsg->setM_Payload(MessageAfterFraming.c_str());
      myMsg->setTrailer(CRC);
      myMsg->setSendingTime((int)simTime().dbl());
      //Seq_Num will increase in ACK only
      myMsg->setSeq_Num(CurrentMsg);
      myMsg->setMessageId(CurrentMsg);
      ModifyMessage(modtype, myMsg);
}

std::string Node::Frame(std::string Msg)
{
    std:: string MsgAfterFraming = "$";
    for(int i = 0 ; i < Msg.size() ; i++)
    {
        if(Msg[i] == '$' || Msg[i] == '/')
        {
            MsgAfterFraming+="/";
        }
        MsgAfterFraming+=Msg[i];
    }
    MsgAfterFraming+="$";
    return MsgAfterFraming;
}

std::string Node::DeFrame(std::string Msg)
{
    Msg = Msg.substr(1, Msg.size()-2);
    std:: string MsgAfterDeFraming ="";
    for(int i = 0 ; i < Msg.size() ; i++)
    {
        if(Msg[i] == '/')
        {
           i++;
        }
        MsgAfterDeFraming+=Msg[i];
    }
    return MsgAfterDeFraming;
}

void Node::ModifyMessage(std::string modificationType, MyMessage_Base *msg)
{
    if (modificationType[0] == '1')
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
        int ChosenBit = uniform(0, 8);
        //Logs
        //MyFile << "The Error will happen in word number " << ChosenWord << " in Bit Number " << ChosenBit << endl;
        myBits[ChosenWord][ChosenBit] = ~myBits[ChosenWord][ChosenBit];

        //5) loop on the vector convert every bitset to char
        std::string final = "";
        for (std::size_t i = 0; i < myBits.size(); i++)
        {
            final += (char)myBits[i].to_ulong();
        }
        //6) convert modified payload to cstring
        std::cout<<final<<endl;
        msg->setM_Payload(final.c_str());
        std::cout <<msg->getM_Payload()<<endl;
        //7) send the modified message
        if(modificationType[1] == '0' && modificationType[2] == '0' && modificationType[3] == '0')
        {
            send((cMessage *)msg, "out");
            MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<" with Modification"<<endl;
           // MyFile.close();
        }

    }

    if (modificationType[1] == '1')
    {
            /*Loss: the whole message should not be sent using the send function,
             but it should be included in the log file and the system calculations*/

            // 1) No send but must write in log file

        //TimeOut After
        MyFile<<" - "<<getName()<<" drops Message with id = "<< to_string(msg->getMessageId()) <<"at "<<to_string((int)simTime().dbl())<<endl;
        //MyFile.close();
        MyMessage_Base *myMsg = new MyMessage_Base();
        myMsg->setM_Type(ACK);
        scheduleAt(simTime() + 2, (cMessage*)myMsg);

    }

    if (modificationType[2] == '1')
    {
        /* Duplicated: the whole message should be sent
         twice with a small difference in time (0.01s)*/
         std::string isModified ="";
         if(modificationType[0] == '1')
             isModified = " with Modification";
        // 1) send the first message
        send((cMessage *)msg, "out");
        MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<isModified<<endl;
        // 2) send the second message with the same message after 0.01 second
        //sendDelayed((cMessage *)msg, 0.01, "out");
        MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<isModified<<endl;
        //MyFile.close();
    }

    if (modificationType[3] == '1')
    {
        /* Delay: the whole message should be delayed 
        using the delay value from the .ini file. [dont use busy waiting].*/

        // 1)get delay time from omnetpp.ini
        std::string isModified ="";
        if(modificationType[0] == '1')
               isModified = " with Modification";
        MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<isModified<<"Delayed with time = "<<to_string(getParentModule()->par("delay").intValue())<<endl;
        sendDelayed((cMessage *)msg, getParentModule()->par("delay").intValue(), "out");
        //MyFile.close();
    }

    if(modificationType[0] == '0' && modificationType[1] == '0' && modificationType[2] == '0' && modificationType[3] == '0')
    {
        send((cMessage *)msg, "out");
        MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<to_string((int)simTime().dbl())<<endl;
        //MyFile.close();
    }

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
        MyMessage_Base *myMsg = new MyMessage_Base();
        myMsg->setM_Type(Self_Message);
        scheduleAt(simTime() + Start, (cMessage*)myMsg);
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

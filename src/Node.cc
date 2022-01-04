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
#include <iomanip>
Define_Module(Node);
std::ofstream MyFile("out5.txt");

void Node::handleMessage(cMessage *msg)
{
    /////////////
    //Cancel and Delete don't forget

    MyFile << std::fixed << std::setprecision(2);
    //MyFile.open("pair01.txt");


    //this will initial if it is not Initialized
    if (!isInitialized)
    {
        Initial(msg);
        return;
    }

    MyMessage_Base* MsgRecived = (MyMessage_Base*) msg;

    std::cout<<"Message ID: "<< MsgRecived->getSeq_Num() << " " << getName() << " " <<to_string(MsgRecived->getM_Type())<<endl; 

    //To wake Up from Coordinator
    if(MsgRecived->getM_Type() == Self_Message)
    {
        //Start sending
        // must cancel and delete since you send yourself a message
        cancelAndDelete(msg);
        SendMsg();
    }
    else if(MsgRecived->getM_Type() == DATA)
    {
        // Receiver Code

        // FRAME ARRIVAL -------------------------------------------------------------------

        std::cout << "Received Message with id = "<< to_string(MsgRecived->getMessageId()) <<std::endl;
        //Check on SeqNum that i recieved the correct message within my buffer.
        // checking if you get the first one
        if (MsgRecived->getAckId() == left_recieveBuffer)
        {
            //Therefore you recieved the first one in the window 
            //and need to check if the next ones already got to advance them all
            std::cout << "equal  left_recieveBuffer"<<std::endl;
            framesRecieved.insert(left_recieveBuffer);
            //set<int>::iterator itr = acksRecieved.begin();
            int DelayMult = 0;
            while (*framesRecieved.begin() == left_recieveBuffer) // checking that next are already got
            {
                framesRecieved.erase(left_recieveBuffer);
                left_recieveBuffer++;
                right_recieveBuffer++;
                CurrentSeqNum++;
                

                // take data and sends nack if the data is corrupted no matter order
                // - - 2 - -
                // if 2 came with wrong crc we need to nak the 2 even before 0 in order not to save wrong data
                // and when sender sends 2 again correctly , the code will handle the zero nak already..
                bool errorHappened  = ReceiveData(msg,MsgRecived);
                if(errorHappened == false)
                {
                    // he already sent nack 
                    std:: cout << " Crc error happened !" <<std::endl ;
                    return ;
                } 

            }
        }else{
            // check if between left , right
            // send nack on the left
            
            // TODO send nack !!!!!!!!!!!!!!!!!!!!!!!! 

            // TODO 2 : != framesRecieved.end() ? CHANGED TO == 
            if(MsgRecived->getAckId() > left_recieveBuffer && MsgRecived->getAckId() <= right_sendBuffer 
            && framesRecieved.find(MsgRecived->getAckId()) == framesRecieved.end())
            {
                framesRecieved.insert(MsgRecived->getAckId());
            }
        }

        // ACK ARRIVAL -------------------------------------------------------------------

        MyFile  <<" - "<<getName()<<" Received ACK with id = " << 
        std::to_string(MsgRecived->getAckId()) << " data : " << MsgRecived->getM_Payload() 
        <<" , at " << " "<<(double)simTime().dbl() << endl;

        std::cout<<"ACK ID: "<< MsgRecived->getAckId() << " " << getName() << " " <<to_string(MsgRecived->getM_Type())<<endl; 
        // checking if you get the first one
        if (MsgRecived->getAckId() == left_sendBuffer+1)
        {
            std::cout <<" ack recieved = left +1 now so we can advance" << std::endl ;

            //Therefore you recieved the first one in the window and need to check if the next ones already got to advance them all
            acksRecieved.insert(left_sendBuffer);
            //set<int>::iterator itr = acksRecieved.begin();
            int DelayMult = 0;
            while (*acksRecieved.begin() == left_sendBuffer) // checking that next are already got
            {

                acksRecieved.erase(left_sendBuffer);
                left_sendBuffer++;
                right_sendBuffer++;
                // TODO :: if we recieved  - - 2 - - ? what do we send first ? or third ?  
                
                // TODO :: i removed sending multiples as we send only one  :: currentSent ??
                
                // SendOneMsg(right_sendBuffer,DelayMult*WindowsDelay); // since that sequence number = index 
                // //Sending the Right
                // correct++;
                // // incrementing currentMsg for sync , currentMsgIndex , msgId
                // CurrentMsg++;
                // CurrentMsgIndex++;
                // MsgId++;
                // //SendMsg();
            }
        }
        else
        {
            // so you recieved ack but not for the first one you need to mark it if its within window
            // TODO acksRecieved.find(MsgRecived->getAckId()) == acksRecieved.end()) -> == ??
            if(MsgRecived->getAckId() > left_sendBuffer && MsgRecived->getAckId() <= right_sendBuffer && acksRecieved.find(MsgRecived->getAckId()) == acksRecieved.end())
            {
                std::cout <<" ack got not equal first one but equal " << MsgRecived->getAckId()  << std::endl ;
                acksRecieved.insert(MsgRecived->getAckId());
            }
            // TODO :: now ack = 1, left = 0 mean we need frame 1 so we will advance window 
            // duplicate we need to change it to check if its within acksrecieved or not 
            else if(acksRecieved.find(MsgRecived->getAckId()) != acksRecieved.end() )
            {
                // mean that we recieved same twice
                //wrong ack recieved (discard ?? or duplicate ?? )
                duplicate++ ;
                EV<< "recieved same ack so will discard" << std::endl ;
                //cancelAndDelete(msg);
                return;
            } 
        }

        std::cout <<" starting data pipeling stage" << std::endl ;
        // DATA PIPELING WITH THE ACK ------------------------------------
        // we need to check if there exists data to send with the ack
        if (left_sendBuffer == MessageQueueEffect.size() || currentSent > right_sendBuffer)
        {
            // therefore no more data and we need to send acktimeout !
            MsgRecived->setM_Type(ACK_TimeOut);
            MsgRecived->setAckId(CurrentSeqNum);
            // no data so send here
            std::cout <<" no data , so will send Ack_timeout" << std::endl ;
            sendDelayed((cMessage *)MsgRecived, getParentModule()->par("delay").doubleValue(), "out");
            return;
        }
        std::cout <<" current sent , left sendbuffer , right send buffer = " <<currentSent
                << " " << left_sendBuffer<<" " << right_sendBuffer << std::endl ;
        if (currentSent <= right_sendBuffer && currentSent >= left_sendBuffer )
        {
            std::cout <<" sending message packed with new ack " << std::endl ;
            // therefore there's more to send
            // TODO : windows delay ? or getParentModule()->par("delay").doubleValue() ? or 0 ? 
            SendOneMsg(currentSent , getParentModule()->par("delay").doubleValue()) ; 
            currentSent++;
        }
    
    }
    else if(MsgRecived->getM_Type() == NACK)
    {
        // Sender Code
        // recieved nack so we need to resend this one again
        incorrect++;
        // check to see that the nack is on one of the frames within the window
        if(MsgRecived->getAckId() > left_sendBuffer && MsgRecived->getAckId() <= right_sendBuffer)
        {
            // since that sequence number = index 
            
            // TODO : windows delay in nack not 0?
            SendOneMsg(left_sendBuffer,getParentModule()->par("delay").doubleValue()); 
        }
                
    }
    // timeOut case occurred
    else if(MsgRecived->getM_Type() == TimeOut)
    {
        // handling the case of timeOut where loss occurred
        if(MsgRecived->getAckId() == CurrentMsg)
        {
            std::cout << "entered timeout" << std::endl;
            // losses++ ;
            // // handling loss case since we wont be sending it again so will increment index inside vector messages only
            // CurrentMsgIndex++ ; // incrementing index only to send next message but with same id
            // MsgId++ ;
            // SendMsg();
            SendOneMsg(MsgRecived->getAckId(),0); // since that sequence number = index 
        }
        cancelAndDelete(msg);

    // duplicate case occurred
    }
    else if(MsgRecived->getM_Type() == Duplicate)
    {
            MsgRecived->setM_Type(DATA);
            send((cMessage *)MsgRecived, "out");
            string isModified = "";
            if(MsgRecived->getIsModified() == true)
                isModified = " with Modification";
            else
                isModified = " without Modification" ;

            MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(MsgRecived->getMessageId()) <<" and Content = "<<MsgRecived->getM_Payload()<<" at "<<((double)simTime().dbl()) << isModified
                    << " Ack number " << std::to_string(MsgRecived->getAckId()) <<endl;
    }
}


bool Node::ReceiveData(cMessage *msg,MyMessage_Base* MsgRecived)
{
    std::string MsgPayLoad = MsgRecived->getM_Payload();
    int reminder = computeCrcAtReciever(MsgPayLoad,MsgRecived->getTrailer());
    std::string MsgPayLoadDeframe = DeFrame(MsgPayLoad);
    std::cout<<"Receiver Received Seq Num "<<MsgRecived->getSeq_Num()<<endl;
    if(reminder == 0)
    {
        //No error Happened log and Send ACK
        MyFile<<" - "<<getName()<<" Received Message with id = "<< to_string(MsgRecived->getMessageId()) <<" and Content = "<<MsgRecived->getM_Payload()<<" at "<<((double)simTime().dbl())<<" without Modification"
                << " Ack number " << std::to_string(MsgRecived->getAckId()) <<endl;
        // MyMessage_Base* ACKMsg = new MyMessage_Base();
        // here we need to put data in message if exists
        return true; 

    }
    else
    {
        //Error happened log and Send NACK
        MyFile<<" - "<<getName()<<" Received Message with id = "<< to_string(MsgRecived->getMessageId()) <<" and Content = "<<MsgRecived->getM_Payload()<<" at "<<((double)simTime().dbl())<<" with Modification"
                << " Ack number " << std::to_string(MsgRecived->getAckId()) <<endl;

        MyMessage_Base* ACKMsg = new MyMessage_Base();
        ACKMsg->setM_Type(NACK);
        ACKMsg->setSeq_Num(CurrentSeqNum);
        sendDelayed((cMessage *)ACKMsg, getParentModule()->par("delay").doubleValue(), "out");
        return false;
    }
}

void Node::SendMsg()
{
      //fe for loop with delay with currentMsgIndex
      int DelayIncrement = 0;
      for(int i = left_sendBuffer ; i <= right_sendBuffer ; i++)
      {
        //assume i have it
        SendOneMsg(i,DelayIncrement*WindowsDelay);
        currentSent++;
        DelayIncrement++;
      }
}



void Node::SendOneMsg(int index, double FrameDelay)
{
    if(left_sendBuffer >= MessageQueue.size())
    {
            ///////End Simulation
            PrintOutput();
            return;
    }

    if(index >= MessageQueue.size())
        return;
        
    MyMessage_Base* myMsg = new MyMessage_Base();
    std::string modtype = MessageQueueEffect[index];
    std::string MessageAfterFraming = Frame(MessageQueue[index]);
    char CRC = computeCrcAtSender(MessageAfterFraming);
    myMsg->setM_Type(DATA);
    myMsg->setM_Payload(MessageAfterFraming.c_str());
    myMsg->setTrailer(CRC);
    myMsg->setSendingTime((double)simTime().dbl());
    
    //Seq_Num will increase in ACK only
    myMsg->setSeq_Num(index);
    myMsg->setMessageId(index);
    
    // setting ack with the message i need
    myMsg->setAckId(left_recieveBuffer);

    ModifyMessage(modtype, myMsg,FrameDelay);
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

void Node::ModifyMessage(std::string modificationType, MyMessage_Base *msg, double FrameDelay)
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
            sendDelayed((cMessage *)msg, FrameDelay, "out");
            MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<((double)simTime().dbl()+FrameDelay)<<" with Modification"
                    << " Ack number " << std::to_string(CurrentMsg) <<endl;
           // MyFile.close();
        }

    }

    if (modificationType[1] == '1')
    {
        /*Loss: the whole message should not be sent using the send function,
        but it should be included in the log file and the system calculations*/
        MyFile<<" - "<<getName()<<" drops Message with id = "<< to_string(msg->getMessageId()) <<" at "<<((double)simTime().dbl() + FrameDelay)<<endl;
    }
    bool delayed = 0;
    if (modificationType[3] == '1')
    {
        /* Delay: the whole message should be delayed
        using the delay value from the .ini file. [dont use busy waiting].*/

        // check if not loss
        if(modificationType[1] != '1'){
            // 1)get delay time from omnetpp.ini
            std::string isModified ="";
            if(modificationType[0] == '1')
                   isModified = " with Modification";


            if(modificationType[2] == '0'){
                MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<((double)simTime().dbl()+FrameDelay)<<isModified<<" Delayed with time = "<<(double)(getParentModule()->par("delay").doubleValue())
                        << " Ack number " << std::to_string(CurrentMsg) <<endl;

                sendDelayed((cMessage *)msg, FrameDelay + getParentModule()->par("delay").doubleValue(), "out");
            }
            else{
                delayed = 1;
            }
        }
    }

    if (modificationType[2] == '1')
    {
        /* Duplicated: the whole message should be sent
         twice with a small difference in time (0.01s)*/
         string delayString = "";
         double delayValue = 0 ;
         if(delayed == 1){
             delayString = ", started 0.2 later due to delay" ;
             delayValue = getParentModule()->par("delay").doubleValue();
         }
         std::string isModified =" without Modification";
         if(modificationType[0] == '1')
             isModified = " with Modification";

         // 1) send the first message
         if(modificationType[1] != '1'){
            sendDelayed((cMessage *)msg , delayValue + FrameDelay, "out");
            MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<((double)simTime().dbl() + delayValue + FrameDelay)<< isModified << delayString
                    << " Ack number " << std::to_string(CurrentMsg) <<endl;
            MyMessage_Base* NewMsg = new MyMessage_Base(*msg);
            NewMsg->setM_Type(Duplicate);
            NewMsg->setIsModified(modificationType[0]=='1'? true : false);
            // 2) send the second message with the same message after 0.01 second
            scheduleAt(simTime() + 0.01 + delayValue + FrameDelay, (cMessage*)NewMsg);
        }else
            duplicate++;

    }



    if(modificationType[0] == '0' && modificationType[1] == '0' && modificationType[2] == '0' && modificationType[3] == '0')
    {
        sendDelayed((cMessage *)msg, FrameDelay, "out");
        MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<((double)simTime().dbl()+FrameDelay)
        << " Ack number " << std::to_string(CurrentMsg) <<endl;
        //MyFile.close();
    }

    Timer();
}

void Node::Timer(){
    MyMessage_Base *myMsg = new MyMessage_Base();
    myMsg->setM_Type(TimeOut);
    myMsg->setSeq_Num(CurrentMsg);
    scheduleAt(simTime() + 1, (cMessage*)myMsg);
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
        startTime = Start;
        MyMessage_Base *myMsg = new MyMessage_Base();
        myMsg->setM_Type(Self_Message);
        scheduleAt(simTime() + Start, (cMessage*)myMsg);
    }
}

void Node::ReadFromFile(std::string FileName)
{
    //"E:/5th Semester CCE/Computer Networks/Project/Project Code/Data-Link-Layer-Protocols-Simulation/inputs_samples/coordinator.txt";
    std::string location = "E:/5th Semester CCE/Computer Networks/Project/Project Code/Data-Link-Layer-Protocols-Simulation/inputs_samples/" + FileName;
    std::ifstream file(location);
    std::string str;
    while (std::getline(file, str))
    {
        // Process str
        MessageQueueEffect.push_back(str.substr(0, 4));
        MessageQueue.push_back(str.substr(5));
    }

    right_sendBuffer = getParentModule()->par("WindowSize").intValue()-1;
    right_recieveBuffer = getParentModule()->par("WindowSize").intValue()-1;

    WindowsDelay = getParentModule()->par("WindowDelay").doubleValue();
}


void Node::PrintOutput()
{
    MyFile << " ------------------------------------------------------------- " << endl ;
    MyFile << " - " << getName() << " end of input file " << endl ;
    MyFile << " - " << "Total Transmission time : " << ((double)simTime().dbl()-startTime) << std::endl ;
    MyFile << " - " << "correct : " << correct << std::endl ;
    MyFile << " - " << "duplicate : " << duplicate << std::endl ;
    MyFile << " - " << "losses : " << losses << std::endl ;
    MyFile << " - " << "incorrect : " << incorrect << std::endl ;
    MyFile << " - " << "Total number of Transmission : " << correct + duplicate + losses + incorrect << std::endl ;
    MyFile << " - " << "Throughput : " <<  (double)correct /  ((double)simTime().dbl()-startTime) << std::endl;
    std::cout<<"Finished"<<std::endl;
    MyFile.close();
    return;
}

void Node::initialize()
{
    // TODO - Generated method body

}

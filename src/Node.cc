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
//std::ofstream MyFile("out5.txt");

void Node::handleMessage(cMessage *msg)
{
    /////////////
    //Cancel and Delete don't forget


    //MyFile.open("pair01.txt");


    //this will initial if it is not Initialized
    if (!isInitialized)
    {
        Initial(msg);
        return;
    }

    MyFile << std::fixed << std::setprecision(2);
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
        if (MsgRecived->getSeq_Num() == left_recieveBuffer)
        {
            //Therefore you recieved the first one in the window 
            //and need to check if the next ones already got to advance them all
            std::cout << "equal  left_recieveBuffer"<<std::endl;
            framesRecieved.insert(left_recieveBuffer);
            //set<int>::iterator itr = acksRecieved.begin();
            int DelayMult = 0;

            // to check on first one only
            bool noError  = ReceiveData(msg,MsgRecived);
            if(noError == false)
            {
                // he already sent nack 
                std:: cout << " Crc error happened !" <<std::endl ;
                return ;
            }

            while (*framesRecieved.begin() == left_recieveBuffer) // checking that next are already got
            {
                framesRecieved.erase(left_recieveBuffer);
                left_recieveBuffer++;
                right_recieveBuffer++;

                // TODO :: ??????????????
                // CurrentSeqNum++;

                // TODO :: ??????????????
                // take data and sends nack if the data is corrupted no matter order
                // 0 - - - -
                // if 2 came with wrong crc we need to nak the 2 even before 0 in order not to save wrong data
                // and when sender sends 2 again correctly , the code will handle the zero nak already..
            }
        }else{
            // check if between left , right
            // send nack on the left
            
            // TODO 2 : != framesRecieved.end() ? CHANGED TO == 
            if(MsgRecived->getSeq_Num() > left_recieveBuffer && MsgRecived->getSeq_Num() <= right_recieveBuffer 
            && framesRecieved.find(MsgRecived->getSeq_Num()) == framesRecieved.end())
            {
                framesRecieved.insert(MsgRecived->getSeq_Num());
                // TODO send nack !!!!!!!!!!!!!!!!!!!!!!!! 
                bool noError  = ReceiveData(msg,MsgRecived);
                if(noError == false)
                {
                    // he already sent nack 
                    std:: cout << " Crc error happened !" <<std::endl ;
                    return ;
                }
                // send nack on the one it didnt come
                // TODO:: !!!!!!!!
                if(noMoreNak != left_recieveBuffer){
                    noMoreNak = left_recieveBuffer;

                    MyFile  <<" - "<<getName()<<" sends nack on " <<
                                        std::to_string(left_recieveBuffer) <<" , at " << " "<<(double)simTime().dbl() << endl;
                    incorrect++;
                    MsgRecived->setAckId(left_recieveBuffer);
                    SendOneMsg(MsgRecived->getAckId(),getParentModule()->par("delay").doubleValue(), NACK);
                    return ;
                }
            }else{
                std::cout <<getName() << " get Seq sent ,  left_recieveBuffer , right_sendBuffer = " <<MsgRecived->getSeq_Num()
                << " " << left_recieveBuffer<<" " << right_sendBuffer << std::endl ;

                MyFile  <<" - "<<getName()<<" Received duplicate with id = " <<
                    std::to_string(MsgRecived->getSeq_Num()) << " data : " << MsgRecived->getM_Payload()
                    <<" , at " << " "<<(double)simTime().dbl() << endl;
                
                cancelAndDelete(msg);
                return ; 
            }
        }

        std::cout <<"after recieving," << getName()  << " : current sent , left_recieveBuffer , right_recieveBuffer = " 
            <<currentSent << " " << left_recieveBuffer<<" " << right_recieveBuffer << std::endl ;
        

        // ACK ARRIVAL -------------------------------------------------------------------
//        MyFile  <<" - "<<getName()<<" Received ACK with id = " <<
//        std::to_string(MsgRecived->getAckId()) << " data : " << MsgRecived->getM_Payload()
//        <<" , at " << " "<<(double)simTime().dbl() << endl;

        // std::cout<<"ACK ID: "<< MsgRecived->getAckId() << " " << getName() << " " <<to_string(MsgRecived->getM_Type())<<endl; 
        // checking if you get the first one
        
        std::cout <<" starting data pipeling stage" << std::endl ;
        // DATA PIPELING WITH THE ACK ------------------------------------
        // we need to check if there exists data to send with the ack
        
        if (currentSent == MessageQueueEffect.size() )
        {
            // therefore no more data and we need to send acktimeout !
            left_sendBuffer = right_sendBuffer ;
            MsgRecived->setM_Type(ACK_TimeOut);
            
            MsgRecived->setAckId(left_recieveBuffer);
            // no data so send here
            std::cout <<" no data , so will send Ack_timeout : " << left_recieveBuffer<< std::endl ;
            sendDelayed((cMessage *)MsgRecived, getParentModule()->par("delay").doubleValue(), "out");
            return;
        }

        std::cout <<getName() << " current sent , left sendbuffer , right send buffer = " <<currentSent
                << " " << left_sendBuffer<<" " << right_sendBuffer << std::endl ;
        
        // advancing send window stage --------------------------------------------
        std :: cout <<" MsgRecived->getAckId() :" << MsgRecived->getAckId() << ",  left_sendBuffer : " << left_sendBuffer << endl;
        if (MsgRecived->getAckId() == left_sendBuffer+1)
        {
            //Therefore you recieved the first one in the window and need to check if the next ones already got to advance them all
            acksRecieved.insert(left_sendBuffer);
            //set<int>::iterator itr = acksRecieved.begin();
            int DelayMult = 0;
            while (*acksRecieved.begin() == left_sendBuffer) // checking that next are already got
            {
                std::cout <<" ack recieved = left +1 now so we can advance" << std::endl ;
                acksRecieved.erase(left_sendBuffer);
                left_sendBuffer++;
                right_sendBuffer++;
                cout << " now after erasing , left_sendBuffer " << left_sendBuffer << " , right_sendBuffer "<< right_sendBuffer << endl ; 

                // TODO :: if we recieved  - - 2 - - ? what do we send first ? or third ?  
                
                // TODO :: i removed sending multiples as we send only one  :: currentSent ??
            }
        }
        else
        {
            std::cout << getName()  <<",,  debug !! ack got equal " << MsgRecived->getAckId()  << std::endl ;
            // left = 2 , right = 3
            // ack = 4
            // so you recieved ack but not for the first one 
//            if(MsgRecived->getAckId() > left_sendBuffer+1 && MsgRecived->getAckId() <= right_sendBuffer+1
//            && acksRecieved.find(MsgRecived->getAckId()-1) == acksRecieved.end())
//            {
//                std::cout <<" ack got not equal first one but equal " << MsgRecived->getAckId()  << std::endl ;
//                acksRecieved.insert(MsgRecived->getAckId() -1);
//            }
            // TODO :: now ack = 1, left = 0 mean we need frame 1 so we will advance window 
            // duplicate we need to change it to check if its within acksrecieved or not 
            if(acksRecieved.find(MsgRecived->getAckId()) != acksRecieved.end() )
            {
                // mean that we recieved same twice
                //wrong ack recieved (discard ?? or duplicate ?? )
                duplicate++ ;
                EV<< "recieved same ack so will discard" << std::endl ;
                //cancelAndDelete(msg);
                return;
            }else if(MsgRecived->getAckId() >= right_sendBuffer +1){
                // cumulative ack :
                // left 2  , right = 3 --> ack = 4
                std::cout <<" accumulative ack !!!" << std::endl ;
                acksRecieved.clear() ;
                left_sendBuffer = MsgRecived->getAckId() ;
                right_sendBuffer = left_sendBuffer + getParentModule()->par("WindowSize").intValue()-1;
//                acksRecieved.insert(left_sendBuffer);
//                while (*acksRecieved.begin() == left_sendBuffer) // checking that next are already got
//                {
//                    acksRecieved.erase(left_sendBuffer);
//                    left_sendBuffer++;
//                    right_sendBuffer++;
//                }
                cout << " now after erasing , left_sendBuffer " << left_sendBuffer << " , right_sendBuffer "<< right_sendBuffer << endl ;
            }
        }

        //Sending message phase ---------------------------------------------------------------
        if (currentSent <= right_sendBuffer && currentSent >= left_sendBuffer )
        {
            std::cout <<" sending message packed with new ack " << left_recieveBuffer << std::endl ;
            
            // therefore there's more to send 
            SendOneMsg(currentSent , getParentModule()->par("delay").doubleValue()) ; 
            std:: cout << getName() << ", currentSent : " << currentSent << endl ;
            currentSent++;
        }

    }
    else if(MsgRecived->getM_Type() == NACK)
    {
        std::cout <<" entered nack " << endl ;
        // Sender Code
        // recieved nack so we need to resend this one again
        incorrect++;
        // check to see that the nack is on one of the frames within the window
        if(MsgRecived->getAckId() >= left_sendBuffer && MsgRecived->getAckId() <= right_sendBuffer)
        {
            // since that sequence number = index 
            
            // TODO : windows delay in nack not 0?
            SendOneMsg(MsgRecived->getAckId(),getParentModule()->par("delay").doubleValue(),DATA_RETRANSMISSION);
            std::cout << "currentSent in the nack block : " << currentSent << endl ;
        }
                
    }
    // TimeOut case occurred
    else if(MsgRecived->getM_Type() == TimeOut)
    {
        // TODO :: changed since TimeOut now can come from 2 cases losses from me , or from reciever
        std::cout << "recieved timeout for " << MsgRecived->getSeq_Num() << ".. will check now " << endl ;
        // first we need to check if this timeout will be cancelled since i recieved ack from reciever
        if(MsgRecived->getSeq_Num() == left_sendBuffer){
            MyFile<<" - "<<getName()<<" got TimeOut on  "<< to_string(MsgRecived->getSeq_Num()) <<" at "<<((double)simTime().dbl())<< endl;
            // so need to send it again
            std::cout << MsgRecived->getSeq_Num() << " Timed out !" << std::endl;
            SendOneMsg(MsgRecived->getSeq_Num(),0,DATA_RETRANSMISSION); // since that sequence number = index
        }
        //else discard 
        cancelAndDelete(msg);
        

        // OLD CODE
        // handling the case of timeOut where loss occurred
        // if(MsgRecived->getSeq_Num() == CurrentMsg)
        // {
        //     std::cout << "entered timeout" << std::endl;
        //     // losses++ ;
        //     // // handling loss case since we wont be sending it again so will increment index inside vector messages only
        //     // CurrentMsgIndex++ ; // incrementing index only to send next message but with same id
        //     // MsgId++ ;
        //     // SendMsg();
        //     SendOneMsg(MsgRecived->getSeq_Num(),0); // since that sequence number = index 
        // }
        // cancelAndDelete(msg);

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
    else if(MsgRecived->getM_Type() == ACK_TimeOut)
    {
        // mean the sender has no data to take and he sent ack only


        // TODO :: REDUNDANT CODE IN DATA SO ->FUNCTIONS
        // ACK ARRIVAL -------------------------------------------------------------------

//        MyFile  <<" - "<<getName()<<" Received ACK with id = " <<
//        std::to_string(MsgRecived->getAckId()) << " data : " << MsgRecived->getM_Payload()
//        <<" , at " << " "<<(double)simTime().dbl() << endl;

        std::cout<<"recieved ACK_TimeOut : "<< MsgRecived->getAckId()<< " , left_sendBuffer : " << left_sendBuffer <<endl;
        // checking if you get the first one
        MyFile<<" - "<<getName()<<" recieved ACK_TimeOut : "<< to_string(MsgRecived->getMessageId()) <<" at "<<((double)simTime().dbl())<<" Ack number " << std::to_string(MsgRecived->getAckId()) <<endl;

        if (MsgRecived->getAckId() == left_sendBuffer+1)
        {
            //Therefore you recieved the first one in the window and need to check if the next ones already got to advance them all
            acksRecieved.insert(left_sendBuffer);
            //set<int>::iterator itr = acksRecieved.begin();
            int DelayMult = 0;
            while (*acksRecieved.begin() == left_sendBuffer) // checking that next are already got
            {

                std::cout <<" ack recieved = left +1 now so we can advance" << std::endl ;
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
            if(MsgRecived->getAckId() > left_sendBuffer+1 && MsgRecived->getAckId() <= right_sendBuffer && acksRecieved.find(MsgRecived->getAckId()) == acksRecieved.end())
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
        std::cout <<getName()  << " current sent , left sendbuffer , right send buffer = " <<currentSent
                << " " << left_sendBuffer<<" " << right_sendBuffer << std::endl ;

        // TODO :: removed currentSent <= rightsend
        if (currentSent == MessageQueueEffect.size() )
        {
            // therefore no more data and we recieved ackTimeout so both nodes have no messages
            PrintOutput();
            return;
        }

        //TODO
        if (currentSent <= right_sendBuffer && currentSent >= left_sendBuffer )
        {
            std::cout <<" sending message packed with new ack " << std::endl ;
            // therefore there's more to send
            // TODO : windows delay ? or getParentModule()->par("delay").doubleValue() ? or 0 ? 
            SendOneMsg(currentSent , getParentModule()->par("delay").doubleValue()) ; 
            std:: cout << getName() << ", currentSent : " << currentSent << endl ;
            currentSent++;
        }
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
        ACKMsg->setSeq_Num(MsgRecived->getSeq_Num());
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
        std:: cout << getName() << ", currentSent after window : " << currentSent << endl ;
}



void Node::SendOneMsg(int index, double FrameDelay ,M_Type myType)
{
    if(left_sendBuffer >= MessageQueue.size())
    {
            ///////End Simulation
            // TODO : REMOVED IT FROM HERE AND put it in acktimeout if both
            // PrintOutput();
            return;
    }

    if(index >= MessageQueue.size())
        return;
        
    MyMessage_Base* myMsg = new MyMessage_Base();
    std::string modtype = MessageQueueEffect[index];
    std::string MessageAfterFraming = Frame(MessageQueue[index]);
    char CRC = computeCrcAtSender(MessageAfterFraming);
    myMsg->setM_Type(myType);
    myMsg->setM_Payload(MessageAfterFraming.c_str());
    myMsg->setTrailer(CRC);
    myMsg->setSendingTime((double)simTime().dbl());
    
    //Seq_Num will increase in ACK only
    myMsg->setSeq_Num(index);
    myMsg->setMessageId(index);
    
    // setting ack with the message i need
    // ack id = ? 

    myMsg->setAckId(left_recieveBuffer);
    std::cout << "ack being sent -> " << left_recieveBuffer << endl ;

    if(myType != DATA_RETRANSMISSION && myType!=NACK )
        ModifyMessage(modtype, myMsg,FrameDelay);
    else if(myType == DATA_RETRANSMISSION){
            myMsg->setM_Type(DATA);
            ModifyMessage("0000", myMsg,FrameDelay);
    }else if(myType == NACK){
        sendDelayed((cMessage *)myMsg, FrameDelay, "out");
    }

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
                    << " Ack number " << std::to_string(msg->getAckId()) <<endl;
           // MyFile.close();
        }

    }

    if (modificationType[1] == '1')
    {
        /*Loss: the whole message should not be sent using the send function,
        but it should be included in the log file and the system calculations*/
        MyFile<<" - "<<getName()<<" drops Message with id = "<< to_string(msg->getMessageId()) <<" at "<<((double)simTime().dbl() + FrameDelay)<<endl;

        // so others can preceed without this lost one
        // currentSent++ ;

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
                        << " Ack number " << std::to_string(msg->getAckId()) <<endl;

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

            MyMessage_Base* NewMsg = new MyMessage_Base(*msg);
            MyFile<<" - "<<getName()<<" Sends Message with id = "<< to_string(msg->getMessageId()) <<" and Content = "<<msg->getM_Payload()<<" at "<<((double)simTime().dbl() + delayValue + FrameDelay)<< isModified << delayString
                                << " Ack number " << std::to_string(NewMsg->getAckId()) <<endl;
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
        << " Ack number " << std::to_string(msg->getAckId()) <<endl;
        //MyFile.close();
    }

    Timer(msg->getSeq_Num());
}

void Node::Timer(int seq_nr){
    MyMessage_Base *myMsg = new MyMessage_Base();
    myMsg->setM_Type(TimeOut);
    myMsg->setSeq_Num(seq_nr);
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

    //MyFile.close();
    if(strcmp((getName()), "node0") == 0 || strcmp((getName()), "node1") == 0)
    {
        MyFile.open("pair01.txt",std::ofstream::out | std::ofstream::app);
        remove("pair01.txt");
    }

    else if(strcmp((getName()), "node2") == 0 || strcmp((getName()), "node3") == 0)
    {
        MyFile.open("pair23.txt",std::ofstream::out | std::ofstream::app);
        remove("pair23.txt");
    }

    else if(strcmp((getName()), "node4") == 0 || strcmp((getName()), "node5") == 0)
    {
        MyFile.open("pair45.txt",std::ofstream::out | std::ofstream::app);
        remove("pair45.txt");
    }


//    std::string filePath ="E:/5th Semester CCE/Computer Networks/Project/Project Code/Data-Link-Layer-Protocols-Simulation/simulations/" + filename;

    //Greater than One means it contains more than the input file name
    if (MessageSplit.size() > 2) // sender
    {

        //get last digit in the string as it represent the start time
        int Start = std::stoi(MessageSplit[MessageSplit.size() - 2]);
        startTime = Start;
        MyMessage_Base *myMsg = new MyMessage_Base();
        myMsg->setM_Type(Self_Message);
        scheduleAt(simTime() + Start, (cMessage*)myMsg);
    }
}

void Node::ReadFromFile(std::string FileName )
{
    //"E:/5th Semester CCE/Computer Networks/Project/Project Code/Data-Link-Layer-Protocols-Simulation/inputs_samples/coordinator.txt";
    std::string location = "D:/Downloads/Compressed/omnetpp-5.7-windows-x86_64/omnetpp-5.7/samples/Data-Link-Layer-Protocols-Simulation/inputs_samples/" + FileName;
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
    MyFile.flush();
    MyFile.close();
    return;
}

void Node::initialize()
{
    // TODO - Generated method body

}

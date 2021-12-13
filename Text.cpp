#include <fstream>
#include <string>
#include <iostream>


std::string Frame(std::string Msg)
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

std::string Deframe(std::string Msg)
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

int main() 
{ 
 
    std::string Frame1 = Frame("He$whoA///AreYou$");
    std::cout<<Frame1<<std::endl;
    std::cout<<Deframe(Frame1)<<std::endl;
}
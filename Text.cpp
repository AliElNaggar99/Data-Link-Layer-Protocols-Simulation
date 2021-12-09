#include <fstream>
#include <string>
#include <iostream>

int main() 
{ 
 
    std::ifstream file ("coordinator.txt");
    std::string str; 

    std::cout<<"start"<<std::endl;;
    if(file.is_open())
    {
        std::cout<<"Opemd"<<std::endl;;
    }
    else
    {
        std::cout<<"not opened"<<std::endl;;
    }
    while (std::getline(file, str))
    {
        // Process str
        std::cout<<str<<std::endl;
    }
}
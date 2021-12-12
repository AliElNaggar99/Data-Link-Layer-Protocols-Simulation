#include<iostream>
#include<bitset>
#include<string>
#include<vector>
using namespace std; 

/////////////////////////////////////////////////////////////////////
// CRC FUNCTIONS

// function to be called at sender 
// return char (8 bits representation of the crc);
// to convert it to bitSet use bitset<8> crc( (int)char.to_ullong());
char computeCrcAtSender(string line){
    int crc = 0 ;
    int divisor = 341 ; // 101010101
	const int divisorSize = 9;

    bitset<divisorSize> divisorBitSet(divisor);
    
	int totalChars = line.size() ; // total no of characters

    int Charindex= 0 ; // current char in window
	bitset<8>currentChar((int)line[Charindex]); // current char bitset representation
	
	int firstShift = max(0 , 8-divisorSize );
	bitset<divisorSize> window((currentChar>>(firstShift)).to_ullong()); // current window
    int index = 8 - divisorSize- 1;
	bitset<8>nextChar = currentChar;
	
	if(index<=-1){
		index = 7;
		Charindex ++ ;
		nextChar = bitset<8>((int)line[Charindex]);
	}
    while (Charindex <=totalChars)
    {
        // if MSB = 1 -> do xor else we need to shift 
        if(window[divisorSize-1]==1){
            window ^= divisorBitSet ;
			//cout << window.to_string() << endl; 	
		}
        
        int shifts = 0 ;
		int tempIndex= index;
        while (window[divisorSize-1] !=1 && tempIndex>=0)
        {
            window <<=1 ;
            shifts++ ;
			tempIndex -- ;
        }

        for (size_t i = 0; i < shifts; i++)
        {
            window[shifts-i-1] =  nextChar[index--];
			
            if(index==-1){
                Charindex++ ;
                if(Charindex==totalChars){
					nextChar = 0 ;
					index = divisorSize -2 ;
					break;
				}
				if(Charindex>totalChars)
					break ;
                nextChar = bitset<8>((int)line[Charindex]);
                index = 7;
            }
        }
    }
	if(window[divisorSize-1]==1)
		window ^= divisorBitSet ;
	//cout << window.to_string() << endl;
    crc = (window).to_ullong();
    return crc;
}

// function to be called at reciever
// return int (remainder of division process )
// you should check its value
		// if 0 : message recieved correctly
		// else : error in message 
int computeCrcAtReciever(string line, char crc){
	int divisor = 341 ; // 101010101
	const int divisorSize = 9;
	
	int remainder = 0 ;
    
    bitset<divisorSize> divisorBitSet(divisor);

    
    int totalChars = line.size() ; // total no of characters
	int Charindex = 0 ;// current char in window
    bitset<8>currentChar((int)line[Charindex]); // current char bitset representation
	
	int firstShift = max(0 , 8-divisorSize );
	bitset<divisorSize> window((currentChar>>(firstShift)).to_ullong()); // current window
    
	int index = 8 - divisorSize- 1;
	bitset<8>nextChar = currentChar;
	if(index<=-1){
		index = 7;
		Charindex ++ ;
		nextChar = bitset<8>((int)line[Charindex]);
	}
    while (Charindex <=totalChars)
    {
        // if MSB = 1 -> do xor else we need to shift 
        if(window[divisorSize-1]==1){
            window ^= divisorBitSet ;
			//cout << window.to_string() << endl; 	
		}
        
        int shifts = 0 ;
		int tempIndex= index;
        while (window[divisorSize-1] !=1 && tempIndex>=0)
        {
            window <<=1 ;
            shifts++ ;
			tempIndex -- ;
        }

        for (size_t i = 0; i < shifts; i++)
        {
            window[shifts-i-1] =  nextChar[index--];
			
            if(index==-1){
                Charindex++ ;
                if(Charindex==totalChars){
					nextChar = bitset<8>((int)crc);
					index = divisorSize -2 ;
					break;
				}
				if(Charindex>totalChars)
					break ;
                nextChar = bitset<8>((int)line[Charindex]);
                index = 7;
            }
        }
    }
	if(window[divisorSize-1]==1)
		window ^= divisorBitSet ;
	//cout << window.to_string() << endl;
    remainder = (window ).to_ullong();
    return remainder;
}
/////////////////////////////////////////////////////////////////////


// testing CRC 
//int main(){
//
//	
//	char crcSender = computeCrcAtSender("ab");
//	cout << "crc at sender : " << crcSender << endl ;
//	cout <<"-------------" << endl;
//	int remainderRecieverCorrect = computeCrcAtReciever("ab" , crcSender ) ;
//	cout << "should be 0 : " << remainderRecieverCorrect << endl ;
//	cout <<"-------------" << endl;
//
//	int remainderRecieverWrong = computeCrcAtReciever("ac" , crcSender ) ;
//	cout << "should be not equal 0 : " << remainderRecieverWrong << endl ;
//	cout <<"-------------" << endl;
//	
//
//
//	system("pause");
//
//}



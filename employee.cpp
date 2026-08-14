#include <iostream>
#include "utilities.cpp"
using namespace std;

void empHomeScreen(){
    string userInput,passcode="1234",tempPasscode;
    if(stoi(userInput)==1){
            cin.ignore();
            cout<<"Enter the passcode: "<<endl;
            getline(cin,tempPasscode);
            if(!(tempPasscode==passcode))
                cout<<"Error: Wrong Passcode entered"<<endl;
            else login();
        }
}
#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include "employee.cpp"
#include "customer.cpp"
#pragma once
using namespace std;

//validation can reuse
int numberValidation(string userInput, int limit){
    for(char x:userInput){
        if(!isdigit(x)){
            cout<<"Error: Please enter a number from 1 to "<<limit<<endl;
            return 1;
        }
        
    }
    int num=stoi(userInput);

    if(num<1||num>limit){
        cout<<"Error: Please enter from 1 to "<<limit<<endl;
        return 1;
    }

    return 0;
}

bool login(int role){
    string username,fileUsername,password,filePassword;
    bool found=false;
    cout<<"Login"<<endl;

    while(true){
        ifstream inFile;
        cout<<"Enter your username [999 to go back]: "<<endl;
        getline(cin,username);

        if(username=="999"){
            return false;
        }
        cout<<"Password: ";
        getline(cin,password);
        
        if(role==1){
            inFile.open("customerData.txt");
            if(inFile.fail()){
                cout<<"Error, file does not exist."<<endl;
                return false;
            }
        }
        else{
            inFile.open("employeeData.txt");
            if(inFile.fail()){
                cout<<"Error, file does not exist."<<endl;
                return false;
            }
        }
        
        while(inFile>>fileUsername>>filePassword){
            if(fileUsername==username){
                found=true;
                break;
            }
                
        }

        inFile.close();
        
        if (!found){
            cout<<"Error: Username does not exist.\n"<<endl;
            return false;
        }
        
        if(password!=filePassword)
            cout<<"Error: Wrong password\n"<<endl;
        else
            return true;
    }
        


}

void roleSelection(){
    string userInput;
        cout<<"Welcome to Hotel Reservation System!"<<endl;
        cout<<"Are you...:"<<endl;
        cout<<"[1] An Employee?"<<endl;
        cout<<"[2] A Customer?"<<endl;
        do{
            cout<<"Enter your choice: ";
            cin>> userInput;
            
        }while(numberValidation(userInput,2)!=0);

        if(stoi(userInput)==1)
            empHomeScreen();
        else
            custHomeScreen();
            
}
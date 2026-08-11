#include <iostream>
#include <string>
#include <cctype>
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
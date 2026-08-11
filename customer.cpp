#include <iostream>
#include <string>
#include <cctype>
#include "utilities.cpp"
#include "tempBook.cpp"
using namespace std;


//login

void login(){
    string username,password,password2;
    bool wantBack=false, passwordMatched=false;
    getline(cin,password);
    cout<<"Login"<<endl;
    do{
        cout<<"Username [999 to go back]: "<<endl;
        getline(cin,username);
        if(username=="999")
            wantBack=true;
            
        while(passwordMatched==false){
            cout<<"Password: ";
            getline(cin,password);
            cout<<"Confirm Password: ";
            getline(cin,password2);

            if(password!=password2)
                continue;
            else {
                passwordMatched=true;
                bookingScreen();
            }
        }

    }while(wantBack==false);
}

void signup(){
    cout<<"sign up";
}
void homeScreen(){
    string userInput;
    cout<<"Welcome to Hotel Reservation System!"<<endl;
    cout<<"[1] Login"<<endl;
    cout<<"[2] Sign up"<<endl;
    
    do{
        cout<<"Enter your choice: ";
        cin>> userInput;
        
    }while(numberValidation(userInput,2)!=0);
    
    if(stoi(userInput)==1)
        login();
        else signup();
    
}

int main(){
    homeScreen();
    return 0;
}

//view profile

//edit profile

//booking history

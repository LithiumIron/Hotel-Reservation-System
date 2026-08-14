#include "booking.h"
#include "utilities.h"

#include <iostream>
#include <string>

using namespace std;


//login
void login(){
    string username,password,password2;
    bool wantBack=false, passwordMatched=false;
    getline(cin,password);
    cout<<"Login"<<endl;

    cout<<"Username [999 to go back]: "<<endl;
    getline(cin,username);
    if(username=="999"){
        return;
    }

    while(true){
        cout<<"Password: ";
        getline(cin,password);

        cout<<"Confirm Password: ";
        getline(cin,password2);

        if(password!=password2){
            cout<<"Error: Both passwords must match\n"<<endl;
            continue;
        }
        if(password.length()<8){
            cout<<"Error: Password must be more than 8 characters\n"<<endl;
            continue;
        }
        break;
    }
    bookingScreen();


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
    cout<<"hi test, im back";
    return 0;
}

//view profile

//edit profile

//booking history

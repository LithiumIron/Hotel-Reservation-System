#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include "utilities.cpp"
#include "tempBook.cpp"
using namespace std;

bool passwordValidation(string password,string password2){
    if(password!=password2){
            cout<<"Error: Password does not match\n"<<endl;
            return false;
        }
    if(password.length()<8){
            cout<<"Error: Password must be more than 8 characters\n"<<endl;
            return false;
        }
    return true;
}
//login
void login(){
    string username,password,password2;
    cout<<"Login"<<endl;

    cout<<"Enter your username [999 to go back]: "<<endl;
    getline(cin,username);

    if(username=="999"){
        return;
    }

    while(true){
        cout<<"Password: ";
        getline(cin,password);

        cout<<"Confirm Password: ";
        getline(cin,password2);

        if(passwordValidation(password,password2))
            break;
        
    }
    ofstream outFile("customerData.txt",ios::app);
    if(outFile.fail()){
        cout<<"Error opening the file.";
    }

    outFile<<username<<"\t"<<password<<endl;
    outFile.close();

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
    
    if(stoi(userInput)==1){
        cin.ignore();
        login();
    }
    else {
        cin.ignore();
        signup();
    }
    
}

int main(){
    homeScreen();
    cout<<"hi test, im back";
    return 0;
}

//view profile

//edit profile

//booking history

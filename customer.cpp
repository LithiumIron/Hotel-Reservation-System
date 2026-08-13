#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include "utilities.cpp"
#include "tempBook.cpp"
using namespace std;

//login
bool signup(){
    string username,password,password2;
    cout<<"Sign Up"<<endl;

    cout<<"Create your username [999 to go back]: "<<endl;
    getline(cin,username);

    if(username=="999"){
        return false;
    }

    while(true){
        cout<<"Password: ";
        getline(cin,password);

        cout<<"Confirm Password: ";
        getline(cin,password2);

        if(password!=password2){
            cout<<"Error: Passwords does not match\n"<<endl;
        }
        else if(password.length()<8){
            cout<<"Error: Password must be more than 8 characters\n"<<endl;
        }
        else break;
        
    }
    ofstream outFile("customerData.txt",ios::app);
    if(outFile.fail()){
        cout<<"Error opening the file.";
        return false;
    }

    outFile<<username<<"\t"<<password<<endl;
    outFile.close();
    return true;


}

bool login(){
    string username,fileUsername,password,filePassword;
    bool found=true;
    cout<<"Login"<<endl;

    while(true){
        cout<<"Enter your username [999 to go back]: "<<endl;
        getline(cin,username);

        if(username=="999"){
            return false;
        }
        cout<<"Password: ";
        getline(cin,password);
        
        ifstream inFile("customerData.txt");
        if(inFile.fail()){
            cout<<"Error, file does not exist."<<endl;
            return false;
        }
        
        while(inFile>>fileUsername>>filePassword){
            if(fileUsername==username){
                found=true;
                break;
            }
                
        }

        inFile.close();
        
        if (!found)
            cout<<"Error: Username does not exist."<<endl;
            return false;
        
        if(password!=filePassword)
            cout<<"Error: Wrong password";
        else
            return true;
    }
        


}
void homeScreen(){
    while(true){
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
            if(login()==true) break;
        }
        else {
            cin.ignore();
            if(signup()==true) break;
        }
    }
    bookingScreen();
    
}

int main(){
    homeScreen();
    cout<<"\nhi test, im back";
    return 0;
}

//view profile

//edit profile

//booking history

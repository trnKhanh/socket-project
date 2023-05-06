#include "Client/Client.h"
#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::cin;

int menu();

int main(){
    Client* c = new Client();
    while(true){
        int type = menu();
        bool is_exit = false;
        switch(type){
            case 1: c->listApp(); break; // done
            case 2:{
                string appName;
                cout << "Application name: ";
                cin.get();
                getline(cin, appName); 
                c->startApp(appName.c_str()); 
                break;
            }
            case 3: { // done
                string appName;
                cout << "Application name: ";
                cin.get();
                getline(cin, appName);
                c->stopApp(appName.c_str()); 
                break;
            }
            case 4: c->listProcesss(); break; // done
            case 5: c->screenShot(); break; // done
            case 6: c->startKeylog(); break;
            case 7: c->stopKeylog(); break;
            case 8: c->dirTree(); break; // done
            case 9: c->disconnect(); is_exit = true; break; // done
            default:
                cout << "Option isn't available.\n";
        }
        if(is_exit)
            break;
    }
    delete c;
}

int menu(){
    cout << "Choose instruction:\n";
    cout << "1. List App.\n";
    cout << "2. Start App.\n";
    cout << "3. Stop App.\n";
    cout << "4. List Process.\n";
    cout << "5. ScreenShot.\n";
    cout << "6. Start Keylog.\n";
    cout << "7. Stop Keylog.\n";
    cout << "8. Dir tree.\n";
    cout << "9. Exit.\n";
    int type;
    cin >> type;
    return type;
}


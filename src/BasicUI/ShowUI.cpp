#include "Menu.h"
#include <string>
#include <vector>
#include <iostream>
#include "../client/Client.h"
#include <thread>
#include <fstream>

int showMenu(std::vector<std::string> &menu)
{
    std::cout << "\n-----------------------------\n";
    for (int i = 0; i < menu.size(); ++i)
    {
        std::cout << i + 1 << ". " << menu[i] << std::endl;
    }

    int choice;
    while(1)
    {
        try{
            std::string buffer;
            std::cout << "Select (1" << '-' << menu.size() << "): ";
            std::getline(std::cin, buffer);
            choice = std::stoi(buffer);
        }
        catch(...)
        {
            choice = 0;
        }
        if (choice < 1 || choice > menu.size()) 
        {
            std::cerr << "Error: Invalid selection.\n";
        } else break;
    }

    return choice;
}
void writeKeylog()
{
    std::mutex m;
    std::lock_guard l(m);
    std::ofstream os("keylog.log");
    
}
int showUI(Client &client)
{
    std::vector<std::string> mainMenu = {
        "App",
        "List process",
        "Keylog",
        "Screenshot",
        "List directory tree",
        "Exit"
    };
    std::vector<std::string> appMenu = {
        "List app",
        "Start app",
        "Stop app",
        "Back"
    };
    std::vector<std::string> keylogMenu = {
        "Start keylog",
        "Stop keylog",
        "Back"
    };
    while (1)
    {
        int choice = showMenu(mainMenu);
        
        if (choice == 1)
        {
            int choice = showMenu(appMenu);
            
            if (choice == 1)
            {
                if (client.listApp())
                {
                    std::cerr << "List app: fail request\n";
                }
            }
            else if (choice <= 3)
            {
                std::string s;
                std::cout << "Enter app name: ";
                std::getline(std::cin, s);
                if (choice == 2)
                {
                    if (client.startApp(s.c_str()))
                    {
                        std::cerr << "Start app: fail request\n";
                    }
                }
                else 
                {
                    if (client.stopApp(s.c_str()))
                    {
                        std::cerr << "Stop app: fail request\n";
                    }
                }
            }
            else continue;
        }
        else if (choice == 2)
        {
            if (client.listProcesss())
            {
                std::cerr << "List processes: fail request\n";
            }
        }
        else if (choice == 3)
        {
            int choice = showMenu(keylogMenu);

            if (choice == 1)
            {
                if (client.startKeylog())
                {
                    std::cerr << "Start keylog: fail request\n";
                }
            } else if (choice == 2)
            {
                if (client.stopKeylog())
                {
                    std::cerr << "Stop keylog: fail request\n";
                }
            } else continue;
        } 
        else if (choice == 4)
        {
            if (client.screenshot())
            {
                std::cerr << "Screenshot: fail request\n";
            }
        }
        else if (choice == 5)
        {
            std::string s;
            std::cout << "Enter path name: ";
            std::getline(std::cin, s);
            if (client.dirTree(s.c_str()))
            {
                std::cerr << "List directory tree: fail request\n";
            }
        } else break;
    }
    return 0;
}
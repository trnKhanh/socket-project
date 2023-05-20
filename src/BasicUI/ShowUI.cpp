#include "ShowUI.h"
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <fstream>

void clrscr()
{
    std::cout << "\033[2J\033[1;1H";
}
void pause_until_press()
{
    std::cout << "\n(Press Enter to continue)\n";
    std::cin.get();
}
int showMenu(std::vector<std::string> &menu)
{
    clrscr();
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
    clrscr();
    return choice;
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
    std::string result;
    while (1)
    {
        int choice = showMenu(mainMenu);
        
        if (choice == 1)
        {
            int choice = showMenu(appMenu);
            
            if (choice == 1)
            {
                if (client.listApp(result))
                {
                    std::cerr << "List app: Fail.\n";
                } else
                {
                    std::cout << "Applications list:\n\n";
                    std::cout << result << "\n";
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
                        std::cerr << "Start app: Fail\n";
                    } else
                    {
                        std::cout << "Start app: Successful\n";
                    }
                }
                else 
                {
                    if (client.stopApp(s.c_str()))
                    {
                        std::cerr << "Stop app: Fail\n";
                    } else
                    {
                        std::cout << "Stop app: successful request\n";
                    }
                }
            }
            else continue;
        }
        else if (choice == 2)
        {
            if (client.listProcesss(result))
            {
                std::cerr << "List processes: Fail\n";
            } else
            {
                std::cout << "Processes list:\n\n";
                std::cout << result << "\n";
            }
        }
        else if (choice == 3)
        {
            int choice = showMenu(keylogMenu);

            if (choice == 1)
            {
                if (client.startKeylog())
                {
                    std::cerr << "Start keylog: Fail\n";
                } else
                {
                    std::cout << "Start keylog: Successful\n";
                }
            } else if (choice == 2)
            {
                if (client.stopKeylog())
                {
                    std::cerr << "Stop keylog: Fail\n";
                } else
                {
                    std::cout << "Stop keylog: Successful\n";
                }
            } else continue;
        } 
        else if (choice == 4)
        {
            if (client.screenshot())
            {
                std::cerr << "Screenshot: Fail\n";
            } else
            {
                std::cout << "Screenshot: Successful\n";
            }
        }
        else if (choice == 5)
        {
            std::string s;
            std::cout << "Enter path name: ";
            std::getline(std::cin, s);
            if (client.dirTree(s.c_str(), result))
            {
                std::cerr << "List directory tree: Fail\n";
            } else
            {
                std::cout << "Directory tree:\n\n";
                std::cout << result << "\n";
            }
        } else break;
        pause_until_press();
    }
    return 0;
}
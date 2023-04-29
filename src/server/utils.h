#pragma once
#include <iostream>
#include <netdb.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <poll.h>
#include <vector>
#include <netinet/in.h>
#include <string>

using namespace std;

void *get_in_address(sockaddr *addr);

void list_file(string path_name, string &res);

void send_all(int &sockfd, const string &buffer, int flag);

void recv_all(int &sockfd, string &res, int flag);
/*
client.cpp
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>
#include <vector>
using namespace std;
#define PORT 9575 //port no. is last 4 digits of student ID
#define IP_ADDR "0.0.0.0"
int main(int argc, char const *argv[])
{
struct sockaddr_in address;
int sock = 0, valread;
struct sockaddr_in serv_addr;
char buffer[1024] = {0};
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
{
cerr << "Socket could not be created!" <<endl;
return -1;
}
memset(&serv_addr, '0', sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(PORT);
if(inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr)<=0)
{
cerr << " The address entered is invalid! " <<endl;
return -1;
}
if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
{
cerr << " Could not establish connection! " <<endl;
return -1;
}
int i = 1;
while (i == 1){
std::string query;
char exit[] = "exit";
cout << "Please enter the command to execute - (cd <path>, list, pwd, quit, do nothing): " << endl;
getline (cin, query);
send(sock , query.c_str() , 1024, 0 );
cout << "query sent: "<< query <<endl;
valread = read( sock , buffer, 1024);
cout << "Response: " <<endl;
cout << buffer << endl;
if (query.compare(exit) == 0){
i++;
}
}
return 0;
}

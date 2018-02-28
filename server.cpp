#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <sstream>
// local libraries
using namespace std;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
//we need to add the id to it, as per the demo skeleton logic
#define PORT 9575
#define MY_ID 0
// supported operations
string CD_OP = "cd";
string PWD_OP = "pwd";
string LIST_OP = "ls";
string QUIT_OP = "quit";
string DO_NOTHING_OP = "do nothing";
//static error codes and custom messages
#define STATUS_OK "Status: OK"
#define SERVER_ERROR "Status: 500 SERVER ERROR"
#define BAD_REQUEST_ERROR "Status: What?"
#define DO_NOTHING_MESSAGE "I am doing nothing"
#define QUIT_CLIENT_MESSAGE "quitting the client"
#define WORKING_DIR "working Directory: "
#define UPDATED_DIR "updated Directory: "
#define GET_WORKING_DIR getcwd
#define CURRRENT_DIR "."
#define DNE "Directory does not exist"
//global variables
int tcpServerFd;
struct sockaddr_in clientAddress;
int buffer = 1024;
DIR *dir;
int receiveTCP();
//setup socket descriptors
fd_set readFds;
pthread_t connThread;
static int connFd;
void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET)
return &(((struct sockaddr_in*)sa)->sin_addr);
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void* executeCommand(void *sockFd){
string cmd;
int n;
int currsockfd =(long) sockFd;
char command[1025];
char buff[FILENAME_MAX];
string currentPath;
GET_WORKING_DIR( buff, FILENAME_MAX);
currentPath = buff;
string list;
while (1){
pthread_mutex_lock(& count_mutex);
n = recv(currsockfd, command, 1025,0);
if(n==0){
close(currsockfd);
}
cmd = string(command);
cout << "Received command: "<< cmd << endl;
//CD
if (cmd.at(0) == CD_OP.at(0)){
stringstream ss(cmd);
string temp;
vector<string> tokens;
while (ss >> temp){
tokens.push_back(temp);
}
currentPath += "/";
currentPath += tokens[1].c_str();
cout << currentPath << endl;
string message = UPDATED_DIR + currentPath;
send(currsockfd, message.c_str(), 1024,0);
cout << STATUS_OK <<endl;
}
//PWD
else if(cmd.at(0) == PWD_OP.at(0)){
string message = WORKING_DIR + currentPath;
send(currsockfd, message.c_str(), 1024,0);
cout << STATUS_OK <<endl;
}
//LIST
else if(cmd.at(0) == LIST_OP.at(0)){
vector <string> contents;
char *curr = (char *)alloca(currentPath.size() + 1);
memcpy(curr, currentPath.c_str(), currentPath.size() + 1);
dir = opendir(curr);
struct dirent *ep;
while((ep = readdir(dir)) != NULL)
{
contents. push_back(ep->d_name);
}
for (int i = 0; i < contents.size(); i++){
list += contents[i];
list += "\n";
}
send(currsockfd, list.c_str(), 1024,0);
(void) closedir (dir);
contents.clear();
list = "";
cout << STATUS_OK <<endl;
}
//QUIT
else if(cmd.at(0) == QUIT_OP.at(0)){
send(currsockfd, QUIT_CLIENT_MESSAGE, 1024,0);
cout << STATUS_OK <<endl;
close (currsockfd);
}
//DO_NOTHING
else if(cmd.at(0) == DO_NOTHING_OP.at(0)){
send(currsockfd, DO_NOTHING_MESSAGE, 1024,0);
cout << STATUS_OK <<endl;
}
//ERROR CASE
else{
send(currsockfd, BAD_REQUEST_ERROR, 1024,0);
cout << BAD_REQUEST_ERROR <<endl;
}
pthread_mutex_unlock(& count_mutex);
}
return 0;
}
int receiveTCP(){
string tempString;
while(1){
cout << "Ready to listen.." << endl;
socklen_t clientLen = sizeof(clientAddress);
//establishing connection
connFd = accept(tcpServerFd, (struct sockaddr *)&clientAddress, &clientLen);
if (connFd < 0)
{
cerr << "Cannot accept connection" << endl;
return 0;
}
cout << "Connection successful with client" << endl;
pthread_create(&connThread,NULL, &executeCommand, (void *) connFd);
}
if (connFd == 0){
close (connFd);
}
return 0;
}
void start_server(){
struct sockaddr_in serverAddress;
tcpServerFd = socket(AF_INET,SOCK_STREAM,0);
if ((tcpServerFd)== 0){
cerr << "socket creation failed!";
exit(EXIT_FAILURE);
}
int optval =1;
setsockopt(tcpServerFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
memset ((void *) &serverAddress, 0x00, sizeof (serverAddress));
serverAddress.sin_family=AF_INET;
serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
serverAddress.sin_port=htons(PORT);
if (bind (tcpServerFd, (struct sockaddr *) &serverAddress, sizeof (serverAddress)) < 0) {
cerr<< "binding failure: " << SERVER_ERROR <<endl;
exit (EXIT_FAILURE);
}
listen(tcpServerFd, 5);
string str = inet_ntoa(serverAddress.sin_addr);
cout << "Server started at"<< str << ":"<< PORT <<endl;
if (receiveTCP() == 0){
close(tcpServerFd);
}
}
int main(int argc, char const *argv[]) {
start_server();
return 0;
}

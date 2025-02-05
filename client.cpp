#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

#define MAX_MSG_BUFFERSIZE 1024

class Client {
    public:
        char msg[MAX_MSG_BUFFERSIZE]; 
        struct hostent* host; 
        sockaddr_in sendsockaddr;  
        int clientSd;
        char buffer[MAX_MSG_BUFFERSIZE];


        Client(int port, char * serverIpAddress) {
            host = gethostbyname(serverIpAddress);
            bzero((char*)&sendsockaddr, sizeof(sendsockaddr)); 
            sendsockaddr.sin_family = AF_INET; 
            sendsockaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
            sendsockaddr.sin_port = htons(port);
            clientSd = socket(AF_INET, SOCK_STREAM, 0); 
            int status = connect(clientSd, (sockaddr*) &sendsockaddr, sizeof(sendsockaddr));
            if (status < 0) {
                std::cerr << "Cannot connect to server!" << std::endl;
                exit(0);
            }
            while (1)
            {
                
                std::memset(buffer, 0, sizeof(buffer)); 
                ssize_t bytesReceived = recv(clientSd, buffer, sizeof(buffer) - 1, 0);
                buffer[bytesReceived] = '\0'; 
                std::cout << "Server: " << buffer << std::endl;
                if (strcmp(buffer, "END") == 0) {
                    break;
                }

                std::string data;
                std::getline(std::cin, data);
                std::memset(&msg, 0, sizeof(msg));
                strcpy(msg, data.c_str());
                send(clientSd, msg, strlen(msg), 0);


            }
            close(clientSd);


        }
};



int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: <IP_ADDRESS> <PORT>" << std::endl;
    }
    
    char * serverIpAddress = argv[1];
    int port = std::stoi(argv[2]);

    Client client = Client(port, serverIpAddress);
    return 0;

}
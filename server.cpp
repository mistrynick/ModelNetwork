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
#include <signal.h>
#include <fcntl.h>
#include <fstream>

#define MAX_PORT 65535
#define JUPYTER_PORT 8889
#define MAX_MSG_BUFFERSIZE 1024
#define MAX_CLIENTS 1


class Server {
    public:
        pid_t jupyter_pid;

        char msg[MAX_MSG_BUFFERSIZE];
        sockaddr_in socketAddress;
        int socketDescriptor;
        int client_fd;

        std::string get_server_ip(void) {
            char hostbuffer[256];
            struct hostent* host_entry;
            char* ip_buffer;
            if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
                perror("gethostname");
                return "Unknown";
            }

           
            if ((host_entry = gethostbyname(hostbuffer)) == NULL) {
                perror("gethostbyname");
                return "Unknown";
            }
            ip_buffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
            return std::string(ip_buffer);

        }

        void messageClient(const char* message) {
            size_t len = std::strlen(message);
            ssize_t byte_sent = send(client_fd, message, len, 0);
            if (byte_sent == -1) {
                std::cerr << "Error Sending Message" << std::endl;
                return;
            }
            std::cout << "sent: " << message << std::endl; 
        }

        

        Server(uint32_t port) {
            jupyter_pid = -1;
            bzero((char*)&socketAddress, sizeof(socketAddress));
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
            socketAddress.sin_port = htons(port);

            socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
            if (socketDescriptor < 0) {
                std::cerr << "Cannot establish socket!" << std::endl;
                exit(0);
            }
            int status = bind(socketDescriptor, (struct sockaddr*) &socketAddress, sizeof(socketAddress));
            if (status < 0) {
                std::cerr << "Error Binding Socket" << std::endl;
                exit(0);
            }
            std::cout << "Server Started" << std::endl;
            std::string ipaddress_string = get_server_ip();
            std::cout << ipaddress_string << std::endl;

            if (listen(socketDescriptor, MAX_CLIENTS) < 0) {
                perror("Listen failed");
                exit(0);
            }

            sockaddr_in newSockAddr;
            socklen_t newSockAddrSize = sizeof(newSockAddr);
            client_fd = accept(socketDescriptor, (sockaddr *)& newSockAddr, &newSockAddrSize);
            if (client_fd < 0) {
                std::cerr << "Cannot accept client" << std::endl;
                exit(1);
            }

            std::cout << "Client connected" << std::endl;
            messageClient("starting jupyter notebook");
            startJupyter();

            while (1)
            {
                memset(&msg, 0, sizeof(msg));
                ssize_t bytesRead = recv(client_fd, (char*)&msg, sizeof(msg), 0);
                //msg[bytesRead] = '\0';

                if (strcmp(msg, "quit") == 0) {
                    stop_jupyter_kernel();
                    break;
                }

                std::cout << "Executing: " << msg << std::endl;
                
                system(msg);

            }
            close(socketDescriptor);
            close(client_fd);

        }

        void startJupyter(void) {
            if (jupyter_pid > 0) {
                std::cout << "Jupyter is already running (PID: " << jupyter_pid << ")." << std::endl;
                return;
            }

            jupyter_pid = fork();
            if (jupyter_pid == 0) { 
                std::string command = "jupyter notebook --no-browser --ip=0.0.0.0 --port=" + std::to_string(JUPYTER_PORT);
                execl("/bin/sh", "sh", "-c", command.c_str(), (char*)NULL);
                perror("execl failed");
                exit(1);
            } else if (jupyter_pid > 0) {
                std::cout << "Started Jupyter Notebook (PID: " << jupyter_pid << ")" << std::endl;
            } else {
                std::cerr << "Failed to start Jupyter." << std::endl;
            }
        }

        void stop_jupyter_kernel() {
            if (jupyter_pid > 0) {
                std::cout << "Stopping Jupyter Notebook (PID: " << jupyter_pid << ")..." << std::endl;
                kill(jupyter_pid, SIGTERM);
                waitpid(jupyter_pid, nullptr, 0);
                jupyter_pid = -1;
                std::cout << "Jupyter Notebook stopped." << std::endl;
            } else {
                std::cout << "Jupyter is not running." << std::endl;
            }
        }

};


int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "USAGE: <PORT>" << std::endl;
        exit(0);
    }
    uint32_t port;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid Argument Exception" << std::endl;
        exit(0); 
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of Range: " << e.what(); 
    }
    if (port > 65535 || port == JUPYTER_PORT) {
        std::cerr << "Invalid Port!" << std::endl;
    }
    
    Server server = Server(port);
    return 0;


}



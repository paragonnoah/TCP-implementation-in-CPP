#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

struct tcpMessage {
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

class TCPServer {
private:
    static std::vector<int> clientSockets;
    static std::vector<std::string> clientAddresses;
    static pthread_mutex_t mutex;
static void* HandleClient(void* arg) {
    int clientSocket = *((int*)arg);
    tcpMessage msg;

    while (true) {
        ssize_t bytesReceived = recv(clientSocket, &msg, sizeof(msg), 0);

        if (bytesReceived <= 0) {
            // Remove client from the list
            pthread_mutex_lock(&mutex);
            clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());

            std::string info = std::to_string(clientSocket);
            clientAddresses.erase(std::remove_if(clientAddresses.begin(), clientAddresses.end(),
                                                  [info](const std::string& address) { return address.find(info) != std::string::npos; }),
                                  clientAddresses.end());

            pthread_mutex_unlock(&mutex);

            close(clientSocket);
            pthread_exit(NULL);
        }

        // Process the received message
        if (msg.nVersion != 102) {
            // Ignore messages with nVersion not equal to 102
            continue;
        }

        std::cout << "Received Msg Type: " << static_cast<int>(msg.nType) << "; Msg: " << msg.chMsg << std::endl;

        if (msg.nType == 77) {
            // Broadcast the message to all other clients
            pthread_mutex_lock(&mutex);
            for (int otherClientSocket : clientSockets) {
                if (otherClientSocket != clientSocket) {
                    send(otherClientSocket, &msg, sizeof(msg), 0);
                }
            }
            pthread_mutex_unlock(&mutex);
        } else if (msg.nType == 201) {
            // Reverse the message and send it back to the client
            std::reverse(msg.chMsg, msg.chMsg + msg.nMsgLen);
            send(clientSocket, &msg, sizeof(msg), 0);
        }
    }
}


    static void DisplayClients() {
        std::cout << "Number of Clients: " << clientAddresses.size() << "\n";
        for (const std::string& address : clientAddresses) {
            std::cout << address << "\n";
        }
    }

public:
    static void StartServer(int port) {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Bind error");
            exit(EXIT_FAILURE);
        }

        if (listen(serverSocket, 10) == -1) {
            perror("Listen error");
            exit(EXIT_FAILURE);
        }

        std::cout << "Server listening on port " << port << "...\n";

        while (true) {
            sockaddr_in clientAddr{};
            socklen_t clientAddrSize = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

            pthread_t thread;
            pthread_create(&thread, NULL, HandleClient, &clientSocket);

            pthread_mutex_lock(&mutex);
            clientSockets.push_back(clientSocket);
            clientAddresses.push_back("IP Address: " + std::string(inet_ntoa(clientAddr.sin_addr)) +
                                      " | Port: " + std::to_string(ntohs(clientAddr.sin_port)));
            pthread_mutex_unlock(&mutex);

            DisplayClients();
        }

        close(serverSocket);
    }
};

std::vector<int> TCPServer::clientSockets;
std::vector<std::string> TCPServer::clientAddresses;
pthread_mutex_t TCPServer::mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }

    int port = std::stoi(argv[1]);

    TCPServer::StartServer(port);

    return EXIT_SUCCESS;
}

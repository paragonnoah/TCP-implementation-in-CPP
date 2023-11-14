#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <limits>

struct tcpMessage {
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << std::endl;
        return 1;
    }

    const char* serverIP = argv[1];
    int serverPort = std::stoi(argv[2]);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Connect error");
        return 1;
    }

    // User interaction loop
    while (true) {
        // Display user prompt
        std::cout << "Please enter command: ";
        char command;
        std::cin >> command;

        // Process user command
        if (command == 'v') {
            // Example: Sending a version command
            tcpMessage msg;
            msg.nVersion = 102;
            msg.nType = 0;  // Placeholder type for version command
            std::cout << "Enter version number: ";

            // Check if the input is a valid integer
            while (!(std::cin >> msg.nMsgLen)) {
                std::cin.clear();  // Clear the error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Discard invalid input
                std::cout << "Invalid input. Please enter a valid version number: ";
            }

            // Clear the newline character from the input buffer
            std::cin.ignore();

            send(clientSocket, &msg, sizeof(msg), 0);
        } else if (command == 't') {
            // Example: Sending a message with type 201
            tcpMessage msg;
            msg.nVersion = 102;
            msg.nType = 201;
            std::cout << "Enter type number: ";
            
            // Check if the input is a valid integer
            while (!(std::cin >> msg.nMsgLen)) {
                std::cin.clear();  // Clear the error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Discard invalid input
                std::cout << "Invalid input. Please enter a valid type number: ";
            }

            std::cout << "Enter message: ";
            std::cin.ignore();  // Clear newline character from previous input
            std::cin.getline(msg.chMsg, sizeof(msg.chMsg));
            msg.nMsgLen = strlen(msg.chMsg);
            send(clientSocket, &msg, sizeof(msg), 0);
        } else if (command == 'q') {
            // Close the socket and terminate the program
            close(clientSocket);
            break;
        } else {
            // Display an error message for unrecognized commands
            std::cout << "Invalid command. Please enter 'v', 't', or 'q'." << std::endl;
        }
    }

    return 0;
}

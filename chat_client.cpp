#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <cstring>

#define MSGSIZ 512

int connect_chat(int _fd, struct sockaddr* sockAddr, socklen_t len){
    auto res = connect(_fd, sockAddr, len);
    if (res == -1) {
        perror("connect()");
        return -1;
    }
    return 0;
}

void recv_msg(int &sockfd) {
    char buffer[MSGSIZ];
    while (sockfd) {
        memset(buffer, 0, MSGSIZ);
        auto res = recv(sockfd, buffer, MSGSIZ, 0);
        if (res < 0) {
            perror("recv()");
            break;
        }
        if (!strncmp(buffer, "bye...", strlen("bye..."))) {
            std::cout << "Server disconnected." << std::endl;
            if (shutdown(sockfd, SHUT_RDWR)){
                perror("shutdown()");
                std::cerr << "failed to shutdown socket.\n";
                sockfd = 0;
            }
            break;
        }
        if (res > 0) {
            std::cout << buffer << std::endl;
        }
    }
}

void send_msg(int &sockfd) {
    char buffer[MSGSIZ];

    while (true) {
        std::cin.getline(buffer, MSGSIZ);
        if (sockfd == 0)
            return;
        auto res = send(sockfd, buffer, strlen(buffer), 0);
        if (res == -1) {
            perror("send()");
            std::cerr << "Failed to send.\n";
        }
        if (!strcmp(buffer, "bye...")) {
            if (shutdown(sockfd, SHUT_RDWR)){
                perror("shutdown()");
                std::cerr << "failed to shutdown socket.\n";
            }
            sockfd = 0;
            break;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage : netchat_client host port\n";
        return 0;
    }
    int temp = std::stoi(argv[2]);
    if (temp < 1 || temp > 65535) {
        std::cerr << "Invalid port number.\n";
        return 1;
    }
    in_port_t port = temp;
    uint8_t inAddr[16];
    auto af_type = AF_INET;
    int res = inet_pton(AF_INET, argv[1], inAddr);
    if (res == 0) {
        res = inet_pton(AF_INET6, argv[1], inAddr);
        if (res == 0){
            std::cerr << "Specified address is not valid.\n";
            std::cerr << "usage : netchat_client host port\n";
            return 1;
        } else {
            af_type = AF_INET6;
        }
    }

    auto sockfd = socket(af_type, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("socket()");
        return 1;
    }
    if (af_type == AF_INET) {
        struct sockaddr_in sockAddr{};
        sockAddr.sin_family = AF_INET;
        memcpy(&(sockAddr.sin_addr.s_addr), inAddr, sizeof(in_addr_t));
        sockAddr.sin_port = htons(port);

        if (connect_chat(sockfd, (struct sockaddr*) &sockAddr, sizeof sockAddr)) {
            std::cerr << "Failed to Connect.\n";
            return 1;
        }
    }
    else {
        struct sockaddr_in6 sockAddr{};
        sockAddr.sin6_family = AF_INET6;
        memcpy(&(sockAddr.sin6_addr), inAddr, 16);
        sockAddr.sin6_port = htons(port);

        if (connect_chat(sockfd, (struct sockaddr*) &sockAddr, sizeof sockAddr)) {
            std::cerr << "Failed to Connect.\n";
            return 1;
        }
    }

    std::cout << "Communication started.\n";

    std::thread send_thread {send_msg, std::ref(sockfd)};
    std::thread read_thread {recv_msg, std::ref(sockfd)};

    read_thread.join();
    send_thread.join();

    return 0;
}
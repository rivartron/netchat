#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <cstring>

#define MSGSIZ 512

int chat_listen(int &_fd, in_port_t port) {
    struct sockaddr_in6 sockAddr{};

    sockAddr.sin6_family = AF_INET6;
    sockAddr.sin6_addr = in6addr_any;
    sockAddr.sin6_port = htons(port);

    socklen_t sockAddrLen = sizeof sockAddr;

    int res = bind(_fd, (struct sockaddr*) &sockAddr, sockAddrLen);
    if (res == -1) {
        perror("bind()");
        return -1;
    }

    res = listen(_fd, 2);
    if (res == -1){
        perror("listen()");
        return -1;
    }
    std::cout << "Started listening on port : " << port << '\n';
    return 0;
}

int get_chat_socket(int _fd, struct sockaddr* sockAddr, socklen_t* sockAddrLen) {
    int res;
    while (true) {
        res = accept(_fd, sockAddr, sockAddrLen);
        if (res == -1) {
            perror("accept()");
            return -1;
        } else {
            break;
        }
    }
    if (sockAddr->sa_family == AF_INET6) {
        auto temp = (struct sockaddr_in6*) sockAddr;
        char addr[INET6_ADDRSTRLEN];
        std::cout << "Connected to : " << inet_ntop(AF_INET6, &(temp->sin6_addr), addr, INET6_ADDRSTRLEN) << '\n';
    }
    return res;
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
            std::cout << "Client disconnected." << std::endl;
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
    if (argc < 2){
        std::cerr << "usage : netchat_server port\n";
        return 0;
    }
    int temp = std::stoi(argv[1]);
    if (temp > 65535 || temp < 1){
        std::cerr << "Invalid port number.\n";
        return 1;
    }
    in_port_t t_port = temp;
    int v6only = 0;
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (setsockopt(sockfd, SOL_IPV6, IPV6_V6ONLY, &v6only, sizeof v6only) == -1) {
        perror("setsockopt()");
        std::cerr << "Only IPv6 is supported.\n";
    }
    if (chat_listen(sockfd, t_port) == -1){
        return -1;
    }

    struct sockaddr sockAddr{};
    socklen_t sockAddrLen;
    int chatSocket = get_chat_socket(sockfd, &sockAddr, &sockAddrLen);

    std::cout << "Communication started.\n";

    std::thread send_thread {send_msg, std::ref(chatSocket)};
    std::thread read_thread {recv_msg, std::ref(chatSocket)};

    read_thread.join();
    send_thread.join();

    return 0;
}
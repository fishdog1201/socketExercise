#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // create listen socket
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        std::cerr << "Create listen socket failed!\n";
        return -1;
    }

    // bind ip and port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(4444);
    saddr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0, can bind any local ip
    int ret = bind(lfd, (const sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        std::cerr << "Bind IP or port failed!\n";
        return -1;
    }

    // Set listen
    ret = listen(lfd, 128);
    if (ret == -1) {
        std::cerr << "Set listen to lfd failed!\n";
        return -1;
    }

    // Block and wait for clients to connect
    struct sockaddr_in caddr;
    uint32_t addrLen = sizeof(caddr);
    int cfd = accept(lfd, (sockaddr*)&caddr, &addrLen);
    if (cfd == -1) {
        std::cerr << "Accept failed!\n";
        return -1;
    }

    // Connect successfully, print some info about client
    char ip[32];
    inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip));
    std::cout << "Client connected! IP: " << ip << ", and port: " << ntohs(caddr.sin_port) << std::endl;

    // communication
    while (1) {
        char buff[1024];
        int len = recv(cfd, buff, sizeof(buff), 0);
        if (len == 0) {
            std::cerr << "Client has already closed connection!\n";
            break;
        } else if (len == -1) {
            std::cerr << "Recv message from client failed!\n";
            break;
        } else {
            std::cout << "Recv message from cilent successful!\n";
            std::cout << "Client says: " << buff << std::endl;
            send(cfd, buff, sizeof(buff), 0);
        }
    }

    close(lfd);
    close(cfd);

    return 0;
}
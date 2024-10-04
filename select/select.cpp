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

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(lfd, &readSet);
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    int maxFd = lfd;
    while (1) {
        fd_set tmp = readSet;   // Why use a tmp fd_set instead of original fd_set
        int ret = select(maxFd + 1, &tmp, NULL, NULL, &tv);
        // listen fd?
        if (FD_ISSET(lfd, &tmp)) {
            std::cout << "listen fd is detected\n";
            struct sockaddr_in caddr;
            uint32_t addrLen = sizeof(caddr);
            int cfd = accept(lfd, (sockaddr*)&caddr, &addrLen);  // Don't need to know client info, so set NULL
            FD_SET(cfd, &readSet);
            maxFd = maxFd > cfd ? maxFd: cfd;
            // Connect successfully, print some info about client
            char ip[32];
            inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip));
            std::cout << "Client connected! IP: " << ip << ", and port: " << ntohs(caddr.sin_port) << std::endl;
        }
        for (int i = 0; i < maxFd; i++) {
            if (i != lfd && FD_ISSET(i, &tmp)) {   // this fd is used for communication
                std::cout << "Communication fd is detected!\n";
                char buff[1024];
                int len = recv(i, buff, sizeof(buff), 0);
                if (len == 0) {
                    std::cerr << "Client has already closed connection!\n";
                    FD_CLR(i, &readSet);
                    close(i);
                    break;
                } else if (len == -1) {
                    std::cerr << "Recv message from client failed!\n";
                    exit(1);
                } else {
                    std::cout << "Recv message from cilent successful!\n";
                    std::cout << "Client says: " << buff << std::endl;
                    ret = send(i, buff, sizeof(buff), 0);
                    if (ret == -1) {
                        std::cerr << "Send msg to client failed!\n";
                        exit(1);
                    }
                }
            }
        }
    }

    close(lfd);

    return 0;
}
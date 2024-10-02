#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main()
{
    // create commucation socket
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        std::cerr << "Create communication socket failed!\n";
        return -1;
    }

    // bind ip and port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(4444);
    inet_pton(AF_INET, "192.168.164.23", &saddr.sin_addr.s_addr);
    int ret = connect(cfd, (const sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        std::cerr << "Connect to server: " << saddr.sin_addr.s_addr << ":" << saddr.sin_port <<" failed!\n";
        return -1;
    }

    // communication
    uint32_t cnt = 0;
    while (1) {
        char buff[1024];
        sprintf(buff, "Hello, I set you some message...%d", cnt++);
        send(cfd, buff, strlen(buff) + 1, 0);

        memset(buff, 0, sizeof(buff));
        int len = recv(cfd, buff, sizeof(buff), 0);
        if (len == 0) {
            std::cerr << "Server has already closed connection!\n";
            break;
        } else if (len == -1) {
            std::cerr << "Recv message from server failed!\n";
            break;
        } else {
            std::cout << "Recv message from server successful!\n";
            std::cout << "Server says: " << buff << std::endl;
        }
        sleep(3);
    }

    close(cfd);

    return 0;
}
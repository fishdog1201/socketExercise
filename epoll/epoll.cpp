#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

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

    // Create an epoll instance
    int epfd = epoll_create(1);
    if (epfd == -1) {
        std::cerr << "epoll create failed\n";
        exit(0);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    struct epoll_event evs[1024];
    int sz = sizeof(evs)/sizeof(evs[0]);
    while (1) {
        int num = epoll_wait(epfd, evs, sz, -1);
        std::cout << "Ready fd " << num << std::endl;
        for (int i = 0; i < num; i++) {
            int fd = evs[i].data.fd;
            if (fd == lfd) {
                int cfd = accept(lfd, NULL, NULL);
                ev.events = EPOLLIN;
                ev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev); // ev will be copied, no need to create a new epoll_event instance
            } else {
                char buff[1024];
                int len = recv(fd, buff, sizeof(buff), 0);
                if (len == 0) {
                    std::cerr << "Client has already closed connection!\n";
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    break;
                } else if (len == -1) {
                    std::cerr << "Recv message from client failed!\n";
                    break;
                } else {
                    std::cout << "Recv message from cilent successful!\n";
                    std::cout << "Client says: " << buff << std::endl;
                    send(fd, buff, sizeof(buff), 0);
                }
            }
        }
    }

    close(lfd);

    return 0;
}
#include "function.h"

// 为服务端建议一个监听 socket
int tcpInit(int *fd) {
    int ret;
    // 创建监听套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(sockfd, -1, "socket");
    int on = 1;
    // 端口复用
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    ERROR_CHECK(ret, -1, "setsockopt");
    struct sockaddr_in sockinfo;
    bzero(&sockinfo, sizeof(sockinfo));
    sockinfo.sin_addr.s_addr = inet_addr(IP);
    sockinfo.sin_port = htons(PORT);
    sockinfo.sin_family = AF_INET;
    ret = bind(sockfd, (sockaddr *)&sockinfo, sizeof(sockinfo));
    ERROR_CHECK(ret, -1, "bind");
    ret = listen(sockfd, 10);
    ERROR_CHECK(ret, -1, "listen");
    *fd = sockfd;
    return 0;
}
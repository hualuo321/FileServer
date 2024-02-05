#include "function.h"

// 从 fd 中获取数据, 存放在缓冲区中, 直到获取 len 个字节
int recvCycle(int fd, void *p, size_t len) {
    char *pStart = (char *)p;
    size_t size = 0;
    while (size < len) {
        int ret = recv(fd, pStart + size, len - size, 0);
        if (0 == ret) {
            return 0;
        }
        ERROR_CHECK(ret, -1, "recv");
        size += ret;
    }
    return len;
}

// 从缓冲区中获取数据, 发送到 fd 中, 直到发送 len 个字节
int sendCycle(int fd, void *p, size_t len) {
    char *pStart = (char *)p;
    size_t size = 0;
    while (size < len) {
        int ret = send(fd, pStart + size, len - size, 0);
        if (-1 == ret ) return -1;
        size += ret;
    }
    return len;
}

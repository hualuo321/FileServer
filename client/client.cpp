#include "function.h"
#include "md5.h"

// 客户端
int main() {
    int ret;                    // 函数返回值
    int dataLen = 0;            // 客户端报文数据长度
    char buf[1000] = {0};       // 客户端缓冲区大小
    Packet packet;              // 数据报文
    bool flag;                  // 响应信息
    int Dir = 0;                // 目录等级
    string username;            // 用户名
    string password;            // 密码

    /*****************************连接服务器socket**************************/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);   // 创建客户端 socket
    ERROR_CHECK(sockfd, -1, "socket");
    struct sockaddr_in sockinfo;
    bzero(&sockinfo, sizeof(sockinfo));
    sockinfo.sin_addr.s_addr = inet_addr(IP);       // 客户端地址信息
    sockinfo.sin_port = htons(PORT);
    sockinfo.sin_family = AF_INET;
    // 携带客户端地址信息, 请求与服务端连接
    ret = connect(sockfd, (sockaddr *)&sockinfo, sizeof(sockinfo));
    ERROR_CHECK(ret, -1, "connect");
    cout << "connect succuss" << endl;

    /******************连接服务器成功，进入界面：选择 注册 OR 登陆**********************/
console: // (goto 标签)
    cout << "选择登陆或者注册:" << endl
         << "1.登陆\t\t\t"
         << "2.注册\t\t\t"
         << "3.退出"
         << endl;
    string order;                                   // 输入的指令 (gets demo.txt 300)
    int choice;                                     // 选项对应的数字
    do {
        cin >> choice;
        switch (choice) {
            case 1: {                                   // 登陆操作
                cout << "请输入用户名:" << endl;
                cin >> username;
                cout << "请输入密码:" << endl;
                cin >> password;
                order = "logIn " + username;            // 指令内容
                packet.dataLen = strlen(order.c_str());
                // 将指令信息写入报文中, 发送给服务端
                memcpy(packet.buf, order.c_str(), packet.dataLen);
                sendCycle(sockfd, &packet, 4 + packet.dataLen);
                // 获取服务端的响应, flag 为 true 代表用户存在
                recvCycle(sockfd, &flag, 1);
                if (flag == false) {
                    cout << "用户不存在!" << endl;
                    goto console;
                }
                // 从服务端获取用户对应的盐值
                bzero(buf, sizeof(buf));
                recvCycle(sockfd, &dataLen, 4);
                recvCycle(sockfd, buf, dataLen);
                // 根据专属盐值对用户密码进行加密, 将密文发送给服务端进行验证
                char *cipher = crypt(password.c_str(), buf);
                packet.dataLen = strlen(cipher);
                memcpy(packet.buf, cipher, packet.dataLen);
                sendCycle(sockfd, &packet, 4 + packet.dataLen);
                // 获取服务端的响应, flag 为 true 代表验证成功, 进入客户端系统
                recvCycle(sockfd, &flag, 1);
                if (flag == true) {
                    cout << "验证成功！" << endl;
                    goto client_system;
                } else {
                    cout << "验证失败！账号或密码错误！" << endl;
                    goto console;
                }
                break;
            }
            case 2: {                                               // 注册操作
                cout << "请输入用户名:" << endl;
                cin >> username;
                cout << "请输入密码:" << endl;
                cin >> password;
                order = "signIn " + username + " " + password;
                // 将指令信息放入报文中发送给服务端
                packet.dataLen = strlen(order.c_str());
                memcpy(packet.buf, order.c_str(), packet.dataLen);
                sendCycle(sockfd, &packet, 4 + packet.dataLen);
                // 获取服务端的响应, flag 为 true 代表注册成功
                recvCycle(sockfd, &flag, 1);
                if (flag == true) {
                    cout << "注册成功！" << endl;
                    goto console;
                } else {
                    cout << "注册失败！用户名已经存在！" << endl;
                    goto console;
                }
                break;
            }
            case 3: {                                           // 退出操作
                order = "quit ";
                // 将指令信息放入报文中发送给服务端, 直接退出
                packet.dataLen = strlen(order.c_str());
                memcpy(packet.buf, order.c_str(), packet.dataLen);
                sendCycle(sockfd, &packet, 4 + packet.dataLen);
                exit(0);
            }
            default: {
                cout << "输入错误！请重新输入！" << endl;
                break;
            }
        }
    } while (1);

    /******************************进入客户端系统******************************/
client_system:
    while (1) {
        // 读取命令
        cout << username << "@client:";     // 将字符串输出到终端 hualuo@client:
        fflush(stdout);                     // 刷新输出流
        // 从标准输入中读取数据, 放入到报文中 (阻塞等待键盘输入)
        bzero(&packet, sizeof(packet));
        packet.dataLen = read(STDIN_FILENO, packet.buf, sizeof(packet.buf));
        ERROR_CHECK(packet.dataLen, -1, "read");

        // 分析键盘写入的命令
        string orders(packet.buf);
        stringstream ss(orders);
        // order代表命令，name代表跟在name后面的文件名，order2代表补充命令（可为空）
        string order, name, order2; 
        ss >> order >> name >> order2;

        /*******************************ls*****************************/
        if (order == "ls") {                            // 如果是显示当前目录内容
            // 向服务器发送命令
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送用户姓名
            packet.dataLen = strlen(username.c_str());
            memcpy(packet.buf, username.c_str(), packet.dataLen);
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送目录号
            sendCycle(sockfd, &Dir, 4);
            // 读取服务器返回的信息
            bzero(buf, sizeof(buf));
            recvCycle(sockfd, &dataLen, 4);
            recvCycle(sockfd, buf, dataLen);
            cout << "-----------所有文件------------" << endl;
            cout << buf;
            cout << "-----------所有文件------------" << endl;
        }
        /*******************************mkdir*****************************/
        else if (order == "mkdir") {
            // 向服务器发送命令
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送用户姓名
            packet.dataLen = strlen(username.c_str());
            memcpy(packet.buf, username.c_str(), packet.dataLen);
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送目录号
            sendCycle(sockfd, &Dir, 4);
            // 读取服务器返回的信息
            recvCycle(sockfd, &flag, 1);
            if (flag) {
                cout << "创建目录成功!" << endl;
            } else {
                cout << "创建失败!有同名目录!" << endl;
            }
        }
        /*******************************rmdir*****************************/
        else if (order == "rmdir") {
            // 向服务器发送命令
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送用户姓名
            packet.dataLen = strlen(username.c_str());
            memcpy(packet.buf, username.c_str(), packet.dataLen);
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送目录号
            sendCycle(sockfd, &Dir, 4);
            // 读取服务器返回的信息
            recvCycle(sockfd, &flag, 1);
            if (flag) {
                cout << "删除成功!" << endl;
            } else {
                cout << "删除失败!没有该文件!" << endl;
            }
        }
        /*******************************cd*****************************/
        else if ("cd" == order) {
            // 先自行分析命令
            if (Dir == 0 && ".." == name)
            {
                cout << "已经到达根目录" << endl;
            } else if ("." == name)
            {
                // cd 当前目录, 什么也不做
            } else {
                // 切换到其他目录, 向服务器发送命令
                sendCycle(sockfd, &packet, 4 + packet.dataLen);
                // 发送用户姓名
                packet.dataLen = strlen(username.c_str());
                memcpy(packet.buf, username.c_str(), packet.dataLen);
                sendCycle(sockfd, &packet, 4 + packet.dataLen);
                // 发送目录号
                sendCycle(sockfd, &Dir, 4);
                // 读取服务器返回的信息
                recvCycle(sockfd, &flag, 1);
                if (flag) {
                    recvCycle(sockfd, &Dir, 4);
                } else {
                    cout << "没有" << name << "文件夹!" << endl;
                }
            }
        }
        /*******************************rm*****************************/
        else if ("rm" == order) {
            // 向服务器发送命令
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送用户姓名
            packet.dataLen = strlen(username.c_str());
            memcpy(packet.buf, username.c_str(), packet.dataLen);
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            // 发送目录号
            sendCycle(sockfd, &Dir, 4);
            // 读取服务器返回的信息
            recvCycle(sockfd, &flag, 1);
            if (flag) {
                cout << "删除成功!" << endl;
            } else {
                cout << "删除失败!没有该文件!" << endl;
            }
        }
        /*******************************puts*****************************/
        else if (order == "puts") {
            // 不向服务器发送命令，而是启动多线程，帮助上传
            LoadTask task;
            task.orders = orders;
            task.username = username;
            task.Dir = Dir;
            pthread_t pth1;
            pthread_create(&pth1, NULL, upLoad, &task);
            pthread_join(pth1, NULL);
        }
        /*******************************gets*****************************/
        else if (order == "gets") {
            // 断点续传功能
            struct stat statbuf;
            // 如果文件不存在, stat 会返回 -1, 从头开始下载; 如果文件存在, 则从断开出继续下载
            ret = stat(name.c_str(), &statbuf);
            if (-1 == ret) {
                orders = order + " " + name + " 0";
            } else {
                orders = order + " " + name + " " + to_string(statbuf.st_size);
            }
            // 不向服务器发送命令，而是启动多线程，帮助下载
            LoadTask task;
            task.orders = orders;
            task.username = username;
            task.Dir = Dir;
            pthread_t pth2;
            pthread_create(&pth2, NULL, downLoad, &task);
            pthread_join(pth2, NULL);
        }
        /*******************************quit*****************************/
        else if (order == "quit") {
            // 向服务器发送命令
            sendCycle(sockfd, &packet, 4 + packet.dataLen);
            goto end;
        } else {
            cout << "Wrong Command!" << endl;
        }
    }
end:
    close(sockfd);
    return 0;
}

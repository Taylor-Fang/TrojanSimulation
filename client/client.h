#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
#include <iomanip> //IO格式设置头文件
#include <sys/socket.h>
#include <cstdlib>
#include <sys/types.h>
#include <netinet/in.h> //定义数据结构sockaddr_in
#include <arpa/inet.h> //IP地址转换函数
#include <netdb.h> //设置及获取域名的函数
#include <unistd.h>
#include <cstring>
#include <time.h>
#include <stdio.h>
#include <sys/select.h>

//命令ID
#define CMD_NULL 	  100 //关闭连接
#define CMD_HOST_INFO 101 //获得主机信息
#define CMD_IP_MAC	  102 //获得局域网内其他主机IP和MAC地址
#define CMD_SCAN_TCP_PORT 103 //扫描TCP端口
#define CMD_SCAN_UDP_PORT 104 //扫描UDP漏洞
#define CMD_FILE_RECV 105 //控制端接收文件
#define CMD_FILE_SEND 106 //控制端发送文件
#define CMD_SCAN_LEAK 107 //扫描网络漏洞
int handle_connect(int sockfd);
void handle_request(int s_c,struct sockaddr *from);

#endif

#ifndef SCANIPANDMAC_H_
#define SCANIPANDMAC_H_

#include <sys/time.h>
#include <sys/select.h>

#define sendmaxsize 50
#define recvmaxsize 256
#define NBNS_PORT 137 //NetBios-ns端口
/*网络基本输入/输出系统NetBIOS名称服务器NBNS协议是TCP/IP协议上
的NetBIOS协议族的一部芬，它基于NetBIOS名称访问的网络上提供主机
名和地址映射方法*/

struct Q_NETBIOSNS //Netbios-ns询问包的结构***参考DNS报文结构
{
	unsigned short ID;
	unsigned short flags;
	unsigned short questions; //=1,表示询问
	unsigned short ans; //=1,表示回答
	unsigned short authRRs;
	unsigned short addRRs;
	unsigned char name[34]; //查询名
	unsigned short type;	//查询类型
	unsigned short clas;	//	查询类
};

int ScanMac(int sock,struct sockaddr *dest);

#endif

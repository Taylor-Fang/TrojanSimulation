#ifndef SYNSCAN_H_
#define SYNSCAN_H_

#define bool int
#define true 1

/*IP头部*/
typedef struct _iphdr
{
	unsigned char h_verlen; //4位首部长度+4位IP版本
	unsigned char ip_tos;  	//8位服务类型TOS
	unsigned short ip_len;  //总长度
	unsigned short ip_id;   //标识值
	unsigned short frap_and_flags;//3位标志位和片偏移
	unsigned char ip_ttl;	//TTL,生存时间
	unsigned char ip_proto; //协议类型
	unsigned short ip_sum;	//IP首部校验和
	unsigned int ip_src,ip_dst; //源地址和目的地址
}IP_HEADER;

/*TCP伪首部*/
typedef struct psd_hdr
{
	unsigned long saddr; //源地址
	unsigned long daddr; //目的地址
	char mbz;
	char ptcl; //协议类型
	unsigned short tcpl; //TCP长度
}PSD_HEADER;

/*TCP首部*/
typedef struct _tcphr
{
	unsigned short tcp_sport; //源端口号
	unsigned short tcp_dport; //目的端口号
	unsigned int tcp_seq;     //序列号
	unsigned int tcp_ack;     //确认号
	unsigned char tcp_len;
	unsigned char tcp_flag;
	unsigned short tcp_win;   //滑动窗口大小
	unsigned short tcp_sum;	  //校验和
	unsigned short tcp_urp;   //紧急字段指针
}TCP_HEADER;


unsigned short checksum(unsigned short *buff, int size);
int SYN_Scan(int sock,struct sockaddr *dest);
IP_HEADER fill_ip_header(struct sockaddr_in src,struct sockaddr_in dest);
TCP_HEADER fill_tcp_header(unsigned int i,unsigned short port,char flag);
PSD_HEADER fill_psd_header(IP_HEADER iph,TCP_HEADER tcph);

#endif


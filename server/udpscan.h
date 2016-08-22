#ifndef UDPSCAN_H_
#define UDPSCAN_H_

#include <netinet/ip_icmp.h>
#include <netinet/in_systm.h>

void udpscan(char *destip,unsigned int sport,unsigned eport);

#endif

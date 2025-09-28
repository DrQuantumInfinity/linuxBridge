#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include "Log.h"

// Define the Packet Constants
#define PING_PKT_S 64       // ping packet size
#define RECV_TIMEOUT 1      // timeout for receiving packets (in seconds)


// Ping packet structure
struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};


// Calculate the checksum (RFC 1071)
uint16_t checksum(void* b, int len)
{
    uint16_t* buf = (uint16_t*)b;
    uint32_t sum = 0;
    uint16_t result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(uint8_t*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = (uint16_t)~sum;
    return result;
}

// Make a ping request
bool send_ping(const char* addr)
{
    int ttl_val = 64, i;
    unsigned int addr_len;
    char rbuffer[128];
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    int ping_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    struct sockaddr_in sock;

    unsigned int  addrlen = sizeof(sock);
    memset(&sock, 0, addrlen);
    sock.sin_family = AF_INET;
    sock.sin_port = htons(0);
    if (inet_pton(AF_INET, addr, &sock.sin_addr) != 1)
    {
        log_error("Error parsing ip from addr! addr=%s\n", addr);
        return false;
    }


    // Set socket options at IP to TTL and value to 64
    if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    {
        log_error("Setting socket options to TTL failed!\n");
        return false;
    }

    // Setting timeout of receive setting
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

    // Send ICMP packet

    // Fill the packet
    bzero(&pckt, sizeof(pckt));
    pckt.hdr.type = ICMP_ECHO;
    pckt.hdr.un.echo.id = (uint16_t)getpid();

    for (i = 0; i < (int)sizeof(pckt.msg) - 1; i++)
        pckt.msg[i] = (char)i + '0';

    pckt.msg[i] = 0;
    pckt.hdr.un.echo.sequence = 1;
    pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

    // Send packet
    if (sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&sock, sizeof(sock)) <= 0)
    {
        log_error("Packet Sending Failed!\n");
        return false;
    }

    // Receive packet
    addr_len = sizeof(r_addr);
    if (recvfrom(ping_sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0)
    {
        log_error("Packet receive failed! addr=%s\n", addr);
        return false;
    }
    else
    {
        struct iphdr* ip_hdr = (struct iphdr*)rbuffer;
        struct icmphdr* icmp_hdr = (struct icmphdr*)(rbuffer + (ip_hdr->ihl * 4));
        if (!(icmp_hdr->type == 0 && icmp_hdr->code == 0))
        {
            log_error("Error... Packet received with ICMP type %d code %d\n", icmp_hdr->type, icmp_hdr->code);
            return false;
        }
        else
        {
            log_error("Packet receive success!\n");
            return true;
        }
    }
}
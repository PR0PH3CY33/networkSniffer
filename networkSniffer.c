#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define BUFSIZE 65536

int main(int argc, char *argv[])
{
    int sockfd;
    struct ifreq ifr;
    char *dev;
    unsigned char *buf;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    dev = argv[1];
    sockfd = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);
    if (sockfd < 0) {
        perror("socket()");
        return 2;
    }

    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl()");
        close(sockfd);
        return 3;
    }

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
        fprintf(stderr, "malloc() failed\n");
        close(sockfd);
        return 4;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, dev, strlen(dev)) < 0) {
        perror("setsockopt()");
        free(buf);
        close(sockfd);
        return 5;
    }

    while (1) {
        int size;
        struct iphdr *ip;
        struct sockaddr_in sa;
        socklen_t sa_len = sizeof(sa);

        size = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&sa, &sa_len);
        if (size < 0) {
            perror("recvfrom()");
            continue;
        }

        ip = (struct iphdr*)buf;
        printf("Destination IP: %s\n", inet_ntoa(sa.sin_addr));
    }

    free(buf);
    close(sockfd);
    return 0;
}

#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define URL "time.google.com"
#define PORT "123"

// Linux starts its epoch time from 1970
// but NTP starts from 1900 so we need to take it into consideration
#define JAN_1970 2208988800ULL

typedef struct {
    uint16_t seconds;
    uint16_t fraction;
} NTP_ShortFormat;

typedef struct {
    uint32_t seconds;
    uint32_t fraction;
} NTP_TimestampFormat;

typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    int8_t precision;

    NTP_ShortFormat root_delay;
    NTP_ShortFormat root_dispersion;
    uint32_t reference_id;

    NTP_TimestampFormat reference_timestamp;
    NTP_TimestampFormat origin_timestamp;
    NTP_TimestampFormat receive_timestamp;
    NTP_TimestampFormat transmit_timestamp;
} NTP_PacketHeader;

void print_usage(void) {}

int main(void) {
    struct addrinfo hints = {0};
    struct addrinfo *res;

    // We only need UDP connections
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(URL, PORT, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    NTP_PacketHeader header = {
        .li_vn_mode = 0x23,
    };

    ssize_t result = sendto(
        sockfd, &header, sizeof header, 0, res->ai_addr,
        sizeof(struct sockaddr_storage)
    );
    if (result < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    NTP_PacketHeader rs = {0};
    socklen_t their = sizeof(struct sockaddr_storage);

    result = recvfrom(sockfd, &rs, sizeof rs, 0, res->ai_addr, &their);
    if (result < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    uint32_t seconds = ntohl(rs.transmit_timestamp.seconds) - JAN_1970;

    printf("current time is %s", ctime((const time_t *)&seconds));
    close(sockfd);
}

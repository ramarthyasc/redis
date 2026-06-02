#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const size_t HEADER = 4;
const size_t K_MAX_MESSAGE = 4096;

int32_t write_all(int fd, char *wbuf, size_t n) {
    while(n > 0) {
        int rv = write(fd, wbuf, n);
        if (rv < 1) {
            return -1;
        }

        n -= rv;
        wbuf += rv;
    }
    return 0;

}

int32_t read_all(int fd, char *rbuf, size_t n) {
    while(n > 0) {
        int rv = read(fd, rbuf, n);
        if (rv < 1) {
            return -1;
        }

        n -= rv;
        rbuf += rv;
    }
    return 0;
}

int msg(char *msg) {
    perror(msg);
    return -1;
}

int32_t fetch(int fd, char *wbuf, size_t n, char *rbuf) {
    // write
    errno = 0;
    int32_t err = write_all(fd, wbuf, n);
    if (err) {
      return msg(errno == 0 ? "EOF": "write_all");
    }

    // read header
    errno = 0;
    err = read_all(fd, rbuf, HEADER);
    if (err) {
        return msg(errno == 0 ? "EOF" : "read_all header");
    }
    
    // read message using the header encoded length
    errno = 0;
    err = read_all(fd, rbuf, K_MAX_MESSAGE);
    if (err) {
        return msg(errno == 0 ? "EOF" : "read_all() message");
    }

    return 0;
}

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    // create socket and connect to the server
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket");
    }

    struct sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(1234);
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(fd, (const struct sockaddr *)&server_address, sizeof(server_address));
    if (rv) {
        die("connect()");
    }

    
    char rbuf[HEADER + K_MAX_MESSAGE]; 

    char request[] = "helloo guys";
    uint32_t len = (uint32_t)strlen(request);
    char wbuf[HEADER + len];
    memcpy(wbuf, &len, HEADER);
    memcpy(&wbuf[HEADER], request, len);

    // send request (write)
    int err = fetch(fd, wbuf, len, rbuf); // we get the rbuf as the side effect - which is the outcome. Good design pattern
    if (err) {
        return -1;
    }

    printf("server responds with this %.*s\n", len, &rbuf[HEADER]);

}

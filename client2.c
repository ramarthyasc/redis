#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


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
        printf("hiTwo %zu \n", n);
        printf("hiOne %d\n", rv);
    }
    return 0;
}

int msg(char *msg) {
    perror(msg);
    return -1;
}

int32_t fetch(int fd, char *req) {
    const size_t HEADER = 4;
    const size_t K_MAX_MESSAGE = 4096;

    char rbuf[HEADER + K_MAX_MESSAGE]; 

    uint32_t len = (uint32_t)strlen(req);
    if (len > K_MAX_MESSAGE) {
        return -1;
    }


    char wbuf[HEADER + len]; // like arraybuffer
    memcpy(wbuf, &len, HEADER); // like view into a buffer
    memcpy(&wbuf[HEADER], req, len);
    // write
    errno = 0;
    int32_t err = write_all(fd, wbuf, HEADER + len);
    if (err) {
      return msg(errno == 0 ? "EOF": "write_all");
    }

    // read header
    errno = 0;
    printf("4\n");
    err = read_all(fd, rbuf, HEADER);
    printf("5\n");
    if (err) {
        return msg(errno == 0 ? "EOF" : "read_all header");
    }
    printf("6\n");
    
    memcpy(&len, rbuf, HEADER); 

    // read message using the header encoded length
    errno = 0;
    err = read_all(fd, &rbuf[HEADER], len);
    printf("7\n");
    if (err) {
        return msg(errno == 0 ? "EOF" : "read_all() message");
    }

    printf("server responds with this %.*s\n", len, &rbuf[HEADER]); // prints the required chars 
                                                                    // ( used when no \0 is present)

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


    // send request (write)
    int32_t err = fetch(fd, "Hello 1!"); // we get the rbuf as the side effect - which is the outcome. Good design pattern
    printf("1\n");
    if (err) {
        goto L_DONE;
    }

    printf("2\n");
    err = fetch(fd, "Hello 2");
    if (err) {
        goto L_DONE;
    }

L_DONE: 
    close(fd);
    return 0;

}

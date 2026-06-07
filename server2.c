#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int32_t msg(char *msg) {
    perror(msg);
    return -1;
}
void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
int read_full(int fd, char *buf, size_t n) {
    int rv = 0;
    // n is the no. of bytes that we need 
    while (n > 0) {
       rv = read(fd, buf, n); 

       errno = 0;
       if (rv < 1) {
            if (errno == EINTR) {
                continue;
            }
           return -1;
       }
       n -= (size_t)rv;
       buf += rv;
    }
    return 0;
}

int write_all(int fd, char *buf, size_t n) {
    
    int rv = 0;
    while(n > 0) {
        rv = write(fd, buf, n);
        if(rv < 1) {
            return -1;
        }
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;

}

int32_t request_handler(int connfd, char *response) {
    // protocol : 4bytes for length, then message
    // read the full data, then return error as -ve & 0 for success
    const size_t K_MAX_MESSAGE = 4096;
    const size_t HEADER = 4;

    char buf[HEADER + K_MAX_MESSAGE]; //junk data added

    errno = 0;
    // GOOD PATTERN - where the Side effect is the outcome, and the return is the error
    int32_t err = read_full(connfd, buf, HEADER);
    if (err) {
        return msg(errno == 0 ? "EOF" : "read_full()");
    }

    
    // NOTE: // now read the content using that length;
    uint32_t len = 0;

    memcpy(&len, buf, (size_t)HEADER);

    if (len > K_MAX_MESSAGE) {
        return -1;
    }

    errno = 0;
    err = read_full(connfd, &buf[4], len);
    if (err) {
        return msg(errno == 0 ? "EOF": "read_full()");
    }


    printf("client requested with this message: %.*s\n", len, &buf[HEADER]);

    // response send : 

    // now write the content

    // write the length bytes
    len = (uint32_t)strlen(response);
    if (len > K_MAX_MESSAGE) {
        return -1;
    }

    char wbuf[HEADER + len];
    memcpy(wbuf, &len, HEADER);
    memcpy(&wbuf[HEADER], response, len);

    errno = 0;
    err = write_all(connfd, wbuf, HEADER + len);
    if (err) {
        return msg("write_all()");
    }

    return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(0);

    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));

    if (rv) {
        die("bind()");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen");
    }


    while (1) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);

        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0) {
            continue;
        }

        while (1) {
            int err = request_handler(connfd, "Hellooo from server");
            if (err) {
                //after EOF (ie; after client closed connection) - close the server fd connection
                goto L_DONE;
            }
        }

L_DONE:
        close(connfd);
    }
}

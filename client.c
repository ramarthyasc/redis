#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

void die(char *funcName) {
    perror(funcName);
    exit(EXIT_FAILURE);
}

void write_and_read(int clientfd) {
    int i = 0;
    while (1) {
        char wbuff[50];
        snprintf(wbuff, sizeof(wbuff), "Hey serverrr, Iam here: %d", i);
        ssize_t n = write(clientfd, wbuff, strlen(wbuff));
        if (n < 0) {
            die("write()");
        }

        char rbuff[64] = {0};
        n = read(clientfd, rbuff, sizeof(rbuff) - 1); // read() function blocks & cpu sleeps if there is nothing in the 
                                                      // RECEIVE BUFFER
        if (n<0) {
            die("read()");
        }
        printf("Server send this : %s \n", rbuff);

        i++;
    }
}


int main() {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        die("socket()");
    }

    // connect to server address
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(clientfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rv) {
        die("connect()");
    }

    write_and_read(clientfd);
    close(clientfd);

}


// NOTE: RECEIVE BUFFER - which stores the messages from the peer, and 
// 1. then reorders the stored packets, 
// 2. merges segments into a stream
// 3. Acts like a backpressure system (ie; if the host process is slow, and the peer is sending msgs fast, then 
// buffer will fillup and kernel sends TCP window = 0. And then, Server pauses transmission.) - A Backpressure System

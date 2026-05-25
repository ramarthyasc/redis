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

static void read_and_write(int connfd) {
    int i = 0;
    while (1) {
        //read

        char rbuff[64] = {0};
        ssize_t n = read(connfd, rbuff, sizeof(rbuff) - 1); // read() function blocks & cpu sleeps 
                                                            // if there is nothing in the RECEIVE BUFFER
        if (n < 0) {
            printf("read() error");
            return;
        }

        printf("client says %s \n", rbuff);

        //write
        char msg[50];
        snprintf(msg, sizeof(msg), "Hey clientee iam the server heree: %d", i);
        n = write(connfd, msg, strlen(msg)); // strlen only returns the length of string part -until the null terminator
        if (n < 0) {
            perror("write()");
            return;
        }

        i++;
    }
}

int main() {

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  int val = 1;
  //configure the server socket to reuse address by ignoring timeout after 4 way handshake disconnection
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); // sol_socket means the reuse address is put on socket layer
                                                               // val is used to set the value of reuseaddr to 1;

  // configuring the socket for binding 
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234); // port
  addr.sin_addr.s_addr = htonl(0); // wildcard IP 0.0.0.0

  int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) { die("bind()");}


  // listening
  rv = listen(fd, SOMAXCONN); // 4096 in linux
  if (rv) {
      die("listen()");
  }

  //accept all the clients in the queue
  while (1) {
      struct sockaddr_in client_addr = {}; // initializing every field in the struct to 0.
      socklen_t addrlen = sizeof(client_addr);
      int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen); // fills the client_addr struct with client address, port & family
      if (connfd < 0) {
          continue; // error
      }

      read_and_write(connfd);
      close(connfd);
  }
  

}
// NOTE: RECEIVE BUFFER - which stores the messages from the peer, and  - read() function reads from the front
// 1. then reorders the stored packets, 
// 2. merges segments into a stream
// 3. Acts like a backpressure system (ie; if the host process is slow, and the peer is sending msgs fast, then 
// buffer will fillup and kernel sends TCP window = 0. And then, Server pauses transmission.) - A Backpressure System

//NOTE: When we call listen(), the kernel creates 2 queues.
//There is a SYN queue (half open connections (the client didn't send the final ack)) and ACCEPT queue (
// completed connections )_

//NOTE: 
// Every TCP packet has:
//
// IP layer:
// Source IP (client)
// Destination IP (server)
// TCP layer:
// Source port (client ephemeral port)
// Destination port (server port)
//
// So a packet looks like:
//
// Client → Server
//
// Src IP:   192.168.1.10
// Src Port:  54321
//
// Dst IP:   192.168.1.5
// Dst Port:  1234

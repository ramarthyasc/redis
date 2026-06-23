# Redis building -

### Step 1 - server - client sockets connection build **(server1, client1)**: 

<img width="860" height="598" alt="image" src="https://github.com/user-attachments/assets/e9ee9677-824b-45f5-bee0-7d3d8972c820" />

##### Server ->
Create socket using AF_INET (IP v4) + SOCK_STREAM (connectionful stream) = TCP -> set the reuseaddress option (for overriding the wait period on the port after unbinding, & then binding onto it in that wait period itself) as true for the socket -> bind to an IP address (Private ips mostly) and Port on the machine. Then we configure the socket as listenable - using listen syscall (doesn't block) which returns intantly after configuring the kernel queues for incoming connections.
will be continued ..

TCP connection mechanics :
When connection happens, a SYN -> SYN-ACK -> ACK handshake happens b/w client and server. So, in SYN packet itself, there will be a sequence number, 'sourceport-sourceip-destport-destip' detail, so that the server knows which source is trying to connect. The listening server creates a new partial ConnectionSocket object and then puts it into a 'syn queue' of the listening socket. When the handshake completes, ie; when the server receives the ACK from the client, the transformed fully established ConnectionSocket in the 'syn queue' is transferred to 'accept queue'.

continues...
We do accept syscall, which removes the fully established ConnectionSocket object from the accept queue and returns the ConnectionSocket's fd.
If there is nothing in the accept queue, then it's blocked until a fully established connectionsocket is inside the accept queue.
-> Then send/receive messages between the connected server and client socket.

##### Client ->

### Step 2 - Blocking read and write considering receive and send buffers : server-client tcp connection **(server2, client2)** :

- Use a binary protocol ie; Each message contains :  Fixed no. of bytes for storing the length of the data, then the data bytes.
- We use a loop to read the number of bytes we want - because, client socket reads from / interacts with the receive buffer and then
copies the data to the Application's memory/buffer, and the receive buffer may not have the complete data that we need, so the syscall
returns with the no. of bytes it has copied to our app's memory - which would be <= the needed data. So a Loop is needed to collect all
the data that's needed.. 
Similarly - write syscall for server socket too.

### Step 3 - Non blocking read and write (Event loop system) - **(server3, client3)** :

- Use poll instead of epoll for readiness api
- Here, we make the sockets nonblocking using fcntl function - so that, whenever there is no data in the buffer for accept queue or recieve buffer or send buffer, then the syscalls (accept, read, send,..) will return -1 with errorno==EAGAIN. Otherwise, it will return normally (interactions with RAM) as in blocking socket's syscalls.

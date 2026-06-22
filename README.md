# Redis building -

### Step 1 - server - client sockets connection build **(server1, client1)**: 

<img width="860" height="598" alt="image" src="https://github.com/user-attachments/assets/e9ee9677-824b-45f5-bee0-7d3d8972c820" />


### Step 2 - Blocking read and write considering receive and send buffers : server-client tcp connection **(server2, client2)** :

We use a loop to read the number of bytes we want - because, client socket reads from / interacts with the receive buffer and then
copies the data to the Application's memory/buffer, and the receive buffer may not have the complete data that we need, so the syscall
returns with the no. of bytes it has copied to our app's memory - which would be <= the needed data. So a Loop is needed to collect all
the data that's needed.. 
Similarly - write syscall for server socket too.

### Step 3 - Non blocking read and write (Event loop system) - **(server3, client3)** :

- Use poll instead of epoll for readiness api

Socket architecture - ease of remember ->

Step 1 : 

<img width="1002" height="697" alt="image" src="https://github.com/user-attachments/assets/d1a0f05e-97ea-4de0-9bba-01aef892c0c0" />


We use a loop to read the number of bytes we want - because, client socket reads from / interacts with the receive buffer and then
copies the data to the Application's memory/buffer, and the receive buffer may not have the complete data that we need, so the syscall
returns with the no. of bytes it has copied to our app's memory - which would be <= the needed data. So a Loop is needed to collect all
the data that's needed.. 
Similarly - write syscall for server socket too.

# MultiClientChattingApp
Multiple client and single server chat application in C language using Socket Programming.
Where each Every Task or message will go through the Server Server is the bridg to All Clients that are communicating.
Methodology is simply Like Star Topology idea wher Server is HUB and Nodes are the Clients.

# To compile the File

1.Go to the file Directory where the code is stored
2.Run the below command in the terminal

```
gcc chatApp.c -o chatApp.exe
```

# To Run Server Side Program

1.Go to the file Directory where the code's exe (executive file) generated or compiled file location
2.Run the below command in the terminal

```
./chatApp.exe s <port>
```

# To Run Client Side Program
```
./chatApp.exe c <port>
```

<port> enter any port 


> [!NOTE]
> Make sure it the port is free to use before using it.


# Server Side Functionalities


1.To get Author of the Code
> AUTHOR


2.To get Server IPv4
> IP <IPV4>


3.To get Server Port
> PORT <PORT>


4.This below one will be sent by client to server to login, then server will read this message.
> LOGIN <IPv4> <Port> <Name>


5.To get the LIST of client when  client request to the server, server will read it. Even Works for server.
> LIST


6.To get Statistics of send and recv message count by each client it will only work on server side.
> STATISTICS


7.To check Who are still logged it and in active state. Work on both sides(Server/Client).
> REFRESH


8.This will be sent by the client to server and read by the server to send from one client to another client.
> SEND <Desti-IPV4> <msg>


9.Broadcast a message that is sent by the client to server, server will Broadcast the message to every client Except the sender.
> BROADCAST


10.Sent by the client to server to logout.
> LOGOUT


11.To Block a certain client.
> BLOCK <Ipv4>


12.For Exiting the Application
> EXIT



# Client Side Functionalities 


1.To get Author of the Code
> AUTHOR


2.To get Server IPv4
> IP <IPV4>


3.To get Server Port
> PORT <PORT>


4.This below one will be sent by client to server to login, then server will read this message.
> LOGIN <IPv4> <Port> <Name>


5.To get the LIST of client when  client request to the server, server will read it. Even Works for server.
> LIST


6.To check Who are still logged it and in active state. Work on both sides(Server/Client).
> REFRESH


7.This will be sent by the client to server and read by the server to send from one client to another client.
> SEND <Desti-IPV4> <msg>


8.Broadcast a message that is sent by the client to server, server will Broadcast the message to every client Except the sender.
> BROADCAST


9.Sent by the client to server to logout.
> LOGOUT


To Block a certain client.
> BLOCK <Ipv4>


For Exiting the Application
> EXIT


[^1] My Refernce material. https://beej.us/guide/bgnet/html/split/

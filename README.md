# MultiClientChattingApp
Multiple client and single server chat application in C language using Socket Programming.
Where each Every Task or message will go through the Server Server is the bridg to All Clients that are communicating.
Methodology is simply Like Star Topology idea wher Server is HUB and Nodes are the Clients.

# To compile the File
```
gcc chatApp.c -o chatApp.exe
```

# To Run Server Side Program
```
./chatApp.exe s <port>
```

# To Run Client Side Program
```
./chatApp.exe c <port>
```

<port> enter any port 
note:- make sure it the port is free to use before using it.


# Server Side Functionalities


To get Author of the Code
> AUTHOR


To get Server IPv4
> IP <IPV4>


To get Server Port
> PORT <PORT>


This below one will be sent by client to server to login, then server will read this message.
> LOGIN <IPv4> <Port> <Name>


To get the LIST of client when  client request to the server, server will read it. Even Works for server.
> LIST


To get Statistics of send and recv message count by each client it will only work on server side.
> STATISTICS


To check Who are still logged it and in active state. Work on both sides(Server/Client).
> REFRESH


This will be sent by the client to server and read by the server to send from one client to another client.
> SEND <Desti-IPV4> <msg>


Broadcast a message that is sent by the client to server, server will Broadcast the message to every client Except the sender.
> BROADCAST


Sent by the client to server to logout.
> LOGOUT


To Block a certain client.
> BLOCK <Ipv4>


For Exiting the Application
> EXIT



# Client Side Functionalities 


To get Author of the Code
> AUTHOR


To get Server IPv4
> IP <IPV4>


To get Server Port
> PORT <PORT>


This below one will be sent by client to server to login, then server will read this message.
> LOGIN <IPv4> <Port> <Name>


To get the LIST of client when  client request to the server, server will read it. Even Works for server.
> LIST


To check Who are still logged it and in active state. Work on both sides(Server/Client).
> REFRESH


This will be sent by the client to server and read by the server to send from one client to another client.
> SEND <Desti-IPV4> <msg>


Broadcast a message that is sent by the client to server, server will Broadcast the message to every client Except the sender.
> BROADCAST


Sent by the client to server to logout.
> LOGOUT


To Block a certain client.
> BLOCK <Ipv4>


For Exiting the Application
> EXIT

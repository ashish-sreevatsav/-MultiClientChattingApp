#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

#define MAXSIZE 259
#define MAXDATASIZEBACKGROUND 99999
#define STDIN 0





struct host{
    char host_name[MAXSIZE];
    char ip[MAXSIZE];
    char port[MAXSIZE];
    int fd;
    int sent_no;
    int recv_no;
    char status[MAXSIZE];
    bool is_loggedin;
    bool is_server;
    struct host *blocked;
    struct host *next;
    struct message * queued_messages;
};

struct host * new_client = NULL;
struct host * clients = NULL;
struct host * localhost = NULL;
struct host * server = NULL; // this is used only by the clients to store server info
int yes = 1;

char *clientsDataList = NULL;

//Function declaration

void execute_server(char *command, int fd);
void host__set_hostname_and_ip(struct host * h);
void *get_in_addr(struct sockaddr *sa);
void server_start(char *port);
void execute_client(char command[]);
int client_connect_server(char *ip,char *port);
void client_login(char *ip,char *port);
void client_start(char *port);
bool host__check_valid_ip_addr(char ip_addr[MAXSIZE]);
void send_msg(int fd, char msg[]);
char* copyString(char s[],int start,int end);
//

char* copyString(char s[],int start,int end)
{
    int i;
    char* s2;
    s2 = (char*)malloc(20);
 
    // Executing till null character
    // is found
    int j =0;
    for (i = start; i<end; i++) {
 
        // Copy the character one
        // by one from s1 to s2
        s2[j] = s[i];
        j++;
    }
 
    // Return the pointer of newly
    // created string
    return (char*)s2;
}

void send_msg(int fd, char msg[]){
    int rv;
    if((rv=send(fd,msg,strlen(msg),0))==-1)
    {
        perror("Error in sending:");
    }
    //printf("%s",msg);
}

bool host__check_valid_ip_addr(char ip_addr[MAXSIZE]) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_addr, & (sa.sin_addr));
    return result != 0;
}

void execute_server(char *command, int fd){
    if(strstr(command,"AUTHOR") != NULL){
        printf("[AUTHOR:SUCCESS]\n");
        printf("I, Ashish, have read and understood the course academic integrity policy.\n");
        printf("[AUTHOR:END]\n");
    }
    else if(strstr(command,"IP") != NULL)
    {
        printf("[IP:SUCCESS]\n");
        printf("IP:%s\n", localhost -> ip);
        printf("[IP:END]\n");
    }
    else if(strstr(command,"PORT") != NULL){
        printf("[PORT:SUCCESS]\n");
        printf("PORT:%s\n",localhost->port);
        printf("[PORT:END]\n");
    }
    else if(strncmp(command,"LOGIN",5) == 0){
        char ip[INET_ADDRSTRLEN],port[MAXSIZE],name[MAXSIZE];
        sscanf(command,"LOGIN %s %s %s\n",ip,port,name);
        struct host * temp = clients;
        bool is_new = true;
        struct host * requesting_client = malloc(sizeof(struct host));

        while (temp != NULL) {
            if (temp -> fd == fd) {
                requesting_client = temp;
                is_new = false;
                break;
            }
            temp = temp -> next;
        }

        if(is_new){
            memcpy(new_client->ip,ip,sizeof(new_client->ip));
            memcpy(new_client->port,port,sizeof(new_client->port));
            memcpy(new_client->host_name,name,sizeof(new_client->host_name));
            requesting_client->fd = fd;
            int c_port = atoi(port);
            if (clients == NULL) {
                clients = malloc(sizeof(struct host));
                clients = new_client;
            } else if (c_port < atoi(clients -> port)) {
                new_client -> next = clients;
                clients = new_client;
            } else {
                struct host * temp = clients;
                while (temp -> next != NULL && atoi(temp -> next -> port) < c_port) {
                    temp = temp -> next;
                }
                new_client -> next = temp -> next;
                temp -> next = new_client;
            }

        }else{
            requesting_client->is_loggedin = true;
        }

        char res[MAXDATASIZEBACKGROUND] = "REFRESHRESPONSE FIRST\n";
        temp = clients;
        int id = 1;
        while (temp != NULL) {
            if (temp -> is_loggedin) {
                char clientString[MAXSIZE * 4];
                sprintf(clientString,"%-5d%-35s%-20s%-8s\n", id, temp -> host_name, temp -> ip, (temp -> port));
                strcat(res, clientString);
                id++;
            }
            temp = temp -> next;
        }
        strcat(res,"REFRESHRESPONSE FIRST\n");
        send_msg(fd,res);   
    }
    else if(strstr(command,"LIST") != NULL)
    {
        printf("[LIST:SUCCESS]\n");

        struct host * temp = clients;
        int id = 1;
        while (temp != NULL) {
            // SUSPICIOUS FOR REFRESH
            if (temp -> is_loggedin) {
                printf("%-5d%-35s%-20s%-8s\n", id, temp -> host_name, temp -> ip, (temp -> port));
                id = id + 1;
            }
            temp = temp -> next;
        }

        printf("[LIST:END]\n");
    }
    else if(strstr(command,"STATISTICS") != NULL){
        printf("[STATISTICS:SUCCESS]\n");

        struct host * temp = clients;
        int id = 1;
        while (temp != NULL) {
            printf("%-5d%-35s%-8d%-8d%-8s\n", id, temp -> host_name, temp->sent_no, temp -> recv_no, temp -> is_loggedin ? "logged-in" : "logged-out");
            id = id + 1;
            temp = temp -> next;
        }

        printf("[STATISTICS:END]\n");
    }
    else if(strstr(command,"REFRESH") != NULL){
        char res[MAXDATASIZEBACKGROUND] = "REFRESHRESPONSE NOTFIRST\n";
        struct host *temp = clients;
        int id = 1;
        while (temp != NULL) {
            if (temp -> is_loggedin) {
                char clientString[MAXSIZE * 4];
                sprintf(clientString,"%-5d%-35s%-20s%-8s\n", id, temp -> host_name, temp -> ip, (temp -> port));
                strcat(res, clientString);
                id++;
            }
            temp = temp -> next;
        }
        strcat(res,"REFRESHRESPONSE NOTFIRST\n");
        send_msg(fd,res);   
    }
    else if(strstr(command,"SEND") != NULL){
        char ip[INET_ADDRSTRLEN],msg[MAXDATASIZEBACKGROUND];
        sscanf(command,"SEND %s %[^\n]%*c",ip,msg);
        struct host * temp = clients;
        struct host * to_client = malloc(sizeof(struct host));
        struct host * from_client = malloc(sizeof(struct host));
        int i = 0;
        while (temp != NULL) {
            if (strstr(temp->ip,ip) != NULL) {
                to_client = temp;
                i++;
            }
            if(temp->fd == fd)
            {
                from_client = temp;
            }
            if(i==2){
                break;
            }
            temp = temp -> next;
        }
        send_msg(to_client->fd,msg);
        printf("msg from:%s, to:%s\n[msg]:%s\n",from_client->ip,to_client->ip,msg);
        from_client->sent_no++;
        to_client->recv_no++;
        send_msg(from_client->fd,"SUCCESSSEND");
    }
    else if(strstr(command,"BROADCAST") != NULL)
    {
        char msg[MAXDATASIZEBACKGROUND];
        sscanf(command,"BROADCAST %[^\n]%*c",msg);
        struct host * temp = clients;
        struct host * from_client = malloc(sizeof(struct host));
        while (temp != NULL) {
            if(temp->fd == fd)
            {
                from_client = temp;
                break;
            }
            temp = temp -> next;
        }
        temp = clients;
        while(temp != NULL){
            if(temp->is_loggedin && temp->fd != fd){
                send_msg(temp->fd,msg);
                printf("msg from:%s, to:%s\n[msg]:%s\n",from_client->ip,temp->ip,msg);
            }
            temp = temp->next;
        }
        send_msg(fd,"SUCCESSBROADCAST");
    }
    else if(strstr(command,"LOGOUT") != NULL){
        struct host * temp = clients;
        while (temp != NULL) {
            if (temp -> fd == fd) {
                send_msg(fd, "SUCCESSLOGOUT\n");
                temp -> is_loggedin = false;
                break;
            }
            temp = temp -> next;
        }
        if (temp == NULL) {
            send_msg(fd, "ERRORLOGOUT\n");
        }
    }
    else if(strstr(command,"BLOCK") != NULL){
        char ip[INET_ADDRSTRLEN];
        sscanf(command,"BLOCK %s\n",ip);
        struct host *temp = clients;
        struct host *req_client;
        while(temp != NULL){
            if(temp->fd == fd){
                req_client = temp;
                break;
            }
            if(strstr(temp->ip,ip) == NULL){
                send_msg(fd,"ERRORBLOCK");
            }
            temp = temp->next;
        }
        if(req_client->blocked != NULL){
            struct host *temp = req_client->blocked;
            while(temp!=NULL){
                if(strstr(temp->ip,ip) != NULL){
                    send_msg(fd,"ALREADYBLOCKED");
                }
                temp = temp->next;
            }
            while(temp!=NULL){
                
            }
        }
        else{
            struct host *temp = clients;
            while(temp!=NULL){
                if(strstr(temp->ip,ip) != NULL){
                    req_client->blocked = temp;
                }
                temp = temp->next;
            }
            send_msg(fd,"SUCCESSBLOCK");
        }
        
    }
    else if(strstr(command,"EXIT") != NULL){
        struct host * temp = clients;
        if (temp -> fd == fd) {
            clients = clients -> next;
        } else {
            struct host * previous = temp;
            while (temp != NULL) {
                if (temp -> fd == fd) {
                    previous -> next = temp -> next;
                    temp = temp -> next;
                    break;
                }
                temp = temp -> next;
            }
        }
    }
}

void host__set_hostname_and_ip(struct host * h) {
    char hostbuffer[MAXDATASIZEBACKGROUND];
    struct hostent * host_entry = malloc(sizeof(struct hostent));
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    // To retrieve host information

    host_entry = gethostbyname(hostbuffer);
    memcpy(h -> ip, inet_ntoa( * ((struct in_addr * ) host_entry -> h_addr_list[0])), sizeof(h -> ip));
    memcpy(h -> host_name, hostbuffer, sizeof(h -> host_name));

    return;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void server_start(char *port)
{
    localhost = (struct host*) malloc (sizeof(struct host));
    host__set_hostname_and_ip(localhost);
    strcpy(localhost->port,port);
    
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char data_buffer[MAXDATASIZEBACKGROUND]; // buffer for client data
    int data_buffer_bytes; // holds number of bytes received and stored in data_buffer

	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    FD_SET(STDIN, & master);
	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    localhost->fd=listener;
    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } else {
                        new_client = malloc(sizeof(struct host));
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        memcpy(new_client -> ip,
                            inet_ntop(
                                remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr), // even though new_client_addr is of type sockaddr_storage, they can be cast into each other. Refer beej docs.
                                remoteIP,
                                INET6_ADDRSTRLEN
                            ), sizeof(new_client -> ip));
                        new_client -> fd = newfd;
                        new_client -> sent_no = 0;
                        new_client -> recv_no = 0;
                        new_client -> is_loggedin = true;
                        new_client -> next = NULL;
                        new_client -> blocked = NULL;
                        
                    }
                } 
                else if(i == STDIN){
                    // handle data from standard input
                    char * command = (char * ) malloc(sizeof(char) * MAXDATASIZEBACKGROUND);
                    memset(command, '\0', MAXDATASIZEBACKGROUND);
                    if (fgets(command, MAXDATASIZEBACKGROUND - 1, stdin) == NULL) { // -1 because of new line
                    } else {
                        execute_server(command, i);
                    }
                    fflush(stdout);
                }
                else {
                    // handle data from a client
                    data_buffer_bytes = recv(i, data_buffer, sizeof data_buffer, 0);
                    if (data_buffer_bytes == 0) {
                        close(i); // Close the connection
                        FD_CLR(i, & master); // Remove the fd from master set
                    } else if (data_buffer_bytes == -1) {
                        close(i); // Close the connection
                        FD_CLR(i, & master); // Remove the fd from master set
                    } else {
                        
                        execute_server(data_buffer, i);
                    }
                    fflush(stdout);
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    
}



void execute_client(char command[])
{
    if(strstr(command,"AUTHOR") != NULL){
        printf("[AUTHOR:SUCCESS]\n");
        printf("I, Ashish, have read and understood the course academic integrity policy.\n");
        printf("[AUTHOR:END]\n");
    }
    else if(strstr(command,"IP") != NULL)
    {
        printf("[IP:SUCCESS]\n");
        printf("IP:%s\n", localhost -> ip);
        printf("[IP:END]\n");
    }
    else if(strstr(command,"PORT") != NULL){
        printf("[PORT:SUCCESS]\n");
        printf("PORT:%s\n",localhost->port);
        printf("[PORT:END]\n");
    }
    else if(strncmp(command,"LOGIN",5) == 0){
        char server_ip[INET_ADDRSTRLEN];
        char server_port[MAXSIZE];
        sscanf(command,"LOGIN %s %s\n",server_ip,server_port);
        client_login(server_ip,server_port);
    }else if (strstr(command, "REFRESHRESPONSE") != NULL) {
        bool is_refresh = false;
        if(strstr(command,"NOTFIRST") != NULL){
            is_refresh = true;
            clientsDataList = copyString(command,25,strlen(command)-25);
        }
        else{
            
            clientsDataList = copyString(command,22,strlen(command)-22);
        }

        if(!is_refresh){
            printf("[LOGIN:SUCCESS]\n");
            printf("[LOGIN:END]\n");
        }
        else{
            printf("[REFRESH:SUCCESS]\n");
            printf("[REFRESH:END]\n");
        }
    }
    else if(strstr(command,"SUCCESSLOGIN") != NULL)
    {
        printf("[LOGIN:SUCCES]\n");
        printf("[LOGIN:END]\n");
    }
    else if(strstr(command,"SUCCESSLOGOUT") != NULL){
        localhost->is_loggedin = false;
        printf("[LOGOUT:SUCCESS]\n");
        printf("[LOGOUT:END]\n");
    }
    else if(strstr(command,"ERRORLOGOUT") != NULL){
        printf("[LOGOUT:ERROR]\n");
        printf("[LOGOUT:END]\n");
    }
    else if(strstr(command,"SUCCESSEND") != NULL){
        printf("[SEND:SUCCES]\n");
        printf("[SEND:END]\n");
    }
    else if(strstr(command,"SUCCESSBROADCAST") != NULL){
        printf("[BROADCAST:SUCCESS]\n");
        printf("[BROADCAST:END]\n");
    }
    else if(strstr(command,"SUCCESSBLOCK") != NULL){
        printf("[BLOCK:SUCCESS]\n");
        printf("[BLOCK:END]\n");
    }
    else if(strstr(command,"LIST") != NULL)
    {

        if(localhost->is_loggedin)
        {   printf("[LIST:SUCCESS]\n");
            printf("%s",clientsDataList);
        }
        else{
            printf("[LIST:ERROR]\n");
        }
        printf("[LIST:END]\n");
    }
    else if(strstr(command,"REFRESH") != NULL)
    {
        if (localhost -> is_loggedin) {
            send_msg(server -> fd, "REFRESH\n");
        } else {
            printf("[REFRESH:ERROR]\n");
            printf("[REFRESH:END]\n");
        }
    }
    else if(strncmp(command,"SEND",4) == 0){
        if(localhost->is_loggedin){
            send_msg(server->fd,command);
        }
        else{
            printf("[SEND:ERROR]\n");
            printf("[SEND:END]\n");
        }
    }
    else if(strncmp(command,"BROADCAST",9) == 0){
        if(localhost->is_loggedin){
            send_msg(server->fd,command);
        }
        else{
            printf("[BROADCAST:ERROR]\n");
            printf("[BROADCAST:END]\n");
        }
    }
    else if(strncmp(command,"LOGOUT",6) == 0){
        if(localhost->is_loggedin){
            send_msg(server->fd,"LOGOUT");
        }
        else{
            printf("[LOGOUT:ERROR]\n");
            printf("[LOGOUT:END]\n");
        }
    }
    else if(strncmp(command,"BLOCK",5) == 0){
        char ip[INET_ADDRSTRLEN];
        sscanf(command,"BLOCK %s\n",ip);
        struct host temp;
        if(localhost->is_loggedin){
            send_msg(server->fd,command);
        }
        else{
            printf("[BLOCK:ERROR]\n");
            printf("[BLOCK:END]\n");
        }
    }
    else if(strstr(command,"EXIT") != NULL){
        send_msg(server->fd,"EXIT");
        printf("[EXIT:SUCCESS]\n");
        printf("[EXIT:END]\n");
        exit(0);
    }
}

int client_connect_server(char ip[],char port[]){
    server = malloc(sizeof(struct host));
    memcpy(server -> ip, ip, sizeof(server -> ip));
    memcpy(server -> port, port, sizeof(server -> port));
    int server_fd = 0, status;
    struct addrinfo hints, * server_ai, * temp_ai;

    // get a socket and bind it
    memset( & hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(server -> ip, server -> port, & hints, & server_ai)) != 0) {
        perror("error: ");
        return 0;
    }

    for (temp_ai = server_ai; temp_ai != NULL; temp_ai = temp_ai -> ai_next) {
        server_fd = socket(temp_ai -> ai_family, temp_ai -> ai_socktype, temp_ai -> ai_protocol);
        if (server_fd < 0) {
            continue;
        }
        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int)) == -1){
            perror("error:");
        }
        if (connect(server_fd, temp_ai -> ai_addr, temp_ai -> ai_addrlen) < 0) {
            close(server_fd);
            continue;
        }
        break;
    }

    // exit if could not bind
    if (temp_ai == NULL) {
        perror("binding error: ");
        return 0;
    }

    server -> fd = server_fd;

    freeaddrinfo(server_ai);
    // Initalisze a listener as well to listen for P2P cibbectuibs
    int listener = 0;
    struct addrinfo * localhost_ai;
    if ((status = getaddrinfo(NULL, localhost -> port, & hints, & localhost_ai)) != 0) {
        return 0;
    }

    for (temp_ai = localhost_ai; temp_ai != NULL; temp_ai = temp_ai -> ai_next) {
        listener = socket(temp_ai -> ai_family, temp_ai -> ai_socktype, temp_ai -> ai_protocol);
        if (listener < 0) {
            continue;
        }
        if(setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int)) == -1){
            perror("error:");
        }
        
        if (bind(listener, temp_ai -> ai_addr, temp_ai -> ai_addrlen) < 0) {
            
            close(listener);
            continue;
        }
        break;
    }
    // exit if could not bind
    if (temp_ai == NULL) {
        perror("Binding error");
        return 0;
    }

    // listen
    if (listen(listener, 10) == -1) {
        return 0;
    }

    localhost -> fd = listener;

    freeaddrinfo(localhost_ai);

    return 1;
}

void client_login(char *ip,char *port){
    int rv;
    if(ip == NULL || port == NULL)
    {
        printf("[LOGIN:ERROR]\n");
        printf("[LOGIN:END]\n");
        return;
    }

    if (server == NULL) {
        if (!host__check_valid_ip_addr(ip) || client_connect_server(ip, port) == 0) {
            printf("[LOGIN:ERROR]\n");
            printf("[LOGIN:END]\n");
            return;
        }
        
    } else {
        if (strstr(server -> ip, ip) == NULL || strstr(server -> port, port) == NULL) {
            printf("[LOGIN:ERROR]\n");
            printf("[LOGIN:END]\n");
            return;
        }
    }

    localhost -> is_loggedin = true;
    char msg[MAXDATASIZEBACKGROUND];
    sprintf(msg, "LOGIN %s %s %s\n", localhost -> ip, localhost -> port, localhost -> host_name);
    send_msg(server->fd,msg);
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
  
    FD_ZERO( & master); // clear the master and temp sets
    FD_ZERO( & read_fds);
    FD_SET(server -> fd, & master); // Add server->fd to the master list
    FD_SET(STDIN, & master); // Add STDIN to the master list
    FD_SET(localhost -> fd, & master);
    int fdmax = server -> fd > STDIN ? server -> fd : STDIN; // maximum file descriptor number. initialised to listener    
    fdmax = fdmax > localhost -> fd ? fdmax : localhost -> fd;
    char buf[MAXDATASIZEBACKGROUND];    // buffer
    int nbytes;
    int i;
    struct sockaddr_storage new_peer_addr; // client address
    socklen_t addrlen = sizeof new_peer_addr;
    
    while(localhost->is_loggedin){
        read_fds = master; // make a copy of master set
        if (select(fdmax + 1, & read_fds, NULL, NULL, NULL) == -1) {
            exit(EXIT_FAILURE);
        }
        for(i = 0;i<=fdmax;i++)
        {
            if(FD_ISSET(i,&read_fds)){
                if (i == server -> fd) {
                    // handle data from the server
                    memset(buf,'\0',MAXDATASIZEBACKGROUND);
                    nbytes = recv(i, buf, sizeof buf, 0);
                    if (nbytes == 0) {
                        close(i); // Close the connection
                        FD_CLR(i, & master); // Remove the fd from master set
                    } else if (nbytes == -1) {
                        close(i); // Close the connection
                        FD_CLR(i, & master); // Remove the fd from master set
                    } else {
                        execute_client(buf);
                    }
                }
                else if (i == STDIN) {
                    // handle data from standard input
                    char * command = (char * ) malloc(sizeof(char) * MAXDATASIZEBACKGROUND);
                    memset(command, '\0', MAXDATASIZEBACKGROUND);
                    if (fgets(command, MAXDATASIZEBACKGROUND - 1, stdin) != NULL) {
                        execute_client(command);
                    }
                } 
            }
        }
        fflush(stdout);
    }
}


void client_start(char *port){

    localhost = (struct host*) malloc (sizeof(struct host));
    clientsDataList = (char*)malloc(sizeof(char)*MAXDATASIZEBACKGROUND);
    
    host__set_hostname_and_ip(localhost);
    strcpy(localhost->port,port);
    int listener;     // listening socket descriptor
    struct addrinfo hints, *ai, *p;
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    // get a socket and bind it
    memset( & hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, localhost->port, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
    
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		if(setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int)) == -1){
            perror("error:");
        }
        
		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "client: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    localhost->fd=listener;
    char * command = (char * ) malloc(sizeof(char) * MAXDATASIZEBACKGROUND);
    while(true)
    {
        memset(command, '\0', MAXDATASIZEBACKGROUND);
        if (fgets(command, MAXDATASIZEBACKGROUND - 1, stdin) != NULL) { 
            execute_client(command);
        }
    }
    
    
}

int main(int argc,char **argv){
    
    if (argc != 3) {
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"s") == 0){
        
        server_start(argv[2]);
    }
    if(strcmp(argv[1],"c") == 0)
    {
        client_start(argv[2]);
    }
    return 0;
}
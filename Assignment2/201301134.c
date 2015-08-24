/***
	##### Yash Patel - 201301134 #####
	#####           CSE          #####
	#####         yash0307       #####
***/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>
#include <signal.h>
char data_recived_by_server[1024];
char data_sent_by_server[1024];

/*----- Data structute and main function -----*/
typedef struct setupData
{
	int serverPortNumber;
	int clientPortNumber;
	char typeProtocol[20];
	pid_t process;
}node;

/*--------------------------------------------*/
int client(int clientPortNumber, char typeProtocol[])
{
	int sock;
	int flag_type;
	if(strcmp(typeProtocol, "tcp")==0)
	{
		flag_type = 0;
	}
	else if(strcmp(typeProtocol,"udp")==0)
	{
		flag_type = 1;
	}
	else
	{
		/*something really weird happened*/
		printf("\nError : Invalid Protocol type\n");
		return 1;
	}
	/*Firstly need to determine IP of machine on which program is running*/
	struct hostent *host;
	host = gethostbyname("127.0.0.1");
	/*Allocate a socket discriptor*/
	/*for tcp*/
	if(flag_type == 0)
	{
		/*AF_INET is the domain for IPv4, SOCK_STREAM for stream type socket, 0 is protocol.*/
		sock = socket(AF_INET, SOCK_STREAM,0);
		if(sock == -1)
		{
			printf("\nError : retriving of socket failed");
			return 1;
		}
	}
	/*for udp*/
	else if(flag_type == 1)
	{
		/*SOCK_DGRAM is connectionless, postal type service.*/
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock == -1)
		{
			printf("\nError : retriving of socket failed");
			return 1;
		}
	}
	/*Refrence for socket address for IPv4*/
	/*
	// (IPv4 only--see struct sockaddr_in6 for IPv6)

	struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
	};
	*/
	/*filling the address information manually, getaddrinfo() can also be used ?*/
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	serverAddress.sin_family = AF_INET;/*is the domain for IPv4*/
	serverAddress.sin_port = htons(clientPortNumber);/*port number for server passed from main function.*//*Convert multi-byte integer types from host byte order to network byte order*/
	serverAddress.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serverAddress.sin_zero), 8);
	int sin_size = sizeof(struct sockaddr_in);
	socklen_t *temp = (socklen_t *) &sin_size;
	if(flag_type == 0)
	{
		if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) == -1)
		{
			printf ("\n Error : Unable to connect to port.\n");
			return 2;
		}
		printf("\n Client connected to port %d\n",clientPortNumber	);
	}
}
int server(int serverPortNumber, char typeProtocol[])
{
	int sock; /*Variable to be used for socket discriptor.*/
	int flag_type;/*flag is zero for tcp, 1 for udp*/
	int sin_size;
	if(strcmp(typeProtocol,"tcp") == 0)
	{
		flag_type = 0;
	}
	else if(strcmp(typeProtocol,"udp")==0)
	{
		flag_type = 1;
	}
	else
	{
		printf("\nError : Invalid Protocol type\n");
		return 1;
	}
	/*Allocate a socket discriptor*/
	/*for tcp*/
	if(flag_type == 0)
	{
		/*AF_INET is the domain for IPv4, SOCK_STREAM for stream type socket, 0 is protocol.*/
		sock = socket(AF_INET, SOCK_STREAM,0);
		if(sock == -1)
		{
			printf("\nError : retriving of socket failed");
			return 1;
		}
	}
	/*for udp*/
	else if(flag_type == 1)
	{
		/*SOCK_DGRAM is connectionless, postal type service.*/
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock == -1)
		{
			printf("\nError : retriving of socket failed");
			return 1;
		}
	}
	/*Refrence for socket address for IPv4*/
	/*
	// (IPv4 only--see struct sockaddr_in6 for IPv6)

	struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
	};
	*/
	/*filling the address information manually, getaddrinfo() can also be used ?*/
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	serverAddress.sin_family = AF_INET;/*is the domain for IPv4*/
	serverAddress.sin_port = htons(serverPortNumber);/*port number for server passed from main function.*//*Convert multi-byte integer types from host byte order to network byte order*/
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serverAddress.sin_zero), 8);
	/*so far so good. Having finished socket() and initialization of serveraddress, time to bind() the socket with IP and port number*/
	int bind_bind; /*name bcs bind() is a function.*/
	bind_bind = bind(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
	if(bind_bind == -1)
	{
		return 2;
	}
	/*listen for incoming connections, need to do for tcp as udp's will be connection less*/
	/*listen() is what saperates server from client!!*/
	/*listen is to be called after bind and backlog parameter shall be increased as load increases.*/
	if(flag_type == 0)
	{
		if(listen(sock, 10)==-1)
		{
			printf("\n Error : in listen\n");
			return 3;
		}
	}
	printf("\nWaiting for client to join\n");
	/*If could listen successfully then the server will wait for client't request.*/
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		socklen_t *temp = (socklen_t *) &sin_size;
		int connectionSocket = 0;
		/*connection for tcp type*/
		if(flag_type == 0)
		{
			connectionSocket = accept(sock, (struct sockaddr *)&clientAddress, temp);
			printf("\nConnection established with %s - %d\n $ ", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
		}
		while(1)
		{
			/*having done with set up of port and socket*/
			int bytesRecieved;
			int last_index;
			/*tcp uses recv and udp uses recvfrom*/
			if(flag_type == 0)
			{
				bytesRecieved = recv(connectionSocket, data_recived_by_server, 1024, 0);
			}
			else
			{
				bytesRecieved = recvfrom(sock, data_recived_by_server, 1024, 0,(struct sockaddr *)&clientAddress, temp);
			}
		}
	}
	close(sock);
	return 0;
}
int main()
{
	node data;
	printf("\n\nwelcome\n\n");
	printf("Enter Server Port Number\n\n");
	scanf("%d",&data.serverPortNumber);
	printf("Enter Cliend Port Number\n\n");
	scanf("%d",&data.clientPortNumber);
	printf("Enter type of protocol to be used : (tcp/udp)");
	scanf("%s",data.typeProtocol);
	data.process = fork();
	if(data.process == -1)
	{
		printf("Error in forking data\n");
		exit(1);
	}
	else if(data.process == 0)
	{
		server(data.serverPortNumber, data.typeProtocol);
	}
	else
	{
		int p;
		p = client(data.clientPortNumber, data.typeProtocol);
		while(1)
		{
			if(p<0)
				break;
		}
		sleep(1);
	}
	kill(data.process, SIGQUIT);
	return 0;
}
/*------ ############### -----*/
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
	int listenSocket = 0;	// This is my server's socket which is created to 
							//	listen to incoming connections
	int connectionSocket = 0;

	struct sockaddr_in serv_addr;		// This is for addrport for listening

	// Creating a socket

	listenSocket = socket(AF_INET,SOCK_STREAM,0);
	if(listenSocket<0)
	{
		printf("ERROR WHILE CREATING A SOCKET\n");
		return 0;
	}
	else
		printf("[SERVER] SOCKET ESTABLISHED SUCCESSFULLY\n\n");

	// Its a general practice to make the entries 0 to clear them of malicious entry

	bzero((char *) &serv_addr,sizeof(serv_addr));

	// Binding the socket

	int portno = 5005;
	serv_addr.sin_family = AF_INET;	//For a remote machine
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);

	if(bind(listenSocket,(struct sockaddr * )&serv_addr,sizeof(serv_addr))<0)
		printf("ERROR WHILE BINDING THE SOCKET\n");
	else
		printf("[SERVER] SOCKET BINDED SUCCESSFULLY\n");

	// Listening to connections

	if(listen(listenSocket,10) == -1)	//maximum connections listening to 10
	{
		printf("[SERVER] FAILED TO ESTABLISH LISTENING \n\n");
	}
	printf("[SERVER] Waiting fo client to connect....\n" );

	// Accepting connections
	while((connectionSocket=accept(listenSocket , (struct sockaddr*)NULL,NULL))<0);

	// NULL will get filled in by the client's sockaddr once a connection is establised

	printf("[CONNECTED]\n");

	char buffer[1024];
	bzero(buffer,1024);
	if(recv(connectionSocket,buffer,1023,0)<0)
		printf("ERROR whiile reading from Client\n");
	printf("Message from Client: %s\n",buffer );
	bzero(buffer,1023);
	printf("Reply to Client: ");
	fgets(buffer,1023,stdin);
	send(connectionSocket,buffer,strlen(buffer),0);
	printf("\nClosing connection\n");
	close(connectionSocket);
	close(listenSocket);
	return 0;
}
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
	int ClientSocket = 0;
	struct sockaddr_in serv_addr;

	// Creating a socket

	ClientSocket = socket(AF_INET,SOCK_STREAM,0);
	if(ClientSocket<0)
	{
		printf("ERROR WHILE CREATING A SOCKET\n");
		return 0;
	}
	else
		printf("[CLIENT] Socket created \n");

	int portno = 5005;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Connection Establishment

	while(connect(ClientSocket,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0);

	char buffer[1024];
	bzero(buffer,1024);
	printf("\nEnter Your Message: ");
	fgets(buffer,1023,stdin);
	if(send(ClientSocket,buffer,strlen(buffer),0)<0)
		printf("ERROR while writing to the socket\n");
	bzero(buffer,1024);
	if(recv(ClientSocket,buffer,1023,0)<0)
		printf("ERROR while reading from the socket\n");

	printf("\nMessage from Server: ");
	printf("%s\n",buffer );

	printf("Closing Connection\n");
	close(ClientSocket);
	return 0;
}
/*
   YASH PATEL 201301134
   DHRUVA DAS 201301151
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

typedef struct dataForProtocol
{
	int serverPortNumber;
	int clientPortNumber;
	int typeProtocol;
	pid_t processID;
}node;

typedef struct Server
{
	int sock;
	int connection;
	int bytesRecieved;
}servernode;
typedef struct client
{
	int sock;
	int bytesReceived;
	char sendData[1024];
	char recvData[1024];
}nodeclient;
typedef struct Global_global
{
	char history[100][1000];
	char complete_command[1000];
	char command[100][100];
	int command_count;
	char server_send_data[1024];
	char server_recv_data[1024];
	char recv_complete_command[1024];
	char recv_command[32][32];
	int recv_command_count;
}Globalnode;
Globalnode Global;
int flag_init = 0;
void global_initialize()
{
	Global.command_count = 0;
	Global.recv_command_count = 0;
}
void take_input()
{
	char c;
	scanf("%[^\n]",Global.complete_command);
	scanf("%c",&c);
	if(Global.complete_command[0]=='I')
		strcpy(Global.history[Global.command_count++],Global.complete_command);
}

int tokenise2(char comm[1000])
{
	char *pch=NULL;
	char copy[1000];
	strcpy(copy,comm);
	pch = strtok(copy," ");
	int p=0;
	if(pch!=NULL)
	{
		strcpy(Global.recv_command[p],pch);
		p++;
		pch=strtok(NULL," ");
		if(pch!=NULL)
		{
			strcpy(Global.recv_command[p],pch);
			p++;
			pch=strtok(NULL," ");
			if(pch!=NULL)
			{
				strcpy(Global.recv_command[p],pch);
				p++;
				pch=strtok(NULL," ");
				if(pch!=NULL)
				{
					strcpy(Global.recv_command[p],pch);
					p++;
					pch=strtok(NULL," ");
				}
			}
		}
	}
	Global.recv_command[p][0]='\0';
	return p;
}
int tokenise1(char comm[1000])
{

	char *pch=NULL;
	char copy[1000];
	strcpy(copy,comm);
	pch = strtok (copy," ");
	int p=0;
	if(pch!=NULL)
	{
		strcpy(Global.command[p],pch);
		p++;
		pch=strtok(NULL," ");
		if(pch!=NULL)
		{
			strcpy(Global.command[p],pch);
			p++;
			pch=strtok(NULL," ");
			if(pch!=NULL)
			{
				strcpy(Global.command[p],pch);
				p++;
				pch=strtok(NULL," ");
				if(pch!=NULL)
				{
					strcpy(Global.command[p],pch);
					p++;
					pch=strtok(NULL," ");
				}
			}
		}
	}
	Global.command[p][0]='\0';
	return p;
}
void server_function(FILE *fp,int connected)
{

	memset(Global.server_send_data,0,1024);
	int sentn;
	int byter;
	while(!feof(fp))
	{
		byter=fread(Global.server_send_data,1,1024,fp);
		Global.server_send_data[byter]='\0';
		sentn = write(connected,Global.server_send_data,1024);
	}
	memset(Global.server_send_data,0,1024);
	char end[1000];
	strcpy(end,"End Of File");
	strcpy(Global.server_send_data,end);
	write(connected,Global.server_send_data,1024);
	fclose(fp);
}
int check_close_connection(servernode dataServer)
{
	if(dataServer.bytesRecieved==0)
	{
		printf("Error : Connection closed\n");
		printf("$");
		close(dataServer.connection);
		return 1;
	}	
	if(strcmp(Global.server_recv_data , "Exit") == 0 || strcmp(Global.server_recv_data , "exit") == 0)
	{
		printf("Error : Connection closed\n");
		printf("$");
		close(dataServer.connection);
		return 1;
	}
	return 0;
}
int myServer(int s_port_no,int type)
{
	servernode dataServer;
	struct sockaddr_in server_addr,client_addr;
	int sin_size;
	char comarr[100];
	char run[1024];
	if(type == 0)
	{
		if ((dataServer.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			printf("\nError : socket can't be stablished\n");
			perror("Socket");
			return 1;
		}
	}
	if(type==1)
	{
		if ((dataServer.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			printf("\nError : socket can't be stablished\n");
			perror("Socket");
			return 1;
		}
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(s_port_no);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero),8);
	if (bind(dataServer.sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) 
	{
		perror("\nError : Unable to bind\n");
		return 0;
	}
	if(type==0)
	{
		if (listen(dataServer.sock, 5) == -1)
		{
			perror("Listen");
			return 0;
		}
	}
	printf("Server Waiting for client on port %d\n $ ",s_port_no);
	fflush(stdout);
	while(1)
	{

		sin_size = sizeof(struct sockaddr_in);
		socklen_t * a = (socklen_t *) &sin_size;
		if(type==0)
		{
			while((dataServer.connection = accept(dataServer.sock, (struct sockaddr *)&client_addr,a))<0)
			{
				printf("Connection refused\n");
				sleep(3);
			}
			printf("Connection received from (%s , %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
			printf("\n");
		}
		while(1)
		{
			if(type==0)
				dataServer.bytesRecieved = recv(dataServer.connection,Global.server_recv_data,1024,0);
			if(type==1)
				dataServer.bytesRecieved = recvfrom(dataServer.sock,Global.server_recv_data,1024,0,(struct sockaddr *)&client_addr, a);
			Global.server_recv_data[dataServer.bytesRecieved] = '\0';
			int check_ = check_close_connection(dataServer);
			if(check_ == 0)
			{
				printf("\n\n");
				printf("Received Command = %s\n" , Global.server_recv_data);
				printf("$");
				fflush(stdout);
				int coun=0;
				coun=tokenise2(Global.server_recv_data);

				if(coun<2)
				{
					printf("Invalid Global.command. Provide more arguments\n");
					continue;
				}				
				if((strcmp(Global.recv_command[0],"FileHash")==0) && (strcmp(Global.recv_command[1],"CheckAll")==0))
				{
					if(coun>2)
					{
						printf("Invalid Command.Need just 2 arguments\n");
						continue;
					}
					if(type==0)
					{
						system("find . -type f -exec sh -c 'printf \"%s %s \n\" \"$(ls -l --time-style=+%Y%m%d%H%M%S $1 )\" \"$(md5sum $1 | cut -d\" \" -f1)\"' '' '{}' '{}' \\; | awk '{print $7, $6, $8}' > checkallfile");
						FILE *fp=fopen("checkallfile","r+");
						if(fp==NULL)
						{
							printf("File doesnt exist\n");
							continue;
						}
						server_function(fp,dataServer.connection);
						system("rm checkallfile");

					}
					else 
					{
						system("find . -type f -exec sh -c 'printf \"%s %s \n\" \"$(ls -l --time-style=+%Y%m%d%H%M%S $1 )\" \"$(md5sum $1 | cut -d\" \" -f1)\"' '' '{}' '{}' \\; | awk '{print $7, $6, $8}' > checkall");

						FILE *fp = fopen("checkall","r");
						if(fp == NULL)
						{
							printf("File doesnot exists\n");
							continue;
						}
						memset(Global.server_send_data,0,1024);
						int byter;
						int sentn;
						while(!feof(fp))
						{
							byter = fread(Global.server_send_data,1,1024,fp);
							Global.server_send_data[byter] = '\0';
							sentn = sendto(dataServer.sock, Global.server_send_data, 1024, 0,(struct sockaddr *)&client_addr,a);
							//printf("%s",Global.server_send_data);
						}
						memset(Global.server_send_data,0,1024);
						char end[]= "End Of File";
						strcpy(Global.server_send_data,end);
						sendto(dataServer.sock, Global.server_send_data, 1024, 0,(struct sockaddr *)&client_addr,a);
						fclose(fp);
						system("rm checkallfile");


						//printf("recvfrom_data after check all : %s \n",Global.server_recv_data);
					}
				}
				else if((strcmp(Global.recv_command[0],"FileHash")==0) && (strcmp(Global.recv_command[1],"Verify")==0))
				{
					if(coun!=3)
					{
						printf("Invalid Global.command. Provide more arguments\n");
						continue;
					}
					strncpy(comarr,Global.recv_command[2],100);
					//	printf("DEBUG--%s\n",Global.command[2]);
					//		printf("Verifying %s\n",comarr);

					strcpy(run,"openssl md5 ");
					strcat(run,comarr);
					strcat(run," | cut -d\" \" -f2 > md5");
					system(run);
					strcpy(run,"date -r ./");
					strcat(run,comarr);
					strcat(run," +%Y%m%d%H%M%S > date");
					system(run);
					system("paste md5 date > verify");
					system("rm md5 date");

					FILE *fp = fopen("verify","r+");
					if(fp == NULL)
					{
						printf("File doesnot exists\n");
						continue;
					}

					memset(Global.server_send_data,0,1024);
					int byter,sentn;
					while(!feof(fp))
					{
						//printf("entered while loop\n");
						byter = fread(Global.server_send_data,1,1024,fp);
						Global.server_send_data[byter] = '\0';
						if(type==0)
							sentn = write(dataServer.connection,Global.server_send_data,1024);
						else
							sentn = sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					}
					memset(Global.server_send_data,0,1024);
					char end[]= "End Of File";
					strcpy(Global.server_send_data,end);
					//printf("NOw server data is %s\n",Global.server_send_data);
					if(type==0)
						write(dataServer.connection,Global.server_send_data,1024);
					else
						sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					fclose(fp);
					system("rm verify");


				}
				else if((strcmp(Global.recv_command[0],"IndexGet")==0) && (strcmp(Global.recv_command[1],"LongList")==0))
				{
					if(coun!=2)
					{
						printf("Invalid Global.command. Provide more arguments\n");
						continue;
					}

					system("find . -printf '%p %TY-%Tm-%Td %TH:%Tm %kKB \n' > longlist");

					FILE *fp = fopen("longlist","r+");
					if(fp == NULL)
					{
						printf("File doesnot exist\n");
						continue;
					}

					memset(Global.server_send_data,0,1024);
					int byter,sentn;
					while(!feof(fp))
					{
						byter = fread(Global.server_send_data,1,1024,fp);
						Global.server_send_data[byter] = '\0';
						if(type==0)
							sentn = write(dataServer.connection,Global.server_send_data,1024);
						else
							sentn = sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					}
					memset(Global.server_send_data,0,1024);
					char end[100];
					strcpy(end,"End Of File");
					strcpy(Global.server_send_data,end);
					if(type==0)
						write(dataServer.connection,Global.server_send_data,1024);
					else
						sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					fclose(fp);
					system("rm longlist");

					//printf("recv_data after longlist : %s \n",Global.server_send_data);
				}
				else if((strcmp(Global.recv_command[0],"IndexGet")==0) && (strcmp(Global.recv_command[1],"RegEx")==0))
				{
					if(coun!=3)
					{
						printf("Invalid Global.command. Provide more arguments\n");
						continue;
					}
					char comarr[100];
					char run[100];

					strncpy(comarr,Global.recv_command[2],100);
					strcpy(run,"ls ");
					strcat(run,comarr);
					strcat(run," > regex");
					system(run);


					FILE *fp = fopen("regex","r+");
					if(fp == NULL)
					{
						printf("File doesnot exist\n");
						continue;
					}
					memset(Global.server_send_data,0,1024);
					int byter,sentn;
					while(!feof(fp))
					{
						byter = fread(Global.server_send_data,1,1024,fp);
						Global.server_send_data[byter] = '\0';
						if(type==0)
							sentn = write(dataServer.connection,Global.server_send_data,1024);
						else
							sentn=sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					}
					memset(Global.server_send_data,0,1024);
					char end[1000];
					strcpy(end,"End Of File");
					strcpy(Global.server_send_data,end);
					if(type==0)
						write(dataServer.connection,Global.server_send_data,1024);
					else
						sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					fclose(fp);
					system("rm regex");
				}
				else if((strcmp(Global.recv_command[0],"IndexGet")==0) && (strcmp(Global.recv_command[1],"ShortList")==0))
				{
					int cntr=0;
					char buff[1000],copy[1000];
					int place=0;
					if(coun!=4)
					{
						printf("Invalid Global.command. Provide more arguments\n");
						continue;
					}
					char comarr[100];
					strncpy(comarr,Global.command[2],100);
					system("ls -l --time-style=+%Y%m%d%H%M%S -t > listing");

					FILE *fpr;
					FILE *fpr2;

					fpr = fopen( "listing", "r+" );
					fpr2 = fopen( "shortlist", "w+" );

					char *pch;

					while ( fgets( buff, 1000, fpr ) != NULL )
					{	
						if(cntr!=0 && cntr!=1)
						{
							strcpy(copy,buff);
							place=0;
							pch = strtok (buff," ");
							while (pch != NULL)
							{
								if(place==5)
								{
									if(strcmp(pch,Global.recv_command[2])>0 && strcmp(pch,Global.recv_command[3])<0)
									{
										fprintf(fpr2,"%s",copy);
									}
								}
								place++;
								pch = strtok (NULL," ");
							}
						}
						cntr++;
					}
					fclose( fpr );
					fclose( fpr2 );

					FILE *fp = fopen("shortlist","r+");
					if(fp == NULL)
					{
						printf("File doesnot exist\n");
						continue;
					}
					memset(Global.server_send_data,0,1024);
					int byter,sentn;
					while(!feof(fp))
					{
						byter = fread(Global.server_send_data,1,1024,fp);
						Global.server_send_data[byter] = '\0';
						if(type==0)
							sentn = write(dataServer.connection,Global.server_send_data,1024);
						else
							sentn=sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					}
					memset(Global.server_send_data,0,1024);
					char end[1000];
					strcpy(end,"End Of File");
					strcpy(Global.server_send_data,end);
					if(type==0)
						write(dataServer.connection,Global.server_send_data,1024);
					else
						sendto(dataServer.sock,Global.server_send_data,1024,0,(struct sockaddr *)&client_addr,a);
					fclose(fp);
					system("rm listing shortlist");
				}
				else
					printf("Invalid Command\n");
			}
		}
		fflush(stdout);

	}

	close(dataServer.sock);
	return 0;
}
int myClient(int c_port_no,int type)
{
	nodeclient dataClient;
	struct hostent *host;
	struct sockaddr_in server_addr;
	host = gethostbyname("127.0.0.1");
	if(type==0)
	{
		if ((dataClient.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("Socket");
			return 1;
		}
	}
	if(type==1)
	{
		if ((dataClient.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(c_port_no);
	memcpy(&server_addr.sin_addr,host->h_addr,host->h_length);
	bzero(&(server_addr.sin_zero),8);
	int con;
	int sin_size = sizeof(struct sockaddr_in);
	socklen_t * a = (socklen_t *) &sin_size;
	if(type==0)
	{
		while((con=connect(dataClient.sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr))) == -1)
		{
			printf("Connection failed\n");
			sleep(3);
		}
		printf("\n Client Connected to Port No %d\n",c_port_no);
	}
	char c;
	scanf("%c",&c);
	printf("$\n");
	take_input();
	//printf("%s",Global.complete_command);
	while(strcmp(Global.complete_command,"Exit")!=0 && strcmp(Global.complete_command,"q")!=0 && strcmp(Global.complete_command,"Q")!=0 && strcmp(Global.complete_command,"exit")!=0)
	{
		if(type==0)
			send(dataClient.sock,Global.complete_command,strlen(Global.complete_command),0);
		else
			sendto(dataClient.sock,Global.complete_command,strlen(Global.complete_command),0,(struct sockaddr *)&server_addr,a);
		if(strcmp(Global.complete_command,"")==0)
			printf("INVALID COMMAND\n",Global.complete_command);
		int co=0;
		co=tokenise1(Global.complete_command);
		if(co<2)
		{
			printf("Invalid Global.command. Provide more arguments\n");
			printf("\n$");
			take_input();
			continue;
		}
		if((strcmp(Global.command[0],"FileHash")==0) && (strcmp(Global.command[1],"Verify")==0))
		{
			if(co!=3)
			{
				printf("Invalid Global.command. Provide more arguments\n");
				printf("\n$");
				take_input();
				continue;
			}
			while(1)
			{
				if(type==0)
					dataClient.bytesReceived=recv(dataClient.sock, dataClient.recvData,1024,0);
				else
					dataClient.bytesReceived=recvfrom(dataClient.sock,dataClient.recvData,1024,0,(struct sockaddr *)&server_addr,a);
				dataClient.recvData[dataClient.bytesReceived]='\0';
				if(strcmp(dataClient.recvData,"End Of File")==0)
					break;
				fwrite(dataClient.recvData,1,dataClient.bytesReceived,stdout);
			}
		}
		else if((strcmp(Global.command[0],"FileHash")==0) && (strcmp(Global.command[1],"CheckAll")==0))
		{
			if(co!=2)
			{
				printf("Invalid Global.command. Provide more arguments\n");
				printf("\n$");
				take_input();
				continue;
			}
			while(1)
			{
				if(type==0)
					dataClient.bytesReceived=recv(dataClient.sock, dataClient.recvData,1024,0);
				else
					dataClient.bytesReceived=recvfrom(dataClient.sock,dataClient.recvData,1024,0,(struct sockaddr *)&server_addr,a);
				dataClient.recvData[dataClient.bytesReceived]='\0';
				if(strcmp(dataClient.recvData,"End Of File")==0)
					break;
				fwrite(dataClient.recvData,1,dataClient.bytesReceived,stdout);
			}
		}
		else if((strcmp(Global.command[0],"IndexGet")==0) && (strcmp(Global.command[1],"LongList")==0))
		{
			if(co!=2)
			{
				printf("Invalid Global.command. Provide more arguments\n");
				printf("\n$");
				take_input();
				continue;
			}
			while(1)
			{
				if(type==0)
					dataClient.bytesReceived=recv(dataClient.sock, dataClient.recvData,1024,0);
				else
					dataClient.bytesReceived=recvfrom(dataClient.sock,dataClient.recvData,1024,0,(struct sockaddr *)&server_addr,a);
				dataClient.recvData[dataClient.bytesReceived]='\0';
				if(strcmp(dataClient.recvData,"End Of File")==0)
					break;
				fwrite(dataClient.recvData,1,dataClient.bytesReceived,stdout);
			}
		}
		else if((strcmp(Global.command[0],"IndexGet")==0) && (strcmp(Global.command[1],"RegEx")==0))
		{
			if(co!=3)
			{
				printf("Invalid Global.command. Provide more arguments\n");
				printf("\n$");
				take_input();
				continue;
			}
			while(1)
			{
				if(type==0)
					dataClient.bytesReceived=recv(dataClient.sock, dataClient.recvData,1024,0);
				else
					dataClient.bytesReceived=recvfrom(dataClient.sock,dataClient.recvData,1024,0,(struct sockaddr *)&server_addr,a);
				dataClient.recvData[dataClient.bytesReceived]='\0';
				if(strcmp(dataClient.recvData,"End Of File")==0)
					break;
				fwrite(dataClient.recvData,1,dataClient.bytesReceived,stdout);
			}
		}
		else if((strcmp(Global.command[0],"IndexGet")==0) && (strcmp(Global.command[1],"ShortList")==0))
		{
			if(co!=4)
			{
				printf("Invalid Global.command. Provide more arguments\n");
				printf("\n$");
				take_input();
				continue;
			}
			while(1)
			{
				if(type==0)
					dataClient.bytesReceived=recv(dataClient.sock, dataClient.recvData,1024,0);
				else
					dataClient.bytesReceived=recvfrom(dataClient.sock,dataClient.recvData,1024,0,(struct sockaddr *)&server_addr,a);
				dataClient.recvData[dataClient.bytesReceived]='\0';
				if(strcmp(dataClient.recvData,"End Of File")==0)
					break;
				fwrite(dataClient.recvData,1,dataClient.bytesReceived,stdout);
			}
		}
		else
			printf("Invalid Command\n");

		printf("\n$");
		take_input();
	}
	return 0;

}
int main()
{
	if(flag_init == 0)
	{
		flag_init = 1;
		global_initialize();
	}
	node dataForProtocol;
	char type[10];
	printf("Input : Port Number For Server :");
	scanf("%d",&dataForProtocol.serverPortNumber);
	printf("Input : Port Number For Client :");
	scanf("%d",&dataForProtocol.clientPortNumber);
	printf("Input : Protocol used ['tcp'/'udp']");
	scanf("%s",type);
	/*check type of protocol 0 is tcp and 1 is udp*/
	if(strcmp(type,"tcp") == 0)
	{
		dataForProtocol.typeProtocol = 0;
	}
	else if(strcmp(type,"udp") == 0)
	{
		dataForProtocol.typeProtocol = 1;
	}
	else
	{
		printf("Error : Invalid Protocol type");
		exit(0);
	}
	dataForProtocol.processID = fork();
	if(dataForProtocol.processID == -1)
	{
		printf("Error in creating Fork\n");
		exit(0);
	}
	if (dataForProtocol.processID==0)
	{
		myServer(dataForProtocol.serverPortNumber,dataForProtocol.typeProtocol);
	}
	else
	{
		myClient(dataForProtocol.clientPortNumber,dataForProtocol.typeProtocol);

	}
	kill(dataForProtocol.processID,SIGQUIT);
	return 0;
}

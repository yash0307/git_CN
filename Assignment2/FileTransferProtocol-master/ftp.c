//////////////////////////////////////////////
//											//
// Author :  & Priyanka Suresh	//
// 		  	 201202164	  201256152			//
//											//
//////////////////////////////////////////////


#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctype.h>
#include <fstream>
#include <stddef.h>
#include <dirent.h>
#include <signal.h>

using namespace std;

#define LEN_PATH 256
#define LEN_MD5 32

#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)

typedef struct file {

	char name[1024];
	char time[1024];
	int size;
	char type[100];
	char filemd5[LEN_MD5+1];
}fs;


//Global Variables:

//Server:
char serverSendData[1024];
char serverRecvData[1024];
fs serverFileStructure[1024];
int serverFileCount = 0;
char recvCompleteCommand[1024];
char recvCommand[20][100];
int recvCommandCount = 0;
char globalTime[100];

//Client.
char completeCommand[1024];
char command[20][100];
int commandCount = 0;
fs fileStructure[1024];

/* ---------------- Function to MD5 check sum -----------------------*/
int calcMD5(char *fname, char *sum)
{
	#define MD5SUM_CMD_FMT "md5sum %." STR(LEN_PATH) "s 2>/dev/null"
	char cmd[LEN_PATH + sizeof (MD5SUM_CMD_FMT)];
	sprintf(cmd, MD5SUM_CMD_FMT, fname);
	#undef MD5SUM_CMD_FMT

	FILE *p;
	p = popen(cmd, "r");
	if (p == NULL)
	{
		printf ("hua\n");
		return 0;
	}

	int i,c;
	for (i=0; i<LEN_MD5 && isxdigit(c = fgetc(p)); i++)
	{
		*sum++ = c;
	}

	*sum = '\0';
	pclose(p);

	return i == LEN_MD5;
}

/* ----------- Function to make sure file structure is updated ------------*/
void updateFiles()
{
	int i;
	DIR *dir;
	struct dirent *ep;

	dir = opendir("./");
	if (dir)
	{
		for(i=0; (ep = readdir(dir)); i++) 
		{
			//name
			strcpy(serverFileStructure[i].name,ep->d_name);

			struct stat details;
			stat(ep->d_name, &details);

			int size = details.st_size;
			//size
			serverFileStructure[i].size = size;

			char inCommand[100];

			strcpy(inCommand,"file ");
			strcat(inCommand, serverFileStructure[i].name);
			strcat(inCommand, "> filetype");
			system(inCommand);

			ifstream input;
			string line;
			input.open("filetype");
			getline(input,line);
			input.close();
			//type
			strcpy(serverFileStructure[i].type, line.c_str());

			//time
			strcpy(serverFileStructure[i].time, ctime(&details.st_mtime));
			//MD5
			calcMD5(serverFileStructure[i].name, serverFileStructure[i].filemd5);
		}
		serverFileCount = i-1;
		closedir(dir);
	}
	else
	{
		printf("\n Error : could not open directory.\n");
	}
}

/* ----------- Function to split a command to its entities -------*/
void parsePacket()
{
	int count =0;

	for (int i=0; i<strlen(serverRecvData); i++)
	{
		recvCompleteCommand[count++] = serverRecvData[i];
	}
	recvCompleteCommand[count++]='\0';
	recvCommandCount = 0;

	count = 0;

	for (int i=0; i<strlen(recvCompleteCommand); i++)
	{
		if (recvCompleteCommand[i] == ' ')
		{
			recvCommand[recvCommandCount][count++] = '\0';
			recvCommandCount++;
			count = 0;
			continue;
		}
		recvCommand[recvCommandCount][count] = recvCompleteCommand[i];
		count++;
	}
	recvCommand[recvCommandCount][count] = '\0';
	recvCommandCount++;
}

void scanForInput()
{
	char c;
	int count = 0;

	scanf("%c", &c);

	while(c!='\n')
	{
		completeCommand[count++] = c;
		scanf("%c",&c);
	}

	completeCommand[count++] = '\0';
	commandCount = 0;

	count = 0;

	for (int i=0 ; i<strlen(completeCommand); i++)
	{
		if (completeCommand[i] == ' ')
		{
			command[commandCount][count++] = '\0';
			commandCount++;
			count = 0;
			continue;
		}
		command[commandCount][count] = completeCommand[i];
		count++;
	}
	command[commandCount][count++] = '\0';
	commandCount++;

}

int actAsClient (int clientPortNo, char type[])
{
	char md5[LEN_MD5+1];
	char recvMD5[LEN_MD5+1];

	int sock, bytesRecieved;
	char sendData[1024], recvData[1024];
	int recvData_int;

	struct hostent *host;
	struct sockaddr_in server_addr;

	host = gethostbyname("127.0.0.1");
	if (!strcmp(type, "tcp"))
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
			printf("\n #Error : Unable to retrieve socket.\n");
			return 1;
		}
	}
	else if (!strcmp(type, "UDP"))
	{
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == -1)
		{
			printf("\n Error : Unable to retrieve socket.\n");
			return 1;
		}
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(clientPortNo);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);

	int sin_size = sizeof(struct sockaddr_in);
	socklen_t *temp = (socklen_t *) &sin_size;

	/* ----------------- Establish Connection --------------------------*/

	if (!strcmp(type, "tcp"))
	{
		if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
		{
			printf ("\n Error : Unable to connect to port.\n");
			return 2;
		}
		printf("\n Client connected to port %d\n",clientPortNo);
	}

	/* ----------------- Parse the input commands -----------------*/
	scanForInput();
	while(strcmp(command[0], "exit"))
	{
		if (!strcmp(command[0], "download"))
		{
			if (commandCount< 2)
			{
				printf("\n Error : missing arguments for download\n");
				scanForInput();
				continue;
			}
			else
			{
				if (!strcmp(type, "tcp"))
				{
					send (sock, completeCommand, strlen(completeCommand), 0);
				}
				else
				{
					sendto(sock, completeCommand, strlen(completeCommand), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
				}

				if (!strcmp(type, "tcp")) 
				{
					bytesRecieved = recv(sock, recvData, 1024, 0);
				}
				else
				{
					bytesRecieved = recvfrom(sock, recvData,1024,0,(struct sockaddr *)&server_addr, temp);
				}
				recvData[bytesRecieved] = '\0';

				if (strcmp(recvData, "no such file")!=0)
				{
					/*------- recieve the MD5 packet -----------*/
					if (!strcmp(type,"tcp"))
					{
						recv(sock, recvData, LEN_MD5+1, 0);
					}
					else
					{
						recvfrom(sock, recvData, LEN_MD5+1, 0,(struct sockaddr *)&server_addr, temp);
					}
					strcpy(recvMD5,recvData);
					/*-----start recieving the packets ------------*/

					// size of packet.
					if (!strcmp(type, "tcp"))
					{
						recv (sock, &recvData_int, sizeof(int), 0);
					}
					else
					{
						recvfrom(sock, &recvData_int, sizeof(int), 0,(struct sockaddr *)&server_addr, temp);
					}	
					//packet contents.
					if (!strcmp(type, "tcp"))
					{
						bytesRecieved = recv(sock, recvData, 1024, 0);
					}
					else
					{
						bytesRecieved = recvfrom (sock, recvData, 1024, 0,(struct sockaddr *)&server_addr, temp);
					}

					FILE *fp;
					fp = fopen(command[1], "w");

					printf ("\n#");

					while(strcmp(recvData,"end of file"))
					{
						printf("~");
						for (int i=0; i<recvData_int; i++)
						{
							fprintf(fp,"%c",recvData[i]);
						}

						if(!strcmp(type, "tcp"))
						{
							recv(sock, &recvData_int, sizeof(int), 0);
						}
						else
						{
							recvfrom(sock, &recvData_int, sizeof(int), 0, (struct sockaddr *)&server_addr, temp);
						}

						if (!strcmp(type,"tcp"))
						{
							bytesRecieved = recv(sock, recvData, 1024, 0);
						}
						else
						{
							bytesRecieved = recvfrom(sock, recvData, 1024, 0, (struct sockaddr *)&server_addr, temp);
						}
						recvData[bytesRecieved] = '\0';
					}
					printf("#\n");

					/* ---------------- MD5 sum check ------------------*/
					printf("Checking MD5 sum.\nMD5 sum of to be dowloaded file : %s\n", recvMD5);

					fclose(fp);

					if (!calcMD5(command[1], md5))
					{
						printf("Error : failed to calculate MD5sum\n");
					}
					else
					{
						printf("MD5sum of file : %s\n", md5);
					}

					if (!strcmp(md5, recvMD5))
					{
						printf("MD5sum matched. File Download Complete!\n");
					}
					else
					{
						printf("\n Error : MD5 check sum error.\n");
					}
				}
				else
				{
					printf("No such file or directory\n");
				}
			}
		}

		/*------------------------------indexGet----------------------------*/
		else if (!strcmp(command[0], "indexget"))
		{
			if (!strcmp(type,"tcp"))
			{
				send(sock, completeCommand,strlen(completeCommand),0);
			}
			else
			{
				sendto(sock, completeCommand, strlen(completeCommand), 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
			}

			/*-------num of file------------*/
			if (!strcmp(type, "tcp"))
			{
				recv(sock, &recvData_int, sizeof(recvData_int), 0);
			}
			else
			{
				recvfrom(sock, &recvData_int, sizeof(recvData_int), 0,(struct sockaddr *)&server_addr, temp);
			}

			int fileCount = recvData_int;

			for(int i=0; i<fileCount; i++)
			{
				if (!strcmp(type,"tcp"))
				{
					bytesRecieved = recv(sock, recvData,1024, 0);
				}
				else
				{
					bytesRecieved = recvfrom(sock, recvData, 1024, 0,(struct sockaddr *)&server_addr, temp);
				}
				recvData[bytesRecieved]='\0';

				strcpy(fileStructure[i].name, recvData);

				/*-----------type-----------*/
				if (!strcmp(type, "tcp"))
				{
					bytesRecieved = recv(sock, recvData,1024,0);
				}
				else
				{
					bytesRecieved = recvfrom(sock, recvData, 1024, 0,(struct sockaddr *)&server_addr, temp);
				}
				recvData[bytesRecieved]='\0';
				strcpy(fileStructure[i].type, recvData);

				/*--------size-------*/
				if (!strcmp(type, "tcp"))
				{
					recv(sock, &recvData_int,sizeof(recvData_int), 0);
				}
				else
				{
					recvfrom(sock, &recvData_int, sizeof(recvData_int),0,(struct sockaddr *)&server_addr, temp);
				}
				fileStructure[i].size = recvData_int;

				/*------last mod time----------*/
				if (!strcmp(type, "tcp"))
				{
					recv(sock, recvData, 1024, 0);
				}
				else
				{
					recvfrom(sock, recvData, 1024, 0,(struct sockaddr *)&server_addr,temp);
				}
				strcpy(fileStructure[i].time, recvData);
			}
			printf("\n\n");
			for (int i =0; i<fileCount;i++)
			{
				printf("Name : %s\nSize : %d\nType : %s\nTime : %s\n",fileStructure[i].name,fileStructure[i].size,fileStructure[i].type,fileStructure[i].time);
				printf("\n----------------------------\n\n");
			}
		}

		/*---------------------------------------Help------------------------------------------*/
		else if (!strcmp(command[0], "help"))
		{
			printf("\n###################### We are here to Help! ##########################\n\n");
			printf("download <file path>	: to download file present on peer's shared folder.\n");
			printf("upload <file path> 	: to upload a file onto the remote host server.\n");
			printf("upload <flag>      	: to set the sharing security. flag=>{allow,deny}\n");
			printf("indexget    		: to get files on the remote host server\n");
			printf("filehash <flag> <file>	: to view files on the remote host. flag=>{verify, checkall} file=>{filepath(if verify flag)}\n");
			printf("help                 	: to get a detailed list of all commands.\n");
			printf("exit               	: to close a connection/leave JushtShare.\n");
			printf("#######################################################################\n\n");
		}

		/*------------------------------ Upload --------------------------------------------*/
		else if (!strcmp(command[0], "upload"))
		{
			if (commandCount < 2)
			{
				printf ("\n Error : missing arguments.\n");
			}
			else
			{
				/*------------- self param setter ---------------------*/
				if (!strcmp(command[1],"allow") || !strcmp(command[1],"deny"))
				{
					FILE *up;
					up = fopen("upload_command","w");
					fprintf(up,"%s",command[1]);
					fclose(up);
				}

				/*----------------- upload file to remote host -------------------*/
				else
				{
					ifstream ifile(command[1]);
					if (ifile)
					{
						int fal;

						/*----------header -----------*/
						if (!strcmp(type,"tcp"))
						{
							send(sock, completeCommand, 1024, 0);
						}
						else
						{
							sendto(sock, completeCommand, 1024, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
						}

						/*------ recieve permission -----------------*/
						if (!strcmp(type, "tcp"))
						{
							bytesRecieved = recv(sock, recvData, 1024, 0);
						}
						else
						{
							bytesRecieved = recvfrom(sock, recvData, 1024, 0, (struct sockaddr *)&server_addr, temp);
						}
						recvData[bytesRecieved] = '\0';

						if (!strcmp(recvData,"denied"))
						{
							printf("#Permission denied\n");
						}
						else
						{
							printf("#permission granted\n");

							if (!calcMD5(command[1], md5))
							{
								printf("\n Error : md check sum failed\n");
							}
							else
							{
								printf ("Check sum for file : %s\n",md5);
							}
							int count;
							char c;
							FILE *fp = fopen(command[1], "r");

							/*---------SEND md5 PACKET ------------*/
							if (!strcmp(type, "tcp"))
							{
								send(sock, md5, LEN_MD5+1, 0);
							}
							else
							{
								sendto(sock, md5, LEN_MD5+1, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
							}

							while(fscanf(fp,"%c",&c)!=EOF)
							{
								count = 0;
								serverSendData[count] = c;
								count++;

								while (count <1024 && fscanf(fp, "%c", &c)!=EOF)
								{
									serverSendData[count] = c;
									count++;
								}

								/* --- send packet size ----*/
								if (!strcmp(type, "tcp"))
								{
									send(sock, &count, sizeof(int), 0);
								}
								else
								{
									sendto(sock, &count, sizeof(int), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
								}

								/*---- send data ---- */
								if (!strcmp(type, "tcp"))
								{
									send (sock, serverSendData, 1024, 0);
								}
								else
								{
									sendto (sock, serverSendData, 1024, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
								}
							}
							/* ----- eof -----*/
							if (strcmp(type,"tcp"))
							{
								send(sock, &fal, sizeof(int), 0);
							}
							else
							{
								sendto(sock, &fal, sizeof(int), 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
							}
							if (strcmp(type,"tcp"))
							{
								send(sock, "end of file", 1024, 0);
							}
							else
							{
								sendto(sock, "end of file", 1024, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
							}
						}
					}
					else if (strcmp(completeCommand,""))
					{
						printf("\n Error : No such file or directory.\n");
					}
				}
			}
		}


		/*------------------------------ File Hash -------------------------------------------*/
		else if (!strcmp(command[0], "filehash"))
		{
			/*--------------header------------*/
			if(!strcmp(type, "tcp"))
			{
				send(sock, completeCommand, sizeof(completeCommand), 0);
			}
			else
			{
				sendto(sock, completeCommand, sizeof(completeCommand), 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
			}

			/*----------- packet count -----------*/
			if (!strcmp(type, "tcp"))
			{
				recv(sock, &recvData_int, sizeof(recvData_int), 0);
			}
			else
			{
				recvfrom(sock, &recvData_int, sizeof(recvData_int), 0, (struct sockaddr *)&server_addr, temp);
			}
			/*---------------- for each file --------------*/
			int t = recvData_int;
			for (int i=0; i<t; i++) 							// recvData_int is file count
			{
				/*------ name ---------*/
				if (!strcmp(type, "tcp"))
				{
					bytesRecieved = recv(sock, recvData, 1024, 0);
				}
				else
				{
					bytesRecieved = recvfrom(sock, recvData, 1024, 0, (struct sockaddr *)&server_addr, temp);
				}
				recvData[bytesRecieved] = '\0';
				strcpy(fileStructure[i].name, recvData);

				/* --------size---------*/
				if (!strcmp(type, "tcp"))
				{
					recv(sock, &fileStructure[i].size, sizeof(int), 0);
				}
				else
				{
					recvfrom(sock, &fileStructure[i].size, sizeof(int), 0, (struct sockaddr *)&server_addr, temp);
				}

				/* ---------type----------*/
				if (!strcmp(type, "tcp"))
				{
					bytesRecieved = recv(sock, recvData, 1024, 0);
				}
				else
				{
					bytesRecieved = recvfrom(sock, recvData, 1024, 0, (struct sockaddr *)&server_addr, temp);
				}
				recvData[bytesRecieved] = '\0';
				strcpy(fileStructure[i].type, recvData);

				/*-------- time -----------*/
				if (!strcmp(type, "tcp"))
				{
					bytesRecieved = recv(sock, recvData, 1024, 0);
				}
				else
				{
					bytesRecieved = recvfrom(sock, recvData, 1024, 0, (struct sockaddr *)&server_addr, temp);
				}
				recvData[bytesRecieved] = '\0';
				strcpy(fileStructure[i].time,recvData);

				/* --------MD5 sum--------*/
				if (!strcmp(type, "tcp"))
				{
					recv(sock, fileStructure[i].filemd5, LEN_MD5+1, 0);
				}
				else
				{
					recvfrom(sock, fileStructure[i].filemd5, LEN_MD5+1, 0, (struct sockaddr *)&server_addr, temp);
				}
			}
			/* -------- for 1 file --------- */
			if (!strcmp(command[1],"verify"))
			{
				if (command[2] == "")
				{
					printf("\n Error : arguments missing.\n");
					continue;
				}
				int i;
				for (i=0; i<t; i++)
				{
					if (!strcmp(command[2],fileStructure[i].name))
					{
						printf("\nFile : %s\nSize: %d\nType : %s\nLast-Edited : %s\nMD5sum : %s\n\n",fileStructure[i].name,fileStructure[i].size,fileStructure[i].type,fileStructure[i].time,fileStructure[i].filemd5);
						break;
					}
				}
				if (i==t)
				{
					printf ("\n Error : no such file %s\n",fileStructure[i].name);
				}
			}
			else if (!strcmp(command[1], "checkall"))
			{
				int i;
				for (i=0; i<t; i++)
				{
					printf("\nFile : %s\nSize: %d\nType : %s\nLast-Edited : %s\nMD5sum : %s\n\n",fileStructure[i].name,fileStructure[i].size,fileStructure[i].type,fileStructure[i].time,fileStructure[i].filemd5);
					printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
				}
			}
			else
			{
				printf("\n Error : missing arguments.\n");
				continue;
			}
		}
		else
		{
			if (strcmp(completeCommand, ""))
			{
				printf("\n Error : Invalid command. Enter 'help' for a detailed list of valid commands.\n");
			}
		}
		scanForInput();
	}
	return 0;
}

/* ---------- When this user acts as a server ---------------------*/
int actAsServer(int serverPortNo, char type[])
{
	char md5[LEN_MD5+1];
	char recvMD5[LEN_MD5+1];

	int sock, connected, bytesRecieved;

	struct sockaddr_in server_addr, client_addr;
	int sin_size;

	int serverRecvData_int;
	int junk_int = 0;

	if (!strcmp(type, "tcp"))
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
			printf("\n #Error: Unable to retrieve socket.\n");
			return 1;
		}
	}
	else if (!strcmp(type, "udp")) 
	{
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == -1)
		{
			printf("\n #Error: Unable to retrieve socket.\n");
			return 1;
		}
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverPortNo);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
	{
		printf ("\n #Error : Unable to bind socket.\n");
		return 2;
	}

	if (!strcmp(type, "tcp"))
	{
		if (listen(sock, 10) == -1)
		{
			printf("\n #Error : Failed to listen.\n");
			return 3;
		}
	}

	printf("\nWaiting for client port %d to respond...\n $ ",serverPortNo);

	/* --------------- Check for requests --------------------------*/
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		socklen_t *temp = (socklen_t *) &sin_size;
		if (!strcmp(type, "tcp"))
		{
			connected = accept(sock, (struct sockaddr *)&client_addr, temp);
			printf("\nConnection established with %s - %d\n $ ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		}

		while(1)
		{
			if (!strcmp(type, "tcp"))
			{
				bytesRecieved = recv(connected, serverRecvData, 1024, 0);
			}
			else if (!strcmp(type, "udp"))
			{
				bytesRecieved = recvfrom(sock, serverRecvData, 1024, 0,(struct sockaddr *)&client_addr, temp);
			}

			serverRecvData[bytesRecieved] = '\0';

			/* ---------------------- Parsing the recieved packet --------------------------------------------*/

			parsePacket();			//split command to basic entities.			

			if (!bytesRecieved)
			{
				printf("Connection closed\n $ ");
				close(connected);
				break;
			}
			if (!strcmp(serverRecvData, "exit"))
			{
				printf("Connection closed\n $ ");
				close(connected);
				break;
			}
			else
			{
				printf("\nRequest : %s\n $ ", recvCompleteCommand);

				/*---------------------------- File hash -----------------------*/
				if (!strcmp(recvCommand[0], "filehash"))
				{
					//Make sure files are updated.
					updateFiles();
					/*------- send no of files ------*/
					if (!strcmp(type, "tcp"))
					{
						send(connected, &serverFileCount, sizeof(int), 0);
					}
					else
					{
						sendto(sock, &serverFileCount, sizeof(int), 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					}
					/* ------------ for each file now ---------------*/

					int i;
					for (i=0; i<serverFileCount ; i++)
					{
						/* ------- name ---------*/
						if (!strcmp(type, "tcp"))
						{
							send(connected, serverFileStructure[i].name, 1024, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].name, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/* -------- size ---------*/
						if (!strcmp(type, "tcp"))
						{
							send(connected, &serverFileStructure[i].size, sizeof(int), 0);
						}
						else
						{
							sendto(sock, &serverFileStructure[i].size, sizeof(int), 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/*--------- type --------*/
						if (!strcmp(type, "tcp"))
						{
							send (connected, serverFileStructure[i].type, 1024, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].type, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/*--------- time ---------*/
						if (!strcmp(type, "tcp"))
						{
							send (connected, serverFileStructure[i].time, 1024, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].time, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/*--------- MD5 sum ---------*/
						if (!strcmp(type, "tcp"))
						{
							send (connected, serverFileStructure[i].filemd5, LEN_MD5+1, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].filemd5, LEN_MD5+1, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
				}

				else if (!strcmp(recvCommand[0], "indexget"))
				{
					updateFiles();

					if (!strcmp(type,"tcp"))
					{
						send(connected, &serverFileCount, sizeof(int), 0);
					}
					else
					{
						sendto(sock, &serverFileCount, sizeof(int), 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					}

					for (int i=0; i<serverFileCount; i++)
					{
						if (!strcmp(type, "tcp"))
						{
							send(connected, serverFileStructure[i].name, 1024, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].name, 1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/*-----type------*/
						if (!strcmp(type, "tcp"))
						{
							send(connected, serverFileStructure[i].type, 1024, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].type, 1024, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/*------size-----*/
						if (!strcmp(type, "tcp"))
						{
							send(connected, &serverFileStructure[i].size, sizeof(int), 0);
						}
						else
						{
							sendto(sock, &serverFileStructure[i].size, sizeof(int), 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/*-----time-----*/
						if(!strcmp(type, "tcp"))
						{
							send(connected, serverFileStructure[i].time, 1024, 0);
						}
						else
						{
							sendto(sock, serverFileStructure[i].time, 1024, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
				}

				/*------------------------Upload---------------------------------*/
				else if (!strcmp(recvCommand[0], "upload"))
				{
					/*-----check permission----------*/
					char perm[20];

					FILE *up;
					up = fopen("upload_command","r");

					fscanf(up, "%s",perm);
					fclose(up);

					if (!strcmp(perm,"deny"))
					{
						if (!strcmp(type, "tcp"))
						{
							send(connected, "denied", 1024, 0);
						}
						else
						{
							sendto(sock, "denied", 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
					else if (!strcmp(perm,"allow"))
					{
						/*---- send permission------*/
						if (!strcmp(type, "tcp"))
						{
							send(connected, "allowed", 1024, 0);
						}
						else
						{
							sendto(sock ,"allowed", 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/* ----get MD% sum -----*/
						if (!strcmp(type, "tcp"))
						{
							recv(connected, serverRecvData, LEN_MD5+1, 0);
						}
						else
						{
							recvfrom(sock, serverRecvData, LEN_MD5+1, 0, (struct sockaddr *)&client_addr, temp);
						}

						/*----- get packet size -----*/
						if (!strcmp(type, "tcp"))
						{
							recv(connected, &serverRecvData_int, sizeof(int), 0);
						}
						else
						{
							recvfrom(sock, &serverRecvData_int, sizeof(int), 0, (struct sockaddr *)&client_addr, temp);
						}

						/*------------ get first pack -------*/
						if (!strcmp(type, "tcp"))
						{
							bytesRecieved = recv(connected, serverRecvData, 1024, 0);
						}
						else
						{
							bytesRecieved = recvfrom(sock, serverRecvData, 1024, 0, (struct sockaddr *)&client_addr, temp);
						}
						serverRecvData[bytesRecieved] = '\0';

						FILE *fp = fopen(recvCommand[1], "w");
						while(strcmp(serverRecvData,"end of file"))
						{
							printf("#");
							for (int i=0; i<serverRecvData_int;i++)
							{
								fprintf(fp, "%c", serverRecvData[i]);
							}

							/*---- get file size ----*/
							if (!strcmp(type,"tcp"))
							{
								recv(connected, &serverRecvData_int, sizeof(int), 0);
							}
							else
							{
								recvfrom(sock, &serverRecvData_int, sizeof(int), 0,(struct sockaddr *)&client_addr, temp);
							}

							/*----- get data ----*/

							if (!strcmp(type, "tcp"))
							{
								bytesRecieved = recv(connected, serverRecvData, 1024, 0);
							}
							else
							{
								bytesRecieved = recvfrom(sock, serverRecvData, 1024, 0, (struct sockaddr *)&client_addr, temp);
							}
							serverRecvData[bytesRecieved] = '\0';
						}

						printf("md5sum of uploaded file : %s\n",recvMD5);
						if (calcMD5(recvCommand[1], md5))
						{
							printf("\n Error : md5 check sum failed\n");
						}
						else
						{
							printf("md5sum of recieved file : %s\n",md5);
						}
						if (strcmp(md5, recvMD5))
						{
							printf("\n Error : md5 check sum match failed\n");
						}
						printf("md5sum match succes!\n");
						printf("File Uploaded Successfully!\n");
						fclose(fp);
					}
				}

				/* ------------------------- Download --------------------------*/
				else if (!strcmp(recvCommand[0], "download"))
				{
					/*------------------------------- Check if the file exists --------------------*/
					ifstream ifile(recvCommand[1]);
					if (ifile)
					{
						if (!strcmp(type, "tcp"))
						{
							send (connected, "file exists",1024, 0);
						}
						else
						{
							sendto(sock, "file exists",1024, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						/* ----------------------- Check for MD5 sum --------------------*/
						if (!calcMD5(recvCommand[1], md5))
						{
							printf ("\n Error : MD5 check sum failed.\n");
						}
						else
						{
							printf(" MD5sum for the file : %s\n",md5);
						}

						char c;
						int count;
						FILE *fp;
						fp = fopen(recvCommand[1], "r");

						/*------------------------------ Send MD5sum value to the client --------------*/
						if (!strcmp(type, "tcp"))
						{
							send (connected, md5, LEN_MD5+1,0);
						}
						else
						{
							sendto(sock, md5, LEN_MD5+1,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						while(fscanf(fp, "%c",&c)!=EOF)
						{
							count = 0;
							serverSendData[count++] = c;

							while(count < 1024 && fscanf(fp, "%c",&c)!=EOF)
							{
								serverSendData[count++] = c;
							}

							if (!strcmp(type, "tcp"))
							{
								send (connected, &count, sizeof(int), 0);
							}
							else
							{
								sendto(sock, &count, sizeof(int), 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
							}

							/* -------------- send file data ------------*/
							if (!strcmp(type, "tcp"))
							{
								send (connected, serverSendData, 1024,0);
							}
							else
							{
								sendto(sock, serverSendData, 1024, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
							}
						}

						/*------------------------- header for EOF --------------*/
						if (!strcmp(type, "tcp"))
						{
							send(connected, &junk_int, sizeof(int), 0);
						}
						else
						{
							sendto(sock, &junk_int,  sizeof(int), 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}

						if (!strcmp(type, "tcp"))
						{
							send(connected, "end of file", 1024, 0);
						}
						else
						{
							sendto (sock, "end of file", 1024, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
					else
					{
						if (!strcmp(type, "tcp"))
						{
							send (connected, "no such file",1024,0);
						}
						else
						{
							sendto (sock, "no such file",1024, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
				}
			}
		}
	}
	close(sock);
	return 0;
}



/*----------------------------- Main Function -----------------------------------*/

int main()
{
	int servPortNo, clientPortNo;
	char type[20];

	FILE *upload_file;
	upload_file = fopen("upload_command","w");
	fprintf(upload_file,"allow");
	fclose(upload_file);

	/* ------------------------------ VFX :P -------------------------------------*/
	printf ("####################################################################################################################\n");
	printf ("#                                         Welcome to JushtShare                                                    #\n");
	printf ("#                                                                                                                  #\n");
	printf ("#                                                                                  Made by : Mohit & Priyanka      #\n");
	printf ("####################################################################################################################\n");
	printf ("\n\n");

	/* ---------------- Set-up variables fetch ----------------------*/

	printf("Port to listen to : ");
	scanf("%d",&servPortNo);

	printf("Port to send data : ");
	scanf("%d",&clientPortNo);

	printf("Type of protocol (tcp/udp) : ");
	scanf("%s",type);

	printf("\n#### For a complete list of commands, type 'help'.\n\n");

	/* ----------------- Start the functioning ----------------------*/

	pid_t pid;

	pid = fork();

	if (pid == -1)
	{
		printf ("\n #Error forking process\n");
		exit(1);
	}
	if (pid == 0)									//Server on child thread.
	{
		actAsServer(servPortNo, type);
	}
	else											//Client on parent thread.
	{
		while(1)
		{
			int p;
			p = actAsClient(clientPortNo, type);
			if (p <= 0)
			{
				break;
			}
			sleep(1);								//Keep pinging at 1sec intervals.
		}
	}
	kill(pid, SIGQUIT);
	return 0;
}


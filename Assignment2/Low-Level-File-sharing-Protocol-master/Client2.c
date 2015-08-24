//Author : Sharad Gupta
#include <stdio.h>
#include <stdlib.h>
#include<signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include<arpa/inet.h>

pid_t pid;
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void cFiledownload(char x[],int *a);
void Filedownload(char x[],int *a);
void cFileupload(char x[],int *a);
void Fileupload(char x[],int *a);
void cFilehash(char x[], int *a);
void Filehash(char [],int *a);
void cIndexget(char x[],int *a);
void Indexget(char x[] ,int *a);
void manage_peer();
void manage_host();
void hashing(char l1[],int *ne);

int main(int argc, char *argv[])
{
	pid=fork();
	if(pid==0)
	manage_peer();
	else if(pid)
	manage_host();
	
	return 0;
}

void manage_peer()
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char read_buffer[1024],write_buffer[1024];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    else
	    printf("[Client] Socket created\n");
    
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5001);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero),8);

    while(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
	    sleep(1);
    }
    while(1)
   {
		bzero(write_buffer , 1024);
		printf("Enter the message : ");
		gets(write_buffer);
		printf("\n");
		if(strcmp(write_buffer , "Q")==0 || strcmp(write_buffer , "q")==0)
		{
    			n = write(sockfd,write_buffer,1024);
    			if (n < 0) 
        			 error("ERROR writing to socket");
			printf("[Client] Connection closed\n");
			kill(pid,SIGTERM);
			break;
		//	exit(0);
		}
		else if(strncmp(write_buffer,"IndexGet",8)==0)
		{
			cIndexget(write_buffer,&sockfd);
		}
		else if(strncmp(write_buffer,"FileHash",8)==0)
		{
			cFilehash(write_buffer,&sockfd);
		}
		else if(strncmp(write_buffer,"FileDownload",12)==0)
		{
			cFiledownload(write_buffer,&sockfd);
		}
		else if(strncmp(write_buffer,"FileUpload",10)==0)
		{
			cFileupload(write_buffer,&sockfd);
		}
		else
		{
    			n = write(sockfd,write_buffer, 1024);
    			if (n < 0) 
        			 error("ERROR writing to socket");
		}
   }
   close(sockfd);
   _exit(0);
    return ;
}

void manage_host()
{
	int sockfd, newsockfd;
	socklen_t cli_len;
	char read_buffer[1024];
	struct sockaddr_in servi_addr, cli_addr;
	int n,i;
	char commands[1000][100];
	int flag[1000],h=0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	else
		printf("[Server] Socket intialized \n");

	bzero((char *) &servi_addr, sizeof(servi_addr));
	servi_addr.sin_family = AF_INET;
	servi_addr.sin_addr.s_addr = INADDR_ANY;
	servi_addr.sin_port = htons(5002);

	if (bind(sockfd, (struct sockaddr *) &servi_addr, sizeof(servi_addr)) < 0) 
		error("ERROR on binding");
	else
		printf("[Server] Socket Binded to the Server Address\n");

	if(listen(sockfd,5) < 0) 
		error("ERROR in listening");

	printf("[Server] Server waiting for an client\n");
	fflush(stdout);
//	while(1)
//	{
		cli_len = sizeof(cli_addr);
		newsockfd = accept(sockfd,  (struct sockaddr *) &cli_addr, &cli_len);
		if (newsockfd < 0) 
			error("ERROR on accept");


		while(1)
		{
			bzero(read_buffer,1024);
			n= read(newsockfd, read_buffer , 1024);
			if (n < 0) 
				error("ERROR writing to socket");
			read_buffer[n]='\0';
			strcpy(commands[h],read_buffer);
			if(strcmp(read_buffer, "q")==0 || strcmp(read_buffer, "Q")==0)
			{
				printf("\nHere is the message: %s\n",read_buffer);
				kill(pid,SIGTERM);
				break;
				//exit(0);
			}
			else if(strncmp(read_buffer , "IndexGet",8)==0)
			{
				Indexget(read_buffer,&newsockfd);
			}
			else if(strncmp(read_buffer,"FileHash",8)==0)
			{
				Filehash(read_buffer,&newsockfd);
			}
			else if(strncmp(read_buffer,"FileDownload",12)==0)
			{
				Filedownload(read_buffer,&newsockfd);
			}
			else if(strncmp(read_buffer,"FileUpload",10)==0)
			{
				Fileupload(read_buffer,&newsockfd);
			}
			else
			{
				printf("\nMessage from peer: %s\n",read_buffer);
				flag[h]=1;
				h++;
			}
			fflush(stdout);
			while(waitpid(-1, NULL, WNOHANG) > 0);
		}
		close(newsockfd);
		printf("\n Connection closed by peer\n");
//	}
	close(sockfd);
	exit(1);
	return ;
}

void Indexget(char read_buffer[],int *newsockfd)
{
	char write_buffer[1024];
	bzero(write_buffer,1024);
	char l[8]="LongList",r[5]="RegEx",s[9]="ShortList";
	int t=0,i;
	for(i=0;i<8;i++)
		if(l[i]!=read_buffer[9+i])
			t=1;
	if(t==1)
	{
		for(i=0;i<5;i++)
			if(r[i]!=read_buffer[9+i])
				t=2;
	}
	if(t==2)
	{
		for(i=0;i<9;i++)
			if(s[i]!=read_buffer[9+i])
				t=3;
	}
	if(t==0)
	{
		system("touch Result");
		system("ls -l |grep -v ^d | tr -s ' ' | awk '{print $9\"\\t\"$5\"\\t\"$8}'| tail -n +2 > Result");
		bzero(write_buffer, 1024); 
		int f=0,flag=0;
		FILE *fs = fopen("Result", "r");
		if(fs == NULL)
		{
			error("ERROR: File not found");
			exit(1);
		}
		while((f= fread(write_buffer, sizeof(char),1024, fs)) > 0)
		{
			if(write(*newsockfd, write_buffer, 1024) < 0)
			{
				error("\nERROR: Writing to socket");
				exit(1);
				break;
			}
			bzero(write_buffer, 1024);
			flag=1;
		}
		if(flag==1)
		{
		bzero(write_buffer, 1024);
		strcpy(write_buffer,"END");
		if(write(*newsockfd, write_buffer, 1024) < 0)
		{
			error("\nERROR: Writing to socket");
			exit(1);
		}
		}
		system("rm -rf Result");

	}
	if(t==1)
	{
		char st[500],str[400];
		int k=0;
		i=15;
		while(i!=strlen(read_buffer))
		{
			st[k]=read_buffer[i];
			k++;
			i++;
		}
		st[k]='\0';
		strcpy(str,"find . -name \""); 
		strcat(str,st);
		strcat(str,"\" -exec ls -l {} \\; | awk '{print $9\"\\t\"$5}' | cut -c "" 3- > Result");
		system("touch Result");
		system(str);

		bzero(write_buffer, 1024); 
		int f=0,flag=0;
		FILE *fs = fopen("Result", "r");
		if(fs == NULL)
		{
			error("ERROR: File not found");
			exit(1);
		}
		while((f= fread(write_buffer, sizeof(char),1024, fs)) > 0)
		{
			if(write(*newsockfd, write_buffer, 1024) < 0)
			{
				error("\nERROR: Writing to socket");
				exit(1);
				break;
			}
			bzero(write_buffer, 1024);
			flag=1;
		}
		if(flag==1)
		{
		bzero(write_buffer, 1024);
		strcpy(write_buffer,"END");
		if(write(*newsockfd, write_buffer, 1024) < 0)
		{
			error("\nERROR: Writing to socket");
			exit(1);
		}
		system("rm -rf Result");
		}
	}
	if(t==2)
	{
		char st[400],str[400];
		int k=0,flag=0;
		i=19;
		while(read_buffer[i]!=' ')
		{
			st[k]=read_buffer[i];
			k++;
			i++;
		}
		st[k]='\0';
		k=0;
		i++;
		while(read_buffer[i]!='\0')
		{
			str[k]=read_buffer[i];
			k++;
			i++;
		}
		str[k]='\0';
		//printf("a%sa b%sb\n",st,str);
		system("touch fd");
		system("ls --full-time|grep -v ^d | awk '{print $9\"\\t\"$5\"\\t\"$7}' | tail -n +2 | sort -n > fd");
		FILE *file = fopen ( "fd", "r" );
		FILE *file1 = fopen("result","w");
		if ( file != NULL )
		{   
			char line [400]; 
			while ( fgets ( line, sizeof(line), file ) != NULL ) 
			{ 
				char te[400];
				i=0;
				k=0;
				while(line[i]!=' ')
					i++;
				i++;
		//		printf("%c\n",line[i]);
				while(line[i]!=' ')
					i++;
				i++;
		//		printf("%c\n",line[i]);
				while(i!=strlen(line)-1)
				{
					te[k]=line[i];
					k++;
					i++;
				}
				if(strcmp(te,st)>=0&&strcmp(te,str)<=0)
				{
					fputs(line,file1);
				}
			}   
			fclose ( file );
			fclose(file1);
		} 
		else
			error("Error in file opening");
		file1=fopen("result","r");
		if(file1!=NULL)
		{
		//	printf("send");
			int f=0;
		while((f= fread(write_buffer, sizeof(char),1024, file1)) > 0)
		{
			if(write(*newsockfd, write_buffer, 1024) < 0)
			{
				error("\nERROR: Writing to socket");
				exit(1);
				break;
			}
			bzero(write_buffer, 1024);
			flag=1;
		}
		if(flag==1)
		{
		bzero(write_buffer, 1024);
		strcpy(write_buffer,"END");
		if(write(*newsockfd, write_buffer, 1024) < 0)
		{
			error("\nERROR: Writing to socket");
			exit(1);
		}
		system("rm -rf fd");
		system("rm -rf result");
		}
		}
		else
			error("Error in file opening");


	}
}

void cIndexget(char write_buffer[],int *sockfd)
{
	int n;
	n = write(*sockfd,write_buffer,1024);
	if (n < 0)  
		error("ERROR writing to socket");
	char l[8]="LongList",read_buffer[1024],r[5]="RegEx",s[9]="ShortList";
	int t=0,i;
	for(i=0;i<8;i++)
		if(l[i]!=write_buffer[9+i])
			t=1;
	if(t==1)
	{
		for(i=0;i<5;i++)
			if(r[i]!=write_buffer[9+i])
				t=2;
	}
	if(t==2)
	{
		for(i=0;i<9;i++)
			if(s[i]!=write_buffer[9+i])
				t=3;
	}
	if(t==0)
	{

		int f=0,flag=1;
		bzero(read_buffer, 1024);
		printf("\nRecieved data : %s\n",read_buffer);
		printf("File-name Size Timestamp\n");
		while((f= read(*sockfd, read_buffer,1024)) > 0)
		{
			if(strcmp(read_buffer,"END")==0)
			{flag=0;
				break;
			}
			else
				printf("%s\n",read_buffer);
		}
		if(flag==1)
		exit(0);
	}
	if(t==1)
	{
		int f=0,flag=1;
		bzero(read_buffer, 1024);
		printf("\nRecieved data : %s\n",read_buffer);
		printf("File-name Size\n");
		while((f= read(*sockfd, read_buffer,1024)) > 0)
		{
			if(strcmp(read_buffer,"END")==0)
			{flag=0;
				break;
			}
			else
				printf("%s\n",read_buffer);
		}
		if(flag==1)
		exit(0);
	}
	if(t==2)
	{
		int f=0,flag=1;
		bzero(read_buffer, 1024);
		printf("\nRecieved data : %s\n",read_buffer);
		printf("File-name Size Last-Modified Timestamp\n");
		while((f= read(*sockfd, read_buffer,1024)) > 0)
		{
			if(strcmp(read_buffer,"END")==0)
			{flag=0;
				break;
			}
			else
				printf("%s\n",read_buffer);
		}
		if(flag==1)
		exit(0);
	}
}

void Filehash(char read_buffer[] , int *newsockfd)
{
	char write_buffer[1024];
	bzero(write_buffer,1024);
	char v[6]="Verify",c[8]="CheckAll";
	int t=0,i;
	for(i=0;i<6;i++)
		if(v[i]!=read_buffer[9+i])
			t=1;
	if(t==1)
	{
		for(i=0;i<8;i++)
			if(c[i]!=read_buffer[9+i])
				t=2;
	}
	if(t==0)
	{
		char l1[500],l2[400];
		int k=0;
		i=16;
		while(i!=strlen(read_buffer))
		{
			l1[k]=read_buffer[i];
			k++;
			i++;
		}
		l1[k]='\0';
		hashing(l1,newsockfd);
		bzero(write_buffer,1024);
		strcpy(write_buffer,"END");
		if(write(*newsockfd, write_buffer, 1024) < 0)
		{
			error("\nERROR: Writing to socket");
			exit(1);
		}

	}
	if(t==1)
	{
		system("ls -l |grep -v ^d | awk '{print $9}' | tail -n +2 > fd");
		int flag=0;
		FILE *file = fopen ( "fd", "r" );
		if ( file != NULL )
		{   
			char line [500]; 
			while ( fgets ( line, sizeof(line), file ) != NULL ) 
			{ 
				char te[400];
				int k=0;
				i=0;
				while(line[i]!='\n')
				{
					te[k]=line[i];
					k++;
					i++;
				}
				te[k]='\0';
				hashing(te,newsockfd);
			}
			flag=1;
		}
		else
			error("Error in file opening");
		if(flag==1)
		{
		bzero(write_buffer, 1024);
		strcpy(write_buffer,"END");
		if(write(*newsockfd, write_buffer, 1024) < 0)
		{
			error("\nERROR: Writing to socket");
			exit(1);
		}
		}
		system("rm -rf fd");

	}

}
void hashing(char l1[],int *newsockfd)
{
	int i=0,k=0;
	char write_buffer[1024];
		char str[500],st[100];
		strcpy(str,"md5sum ");
		strcat(str,l1);
		strcat(str," > result");
		system("touch result");
		system("touch result1");
		strcpy(st,"stat ");
		strcat(st,l1);
		strcat(st," | grep Modify | awk '{print $3}' > result1");
		system(str);
		system(st);
		FILE *f=fopen("result","r");
		FILE *f1=fopen("result1","r");
		char u[300],r[300],a[300];
		fgets(u,sizeof(u),f);
		fgets(r,sizeof(r),f1);
		system("rm -rf result*");
		i=0,k=0;
		while(u[i]!=' ')
		{   
			a[k]=u[i];
			i++;
			k++;
		}   
		a[k]='\0';
		strcat(l1,"\t");
		r[strlen(r)-1]='\0';
		strcat(l1,r);
		strcat(l1,"\t");
		strcat(l1,a);
		strcpy(write_buffer,l1);
		if(write(*newsockfd, write_buffer, 1024) < 0)
		{
			error("\nERROR: Writing to socket");
			exit(1);
		}
}
void cFilehash(char write_buffer[], int *sockfd)
{
	int n;
	char read_buffer[1024];
	n = write(*sockfd,write_buffer,1024);
	if (n < 0)  
		error("ERROR writing to socket");
	int t=0,i;
	char v[6]="Verify",c[8]="CheckAll";
	for(i=0;i<6;i++)
		if(v[i]!=write_buffer[9+i])
			t=1;
	if(t==1)
	{
		for(i=0;i<8;i++)
			if(c[i]!=write_buffer[9+i])
				t=2;
	}
	if(t==0)
	{

		int f=0,flag=1;
		bzero(read_buffer, 1024);
		printf("\nRecieved data : %s\n",read_buffer);
		printf("File-name	Latest-Timestamp	Md5hash\n");
		while((f= read(*sockfd, read_buffer,1024)) > 0)
		{
			if(strcmp(read_buffer,"END")==0)
			{flag=0;
				break;
			}
			else
				printf("%s\n",read_buffer);
		}
		if(flag==1)
		exit(0);
	}
	if(t==1)
	{
		int f=0,flag=1;
		bzero(read_buffer, 1024);
		printf("\nRecieved data : %s\n",read_buffer);
		printf("File-name	Latest-Timestamp	Md5hash\n");
		while((f= read(*sockfd, read_buffer,1024)) > 0)
		{
			if(strcmp(read_buffer,"END")==0)
			{flag=0;
				break;
			}
			else
				printf("%s\n",read_buffer);
		}
		if(flag==1)
		exit(0);
	}
}

void cFiledownload(char write_buffer[],int *sockfd)
{
	int n=0;
	char read_buffer[1024],hashm[200],data[1024];
    	n = write(*sockfd,write_buffer, 1024);
    		if (n < 0) 
        		 error("ERROR writing to socket");
	int i=0,k=0;
	char xname[200],timestamp[200],size[100],name[200];
	printf("[Client] File recieving from server\n");
	while(write_buffer[13+i]!='\0')
	{	
		name[i]=write_buffer[13+i];
		i++;
	}
	name[i]='\0';
	bzero(read_buffer,1024);
	if(n= read(*sockfd, read_buffer,1024) > 0)
	{
		if(strncmp(read_buffer,"File Not there",14)==0)
		{
			return;
		}

	}
	else
		error("Error in reading from socket");

	bzero(read_buffer,1024);
	if(n= read(*sockfd, read_buffer,1024) > 0)
	{
	
			if(strncmp(read_buffer,"timestamp",9)==0)
			{
				i=9;
				k=0;
				while(read_buffer[i]!='\0')
				{
					timestamp[k]=read_buffer[i];
					k++;
					i++;
				}
				timestamp[k]='\0';
			}
	}
	else
		error("Error in reading from socket");
	bzero(read_buffer,1024);
	if(n= read(*sockfd, read_buffer,1024) > 0)
	{
			if(strncmp(read_buffer,"size",4)==0)
			{
				i=4;
				k=0;
				while(read_buffer[i]!='\0')
				{
					size[k]=read_buffer[i];
					k++;
					i++;
				}
				size[k]='\0';
				
			}
	}
	else
		error("Error in reading from socket");
	bzero(read_buffer,1024);
	if(n= read(*sockfd, read_buffer,1024) > 0)
	{
			if(strncmp(read_buffer,"name",4)==0)
			{
				i=4;
				k=0;
				while(read_buffer[i]!='\0')
				{
					xname[k]=read_buffer[i];
					k++;
					i++;
				}
				xname[k]='\0';
			
			}
	}
	else
		error("Error in reading from socket");
	bzero(read_buffer,1024);
	if(n= read(*sockfd, read_buffer,1024) > 0)
	{
			if(strncmp(read_buffer,"hash",4)==0)
			{
				i=4;
				k=0;
				while(read_buffer[i]!='\0')
				{
					hashm[k]=read_buffer[i];
					k++;
					i++;
				}
				hashm[k]='\0';

			}
	}
	else
		error("Error in reading from socket");
	printf("Timestamp	%s\n",timestamp);
	printf("Size		%s\n",size);
	printf("Name		%s\n",name);
	printf("Hash		%s\n",hashm);
	bzero(read_buffer,1024);
	FILE *fr = fopen(name, "wb");
	if(fr == NULL)
		printf("File %s Cannot be opened file on peer.\n", name);
	else
	{
		int f = 0,f1=0;
		unsigned int size = 0;
		while(f = recv(*sockfd,&size,sizeof(size),0)>0)
		{
			int p=ntohl(size);
			f1 = recv(*sockfd, read_buffer,1024, 0);
			if(strcmp(read_buffer,"END")==0)
				break;
			if(f1 > 0) {

				int w= fwrite(read_buffer, sizeof(char), p, fr);
				if(w < p)
				{
					error("[Client] File write failed on client.\n");
				}
				if (p == 0 || p!= 1024) 
				{
					break;
				}
			}
			if(f1 < 0)
			{
				error("Error receiving file from client to server.\n");
			}
			if(f < 0)
			{
				error("Error receiving file from client to server.\n");
			}
			bzero(read_buffer, 1024); 
		}
		printf("[Client]File received from server!\n");
		fclose(fr);
	}
}

void Filedownload(char read_buffer[],int *newsockfd)
{
	
	char write_buffer[1024];
	bzero(write_buffer,1024);
	int t=0,i=0,n=0;
	char name[200],com[500];
	while(read_buffer[13+i]!='\0')
	{	
		name[i]=read_buffer[13+i];
		i++;
	}
	name[i]='\0';
	bzero(write_buffer,1024);
	FILE *f=fopen(name,"r");
	if(f==NULL)
	{
		strcpy(write_buffer,"File Not there");
	if(n=write(*newsockfd,write_buffer,1024)<0)
	{
		error("Error in writing in socket");
	}
		return;
	}
	else
	{
	if(n=write(*newsockfd,write_buffer,1024)<0)
	{
		error("Error in writing in socket");
	}
		fclose(f);
	}
	bzero(write_buffer,1024);
	strcpy(write_buffer,"timestamp");
		system("touch r.txt");
		strcpy(com,"stat ");
		strcat(com,name);
		strcat(com," | grep Modify | awk '{print $3}' > r.txt");
	system(com);
	f=fopen("r.txt","r");
	char u[300],a[300];
	fgets(u,sizeof(u),f);
	strcat(write_buffer,u);
	fclose(f);
	system("rm -rf r.txt");
	if(n=write(*newsockfd,write_buffer,1024)<0)
	{
		error("Error in writing in socket");
	}
	bzero(write_buffer,1024);
	strcpy(write_buffer,"size");
	strcpy(com,"stat ");
	strcat(com,name);
	strcat(com," | grep Size | awk '{print $2}' > r.txt");
	system(com);
	f=fopen("r.txt","r");
	fgets(u,sizeof(u),f);
	fclose(f);
	strcat(write_buffer,u);
	system("rm -rf r.txt");
	if(n=write(*newsockfd,write_buffer,1024)<0)
	{
		error("Error in writing in socket");
	}
	bzero(write_buffer,1024);
	strcpy(write_buffer,"name");
	strcat(write_buffer,name);
	if(n=write(*newsockfd,write_buffer,1024)<0)
	{
		error("Error in writing in socket");
	}
	char str[500];
	strcpy(str,"md5sum ");
	strcat(str,name);
	strcat(str," > r.txt");
	system(str);
	f=fopen("r.txt","r");
	fgets(u,sizeof(u),f);
	int k=0;
	i=0,k=0;
	while(u[i]!=' ')
	{   
		a[k]=u[i];
		i++;
		k++;
	}   
	a[k]='\0';
	strcpy(write_buffer,"hash");
	strcat(write_buffer,a);
	if(n=write(*newsockfd,write_buffer,1024)<0)
	{
		error("Error in writing in socket");
	}
	system("rm -rf r.txt");
	FILE *fs = fopen(name, "rb");
	if(fs == NULL)
	{
		printf("ERROR: File %s not found on server.\n", name);
		exit(1);
	}

	bzero(write_buffer, 1024); 
	int f1; 
	while((f1 = fread(write_buffer, sizeof(char), 1024, fs))>0)
	{
		unsigned int sdlen=htonl(f1);
		if(send(*newsockfd,&sdlen,sizeof(sdlen),0) < 0){
			error("[Server] Sending Header Failed : ");
			break;
		}
		if(send(*newsockfd, write_buffer, f1,0) < 0)
		{
			printf("ERROR: Failed to send file %s.\n", name);
			exit(1);
		}
		bzero(write_buffer, 1024); 
	}
	bzero(write_buffer, 1024); 
	strcpy(write_buffer,"END");
	if(send(*newsockfd, write_buffer, 1024,0) < 0)
	{
		printf("ERROR: Failed to write in socket \n");
		exit(1);
	}
	fclose(fs);
	printf("[Server]File sent to client!\n");
}
void cFileupload(char write_buffer[],int *sockfd)
{
	int n=0;
	char read_buffer[1024];
    	n = write(*sockfd,write_buffer, 1024);
    		if (n < 0) 
        		 error("ERROR writing to socket");
	int i=0,k=0;
	char size[100],name[200],u[300],com[400];
	while(write_buffer[11+i]!='\0')
	{	
		name[i]=write_buffer[11+i];
		i++;
	}
	name[i]='\0';
	bzero(write_buffer,1024);
	strcpy(write_buffer,"name");
	strcat(write_buffer,name);
	if(n= write(*sockfd, write_buffer,1024) < 0)
	{
		error("Error in writing in socket");
	}
	bzero(write_buffer,1024);
	strcpy(write_buffer,"size");
	strcpy(com,"stat ");
	strcat(com,name);
	strcat(com," | grep Size | awk '{print $2}' > r.txt");
	system(com);
	FILE *f;
	f=fopen("r.txt","r");
	fgets(u,sizeof(u),f);
	fclose(f);
	strcat(write_buffer,u);
	system("rm -rf r.txt");
	if(n= write(*sockfd, write_buffer,1024) < 0)
	{
		error("Error in writing in socket");
	}
	
	bzero(read_buffer,1024);
	if(n=read(*sockfd,read_buffer,1024)<0)
	{
		error("Error in reading from socket");
	}
	if(strcmp(read_buffer,"FileUpload Deny")==0)
	{
		printf("[Client] Server denied to upload\n");
		return ;
	}
	if(strcmp(read_buffer,"FileUpload Allow")==0)
	{
		printf("%s\n",name);
		printf("[Client ]File uploading to server\n");
		FILE *fs = fopen(name, "rb");
		if(fs == NULL)
		{
			printf("ERROR: File %s not found on server.\n", name);
			exit(1);
		}

		bzero(write_buffer, 1024); 
		int f1; 
		while((f1 = fread(write_buffer, sizeof(char), 1024, fs))>0)
		{
			unsigned int sdlen=htonl(f1);
			if(send(*sockfd,&sdlen,sizeof(sdlen),0) < 0){
				error("[Server] Sending Header Failed : ");
				break;
			}
			if(send(*sockfd, write_buffer, f1,0) < 0)
			{
				printf("ERROR: Failed to send file %s.\n", name);
				exit(1);
			}
			bzero(write_buffer, 1024); 
		}
		bzero(write_buffer, 1024); 
		strcpy(write_buffer,"END");
		if(send(*sockfd, write_buffer, 1024,0) < 0)
		{
			printf("ERROR: Failed to write in socket \n");
			exit(1);
		}
		fclose(fs);

		printf("[Client] File uploaded to the server\n");
	}

}
void Fileupload(char read_buffer[],int *newsockfd)
{
	int n=0;
	int i=0,k=0;
	char size[300],name[300],write_buffer[1024];
	printf("[Server] File recieving from client\n");
	if(n= read(*newsockfd, read_buffer,1024) > 0)
	{
		if(strncmp(read_buffer,"name",4)==0)
		{
			i=4;
			k=0;
			while(read_buffer[i]!='\0')
			{
			name[k]=read_buffer[i];
			i++;k++;
			}
			name[k]='\0';
		}

	}
	else
		error("Error in reading from socket");
	bzero(read_buffer,1024);
	if(n= read(*newsockfd, read_buffer,1024) > 0)
	{
			if(strncmp(read_buffer,"size",4)==0)
			{
				i=4;
				k=0;
				while(read_buffer[i]!='\0')
				{
					size[k]=read_buffer[i];
					k++;
					i++;
				}
				size[k]='\0';
			}
	}
	else
		error("Error in reading from socket");
	printf("[Server] File name	%s\n",name);
	printf("[Server] File size	%s\n",size);
	printf("[Server] Allow/Deny the client to upload: ");
	char p[100];
	kill (pid,SIGSTOP);
	gets(p);
	kill (pid,SIGCONT);
	if(strcmp(p,"Deny")==0)
	{
		bzero(write_buffer,1024);
		strcpy(write_buffer,"FileUpload Deny\0");
		if(n=write(*newsockfd,write_buffer,1024)<0)
		{
        		 error("ERROR writing to socket");
		}	
		return;
	}
	else
	{
		bzero(write_buffer,1024);
		strcpy(write_buffer,"FileUpload Allow\0");
		if(n=write(*newsockfd,write_buffer,1024)<0)
		{
        		 error("ERROR writing to socket");
		}
		printf("[Server] Getting file from client\n");
		FILE *fr = fopen(name, "wb");
		if(fr == NULL)
			printf("File %s Cannot be opened file on peer.\n", name);
		else
		{
				bzero(read_buffer, 1024); 
			int f = 0,f1=0;
			unsigned int size = 0;
			while(f = recv(*newsockfd,&size,sizeof(size),0)>0)
			{
				int p=ntohl(size);
				f1 = recv(*newsockfd, read_buffer,1024, 0);
				if(strcmp(read_buffer,"END")==0)
					break;
				if(f1 > 0) {

					int w= fwrite(read_buffer, sizeof(char), p, fr);
					if(w < p)
					{
						error("[Server] File write failed on client.\n");
					}
					if (p == 0 || p!= 1024) 
					{
						break;
					}
				}
				if(f1 < 0)
				{
					error("Error receiving file from client to server.\n");
				}
				if(f < 0)
				{
					error("Error receiving file from client to server.\n");
				}
				bzero(read_buffer, 1024); 
			}
			fclose(fr);
			printf("[Server] File got from client\n");
		}
	}
}

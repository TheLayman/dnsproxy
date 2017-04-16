
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include <math.h>
#define MAXLINE 100

typedef struct _data
{
	char address[MAXLINE];
	char name[MAXLINE];
}database;

void signalHandler(int sig); //programmer-defined signal handler for Ctrl+C command
void databaseInit(char *); //creates database
int findUPC(int); //searches and returns item_code supplied in the database and returns accordingly
void childprocess(int,int);
database *domain; //Item-Name-Price list.
int records; //No. of Items in the database.
int listensd,connSD; //Socket descriptors.

void databaseInit(char *file_name)
{
	FILE *fp;
	char line_buff[MAXLINE];
	int i=0;
	fp=fopen(file_name,"r");
	if(fp==NULL)
	{
		// printf("\nDatabase Configuration Error.. Terminating Server.%s",file_name);
		signalHandler(0);
		exit(0);
	}
	fgets(line_buff,MAXLINE,fp); //reads the first line that has the number of entries in the text file
	records=atoi(line_buff); //gets number of records
	domain=(database *)malloc(records*sizeof(database)); //allocates memory for database
	for(i=0;i<records;i++)
	{
		fscanf(fp,"%s",domain[i].name);
		fscanf(fp,"%s",domain[i].address);
	}
	// printf("Database Configuration..... Complete\n" );
	return ;
}

int dnsSearch(char** a)
{	
	if(strcmp(domain[0].name,*a)==0) { *a=domain[0].address;return 1;}
	if(strcmp(domain[1].name,*a)==0) { *a=domain[1].address;return 1;}
	if(strcmp(domain[2].name,*a)==0) { *a=domain[2].address;return 1;}
	if(strcmp(domain[3].name,*a)==0) { *a=domain[3].address;return 1;}
	if(strcmp(domain[4].name,*a)==0) { *a=domain[4].address;return 1;}
	if(strcmp(domain[5].name,*a)==0) { *a=domain[5].address;return 1;}
	return 0;
}
void signalHandler(int sig)
{
	char msg[MAXLINE];
	close(listensd);
	// printf("Server terminating!..\n");
	sprintf(msg,"Server terminated!\n");
	send(connSD,msg,MAXLINE,0);
	close(connSD);
	exit(0);
}

void childprocess(int connSD,int id)
{
	int len,flag;
	double total=0.0;
	char buffer[MAXLINE],msg[MAXLINE],*name,*token;
	const char delim[2]=" ";
	len=0;
	memset(msg,0,MAXLINE); //clears contents of msg
	len=recv(connSD,buffer,MAXLINE,0);
	if(len<0)
	{
		sprintf(msg,"Communication Error. Terminating Connection. Please reconnect.\n");
		send(connSD,msg,MAXLINE,0);
		close(connSD);
		exit(0);
	}
	if(strcmp(buffer,"----||||----")==0)
	{
		// printf("\nTerminating Child No: %d 'cause corresponding client terminated\n", getpid());
		close(connSD);
		exit(0);
	}
	token=strtok(buffer,delim);
	flag=dnsSearch(&token);
	if(flag==0)
	{
		sprintf(msg,"Entry not found in the database");
		send(connSD,msg,MAXLINE,0);
		signalHandler(1);
	}
	else if(flag==1)
	{
		sprintf(msg,"%s",token);
		send(connSD,msg,MAXLINE,0);
		signalHandler(1);
	}
}

int main(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("\nToo few arguments to server!..exiting");
		exit(0);
	}
	int clilen; //clilen is the length of the client socket, used as a value-result argument
	struct sockaddr_in ServAddr, CliAddr; //sockaddr structure for sockets; one for server and the other for client
	listensd=socket(AF_INET,SOCK_STREAM,0); //Creating a TCP Socket
	if(listensd<0)
	{
		perror("Cannot create socket!");
		return 0;
	}
	signal(SIGINT,signalHandler);
	//Setting up the Server socket
	ServAddr.sin_family=AF_INET;
	ServAddr.sin_addr.s_addr = INADDR_ANY; //Connection from any IP.
	ServAddr.sin_port = htons(atoi(argv[1])); //self defined server port
	//Binding socket
	if(bind(listensd,(struct sockaddr *) &ServAddr, sizeof(ServAddr))<0)
	{
		perror("Cannot bind port!");
		return 0;
	}

	if (listen(listensd,SOMAXCONN) <0 )
	{
		perror("Unable to set the socket to listen");
		return 0;
	}
	//Initializing Database
	databaseInit("database.txt");
	while(1!=0)
	{
		clilen=sizeof(CliAddr);
		if((connSD=accept(listensd,(struct sockaddr *)&CliAddr,&clilen))<0)
		{
			perror("Cannot establish connection!");
			return 0;
		}

		if((fork())==0)
		{
			close(listensd); //Allowing only one socket to listen by closing child process' listensd.
			// printf("\nRequest from %s servcied with child process %d\n",inet_ntoa(CliAddr.sin_addr) ,getpid());
			childprocess(connSD,getpid()); //A Child Process for a connection.
			close(connSD); //Child closes its version of connSD after transaction is completed.
			exit(0); //Child terminates.
		}
		close(connSD); //Parent looks for more connections by closing the connSD.
	}
}

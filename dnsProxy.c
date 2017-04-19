
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
char lol[MAXLINE];
typedef struct _data
{
	char *address;
	char *name;
}database;
int listensd,connSD,dnsSD; //Socket descriptors.
void signalHandler(int sig); //programmer-defined signal handler for Ctrl+C command
void databaseInit(); //creates database
int findUPC(int); //searches and returns item_code supplied in the database and returns accordingly
void childprocess(int,int);
database *domain ; //Item-Name-Price list.
short cur;
void queueInit(void)
{
	domain=(database *)malloc(3*sizeof(database)); //allocates memory for database
	domain[0].name="----||||----";
	domain[0].address="----||||----";
	domain[1].name="----||||----";
	domain[1].address="----||||----";
	domain[2].name="----||||----";
	domain[2].address="----||||----";
	cur=0;
}

int dnsSearch(char** a,int x)
{ 		memset(lol,0,MAXLINE);
	if(x==1){
	if(strcmp(domain[0].name,*a)==0) { sprintf(lol,"%s",domain[0].address);return 1;}
	if(strcmp(domain[1].name,*a)==0) { sprintf(lol,"%s",domain[1].address);return 1;}
	if(strcmp(domain[2].name,*a)==0) { sprintf(lol,"%s",domain[2].address);return 1;}}
	if(x==2){
		if(strcmp(domain[0].address,*a)==0) { sprintf(lol,"%s",domain[0].name);return 1;}
		if(strcmp(domain[1].address,*a)==0) { sprintf(lol,"%s",domain[1].name);return 1;}
		if(strcmp(domain[2].address,*a)==0) { sprintf(lol,"%s",domain[2].name);return 1;}
	}
	struct sockaddr_in ServAddr;
	dnsSD=socket(AF_INET,SOCK_STREAM,0); //socket(internet_family,socket_type,protocol_value) retruns socket descriptor
	if(dnsSD<0)
	{
		return 0;
	}
	ServAddr.sin_family=AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //using the imput IP
	ServAddr.sin_port = htons(9999); //self defined server port
	if((connect(dnsSD,(struct sockaddr *) &ServAddr,sizeof(ServAddr)))<0)
	{
		return 0;
	}
	char recvBuffer[MAXLINE], *token,sendBuffer[MAXLINE];
	const char de[2]="$";
	memset(recvBuffer,0,MAXLINE);
	memset(sendBuffer,0,MAXLINE);
	sprintf(sendBuffer,"%d$%s",x,*a);
	send(dnsSD,sendBuffer,MAXLINE,0);
	recv(dnsSD,recvBuffer,MAXLINE,0);
	if(recvBuffer[0]-'0'==4) return 0;
	else if (recvBuffer[0]-'0'==3)
	{
		token=strtok(recvBuffer,de);
		token=strtok(NULL,de);

		if(x==1)
		{domain[cur].name=*a;
		domain[cur].address=token;}
		if(x==2){
			domain[cur].address=*a;
			domain[cur].name=token;
		}
		cur=(cur+1)%3;
		sprintf(lol,"%s",token);
		 return 1;
	}
}


void signalHandler(int sig)
{
	char msg[MAXLINE];
	close(listensd);
	// printf("Server terminating!..\n");
	sprintf(msg,"----||||----");
	send(connSD,msg,MAXLINE,0);
	close(connSD);
	exit(0);
}

void childprocess(int connSD,int id)
{
	int len,flag;
	double total=0.0;
	char buffer[MAXLINE],msg[MAXLINE],*name,*token,*temp;
	const char delim[2]="/", de[2]="$";
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
	token=strtok(buffer,de);
	token=strtok(NULL,de);
	temp=token;

	token=strtok(temp,delim);
	name=token;
	token=strtok(NULL,delim);

	if(buffer[0]-'0'==1) flag=dnsSearch(&name,1);
	else if (buffer[0]-'0'==2) flag=dnsSearch(&name,2);
	else signalHandler(1);

	if(flag==0)
	{
		sprintf(msg,"4$Entry not found in the database");
		send(connSD,msg,MAXLINE,0);
		signalHandler(1);
	}
	else if(flag==1)
	{
		if (token!= NULL) sprintf(msg,"3$%s/%s",lol,token);
		else sprintf(msg,"3$%s",lol);
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
	queueInit();
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

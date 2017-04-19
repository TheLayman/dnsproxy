
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

void commandHandler(int); //client places request with this command
int sockfd; //client socket descriptor

void signalHandler(int sig)
{
	char sendBuffer[MAXLINE];
	sprintf(sendBuffer,"----||||----");
	send(sockfd,sendBuffer,MAXLINE,0);
	close(sockfd);
	exit(0);
}

void commandHandler(int sockfd)
{
	printf("Command Format: 1$domainname (or) 2$ipaddr\n" );
	char sendBuffer[MAXLINE],recvBuffer[MAXLINE];
	memset(sendBuffer,0,MAXLINE);
	memset(recvBuffer,0,MAXLINE);
	gets(sendBuffer);
	send(sockfd,sendBuffer,MAXLINE,0);
	recv(sockfd,recvBuffer,MAXLINE,0);
	puts(recvBuffer);
	signalHandler(1);
}

int main(int argc, char *argv[])
{
	int check;
	int SERVER_PORT = atoi(argv[2]);
	struct sockaddr_in ServAddr;
	//Creating the socket
	sockfd=socket(AF_INET,SOCK_STREAM,0); //socket(internet_family,socket_type,protocol_value) retruns socket descriptor
	if(sockfd<0)
	{
		perror("Cannot create socket!");
		return 0;
	}
	//initializing the server socket
	ServAddr.sin_family=AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr(argv[1]); //using the imput IP
	ServAddr.sin_port = htons(SERVER_PORT); //self defined server port

	if((connect(sockfd,(struct sockaddr *) &ServAddr,sizeof(ServAddr)))<0)
	{
		perror("Server is down!");
		return 0;
	}

	signal(SIGINT,signalHandler);
	//printf("\n Connection established :\n ");
	
	{commandHandler(sockfd);}
	return 0;
}

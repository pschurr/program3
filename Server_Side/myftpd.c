#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define SERVER_PORT 41021
#define MAX_PENDING 5
#define MAX_LINE 256

int main(int argc, char * argv[]){
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int len;
	int s, new_s;
	int opt;
	
	/*build address data structure*/
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);
	
	/*setup passive open*/
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("simplex-talk: socket");
		exit(1);
	}
	
	/*Set socket option*/
	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)))<0){
		perror("simplex-talk:socket");
		exit(1);
	}
	if((bind(s,(struct sockaddr *)&sin, sizeof(sin)))<0){
		perror("simplex-talk: bind");
		exit(1);
	}
	if((listen(s,MAX_PENDING))<0){
		perror("simplex-talk:listen");
		exit(1);
	}
	printf("Welcome to the TCP Server\n");
	//Waiting for the connection and then acting
	while(1){
		if((new_s = accept(s,(struct sockaddr *)&sin, &len))<0){
			perror("Server Received Error!");
			exit(1);
		}
		if(len==0) break;
		close(new_s);
	}
}

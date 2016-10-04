#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define MAX_LINE 256
int main(int argc, char * argv[]){
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[MAX_LINE];
	int s;
	int len;
	int server_port;
	char command[MAX_LINE];
	char operation[3];
	char file_name[MAX_LINE];
	if (argc==3) {
		host = argv[1];
		server_port = atoi(argv[2]);
	}else {
		fprintf(stderr, "usage: simplex-talk host\n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);

	if (!hp) {
		fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
		exit(1);
	}

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(server_port);

	/* active open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket"); 
		exit(1);
	}
	
	printf("Welcome to your first TCP client! To quit, type \'Exit\'\n");

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("simplex-talk: connect");
		close(s); 
		exit(1);
	}

	/* main loop: get and send lines of text */
	while (fgets(buf, sizeof(buf), stdin)) {
		buf[MAX_LINE-1] = '\0';
		
		printf("Please enter an operation: ");   // PSchurr prompt user input
		fgets(command, sizeof(command), stdin);
		
		memcpy(operation,&command[0],3);
		operation[3]='\0';
		len=strlen(operation) +1;
		if(strcmp("REQ", operation) == 0) {
			if(send(s,operation,len,0)==-1){
			perror("client send error!"); 
			exit(1);	
			}	
		
			// recvfrom (query for name)
			// send length of filename and filename
			// rec 32-bit file length
			// decode 32 length
			// if -1, break
			// rec md5hash and store
			// receive file and save
			// compute hash
			// notify user
			// break


		} else if( strcmp("UPL", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);	
			}	
		} else if (strcmp("MKD", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
		} else if (strcmp("RMD", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
		} else if (strcmp("CHD", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
		} else if (strcmp("LIS", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);	
			}	
		} else if (strcmp("XIT", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
		} else if (strcmp("DEL", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);	
			}	
		}
		
	
		if (!strncmp(buf, "Exit",4)){
 			printf("Good Bye!\n");
 			break;
		}
		
		
		len = strlen(buf) + 1;
 		if(send(s, buf, len, 0)==-1){
			perror("client send error!"); 
			exit(1);	
		}
	}

	close(s);
}


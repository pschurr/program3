#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mhash.h>
#define MAX_LINE 256
int main(int argc, char * argv[]){
	MHASH td;
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[MAX_LINE];
	int s, ret;
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
	while (1){//fgets(buf, sizeof(buf), stdin)) {
		//buf[MAX_LINE-1] = '\0';
		
		printf("Please enter an operation (REQ, UPL, DEL, LIS, MKD, RMD, CHD, XIT): ");   // PSchurr prompt user input
		fgets(command, sizeof(command), stdin);
		
		memcpy(operation,&command[0],3);
		operation[3]='\0';
		len=strlen(operation) +1;
		if(strcmp("REQ", operation) == 0) {
			if(send(s,operation,len,0)==-1){
			perror("client send error!"); 
			exit(1);	
			}
			printf("Please enter the requested file name: ");
			fgets(file_name, sizeof(file_name), stdin);
			strtok(file_name, "\n");
			int name_len = strlen(file_name)+1;
			char len_str[10];
			snprintf(len_str, 10, "%d", name_len);
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){
				perror("client send error: Error sending file name length!");
				//exit(1);
				continue;
			}
			file_name[name_len] ='\0';
                        if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                //exit(1);
                                continue;
                        }
			char size[10];
  			if((ret = recv(s,size, 10, 0))<0){
				perror("client receive error: Error receiving file length!");
				//exit(1);
				continue;
			}
			int file_size = atoi(size);
			printf("%i\n", file_size);

			if ( file_size >= 0){ // Server returns a negative file length if file doesn't exist on server
				unsigned char hash[16];
				if(recv(s,hash, 16, 0)<0){//Get that hash
                                	perror("client receive error: Error receiving file hash!");
                                	//exit(1);
                                	continue;
                        	}
				unsigned char content[file_size];
				if(recv(s,content, file_size,0)<0){
					perror("client recieve error: Error receiving file content!");
					//exit(1);
					continue;
				}
				fp = fopen(file_name, "w");
				fprintf(fp, content);
				td = mhash_init(MHASH_MD5);
				if (td == MHASH_FAILED) return 1; 
				fread(content, sizeof(char), file_size, fp);
				fclose(fp);
				mhash(td,&content , 1);
				unsigned char * serverHash = mhash_end(td); 
				if(strcmp(serverHash, hash) == 0){
					printf("Successfully received %s.\n",file_name);
				}

				else{
					printf("Failed to received %s.\n", file_name);
				}	
				//Cleanup here
				
			}
			else {
				printf("%s does not exist.\n", file_name);
			}
			
			
			// recvfrom (query for name) EDIT: Updated project document says not to query for name and length. 
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


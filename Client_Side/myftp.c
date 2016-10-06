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
	char ack[MAX_LINE];
	int s, ret, len, new_s1;
	int server_port;
	char command[MAX_LINE];
	char operation[3];
	char decision[3];
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
		fgets(operation, sizeof(operation), stdin);
		
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
			if(send(s,operation,len,0)==-1){  // client sends operation to upload a file to server
				perror("client send error!"); 
				exit(1);	
			}
			// client gets file name and checks if it exists
			printf("Please enter the requested file name: ");
			fgets(file_name, sizeof(file_name), stdin);
			strtok(file_name, "\n");
			int name_len = strlen(file_name)+1;
			char len_str[10];
			snprintf(len_str, 10, "%d", name_len);
			fp = fopen(file_name, "r");
                        if (fp == NULL){ // check if it exists
				perror("File does not exist");
				continue;
			}			
			
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){ // send file name length
				perror("client send error: Error sending file name length!");
				continue;
			}
			file_name[name_len] ='\0';
			// client send file name (char string)
                        if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                continue;
                        }
			// receive ACK
			if(recv(new_s1, buf, MAX_LINE, 0)==-1){
				perror("Client receive error: Error receiving acknowledgement!");
                        	continue;
			}
			if( strcmp("ACK", ack) !=0){ // make sure server sent proper acknowledgement
				perror("Acknowledgement not properly received!");
				continue;
			}
			// client send 32 bit value of file size
			 fseek(fp, 0L, SEEK_END);
                        int size = ftell(fp);
                        rewind(fp);
                        char file_size[10];
                        snprintf(file_size, 10, "%d", size);
                        if(send(new_s1, file_size, 10, 0)==-1){
                                perror("Client send error: Error sending file size");
				continue;
			}
			unsigned char *hash;
                        char content[size];
                        fread(content, sizeof(char),size,fp);
                        fclose(fp);
                        td = mhash_init(MHASH_MD5);
                        if (td == MHASH_FAILED) return 1;
                        mhash(td,&content , 1);
                        hash = mhash_end(td);
                        len = strlen(hash);
                        if(send(new_s1, hash, 16, 0)==-1){
                                perror("Client send error: Error sending hash");
				continue;
			}	
			if(send(new_s1, content, size, 0)==-1){
                                perror("Client send error: Error sending file");
				continue;

			}

			// client sends file
			// client computes hash and sends it
			// recv throughput from server


	
		} else if (strcmp("DEL", operation) == 0) {
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
                                continue;
                        }
                        file_name[name_len] ='\0';
                        if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                continue;
                        }
			char size[10];
  			if(recv(s,size, 10, 0)==-1){
				perror("client receive error: Error receiving file length!");
				continue;
			}
			int file_size = atoi(size);
			printf("%i\n", file_size);
			if(file_size<0){ // make sure file exists on server side 
				continue;
			}
			int c =0;
			while(c==0){
				printf("Are you sure you want to delete this file? Enter Yes or No ");
				fgets(decision, sizeof(decision), stdin);
				if(stricmp("yes",decision)==0){
					c =1;
					// send 1
				}else if(stricmp("no",decision) ==0){
					c =1;
					// send -1
				}else{
					printf("Enter a valid decision\n");
				}
			}
  			if(recv(s,size, 10, 0)==-1){
				perror("client receive error: Error receiving file length!");
				continue;
			}
			int did_del= atoi(size);
			if(file_size<0){ // make sure file exists on server side 
				printf("Delete was not successful\n");
				continue;
			} else{
				printf("Delete successful\n");
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


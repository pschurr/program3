#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mhash.h>
#include <unistd.h>
#define MAX_PENDING 5
#define MAX_COMMAND 256
#define MAX_FILE_MESSAGE 512

int main(int argc, char * argv[]){
	struct sockaddr_in sin;
	char buf[MAX_COMMAND];
	int len;
	int s, new_s1, new_s2, new_s3;
	int opt;
	int server_port;
	MHASH td;
	FILE * fp;
	
	//Input Arugment Error Checking
	if(argc == 2){
		server_port = atoi(argv[1]);		
	} else {
		printf("Please run the server using the following command after compiling: ./myftpd <port_number>\n");
		exit(1);
	}

	/*build address data structure*/
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(server_port);
	
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
	printf("Waiting for client connection. \n");
	//Waiting for the connection and then acting
	if((new_s1 = accept(s,(struct sockaddr *)&sin, &len))<0){ //Moved outside to wait for initial connection
                perror("Server Connection Error!");
                exit(1);
        }
	int ret = -4;
	while(1){
		printf("Prompting for client command.\n");
		ret = recv(new_s1, buf, MAX_COMMAND, 0);
		if (ret == 0){	//Ret == 0 if client has closed connection
			printf("Waiting for client connection.\n");
			if((new_s1 = accept(s,(struct sockaddr *)&sin, &len))<0){
				perror("Server Connection Error!");
				exit(1);
			}
		}
		else if(ret < 0){
			perror("Server receive error: Error receiving command!");
			exit(1);
		}
	
		len = strlen(buf); 
		if(len==0) break;

		if(strcmp("REQ", buf) == 0){
			// I REMOVED THE CURRENT SEND AND ACCEPT BECAUSE THE ASSIGNMENT HAS BEEN UPDATED (SEE PIAZZA) AND ACCEPT IS ONLY FOR ACCEPTING CONNECTIONS)
			//Sending a prompt back to the client to enter a file name for the request
			//char *file_message = "Please enter the name of the file that you would like to request";
			//if(send(s, file_message, sizeof(file_message), 0)==-1){ 
			//	perror("server send file prompt error!"); 
			//	exit(1);
			//}
 
			//accepting the name of the file
			//if((new_s2 = accept(s,(struct sockaddr *)&sin, &len))<0){
			//	perror("Server Received Error!");
			//	exit(1);
			//}
			char name_len[10];
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0) continue; // Client has closed connection continue
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char file_name[l];
                        ret = recv(new_s1, file_name, l,0);
                        if(ret == 0) continue; // Client has closed connection continue
                        else if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }
			fp = fopen(file_name, "r");
			if (fp == NULL){
				ret = send(new_s1, "-1", 2,0);
			//	if(errno == SIGPIPE){//Client has closed connection
			//		continue;
			//	}
				if(ret == -1){
					perror("server send error: Error sending file size");
			//		exit(1);
				}

				continue;
			}
			fseek(fp, 0L, SEEK_END);
			int size = ftell(fp);
			rewind(fp);
			char file_size[10];
			snprintf(file_size, 10, "%d", size);
			ret = send(new_s1, file_size, 10, 0);
                        //if(errno == SIGPIPE){//Client has closed connection
                          //      continue;
                        //}
                        if(ret == -1){
                                perror("server send error: Error sending file size");
                                //exit(1);
                                continue;
                        }
			//Server computes MD5 hash of the file and sends it to client as a 16-byte string. 
			unsigned char *hash;
			char content[size];
			fread(content, sizeof(char),size,fp);
			fclose(fp);
			td = mhash_init(MHASH_MD5);
                        if (td == MHASH_FAILED) return 1;
                        mhash(td,&content , 1);
			hash = mhash_end(td);
			len = strlen(hash);
                        ret = send(new_s1, hash, 16, 0);
                        //if(errno == SIGPIPE){//Client has closed connection
                          //      continue;
                        //}
                        if(ret == -1){
                                perror("server send error: Error sending hash");
                        //        exit(1);
                        	continue;
                        }
                        ret = send(new_s1, content, size, 0);
                        //if(errno == SIGPIPE){//Client has closed connection
                          //      continue;
                        //}
                        if(ret == -1){
                                perror("server send error: Error sending file content");
                                continue;
                        }

			//Server sends the file to client. 

		}else if(strcmp("UPL", buf) == 0){
			char name_len[10];
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0) continue; // Client has closed connection continue
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char file_name[l];
                        ret = recv(new_s1, file_name, l,0);
                        if(ret == 0) continue; // Client has closed connection continue
                        else if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }

			//Sending an acknowledgement to the client that the file name/size was received
			char *ack_string = "ACK";
			int ack_string_len = strlen(ack_string) + 1;
                        ret = send(new_s1, ack_string, ack_string_len, 0);

			//Server receiving the File Size from the client
			char size[10];
			if((ret = recv(s,size, 10, 0))<0){
				perror("server receive error: Error receiving file length!");
				//exit(1);
				continue;							
			}
			int file_size = atoi(size);
			if ( file_size >= 0){ //Server returns an error message if file size is negative
				//Receiving File
				unsigned char content[file_size];
				if(recv(s,content, file_size,0)<0){
					perror("client recieve error: Error receiving file content!");
					continue;
				}

				//Calculate Throughput
				

				//Receiving Hash
				unsigned char hash[16];
				if(recv(s,hash, 16, 0)<0){//Get that hash
                                	perror("client receive error: Error receiving file hash!");
                                      	continue;
				}

				//Writing file and checking the hash
				fp = fopen(file_name, "w");
				fprintf(fp, content);
				td = mhash_init(MHASH_MD5);
				if (td == MHASH_FAILED) return 1; 
				fread(content, sizeof(char), file_size, fp);
				fclose(fp);
				mhash(td,&content , 1);
				unsigned char * clientHash = mhash_end(td); 
				if(strcmp(clientHash, hash) == 0){
					printf("Successfully received %s.\n",file_name);
				}

			} else {
				printf("Failed to received %s.\n", file_name);
			}

		}else if(strcmp("LIS", buf) == 0){

		}else if(strcmp("DIR", buf) == 0){

		}else if(strcmp("RMD", buf) == 0){

		}else if(strcmp("CHD", buf) == 0){

		}else if(strcmp("DEL", buf) == 0){
			
			char name_len[10];
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0) continue; // Client has closed connection continue
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char file_name[l];
                        ret = recv(new_s1, file_name, l,0);
			
			//Sending the client either a 1 or -1 based on if the file exists or not
			fp = fopen(file_name, "r");
			if (fp == NULL){
				ret = send(new_s1, "-1",2,0);
				if(ret == -1){
					perror("server send error: Error sending file size");
				}
				continue;
			}
			ret = send(new_s1, "1",2,0);

			//Receiving the confirmation for deleting the file. If the user confirms the delete, receiving 1, if not, receiving -1
			char confirmation_string[2];
			ret = recv(new_s1, confirmation_string, 2, 0);
			int confirm_delete = atoi(confirmation_string);
			if(confirm_delete == -1) continue;
			
			//Actually deleting the file
			int remove_check = remove(file_name);
			if(ret == 0) {
				ret = send(new_s1, "1",2,0);
				if(ret == -1){
					perror("server send error: Error sending file deletion status");
				}
				
   			} else {
				ret = send(new_s1, "-1",2,0);
				if(ret == -1){
					perror("server send error: Error sending file deletion status");
				}
   			}
 
		}else if(strcmp("XIT", buf) == 0){

		} else {
			char *command_error_message = "Please enter a valid command for this client/server";
			if(send(s, command_error_message, sizeof(command_error_message), 0)==-1){ 
				perror("server send command error message error!"); 
				exit(1);
			}
		}
		
//		close(new_s1);
//		close(new_s2);
	}
}

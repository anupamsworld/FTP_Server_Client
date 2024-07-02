//question: why not using character pointer instead of buffer/message

//Major concern: if the size of incoming total data is multiple of buffer_in
//then the process which is receiving will be blocked as after receiving all
//data one extra "recv()" will be called in next iteration

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<dirent.h>
//#include "server.h"



#define MAX_BACKLOG_CONNECTIONS 3
#define CWD "./server_DIR/"			//CWD=Current Working Directory
#define BUFFER_SIZE 1024

/*
struct sockaddr {
	unsigned short sa_family;	// address family, AF_xxx
	char sa_data[14];		// 14 bytes of protocol address
};

struct addrinfo {
	int ai_flags;			// AI_PASSIVE, AI_CANONNAME, etc.
	int ai_family;			// AF_INET, AF_INET6, AF_UNSPEC
	int ai_socktype;		// SOCK_STREAM, SOCK_DGRAM
	int ai_protocol;		// use 0 for "any"
	size_t ai_addrlen;		// size of ai_addr in bytes
	struct sockaddr *ai_addr;	// struct sockaddr_in or _in6
	char *ai_canonname;		// full canonical hostname
	struct addrinfo *ai_next;	// linked list, next node
};


struct in_addr {
	uint32_t s_addr;		// that's a 32-bit int (4 bytes)
};

struct sockaddr_in {
	short int sin_family;		// Address family, AF_INET
	unsigned short int sin_port;	// Port number
	struct in_addr sin_addr;	// Internet address
	unsigned char sin_zero[8];	// Same size as struct sockaddr
};

*/

/*********** Following are for only IPv6 ***********/
/*
struct sockaddr_in6 {
	u_int16_t sin6_family;		// address family, AF_INET6
	u_int16_t sin6_port;		// port number, Network Byte Order
	u_int32_t sin6_flowinfo;	// IPv6 flow information
	struct in6_addr sin6_addr;	// IPv6 address
	u_int32_t sin6_scope_id;	// Scope ID
};

struct in6_addr {
	unsigned char s6_addr[16];	// IPv6 address
};
*/
/*********** End of IPv6 ***************/


/* Could not understand the type name "sa_family_t" - from where has it come?
struct sockaddr_storage {
	sa_family_t ss_family;		// address family
	
	// all this is padding, implementation specific, ignore it:
	char __ss_pad1[_SS_PAD1SIZE];
	int64_t __ss_align;
	char __ss_pad2[_SS_PAD2SIZE];
};
*/




/*
int getaddrinfo(const char *node,// e.g. "www.example.com" or IP
		const char *service, // e.g. "http" or port number
		const struct addrinfo *hints,
		struct addrinfo **res);
		*/




int sendall(int client_fileDesc, char *message, int *messageLengthSent, int messageLength){
	*messageLengthSent=0;			// how many bytes we've sent
	int bytesleft = messageLength;		// how many we have left to send
	int n;
	while(*messageLengthSent < messageLength) {
		n = send(client_fileDesc, message+*messageLengthSent, bytesleft, 0);
		if (n == -1) { break; }
		*messageLengthSent += n;
		bytesleft -= n;
	}
		return n==-1?-1:1;		// return -1 on failure, 1 on success
}
int extractClientArguments(char *str, int length, char *words[])
{
	char tempStrr[length];
	strcpy(tempStrr, str);
	str = strtok(tempStrr, "\n");		//need to recheck why do we need this
	
	int k = 0;				// Store number of words
	char *ptr = strtok(tempStrr, " ");
	
	while(ptr != NULL)
	{
		words[k++] = ptr;
		ptr = strtok(NULL, " ");
	}
	
	return k;
}
int sendData(int fileDesc, char *message,  int buffer_out_length){
	//int messageLength=strlen(buffer_out);
	int messageLengthSent=-1;
	int status = sendall(fileDesc, message, &messageLengthSent, buffer_out_length);
	if(status != -1) {
		printf("\nMessage is sent to client.\n::%s\n",message);
	}
	else{
		//perror("sendall");
		printf("\nServer could not send all data.");
		printf("\nClient only sent %d bytes because of the error!\n", messageLengthSent);
		return status;			//status is same as return value of sendall
	}
}
char * executeCommand(char *command) {
    FILE *pf;
    //char command[20];
    char data[512];
    char * result;

    // Execute a process listing
    //sprintf(command, "ps aux wwwf"); 

    // Setup our pipe for reading and execute our command.
    pf = popen(command,"r"); 

    // Error handling

    // Get the data from the process execution
    //fgets(data, 512 , pf);
    fgets(result, 512 , pf);

    // the data is now in 'data'

    if (pclose(pf) != 0)
        fprintf(stderr," Error: Failed to close command stream \n");

    return result;
}
void executeCommand1(char *command, char *result) {
	FILE *pf;
	//char command[20];
	char data[512];
	//char * result;

	// Execute a process listing
	//sprintf(command, "ps aux wwwf"); 

	// Setup our pipe for reading and execute our command.
	printf("\nBefore popen is called.");
	pf = popen(command,"r");
	printf("\nAfter popen is called.");

	// Error handling

	// Get the data from the process execution
	fgets(data, 512 , pf);
	//fgets(result, 512 , pf);

	// the data is now in 'data'
	strcpy(result, data);

	if (pclose(pf) != 0)
		fprintf(stderr," Error: Failed to close command stream \n");

}
int receiveData(int fileDesc, char *buffer_in){
	/***** Start of receiving of data from Server *****/
	int isReceiveSuccess=-1;
	int bytesReceived=-1;
	while(1){
		bytesReceived=-1;
		memset(buffer_in, 0, sizeof(buffer_in));
		//bzero(buff,100);
		printf("\nGoing to receive message from client\n");
		fflush(stdout);
		bytesReceived=recv(fileDesc, buffer_in, 1000, 0);
		if(bytesReceived > 0){
			printf("\nbytesReceived: %d, BUFFER_SIZE: %d\n",bytesReceived,BUFFER_SIZE);
			if(bytesReceived==BUFFER_SIZE){
				continue;				//need to improve this function
									//by concatenating buffers of
									//every iteration
				
			}
			else{
				isReceiveSuccess=1;
				break;		//if received number of bytes are
						//less than total buffer_in size then
						//there is no more data going to come
			}
			
		}
		else if(bytesReceived == 0){
			printf("\nClient hase closed the connection.\n");
			isReceiveSuccess=0;
			break;
			//return -1;
		}
		else if(bytesReceived == -1){
			printf("\nNo message from Client.\n");
			isReceiveSuccess=0;
			break;
		}
		else{
			printf("\nSomething unexpected is happening.\n");
			isReceiveSuccess=0;
			break;
		}
	}
	return isReceiveSuccess;
	/***** End of receiving of data from Server *****/
}
int send_file(int sockfd, char *filename/*, int filenameLength*/)
{	
	printf("\nGoing to send file.\n");
	char filepath[256];  // store file path
	//bzero(filepath, 256);
	memset(filepath, 0, sizeof(filepath));
	strcpy(filepath, CWD);
	printf("\nfilepath=%s\n",filepath);
	//printf("\nfilename=%*s\n", filenameLength, filename);
	strcat(filepath, filename);
	printf("\nFull filepath=%s\n",filepath);
	
	FILE *fd = fopen(filepath, "r");
	
	char buffer[BUFFER_SIZE];
	
	if(fd == NULL)
	{
		// if file does not exist on server
		bzero(buffer, BUFFER_SIZE);
		sprintf(buffer, "%s not found on server.", filename);
		//write(sockfd, buffer, strlen(buffer));
		sendData(sockfd, buffer, strlen(buffer));
		return -1;
	}
	
	//write(sockfd, "OK", 2);
	if(sendData(sockfd, "OK", 2) != -1){
		//bzero(buffer, BUFFER_SIZE);
		memset(buffer, 0, sizeof(buffer));
		//read(sockfd, buffer, BUFFER_SIZE);
		if(receiveData(sockfd, buffer) == 1){
			if(strcmp("OK", buffer) != 0)
			{
				//write(sockfd, "ABORT", 5);
				sendData(sockfd, "ABORT", 5);
				fclose(fd);
				return -1;
			}
				
			int size;
			
			fseek(fd, 0L, SEEK_END);
			size = ftell(fd);
			
			fseek(fd, 0L, SEEK_SET);
			do {
				//bzero(buffer, BUFFER_SIZE);
				memset(buffer, 0, sizeof(buffer));
				size = fread(buffer, sizeof(char), BUFFER_SIZE, fd);
				send(sockfd, buffer, size, 0);
			} while(size == BUFFER_SIZE);
			
		}
		
	}
	fclose(fd);
	

	return 0;
}
int send_files_with_ext(int sockfd, char *extension)
{
	char buffer[256];
	
	DIR *di;
	struct dirent *dir;
	di = opendir(CWD);
	char *filename,*filename_only;
	char filename_temp[100];
	char *ext;
	int send_file_response=-5;//default value
	int count=0;
	// look for files with specified extension
	while ((dir = readdir(di)) != NULL) {
		filename = dir->d_name;
		printf("\nfilename=%s\n",filename);
		if(filename == NULL){
			
			continue;
		}
		strcpy(filename_temp,filename);
		filename_only = strtok(filename_temp, ".");
		printf("\nfilename_only=%s\n",filename_only);
		printf("\nfilename_temp=%s\n",filename_temp);
		/*
		if(ext == NULL){
			
			continue;
		}
		*/
		// get extension from file name
		ext = strtok(NULL, "");
		//strcpy(ext,filename_temp);
		//ext=filename_temp;
		printf("\next=%s\n",ext);
		char dot[10]=".";
		if(ext != NULL && strcmp(strcat(dot,ext), extension) == 0)
		{
			//strcat(filename, ".");
			//strcat(filename, extension);

			if(sendData(sockfd, "FILE_NAME", 9) != -1){
				memset(buffer, 0, sizeof(buffer));
				if(receiveData(sockfd, buffer) == 1){
					printf("\nClient: %s\n",buffer);
					if(strcmp(buffer,"OK_FILENAME")==0){
						if(sendData(sockfd, filename, strlen(filename)) != -1){
							memset(buffer, 0, sizeof(buffer));
							if(receiveData(sockfd, buffer) == 1){
								printf("\nClient: %s\n",buffer);
								if(strcmp(buffer,"OK_FILE_NOW")==0){
									send_file_response=send_file(sockfd, filename);
									if(send_file_response==0){
										count++;
										memset(buffer, 0, sizeof(buffer));
										if(receiveData(sockfd, buffer) == 1){
											printf("\nClient: %s\n",buffer);
											if(strcmp(buffer,"NEXT_FILE")==0){
												continue;
											}
											else{
												printf("\nAborting here. As 'NEXT' is not received.\n");
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}

		}
	}
	printf("\n--Sent %d file(s).\n", count);
	
	//write(sockfd, "DONE", 4);
	if(sendData(sockfd, "DONE", 4) != -1){
		return 1;
	}
	else{
		return -1;
	}
	
			
}
int fetch_file(int sockfd, char *filename)
{
	char buffer_in[BUFFER_SIZE], buffer_out[BUFFER_SIZE];
	memset(buffer_in, 0, sizeof(buffer_in));
	memset(buffer_out, 0, sizeof(buffer_out));
	char filepath[256];
	//bzero(filepath, 256);
	memset(filepath, 0, sizeof(filepath));
	
	strcpy(filepath, CWD);
	strcat(filepath, filename);
	
	printf("\nFull filepath=%s\n",filepath);
	// to check if file already exists in client directory
	FILE *fd = fopen(filepath, "r");
	
	if(fd != NULL)
	{
		sprintf(buffer_out, "EXIST");
		//printf("\nClient: %s\n",buffer_out);
		
		if(sendData(sockfd, buffer_out, sizeof(buffer_out)) != -1){
			//read(sockfd, buffer_in, BUFFER_SIZE);
			if(receiveData(sockfd, buffer_in) == 1){
				printf("\nClient: %s\n",buffer_in);
				if(strcmp("Y", buffer_in)){
					//write(sockfd, "ABORT", 5);
					if(sendData(sockfd, "ABORT", 5) != -1){
						//read(sockfd, buffer_in, BUFFER_SIZE);
						//if(receiveData(sockfd, buffer_in) == 1){
							printf("\nServer: Aborting.\n");
							//print("");
							return 0;
						//}
					}
				}
			}
		}
		
		
		
	}
	
	
	
	
	fd = fopen(filepath, "w");
	printf("\nfilepath while creating:%s\n", filepath);
	
	if(fd == NULL)
	{
		// couldn't create file
		//write(sockfd, "ABORT", 5);
		if(sendData(sockfd, "ABORT", 5) != -1){
			//read(sockfd, buffer_in, BUFFER_SIZE);
			if(receiveData(sockfd, buffer_in) == 1){
				printf("\nClient: %s\n",buffer_in);
				printf("\nServer: Error creating file.\n");
				//print("");
				return -1;
			}
		}
	}
	
	//write(sockfd, "OK", 2);
	if(sendData(sockfd, "OK", 2) != -1){
		memset(buffer_in, 0, sizeof(buffer_in));
			//read(sockfd, buffer, BUFFER_SIZE);
			if(receiveData(sockfd, buffer_in) == 1){
				if(strcmp("OK", buffer_in) == 0)
				{
					// fetch file contents
					while(1)
					{
						//bzero(buffer_in, BUFFER_SIZE);
						memset(buffer_in, 0, sizeof(buffer_in));
						recv(sockfd, buffer_in, BUFFER_SIZE, 0);
						//if(receiveData(sockfd, buffer) == 1){
							buffer_in[BUFFER_SIZE] = '\0';
							
							fwrite(buffer_in, sizeof(char), strlen(buffer_in), fd);
							if(strlen(buffer_in) < BUFFER_SIZE)
								break;
						//}
					}
					//bzero(buffer_in, BUFFER_SIZE);
					memset(buffer_in, 0, sizeof(buffer_in));
					sprintf(buffer_in, "\nSuccessfully recieved file %s.", filename);
					printf("\n%s\n",buffer_in);
				}
			}
	}
	fclose(fd);
				
	
	
	return 0;
}

int fetch_files_with_ext(int sockfd, char *extension)
{
	printf("\nFetching files with %s extension.\n", extension);
	char buffer[256];
	int count = 0, fetch_file_status;
	char filename[100];
	if(sendData(sockfd, "OK_COMMAND", 10) == -1){
		printf("\nAborting\n");
		return -1;
	}
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		if(receiveData(sockfd, buffer) == 1){
			printf("\nClient: %s\n",buffer);
			
			if(strcmp(buffer,"FILE_NAME")==0){
				if(sendData(sockfd, "OK_FILENAME", 11) != -1){
					memset(buffer, 0, sizeof(buffer));
					if(receiveData(sockfd, buffer) == 1){
						printf("\nClient: %s\n",buffer);
						strcpy(filename,buffer);
						if(sendData(sockfd, "OK_FILE_NOW", 11) != -1){
							memset(buffer, 0, sizeof(buffer));
							fetch_file_status=fetch_file(sockfd, filename);
							//write(sockfd, "OK", 2);
							if(fetch_file_status == 0){
								count++;
								if(sendData(sockfd, "NEXT_FILE", 9) == -1){
									printf("\nAborting here. As could not send data.\n");
									break;
								}
							}
							else{
								printf("\nProblem in fetching file.\n");
							}
						}
					}
				}
			}
			else if(strcmp(buffer, "DONE") == 0){// Indicates all files recieved
				break;
			}
			else{
				/*
				if(sendData(sockfd, "NEXT", 5) != -1){
				//read(sockfd, buffer, BUFFER_SIZE);
					if(receiveData(sockfd, buffer) == 1){
						if(strcmp(buffer,"NEXT")==0){
							continue;
						}
						else{
							printf("\nAborting here. As 'NEXT' is not received.\n");
							break;
						}
					}
				}
				*/
				printf("\nAborting here. As no known respose is received.\n");
				break;
			}
			
		}
		
		
	}
	printf("\n--Recieved %d file(s).\n", count);
	return 0;
}

int main(){
	printf("\n****Server****\n");
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;		// will point to the results

	memset(&hints, 0, sizeof(hints));	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;		// fill in my IP for me

		
	
	if ((status = getaddrinfo(NULL, "3491", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	else{
		printf("\ngetaddrinfo was successfully executed with status=%d\n", status);
		int socket_fileDesc;
		printf("\nGoing to execute socket operation.");
		socket_fileDesc = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if(socket_fileDesc != -1){
			//below code is for reusing any port if it is still occupied in kernel
			
			/*
			int yes=1;
			if (setsockopt(socket_fileDesc,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
				perror("setsockopt");
				exit(1);
			}*/
			printf("\nGoing to execute bind operation.");
			if(bind(socket_fileDesc, servinfo->ai_addr, servinfo->ai_addrlen) != -1){
				printf("\nGoing to execute listen operation.");
				if(listen(socket_fileDesc, MAX_BACKLOG_CONNECTIONS) != -1){
					struct sockaddr_storage their_addr;
				
					socklen_t addr_size;
					
					// now accept an incoming connection:
					addr_size = sizeof their_addr;
					int client_fileDesc;
					printf("\nGoing to execute accept operation.");
					
					client_fileDesc = accept(socket_fileDesc, (struct sockaddr *)&their_addr, &addr_size);
					
					if(client_fileDesc != -1){
						printf("\nServer is up and accepting connection(s)");
						
						char buffer_in[1000], message[1000];
						int bytesSent=-1, bytesReceived=-1;
						while(1){
							memset(buffer_in, 0, sizeof(buffer_in));
							//bzero(buff,100);
							bytesReceived=recv(client_fileDesc, buffer_in, 1000, 0);
							if(bytesReceived > 0){
								printf("\nData received: %s\n",buffer_in);
								printf("\nData length: %lu\n",strlen(buffer_in));
								
								int messageLength=strlen(message), messageLengthSent=-1;
								char *arguments[10];
								int noOfArguments = extractClientArguments(buffer_in, strlen(buffer_in), arguments);
								
								printf("\nNumber of arguments=%d\n",noOfArguments);
								
								
								if(noOfArguments == 0){
									continue;
								}
								else if(noOfArguments == 1){
									if( (strcmp("LS",arguments[0]) == 0) || (strcmp("LIST",arguments[0]) == 0) ){
										//ls(client_fileDesc);
										//char *lsResult=popen("ls","r");
										char lsResult[512];
										executeCommand1("ls", lsResult);
										printf("\nls result: ");
										fputs(lsResult, stdout);
										printf("\nsizeof(lsResult): %lu",sizeof(lsResult));
										sendData(client_fileDesc, lsResult, strlen(lsResult));
										sendData(client_fileDesc, "OOOKKK", 6);
									}
									else{
										sendData(client_fileDesc, "Acknowledged. No action is taken against the command.", 100);
									}
									
								}
								else if(noOfArguments >= 2){
									printf("\narguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
									printf("\nstrlen(arguments[1])=%zu\n",strlen(arguments[1]));
									printf("\narguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
									
									
									//sprintf(filename,"%*s",(int)strlen(arguments[1]), arguments[1]);
									
									
									if(strcmp("GET",arguments[0]) == 0){
										char filename[200];
										memset(filename, 0, sizeof(filename));
										strcpy(filename, arguments[1]);
										printf("\nfilename(1):=%s\n", filename);

										printf("\nfilename(2):=%s\n", filename);
										printf("\narguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
										//send_file(client_fileDesc, filename/*, strlen(filename)*/);
										send_file(client_fileDesc, arguments[1]/*, strlen(filename)*/);
									}
									else if(strcmp("MGET",arguments[0]) == 0){
										char file_extension[200];
										memset(file_extension, 0, sizeof(file_extension));
										strcpy(file_extension, arguments[1]);
										printf("\nfilename(1):=%s\n", file_extension);

										send_files_with_ext(client_fileDesc, file_extension);
									}
									else if(strcmp("PUT",arguments[0]) == 0){
										char filename[200];
										memset(filename, 0, sizeof(filename));
										strcpy(filename, arguments[1]);
										printf("\nfilename(1):=%s\n", filename);

										fetch_file(client_fileDesc, filename);
									}
									else if(strcmp("MPUT",arguments[0]) == 0){
										char file_extension[200];
										memset(file_extension, 0, sizeof(file_extension));
										strcpy(file_extension, arguments[1]);
										printf("\nfilename(1):=%s\n", file_extension);

										fetch_files_with_ext(client_fileDesc, file_extension);
									}
									else{
										sendData(client_fileDesc, "Acknowledged. No action is taken against the command.", 100);
									}
								}
								
								/*
								if(bytesReceived==sizeof(buffer_in)){
									continue;				//need to improve this function
													//by concatenating buffers of
													//every iteration
								
								}
								else{
									break;		//if received number of bytes are
											//less than total buffer_in size then
											//there is no more data going to come
								}
								*/
							
							
							}
							else if(bytesReceived == 0){
								printf("\nClient hase closed the connection.");
								//break;
								return -1;
							}
							else{
								printf("\nSomething unexpected is happening.");
								break;
							}
						}
					}
					else{
						printf("\nUnable to execute accept operation.");
					}
					
					
				}
				else{
					printf("\nUnable to execute listen operation.");
				}
			
				
			}
			else{
				printf("\nUnable to bind.");
			}
			
			
		}
		else{
			printf("\nUnable to create socket.");
		}
		
		
	}
	
	
	printf("\n");
}

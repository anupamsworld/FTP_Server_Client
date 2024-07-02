#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<dirent.h>

#define CWD "./client_DIR/"			//CWD=Current Working Directory
#define BUFFER_SIZE 1024

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
int sendData(int fileDesc, char *buffer_out, int buffer_out_length){
	//int messageLength=strlen(buffer_out);
	int messageLengthSent=-1;
	int status = sendall(fileDesc, buffer_out, &messageLengthSent, buffer_out_length);
	if(status != -1) {
		printf("\nbuffer_out_length: %d\n",buffer_out_length);
		printf("\nMessage is sent: %*s\n", buffer_out_length, buffer_out);
	}
	else{
		//perror("sendall");
		printf("\nAll data could not be sent.");
		printf("\nOnly %d bytes are sent because of the error!\n", messageLengthSent);
		return status;			//status is same as return value of sendall
	}
}
int receiveData(int fileDesc, char *buffer_in){
	/***** Start of receiving of data from Server *****/
	int isReceiveSuccess=-1;
	int bytesReceived=-1;
	while(1){
		bytesReceived=-1;
		memset(buffer_in, 0, sizeof(buffer_in));
		//bzero(buff,100);
		printf("\nGoing to receive message from server\n");
		fflush(stdout);
		bytesReceived=recv(fileDesc, buffer_in, BUFFER_SIZE, 0);
		if(bytesReceived > 0){
			//printf("\nbytesReceived: %d, BUFFER_SIZE: %d\n",bytesReceived,BUFFER_SIZE);
			if(bytesReceived==BUFFER_SIZE){
				continue;				//need to improve this function
									//by concatenating buffers of
									//every iteration
				
			}
			else{
				printf("\nin else\n");
				isReceiveSuccess=1;
				break;		//if received number of bytes are
						//less than total buffer_in size then
						//there is no more data going to come
			}
			
		}
		else if(bytesReceived == 0){
			printf("\nServer hase closed the connection.\n");
			isReceiveSuccess=0;
			break;
			//return -1;
		}
		else if(bytesReceived == -1){
			printf("\nNo message from server.\n");
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

int fetch_file(int sockfd, char *filename)
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
		
	char filepath[256];
	//bzero(filepath, 256);
	memset(filepath, 0, sizeof(filepath));
	strcpy(filepath, CWD);
	strcat(filepath, filename);
	
	printf("\nFull filepath=%s\n",filepath);


	if(receiveData(sockfd, buffer) == 1){
		printf("\nServer1: %s\n",buffer);
		
		if(strcmp("OK", buffer)){
			printf("\nERROR\n");
			return -1;
		}
	}


	// to check if file already exists in client directory
	FILE *fd = fopen(filepath, "r");
	


	if(fd != NULL)
	{
		sprintf(buffer, "%s already exists. Do you want to overwrite? (Y / N)", filename);
		printf("\nClient: %s\n",buffer);
		fflush(stdin);
		fgets(buffer, BUFFER_SIZE, stdin);
		if(buffer[0] != 'Y')
		{
			//write(sockfd, "ABORT", 5);
			if(sendData(sockfd, "ABORT", 5) != -1){
				//read(sockfd, buffer, BUFFER_SIZE);
				if(receiveData(sockfd, buffer) == 1){
					printf("\nClient: Aborting.\n");
					//print("");
					return -1;
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
			//read(sockfd, buffer, BUFFER_SIZE);
			if(receiveData(sockfd, buffer) == 1){
				//need to printf the buffer
				printf("\nClient: Error creating file.\n");
				//print("");
				return -1;
			}
		}
	}
	
	//write(sockfd, "OK", 2);
	if(sendData(sockfd, "OK", 2) != -1){
		// fetch file contents
		while(1)
		{
			//bzero(buffer, 1023);
			memset(buffer, 0, sizeof(buffer));
			recv(sockfd, buffer, 1023, 0);
			//if(receiveData(sockfd, buffer) == 1){
				buffer[1023] = '\0';
				
				fwrite(buffer, sizeof(char), strlen(buffer), fd);
				if(strlen(buffer) < 1023)
					break;
			//}
		}
		
		//bzero(buffer, BUFFER_SIZE);
		memset(buffer, 0, sizeof(buffer));
		
		sprintf(buffer, "Successfully recieved file %s.", filename);
		printf("\n%s\n",buffer);
		//print("");
	}
	fclose(fd);		
	
	
	return 0;
}
int fetch_files_with_ext(int sockfd, char *extension)
{
	printf("\nFetching files with %s extension.\n", extension);
	char buffer[BUFFER_SIZE];
	int count = 0, fetch_file_status;
	char filename[100];
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		if(receiveData(sockfd, buffer) == 1){
			printf("\nServer1: %s\n",buffer);
			
			if(strcmp(buffer,"FILE_NAME")==0){
				if(sendData(sockfd, "OK_FILENAME", 11) != -1){
					memset(buffer, 0, sizeof(buffer));
					if(receiveData(sockfd, buffer) == 1){
						printf("\nServer1: %s\n",buffer);
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
	char buffer[BUFFER_SIZE];
	char buffer_in[BUFFER_SIZE], buffer_out[BUFFER_SIZE];

	int areAllConditionsOk=0;

	if(receiveData(sockfd, buffer_in) == 1){
		printf("\nServer: %s\n",buffer_in);
		if(strcmp("OK", buffer_in)==0){
			//send_file(sockfd, filename);
			areAllConditionsOk=1;
		}
		else if(strcmp("EXIST", buffer_in)==0){
			memset(buffer_out, 0, sizeof(buffer_out));
			sprintf(buffer_out, "%s already exists. Do you want to overwrite? (Y / N)", filename);
			printf("\nClient: %s\n",buffer_out);
			memset(buffer_out, 0, sizeof(buffer_out));
			fflush(stdin);
			fgets(buffer_out, BUFFER_SIZE, stdin);
			printf("\nbefore copy: strlen(buffer_out)=%zu\n",strlen(buffer_out));
			strcpy(buffer_out, strtok(buffer_out, "\n"));
			printf("\nClient terminal input: %s\n",buffer_out);
			printf("\nafter copy: strlen(buffer_out)=%zu\n",strlen(buffer_out));
			printf("\nClient input: %s\n",buffer_out);
			if( (strcmp("Y", buffer_out) == 0) || (strcmp("N", buffer_out) == 0) ){
				printf("\ninside if before sending.\n");
				if(sendData(sockfd, buffer_out, strlen(buffer_out)) != -1){
					if(receiveData(sockfd, buffer_in) == 1){
						printf("\nServer: %s\n",buffer_in);
						if(strcmp("OK", buffer_in) == 0){
							areAllConditionsOk=1;
							
						}
						else if(strcmp("ABORT", buffer_in) == 0){
							printf("\nClient: File upload is aborted by server.\n");
							return -1;
						}
						else{
							printf("\nServer has sent something else.\n");
							return -1;
						}
					}
					else{
						printf("\nProblem in receiving data from server.\n");
						return -1;
					}
				}
				else{
					printf("\nError sending data to server.\n");
					return -1;
				}
			}
			else{
				printf("\nInput is neither Y nor N\n");
				return -1;
			}
			
		}
	}

	if(areAllConditionsOk==1){
		FILE *fd = fopen(filepath, "r");
									
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
			/*
			if(receiveData(sockfd, buffer) == 1){
				if(strcmp("OK", buffer) != 0)
				{
					//write(sockfd, "ABORT", 5);
					sendData(sockfd, "ABORT", 5);
					fclose(fd);
					return -1;
				}
					*/
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
				
				
			//}
			
		}
		fclose(fd);
		return 0;
	}

	return -1;
}

int send_files_with_ext(int sockfd, char *extension)
{
	char buffer[BUFFER_SIZE];
	
	DIR *di;
	struct dirent *dir;
	di = opendir(CWD);
	char *filename,*filename_only;
	char filename_temp[100];
	char *ext;
	int send_file_response=-5;//default value
	int count=0;

	if(receiveData(sockfd, buffer) == 1){
		printf("\nServer: %s\n",buffer);
		if(strcmp(buffer,"OK_COMMAND")!=0){
			return -1;
		}
	}

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
					printf("\nServer: %s\n",buffer);
					if(strcmp(buffer,"OK_FILENAME")==0){
						if(sendData(sockfd, filename, strlen(filename)) != -1){
							memset(buffer, 0, sizeof(buffer));
							if(receiveData(sockfd, buffer) == 1){
								printf("\nServer: %s\n",buffer);
								if(strcmp(buffer,"OK_FILE_NOW")==0){
									send_file_response=send_file(sockfd, filename);
									if(send_file_response==0){
										count++;
										memset(buffer, 0, sizeof(buffer));
										if(receiveData(sockfd, buffer) == 1){
											printf("\nServer: %s\n",buffer);
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
int main(int argc, char *argv[]){
	printf("\n****Client****\n");
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;		// will point to the results

	memset(&hints, 0, sizeof(hints));	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;		// fill in my IP for me

	//char server_ip[50], server_port[10];
	if (argc < 3) {
		//fprintf(stderr,"usage: showip hostname\n");
		printf("\nNo explicit ip and port is mentioned.\nAborting.");
		return 1;
	}
	else if(argc >= 3){
		printf("\nConnecting to %s:%s...", argv[1], argv[2]);
	}
	
	//getaddrinfo("127.0.0.1", "3490", &hints, &servinfo)
	if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	else{
		printf("\ngetaddrinfo was successfully executed with status=%d", status);
		int socket_fileDesc;
		printf("\nGoing to execute socket operation.");
		socket_fileDesc = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if(socket_fileDesc != -1){
		
			int socket_fileDesc;
			socket_fileDesc = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
			
			//below code is for reusing any port if it is still occupied in kernel
			
			/*
			int yes=1;
			if (setsockopt(socket_fileDesc,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
				perror("setsockopt");
				exit(1);
			}*/
			
			//bind(socket_fileDesc, servinfo->ai_addr, servinfo->ai_addrlen);
			//bind is not necessary for client
			
			
			if(connect(socket_fileDesc, servinfo->ai_addr, servinfo->ai_addrlen) != -1){
			
				printf("\nConnected to server!");
				char buffer_in[BUFFER_SIZE], buffer_out[BUFFER_SIZE];
				
				while(1){
					printf("\n\nClient Terminal >>> ");
					fflush(stdin);
					fgets(buffer_out, 1000, stdin);
					printf("\nbefore copy: strlen(buffer_out)=%zu\n",strlen(buffer_out));
					strcpy(buffer_out, strtok(buffer_out, "\n"));
					printf("\nClient terminal input: %s\n",buffer_out);
					printf("\nafter copy: strlen(buffer_out)=%zu\n",strlen(buffer_out));
					/***** Get the command *****/
					
					char *arguments[10];
					int noOfArguments = extractClientArguments(buffer_out, strlen(buffer_out), arguments);
					
					printf("\nNumber of arguments=%d\n",noOfArguments);
					
					if(noOfArguments == 0){
						continue;
					}
					else if(noOfArguments == 1){
						if( (strcmp("LS",arguments[0]) == 0) || (strcmp("LIST",arguments[0]) == 0) ){
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
								if(receiveData(socket_fileDesc, buffer_in) == 1){
									printf("\nServer: %s\n",buffer_in);
								}
							}
							
						}
						else{
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
								if(receiveData(socket_fileDesc, buffer_in) == 1){
									printf("\nServer: %s\n",buffer_in);
								}
							}
						}
					}
					else if(noOfArguments >= 2){
						if(strcmp("GET",arguments[0]) == 0){
							char filename[200];
							memset(filename, 0, sizeof(filename));
							//sprintf(filename,"%*s",(int)strlen(arguments[1]), arguments[1]);
							strcpy(filename, arguments[1]);
							printf("\nfilename(1):=%s\n", filename);
							char command[100];
							//strcpy(command,arguments[0]);
							
							//1st create a file if does not exist and keep it open for data write
							
							// Tell server command to be executed
							//write(socket_fileDesc, arguments[0], strlen(arguments[0]));
							printf("\nstrlen(buffer_out)=%zu\n",strlen(buffer_out));
							printf("\ntest\n");
							//printf("\nbefore send call 1: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
							printf("\nbefore send call 1.1: arguments[1]=%s\n",arguments[1]);
							printf("\nbefore send call 1.2: arguments[0]=%s\n",arguments[0]);
							printf("\ntest\n");
							printf("\nbefore send call 2: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){// Send Client command to server
								memset(buffer_in, 0, sizeof(buffer_in));
								printf("\nbefore receive call: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);//big confusion, arguments values are getting changed automatically
								fetch_file(socket_fileDesc, filename);
							}
							
							
							
						}
						else if(strcmp("MGET",arguments[0]) == 0){
							char file_extension[200];
							memset(file_extension, 0, sizeof(file_extension));
							//sprintf(file_extensione,"%*s",(int)strlen(arguments[1]), arguments[1]);
							strcpy(file_extension, arguments[1]);
							printf("\nfile_extension(1):=%s\n", file_extension);
							char command[100];
							//strcpy(command,arguments[0]);
							
							//1st create a file if does not exist and keep it open for data write
							
							// Tell server command to be executed
							//write(socket_fileDesc, arguments[0], strlen(arguments[0]));
							printf("\nstrlen(buffer_out)=%zu\n",strlen(buffer_out));
							printf("\ntest\n");
							//printf("\nbefore send call 1: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
							printf("\nbefore send call 1.1: arguments[1]=%s\n",arguments[1]);
							printf("\nbefore send call 1.2: arguments[0]=%s\n",arguments[0]);
							printf("\ntest\n");
							printf("\nbefore send call 2: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
								//receiveData(socket_fileDesc, buffer_in);
								
								//bzero(buffer_out, BUFFER_SIZE);
								memset(buffer_in, 0, sizeof(buffer_in));
								//read(sockfd, buffer, BUFFER_SIZE);
								printf("\nbefore receive call: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);//big confusion, arguments values are getting changed automatically
								
									
									// Send file name to server
									//write(sockfd, args[1], strlen(args[1]));
									//if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
									printf("\nafter receive call: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
										fetch_files_with_ext(socket_fileDesc, file_extension);
										//fetch_file(socket_fileDesc, arguments[1]);
									//}
								
								
							}
						}
						else if(strcmp("PUT",arguments[0]) == 0){
							char filename[200];
							memset(filename, 0, sizeof(filename));
							//sprintf(filename,"%*s",(int)strlen(arguments[1]), arguments[1]);
							strcpy(filename, arguments[1]);
							printf("\nfilename(1):=%s\n", filename);
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
								send_file(socket_fileDesc, filename);
							}
						}
						else if(strcmp("MPUT",arguments[0]) == 0){
							char file_extension[200];
							memset(file_extension, 0, sizeof(file_extension));
							//sprintf(file_extensione,"%*s",(int)strlen(arguments[1]), arguments[1]);
							strcpy(file_extension, arguments[1]);
							printf("\nfile_extension(1):=%s\n", file_extension);
							char command[100];
							//strcpy(command,arguments[0]);
							
							//1st create a file if does not exist and keep it open for data write
							
							// Tell server command to be executed
							//write(socket_fileDesc, arguments[0], strlen(arguments[0]));
							printf("\nstrlen(buffer_out)=%zu\n",strlen(buffer_out));
							printf("\ntest\n");
							//printf("\nbefore send call 1: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
							printf("\nbefore send call 1.1: arguments[1]=%s\n",arguments[1]);
							printf("\nbefore send call 1.2: arguments[0]=%s\n",arguments[0]);
							printf("\ntest\n");
							printf("\nbefore send call 2: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
								//receiveData(socket_fileDesc, buffer_in);
								
								//bzero(buffer_out, BUFFER_SIZE);
								memset(buffer_in, 0, sizeof(buffer_in));
								//read(sockfd, buffer, BUFFER_SIZE);
								printf("\nbefore receive call: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);//big confusion, arguments values are getting changed automatically
								
									
									// Send file name to server
									//write(sockfd, args[1], strlen(args[1]));
									//if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
									printf("\nafter receive call: arguments[0]=%s, arguments[1]=%s\n",arguments[0],arguments[1]);
										send_files_with_ext(socket_fileDesc, file_extension);
										//fetch_file(socket_fileDesc, arguments[1]);
									//}
								
								
							}
						}
						else{
							if(sendData(socket_fileDesc, buffer_out, strlen(buffer_out)) != -1){
								receiveData(socket_fileDesc, buffer_in);
							}
							
						}
					}
					
					///receive
					
				}
				
				
				
				
				char msg[255]="";
				
			}
			
			
		}
		else{
			printf("\nUnable to create socket.");
		}
	}
	


	
}

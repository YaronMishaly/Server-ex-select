#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
extern int h_errno;
#define  TRUE 1
#define  FALSE 0
#define NUM_OF_BYTS_SQR 20 // THE SIZE OF THE BUUFER **2
#define MAX_CLIENTS 5
#define PORT 2034



struct client
{	
	int index;
	int fd;
	int pipefd[2];
	int finished; // is the procces finished
	int is_error; // if error in the fork sys call 
	char error_buffer[100];
};

void close_connection(struct client* clients,int index);
char *trime_whiteSpace(char *str);
void clear_clone(char clone[NUM_OF_BYTS_SQR][NUM_OF_BYTS_SQR]);
int  make_args(char args[NUM_OF_BYTS_SQR][NUM_OF_BYTS_SQR] ,char* command_buffer);
void copy_string(char* src, char* target , int num_of_byts);
void add_client( struct client* clients, fd_set* readfds, fd_set* writefds,int* max_fd,int max_clients);
void add_to_list(int fd , struct client* clients , int max_clients);


/*
This program is a server that handle multiple client using select()
The client send the program he wish to preform and the server creats 
a child procces that preform the task 
using the exec sys call
The child procces and the parent procces communicate via a pipe
Author: Yaron Mishaly
*/
int main(int argc, char const *argv[])
{	
	char* message = "enter command to do: \n";

	


	int error_code;
	int opt = TRUE;
	char* ip = "127.0.0.1";
	int port = PORT;
	int server_socket;
	server_socket = socket(AF_INET,SOCK_STREAM ,IPPROTO_TCP);
	if (server_socket == -1)
	{
		printf("%s\n","error in socket" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}

	//set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("error in setsockopt");
        printf("%s\n", strerror(errno) );
        exit(-1);
    }
  

	struct sockaddr_in server_addres;
	server_addres.sin_family= AF_INET;
	server_addres.sin_port = htons(port);

	error_code = inet_aton(ip,&(server_addres.sin_addr));
	if (error_code == 0)
	{
		printf("%s\n","error in inet_aton" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}

	error_code = bind(server_socket,(struct sockaddr*)&server_addres,sizeof(server_addres));
	if (error_code == -1)
	{
		printf("%s\n","error in bind" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}
	listen(server_socket,MAX_CLIENTS);

	int max_clients = MAX_CLIENTS;
	int max_fd, num_clieant, sd,new_socket,value_read;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	struct client* clients = (struct client*) malloc(sizeof(struct client)*max_clients);

	for (int i = 0; i < max_clients ; ++i)
	{
		clients[i].fd = 0;
	}

	fd_set readfds , writefds , exceptfds;

	max_fd = server_socket;

	int activity;

	char* error_in_command = "sorry your command is incorrect! \n";

	char* command_buffer = (char*) malloc(NUM_OF_BYTS_SQR*NUM_OF_BYTS_SQR);
	char* response_buffer = (char*) malloc(1024);

	printf(" everything good listning for connections! \n");

	while(1){
		memset(command_buffer,'\0',400);
		memset(response_buffer,'\0',1024);

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		add_client(clients, &readfds,&writefds , &max_fd,max_clients);
		FD_SET(server_socket,&readfds);

		activity = select(max_fd+1,&readfds, &writefds,&exceptfds,NULL);
		if (activity<0)
			{
			printf("%s\n","error in select" );
    		printf("%s\n", strerror(errno) );
			}
		
		/*if the server socket is readable it means that
			a client is asking to connect */
		if (FD_ISSET(server_socket,&readfds) > 0)
		{	
			printf("%s\n", "new client!");
			new_socket = accept(server_socket,(struct sockaddr*)&address,&addrlen);

			//make the socket non-blocking
			int flags = fcntl(new_socket, F_GETFL, 0);
			fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);


			if (new_socket<0 )
			{
				printf("%s\n","error in accept" );
    			printf("%s\n", strerror(errno) );
			}
			else{
				add_to_list(new_socket,clients, max_clients); // add the new client to DB
				if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
            	{
                printf("%s\n","error in send" );
    			printf("%s\n", strerror(errno) );
            	}
			}
		}


		for (int i = 0; i < max_clients; ++i)
		{	
			

			memset(command_buffer,'\0',NUM_OF_BYTS_SQR*NUM_OF_BYTS_SQR);

			sd = clients[i].fd;

			//incoming message from client
			if (FD_ISSET(sd ,&readfds) > 0 && sd != 0 )
			{	
					

					int value_read = read(sd , command_buffer, NUM_OF_BYTS_SQR*NUM_OF_BYTS_SQR);



					if (value_read == -1)
					{
						printf("%s\n","error in read" );
	    				printf("%s\n", strerror(errno) );
					}
					else if(strcmp(command_buffer, "close") == 0){
						printf("%s\n","client is asking to leave" );
						close_connection(clients,i);
					}
					else if (value_read != 0)
					{
					char clone[NUM_OF_BYTS_SQR][NUM_OF_BYTS_SQR];

					clear_clone(clone);

					int num_of_args = make_args(clone,command_buffer);


					printf("%s%d\n","the num of args: ",num_of_args );

					char* args[num_of_args];
					for (int i = 0; i < num_of_args; ++i)
					{
						args[i] = clone[i];
					}

					args[num_of_args] = NULL;

					clients[i].finished = TRUE;

					printf("%s\n", "creating procces" );

					pid_t pid = fork();
					if (pid < 0 )
					{
						clients[i].is_error = TRUE;
						int j = 0;
						strcpy(clients[i].error_buffer,error_in_command);
						for (int k = strlen(error_in_command); k < 100; ++k)
						{
							clients[i].error_buffer[k] = strerror(errno)[j];
							j++;
						}
						printf("%s\n","error in fork" );
	    				printf("%s\n", strerror(errno) );
					}
					else if (pid == 0)
					{

						/* entering the child's procces */

						

						close(clients[i].pipefd[0]);
						//redircting the stdout
						int error_code2 =  dup2(clients[i].pipefd[1],1); //// send stdout to the pipe
						if (error_code2 == -1)
						{
							printf("%s\n","error in dup2" );
	    					printf("%s\n", strerror(errno) );
							
						}
						fcntl(clients[i].pipefd[1], F_SETFL, O_WRONLY);

						dup2(clients[i].pipefd[1],2); // send stderr to the pipe

						close(clients[i].pipefd[1]); //// this descriptor is no longer needed

						
						
						printf("The program you send  %s\n", args[0] );
						fflush(stdout);
						int ecode = execv(args[0],args);
						if (ecode == -1)
						{
							fprintf(stderr,"%s\n","Error in the program" );
	    					fprintf(stderr,"%s\n", strerror(errno) );
	    					fflush(stdout);
	    					exit(-1);
						}

					}
					
				}
			}
		}

		//write a message to the client
		sleep(1);
		for (int i = 0; i < max_clients; ++i)
		{
			memset(response_buffer,'\0',1024);
			sd = clients[i].fd;


			if (FD_ISSET(sd,&writefds) > 0 && sd != 0 &&  clients[i].is_error ==TRUE && clients[i].finished == TRUE)
			{

				printf("%s\n", "Trying to write the error" );

				clients[i].finished = FALSE;
				clients[i].is_error = FALSE;


				int bytes_send = send(sd , clients[i].error_buffer, 100 , 0);
				if (bytes_send != 100)
				{
					printf(" 1 error in sending the error  msg to the clients[%d] send only %d byts from total %d byts \n",i,bytes_send,(int)strlen(error_in_command) );
	    			printf("%s\n", strerror(errno) );
				}
			}

			if (FD_ISSET(sd,&writefds) > 0  && sd != 0  && clients[i].is_error== FALSE && clients[i].finished == TRUE)
			{
				printf("%s\n","Trying to write the answer" );

				clients[i].finished = FALSE;
				clients[i].is_error = FALSE;

				int stop = FALSE;
				
				//need a while loop because the pipe is in Non-blocking state
				while(stop ==FALSE){
					value_read = read(clients[i].pipefd[0],response_buffer,1024);
					stop = TRUE;
					printf("%d  byts  will be sent to client number %d \n", value_read,i);
					if (value_read == -1 && errno == EAGAIN)
					{
						printf("%s\n",  "The data has not been recived yet");
						printf("%s\n", strerror(errno));
						stop = FALSE;
					}
				}

				if (value_read <=  0 )
				{
					printf("%s\n","Error in read response from pipe" );
					printf("Pipe fd %d \n",clients[i].pipefd[0] );
	    			printf("%s\n", strerror(errno) );
				}
				else{
					int byts_send = send(sd , response_buffer , value_read , MSG_NOSIGNAL);
					if (byts_send == -1)
					{
						printf("Error in client number %d\n", i);
						close_connection(clients,i);
					}
					if (byts_send != value_read)
					{
						printf("Error in sending the result to the client sent only %d byts from total %d byts \n",byts_send,value_read );
	    				printf("%s\n", strerror(errno) );
					}
				
			}
				
			}
			
		}

		//if error than close the socket
		for (int i = 0; i < max_clients; ++i)
		{
			sd = clients[i].fd;
			if (sd != 0 && FD_ISSET(sd,&exceptfds))
			{
				printf("Error in client in index %d\n",i );
				close_connection(clients,i);
			}
		}



	}


	// server shutdown
	close(server_socket);
	free(response_buffer);
	free(command_buffer);

	printf("%s\n", "Closed!");

	return 0;
}

/*
create the client state in the struct
*/
void add_to_list(int fd , struct client* clients , int max_clients){
	int pipefd[2];
	for (int i = 0; i < max_clients; ++i)
	{
		if (clients[i].fd == 0)
		{
			clients[i].index = i;
			clients[i].fd = fd;
			clients[i].finished = FALSE;
			clients[i].is_error = FALSE;
			int error_code = pipe(pipefd);
			if (error_code == -1)
			{
				printf("%s\n","error in pipe" );
    			printf("%s\n", strerror(errno) );
			}

			clients[i].pipefd[0] = pipefd[0]; //pipe's read fd
			clients[i].pipefd[1] = pipefd[1]; //pipe's write fd

			//make the pipe non-blocking for read only
			int flags = fcntl(clients[i].pipefd[0], F_GETFL, 0);
			fcntl(clients[i].pipefd[0], F_SETFL, flags | O_NONBLOCK);

			break;
		}
	}
}


/*
each iteration of select this function 
add the client that his fd is != 0 in clients[]
 to the readfds and writefds
*/
void add_client( struct client* clients, fd_set* readfds, fd_set* writefds,int* max_fd,int max_clients){
	int fd;
	for (int i = 0; i < max_clients; ++i)
	{
		//memset(clients[i].error_buffer,'\0',100);
		fd = clients[i].fd;

		//printf("fd  is %d \n",fd );
		if (fd > 0 )
		{
			FD_SET(fd,readfds);
			FD_SET(fd,writefds);
		}
		if (fd > *max_fd)
		{
			*max_fd = fd;
		}
	}
}


void copy_string(char* src, char* target , int num_of_byts){
	for (int i = 0; i < num_of_byts; ++i)
	{
		target[i] = src[i];
	}

}

/*
make the args to the exec sys call 
Return value: the number of args 
*/
int  make_args(char args[NUM_OF_BYTS_SQR][NUM_OF_BYTS_SQR] ,char* command_buffer){				
int count = 0;
int j = 0;

trime_whiteSpace(command_buffer);
	
for (int i = 0; i < strlen(command_buffer); ++i)
{
	if (count == NUM_OF_BYTS_SQR)
	{
		printf("%s%d\n","count 1 is : " , count);
		return count;
	}


	if (command_buffer[i] == ' ' ||  command_buffer[i] == '\n')
	{
		args[count][j] = '\0';
		count++;
		j = 0; //for the next word index
	}
	else{
		args[count][j] = command_buffer[i];
		j++;
	}
}


return count+1 ;
}


/*
Trime the space from str in the beginning and the end of the string
Return value:The str without the space
*/
char *trime_whiteSpace(char *str){
	
	char *end;
	while(*str == ' ')str++;

	end = str+ strlen(str) - 1;

	while(*end == ' ' && end > str) end --;

	*(end+1) == '\0';

	return str;


}

//clearing the clone array
void clear_clone(char clone[NUM_OF_BYTS_SQR][NUM_OF_BYTS_SQR] ){
	for (int i = 0; i < NUM_OF_BYTS_SQR; ++i)
	{
		for (int j = 0; j < NUM_OF_BYTS_SQR; ++j)
		{
			clone[i][j] = '\0';
		}
	}
}

//Close the socket and the pipe
void close_connection(struct client* clients,int index){
	close(clients[index].fd);
	close(clients[index].pipefd[0]);
	clients[index].fd = 0;
}
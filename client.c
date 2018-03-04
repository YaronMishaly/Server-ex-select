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
extern int h_errno;
#define  TRUE 1
#define  FALSE 0
#define PORT 2034

int count_arr(char* str , char ch, int len);
void copy_string(char* src, char* target , int num_of_byts);


int main(int argc, char const *argv[])
{
	char* ip = "127.0.0.1";
	int port = PORT;
	int my_socket;
	my_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if (my_socket == 0)
	{
		printf("%s\n","error in socket" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}

	struct sockaddr_in server_addres;
	server_addres.sin_family= AF_INET;
	server_addres.sin_port = htons(port);

	int error_code = inet_aton(ip,&(server_addres.sin_addr));
	if (error_code == 0)
	{
		printf("%s\n","error in inet_aton" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}

	error_code = connect(my_socket,(struct sockaddr*)&server_addres , sizeof(server_addres) );
	if (error_code == -1)
	{
		printf("%s\n","error in connect" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}
	char* command_buffer = (char*) malloc(100);
	char* response_buffer = (char*) malloc(1024);

	for (int i = 0; i < 100; ++i)
	{
		command_buffer[i] = '\0';
	}

	memset(response_buffer,'\0',1024);


	printf(" passing messages in while \n");

	int value_read  = read(my_socket,response_buffer,1024);

	if (value_read <= 0 )
	{
		printf("%s\n","error in read first messages from server" );
    	printf("%s\n", strerror(errno) );
    	exit(-1);
	}
	else{
		printf("%s\n%s","the server sent: " , response_buffer);
	}

	int bytes_send;
	int stop = FALSE;
	//int stop_waiting = FALSE;



	while(stop == FALSE){
		memset(command_buffer,'\0',100);
		printf("%s\n", "enter your command");
		fgets(command_buffer, 100, stdin);

    /* Remove trailing newline, if there. */

    	if ((strlen(command_buffer) > 0) && (command_buffer[strlen (command_buffer) - 1] == '\n'))
        	command_buffer[strlen (command_buffer) - 1] = '\0';

        

		bytes_send = send(my_socket,command_buffer,100, 0);


		if (bytes_send <= 0 )
		{
			printf("%s\n","error in send messages from server" );
    		printf("%s\n", strerror(errno) );

		}
		else{
			printf("the client sending: %s\n",command_buffer );
			//stop_waiting = FALSE;

			if (strcmp(command_buffer,"close") == 0)
			{
				printf("%s\n", "closing connection");
				free(command_buffer);
				free(response_buffer);
				close(my_socket);
				exit(0);
			}
			//while(stop_waiting == FALSE){
				memset(response_buffer,'\0',1024);
				value_read = read(my_socket,response_buffer,1024);

				//printf("the clients value read %d\n",value_read );

				if (value_read > 0 )
				{
					printf("%s\n%s","the server sent:", response_buffer);
				}
			//}
		}
	}

	return 0;
}

void copy_string(char* src, char* target , int num_of_byts){
	for (int i = 0; i < num_of_byts; ++i)
	{
		target[i] = src[i];
	}

}

int count_arr(char* str , char ch, int len){
	for (int i = 0; i < len; ++i)
	{
		if (str[i] == ch)
		{
			return (i+1);
		}
	}
	return len;
}
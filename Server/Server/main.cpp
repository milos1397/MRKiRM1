
//#include <pthread.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

struct thread_data
{
	SOCKET socket;
	int id;
} ;

void checkOpt(char* message, int receivedBytes, int socket, int id)
{
	FILE *fptr;
	char file[30];
	strcpy(file, "Clients/");
	strcpy(file + strlen(file), message + 2);
	strcpy(file + strlen(file),".txt");
	fptr = fopen(file,"r");
	
	if(fptr == NULL)
	{
		printf("Unable to open file, client - %d\n", id);
	}else
	{
		int bufferLength = 255;
		char buffer[255];

		//skip the first line because it's password
		fgets(buffer, bufferLength, fptr);

		while(fgets(buffer, bufferLength, fptr)!=NULL) 
		{
			if(buffer[0] != '\n')
			{
				printf("Check from %d, sending %s", id, buffer);
				send(socket, buffer,strlen(buffer), 0);
				Sleep(100); //da bi mogao odvojeno da posalje, odnosno da bi na prijemu se regularno primilo
			}
		}
		//xxx means end of sending
		Sleep(900);
		char* end;
		end = "xxx\0";
		send(socket, end, strlen(end), 0);
		fclose(fptr);
	}
}

void rcvOpt(char* message, int receivedBytes, int socket, int id)
{
	//number of message client wants to receive == line number in the file
	char lineNumb[3];
	strcpy(lineNumb, message + 2);
	
	//convert to the number
	char* end;
	long int result = strtol(lineNumb, &end, 10);

	//receive username, in order to know which file to read
	char user[20];
	int recbyts = recv(socket , user , sizeof(user) , 0);
	user[recbyts] = '\0';

	//open file
	FILE *fptr;
	char file[30];
	strcpy(file, "Clients/");
	strcpy(file + strlen(file), user);
	strcpy(file + strlen(file), ".txt"); 

	fptr = fopen(file,"r");

	if(fptr==NULL)
	{
		printf("Unable to open file - client %d\n", id);
	}else
	{
		char buffer[255];

		int count = 1;
		fgets(buffer, 255, fptr);
		while (fgets(buffer, 255, fptr) != NULL) /* read a line */
		{
			//skip empty lines
			if(buffer[0]!='\n')
			{
				if (count == result)
				{
					fclose(fptr);
					break;
				}
				else
				{
					count++;
				}
			}
			Sleep(100);
		}
		printf("\n Line which is being sent: %s, client - %d", buffer, id);
		send(socket, buffer, strlen(buffer), 0);
		fclose(fptr);
	}
}

void sendOpt(char* message, int receivedBytes, int socket, int id)
{
	FILE* fptr;
	char file[30];
	strcpy(file, "Clients/");
	strcpy(file + strlen(file), message + 2);
	strcpy(file + strlen(file), ".txt");
	fptr = fopen(file,"r");
	
	//checks is receiver exists
	if(fptr == NULL)
	{
		printf("Unable to open file - client %d\n", id);
		char incorrect[22] = "User does not exist!\n";
		send(socket, incorrect,sizeof(incorrect), 0); 
	}else
	{
			fclose(fptr);
			//I close file and then open it, because if I put "a" at the beginning of the function, it will create the file
			fptr = fopen(file, "a");


			char msg[200];
			int recByt;
			recByt = recv(socket , msg , sizeof(msg) , 0);
			printf(" Received message from client %d to send\n", id);
			msg[recByt] = 0;
			fprintf(fptr, msg);
			fclose(fptr);

			char correct[15] = "Message sent!\n";
			send(socket, correct ,sizeof(correct), 0);
	}
}

void userCheck(char* message, int receivedBytes, int socket, int id)
{
		FILE *fptr;
		char file[30];
		strcpy(file, "Clients/");
		strcpy(file + strlen(file), message + 5);
		strcpy(file + strlen(file), ".txt");

		fptr = fopen(file, "r");
		//if file exists, it means username is correct
		if(fptr == NULL)
		{
			char* incorrect = "-";
			send(socket, incorrect,sizeof(incorrect), 0);        
		}
		else
		{
			char* correct = "+";
			send(socket, correct,sizeof(correct), 0);
			printf("Username correct to the client %d\n", id);
			printf("Waiting for the password from the client %d\n", id);
			//wait for the password
			int recbyts;
			char received[20];
			
			//receive password
			recbyts = recv(socket , received , sizeof(received) , 0);
			if (recbyts == 0)
			{
				printf("Disconnected from server!\n");
			}else if (recbyts < 0) 
			{
				DWORD err = WSAGetLastError();
			}else
			{

				received[recbyts] = '\0';
				printf("Received password from the client %d: %s \n", id, received);

				int bufferLength = 255;
				char buffer[255];
					
				char* password = new char[recbyts-5];
				strcpy(password, received + 5);

				//read the password from the first line of a file
				fgets(buffer, bufferLength, fptr);

				//comparing password from the file and the one which user sent, need to compare length, because
				if(strncmp(buffer, password, strlen(buffer)-1) == 0 && ((strlen(buffer) - 1) == strlen(password)))
				{
					char* correct = "+";
					send(socket, correct,sizeof(correct), 0);
					printf("Password correct - client %d\n", id);
				}else
				{
					char* incorrect = "-";
					send(socket, incorrect,sizeof(incorrect), 0);
					printf("Password incorrect - client %d\n", id);
				}
			}
			fclose(fptr);
		}
	return;
}

void checkMessage(char* message, int receivedBytes, int socket, int id)
{
			
		if(strncmp(message, "user", 4) == 0)
		{
			//checks whether user exists and password
			userCheck(message, receivedBytes, socket, id);
		}else if(strncmp(message, "1 ", 2) == 0)
		{
			//returns all the messages for the current user
			checkOpt(message, receivedBytes, socket, id);
		}else if(strncmp(message, "2 ", 2) == 0)
		{
			//returns the specific message selected by the user
			rcvOpt(message, receivedBytes, socket, id);
		}else if(strncmp(message, "3 ", 2) == 0)
		{
			//sends message to specific user
			sendOpt(message, receivedBytes, socket, id);
		}
		return;
}

DWORD WINAPI socketThread(LPVOID arg)
{
	int nReceivedBytes;
	char clientMessage[200];
	struct thread_data td = *( (struct thread_data *)arg);
	int socket = td.socket;
	int id = td.id;
	char *data = "ACK";

	send(socket, data, sizeof(data), 0);

	do
	{
		nReceivedBytes = recv(socket , clientMessage , sizeof(clientMessage) , 0);
		if (nReceivedBytes == 0)
		{
			printf("Disconnected from server!\n");
			break;
		}else if (nReceivedBytes < 0) 
		{
			printf("error\n");
			DWORD err = WSAGetLastError();
			break;
		}else
		{
			clientMessage[nReceivedBytes] = '\0';
			printf("Received %s \n from client %d \n", clientMessage, id);

			checkMessage(clientMessage, nReceivedBytes, socket, id);
		}
	}while(1);
	return 0;	
}

int main(int argc , char *argv[])
{
	WSADATA wsa;
	SOCKET s , newSocket;
	struct sockaddr_in server , client;
	int c;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	
	printf("Initialised.\n");
	
	//Create a socket
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY; //mozda ovde treba menjati
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	puts("Bind done");

	//Listen to incoming connections
	listen(s , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	
	c = sizeof(struct sockaddr_in);
	
    DWORD tid[60];
    int i = 0;
	struct thread_data td;
	while( (newSocket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		i++;
		puts("Connection accepted");
		td.socket = newSocket;
		td.id = i;
		
        if( CreateThread(NULL, 0, socketThread, (LPVOID)&td, 0, &tid[i]) == NULL )
			printf("Failed to create thread\n");
	}
	
	if (newSocket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		return 1;
	}

	closesocket(s);
	WSACleanup();
	
	return 0;
}

#include <pthread.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

void check_message(char* message,int received_bytes,int socket)
{
			//za ove provere bih mogao jos jednu funkciju da napravim, da bude modularnije jer ce u suprtonom biti ogromna
		if(message[0]=='u' && message[1]=='s' && message[2]=='e' && message[3]=='r')
		{
			FILE *fptr;
		   // use appropriate location if you are using MacOS or Linux
			char* file = new char[received_bytes+4];
			strcpy(file,message+5);
			strcpy(file+strlen(message)-5,".txt");
			//printf("%s",file);
			/*char* path = new char[2+nReceivedBytes+4];
			strcpy(path,"./");
			strcpy(path+2,file);
			printf("%s",path);*/
			fptr = fopen(file,"r");
			if(fptr == NULL)
		    {
				char* incorrect = "-";
				if (send(socket, incorrect,sizeof(incorrect), 0) != sizeof(incorrect)) {
					//delete [] data;
				} else {
					printf("SENT: %s ",incorrect);
					//delete [] data;
				}           
		    }
			else
			{
				char* correct = "+";
				if (send(socket, correct,sizeof(correct), 0) != sizeof(correct)) {
					//delete [] data;
				} else {
					printf("SENT: %s\n",correct);
					printf("Waiting for the password...\n");
					//wait for the passwrod
					int recbyts;
					char rec[20];
					recbyts=recv(socket , rec , sizeof(rec) , 0);
					rec[recbyts]='\0';
					printf("Primio %s \n",rec);

					int bufferLength = 255;
					char buffer[255];
					
					char* password = new char[recbyts-5];
					strcpy(password,rec+5);

					fgets(buffer, bufferLength, fptr);

					printf("%s\n",buffer);
					printf("%s\n",password);

					printf("%d",strlen(buffer));
					if(strncmp(buffer,password,strlen(buffer)-1)==0)
					{
						printf("Dobra sifra");
					}else
					{
						printf("Nije dobra sifra");
					}
				}
				fclose(fptr);
			}
		}
		return;
}

void * socket_Thread(void * arg)
{
	int nReceivedBytes;
	char client_message[20];
	printf("U niti sam\n");
	int socket = *((int*)arg);
	char *data = "ACK";
	if (send(socket, data,sizeof(data), 0) != sizeof(data)) {
		//delete [] data;
	} else {
		printf("SENT: %s ",data);
		//delete [] data;
	}
	do{
		nReceivedBytes=recv(socket , client_message , sizeof(client_message) , 0);
		client_message[nReceivedBytes]='\0';
		printf("Primio %s \n",client_message);
		if (nReceivedBytes == 0)
		{
			printf("Disconnected from server!\n");
			break;
		}
		if (nReceivedBytes < 0) {
			printf("error\n");
			DWORD err = WSAGetLastError();
			break;
		}

		check_message(client_message,nReceivedBytes,socket);

		//Sleep(1000); 
			
	} while(1);
	while(1);
	return 0;	
}

int main(int argc , char *argv[])
{
	WSADATA wsa;
	SOCKET s , new_socket;
	struct sockaddr_in server , client;
	int c;
//	char *message;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
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
	
    pthread_t tid[60];
    int i = 0;
	while( (new_socket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		puts("Connection accepted");
		
		//Reply to the client
		/*message = "Hello Client , I have received your connection. But I have to go now, bye\n";
		send(new_socket , message , strlen(message) , 0);*/
		//thread *t = new thread(on_client_connect, ref(*this), client);

        //for each client request creates a thread and assign the client request to it to process
       //so the main thread can entertain next request
        if( pthread_create(&tid[i], NULL, socket_Thread, &new_socket) != 0 )
           printf("Failed to create thread\n");
        if( i >= 50)
        {
          i = 0;
          while(i < 50)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
	}
	
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		return 1;
	}

	closesocket(s);
	WSACleanup();
	
	return 0;
}
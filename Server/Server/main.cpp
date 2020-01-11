
#include <pthread.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

void * socket_Thread(void * arg)
{
	int nReceivedBytes;
	char client_message[20];
	printf("U niti sam");
	int socket = *((int*)arg);
	char *data = "ACK";
	if (send(socket, data,sizeof(data), 0) != sizeof(data)) {
		//delete [] data;
	} else {
		printf("SENT: %s",data);
		//delete [] data;
	}
	do{
		nReceivedBytes=recv(socket , client_message , sizeof(client_message) , 0);
		client_message[20]='\0';
		printf("Primio %s",client_message);
		if (nReceivedBytes == 0)
		{
			printf("Disconnected from server!\n");
			//pParent->FSM_Ch_Connected_Sock_Disconected();
			break;
		}
		if (nReceivedBytes < 0) {
			printf("error\n");
			DWORD err = WSAGetLastError();
			break;
		}
		//pParent->NetMsg_2_FSMMsg(buffer, nReceivedBytes);

		Sleep(1000); 
			
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
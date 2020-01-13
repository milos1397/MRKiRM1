
#include <pthread.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

void check_opt(char* message,int received_bytes,int socket)
{
	FILE *fptr;
	char *file = new char[received_bytes-2+4]; //2 karaktera za 1 i 4 za .txt
	strcpy(file,message+2);
	strcpy(file+strlen(message)-2,".txt");
	fptr = fopen(file,"r");
	
	if(fptr==NULL)
	{
		printf("Unable to open file\n");
	}else
	{
		int bufferLength = 255;
		char buffer[255];

		fgets(buffer, bufferLength, fptr);

		while(fgets(buffer, bufferLength, fptr)!=NULL) 
		{
			if(buffer[0]!='\n')
			{
				printf("%s",buffer);
				if (send(socket, buffer,strlen(buffer), 0) != strlen(buffer)) 
				{
							//delete [] data;
					printf("neuspeh");
				} else 
				{
					printf("SENT: %s ",buffer);
							//delete [] data;		
				} 
				Sleep(100); //da bi mogao odvojeno da posalje, odnosno da bi na prijemu se regularno primilo
			}
		}
		//oznaka za kraj slanja
		Sleep(900);
		char* end;
		end = "xxx\0";
		if (send(socket,end,strlen(end), 0) != strlen(end)) 
		{
					//delete [] data;
			printf("neuspeh");
		} else 
		{
			printf("SENT: %s",end);
					//delete [] data;		
		} 
		fclose(fptr);
	}

}

void rcv_opt(char* message,int received_bytes,int socket)
{
	char* line_numb = new char[3];
	strcpy(line_numb,message+2);
	
	//printf("%s",line_numb);
	char* end;
	long int result = strtol(line_numb, &end, 10);
	//printf("br_lini %d \n", result);

	//while(1);

	char user[255];
	int recbyts=recv(socket , user , sizeof(user) , 0);
	user[recbyts]='\0';
	//printf("Cela poruka: %s",user);


	FILE *fptr;
	char *file = new char[recbyts]; //2 karaktera za 1 i 4 za .txt i 3 za broj
	strcpy(file,user);
	strcpy(file+received_bytes+1,".txt"); //isto kao gore
	//printf("file %s",file);
	fptr = fopen(file,"r");

	
	if(fptr==NULL)
	{
		printf("Unable to open file\n");
	}else
	{
		int bufferLength = 255;
		char buffer[255];

		//fgets(buffer, bufferLength, fptr);
		printf("Otvorio fajl ");
		printf("Br linije %d ", result);
		//while(1);
		int i;
		for(i = 0;i <= result; i++)
		{
			fgets(buffer, bufferLength, fptr);
		}
		//fprintf(fptr,"Nes");

		printf("\n Bafer %s", buffer);
		if (send(socket, buffer,strlen(buffer), 0) != strlen(buffer)) 
		{
			printf("neuspeh");
		} else 
		{
			printf("SENT: %s ",buffer);	
		} 
		/*while(fgets(buffer, bufferLength, fptr)!=NULL) 
		{
			if(buffer[0]!='\n')
			{
				printf("%s",buffer);
				if (send(socket, buffer,strlen(buffer), 0) != strlen(buffer)) 
				{
							//delete [] data;
					printf("neuspeh");
				} else 
				{
					printf("SENT: %s ",buffer);
							//delete [] data;		
				} 
				Sleep(100); //da bi mogao odvojeno da posalje, odnosno da bi na prijemu se regularno primilo
			}
		}*/
		//oznaka za kraj slanja
		/*Sleep(900);
		char* end;
		end = "xxx\0";
		if (send(socket,end,strlen(end), 0) != strlen(end)) 
		{
					//delete [] data;
			printf("neuspeh");
		} else 
		{
			printf("SENT: %s",end);
					//delete [] data;		
		} */
		fclose(fptr);
	}
}

void check_message(char* message,int received_bytes,int socket)
{
			//za ove provere bih mogao jos jednu funkciju da napravim, da bude modularnije jer ce u suprtonom biti ogromna
		if(strncmp(message,"user",4)==0)
		{
			//ovo bih sve mogao u jednu funkciju
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

					if(strncmp(buffer,password,strlen(buffer)-1)==0)
					{
						char* correct = "+";
						if (send(socket, correct,sizeof(correct), 0) != sizeof(correct)) {
							//delete [] data;
						} else {
							printf("SENT: %s ",correct);
							//delete [] data;
						} 
					}else
					{
						char* incorrect = "-";
						if (send(socket, incorrect,sizeof(incorrect), 0) != sizeof(incorrect)) {
							//delete [] data;
						} else {
							//nesto
						}
					}
				}
				fclose(fptr);
			}
		}else if(strncmp(message,"1 ",2)==0)
		{
			printf("Usao u 1");
			check_opt(message,received_bytes,socket);
		}else if(strncmp(message,"2 ",2)==0)
		{
			printf("Usao u 2");
			rcv_opt(message,received_bytes,socket);
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
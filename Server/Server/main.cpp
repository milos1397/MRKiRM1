
//#include <pthread.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

void checkOpt(char* message, int receivedBytes, int socket)
{
	FILE *fptr;
	char *file = new char[receivedBytes - 2 + 12 + 1]; //2 karaktera za 1 i 4 za .txt I 8 za Clients/, 1 jos mora da se doda
	strcpy(file, "Clients/");
	strcpy(file + strlen(file), message + 2);
	strcpy(file + strlen(file),".txt");
	fptr = fopen(file,"r");
	
	if(fptr == NULL)
	{
		printf("Unable to open file\n");
	}else
	{
		int bufferLength = 255;
		char buffer[255];

		fgets(buffer, bufferLength, fptr);

		while(fgets(buffer, bufferLength, fptr)!=NULL) 
		{
			if(buffer[0] != '\n')
			{
				printf("%s", buffer);
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
		if (send(socket, end, strlen(end), 0) != strlen(end)) 
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
	delete file;

}

void rcvOpt(char* message, int receivedBytes, int socket)
{
	//broj linije koju zelim da primim
	char* lineNumb = new char[3];
	strcpy(lineNumb, message + 2);
	
	//pretvorim je u broj
	char* end;
	long int result = strtol(lineNumb, &end, 10);

	//primim user-a, da bih znao koji fajl da otvorim i procitam tu liniju
	char user[20];
	int recbyts = recv(socket , user , sizeof(user) , 0);
	user[recbyts] = '\0';

	//otvaram fajl
	FILE *fptr;
	char *file = new char[recbyts + 12]; //8 za Clients\ i 4 za .txt
	strcpy(file, "Clients/");
	strcpy(file + strlen(file), user);
	strcpy(file + strlen(file), ".txt"); 

	fptr = fopen(file,"r");

	delete [] file;

	if(fptr==NULL)
	{
		printf("Unable to open file\n");
	}else
	{
		char buffer[255];
		if(buffer == NULL){
			printf("nl");
		}

		printf("Otvorio fajl ");
		printf("Br linije %d ", result);

		int count = 1;
		fgets(buffer, 255, fptr);
		printf("Odradi jednom");
		while (fgets(buffer, 255, fptr) != NULL) /* read a line */
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
			Sleep(100);
		}

		printf("\n Bafer %s", buffer);
		if (send(socket, buffer,strlen(buffer), 0) != strlen(buffer)) 
		{
			printf("neuspeh");
		} else 
		{
			printf("SENT: %s ",buffer);	
		} 
		fclose(fptr);
	}

	delete [] lineNumb;
	delete [] file;
}

void sendOpt(char* message, int receivedBytes, int socket)
{
	FILE* fptr;
	char* file = new char[receivedBytes - 2 + 12 + 1]; //2 za komandu, 4 za.txt i jedan za termination, 8 za Clients/
	strcpy(file, "Clients/");
	strcpy(file + strlen(file), message + 2);
	strcpy(file + strlen(file), ".txt");
	printf("Fajll %s", file);
	fptr = fopen(file,"r");
	
	if(fptr == NULL)
	{
		printf("Unable to open file\n");
		char incorrect[22] = "User does not exist!\n";
		if (send(socket, incorrect,sizeof(incorrect), 0) != sizeof(incorrect)) {
			//delete [] data;
		} else {
			printf("SENT: %s ",incorrect);
			//delete [] data;
		}     
	}else
	{
			fclose(fptr);//ovo uradim jer ako odmah otvorim fajl sa a, on ce ga napraviti ako ne postoji
			fptr = fopen(file, "a");


			char msg[200];
			int recByt;
			recByt = recv(socket , msg , sizeof(msg) , 0);
			msg[recByt] = 0;
			printf("poruka %s", msg);
			fprintf(fptr, msg);
			fclose(fptr);

			char correct[15] = "Message sent!\n";
			if (send(socket, correct,sizeof(correct), 0) != sizeof(correct)) {
				//delete [] data;
			} else {
				printf("SENT: %s ",correct);
				//delete [] data;
			} 
	}
	printf(" %s", message);
}

void userCheck(char* message, int receivedBytes, int socket)
{
		FILE *fptr;
		char* file = new char[receivedBytes + 12]; //8 za Clients/ i 4 za .txt
		strcpy(file, "Clients/");
		strcpy(file + strlen(file), message + 5);
		strcpy(file + strlen(file), ".txt");

		fptr = fopen(file, "r");
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
			} else 
			{
				printf("SENT: %s\n",correct);
				printf("Waiting for the password...\n");
				//wait for the password
				int recbyts;
				char received[20];
			
				recbyts = recv(socket , received , sizeof(received) , 0);
				if (recbyts == 0)
				{
					printf("Disconnected from server!\n");
				}else if (recbyts < 0) 
				{
					printf("error\n");
					DWORD err = WSAGetLastError();
				}else
				{

					received[recbyts] = '\0';
					printf("Primio %s \n", received);

					int bufferLength = 255;
					char buffer[255];
					
					char* password = new char[recbyts-5];
					strcpy(password, received + 5);

					fgets(buffer, bufferLength, fptr);

					printf("%s\n", buffer);
					printf("%s\n", password);

					//moram i duzinu porediti, jer da nisam poredio u slucaju kada bi mi korisnik uneo duzu sifru
					//ali sa istim pocetkom, sve bi bilo ok
					if(strncmp(buffer, password, strlen(buffer)-1) == 0 && ((strlen(buffer) - 1) == strlen(password)))
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
						} else 
						{
							//nesto
						}
					}
				}
			}
			fclose(fptr);
		}
	return;
}

void checkMessage(char* message, int receivedBytes, int socket)
{
			//za ove provere bih mogao jos jednu funkciju da napravim, da bude modularnije jer ce u suprtonom biti ogromna
		if(strncmp(message, "user", 4) == 0)
		{
			//ovo bih sve mogao u jednu funkciju
			userCheck(message,receivedBytes,socket);
		}else if(strncmp(message, "1 ", 2) == 0)
		{
			//printf("Usao u 1");
			checkOpt(message, receivedBytes, socket);
		}else if(strncmp(message, "2 ", 2) == 0)
		{
			//printf("Usao u 2");
			rcvOpt(message, receivedBytes, socket);
		}else if(strncmp(message, "3 ", 2) == 0)
		{
			//printf("Usao u 3");
			sendOpt(message, receivedBytes, socket);
		}
		return;
}

DWORD WINAPI socketThread(LPVOID arg)
{
	int nReceivedBytes;
	char clientMessage[200];
	int socket = *( (int*) arg);
	char *data = "ACK";

	if (send(socket, data, sizeof(data), 0) != sizeof(data)) 
	{
		//delete [] data;
	} else 
	{
		printf("SENT: %s ",data);
		//delete [] data;
	}
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
			printf("Primio %s \n", clientMessage);
			checkMessage(clientMessage, nReceivedBytes, socket);
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
//	char *message;

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
	while( (newSocket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		puts("Connection accepted");
		
        if( CreateThread(NULL, 0, socketThread, (LPVOID)&newSocket, 0, &tid[i]) == NULL )
			printf("Failed to create thread\n");
        if( i >= 50)
        {
          i = 0;
          while(i < 50)
          {
            //pthread_join(tid[i++],NULL);
			//  CloseHandle()
          }
          i = 0;
        }
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
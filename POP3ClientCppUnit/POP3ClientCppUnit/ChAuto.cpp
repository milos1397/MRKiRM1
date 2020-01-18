#include <stdio.h>

#include "const.h"
#include "ChAuto.h"

#define StandardMessageCoding 0x00

bool stop = false;

ChAuto::ChAuto() : FiniteStateMachine(CH_AUTOMATE_TYPE_ID, CH_AUTOMATE_MBX_ID, 1, 3, 3) 
{
}

ChAuto::~ChAuto() 
{
}

uint8 ChAuto::GetAutomate() 
{
	return CH_AUTOMATE_TYPE_ID;
}

/* This function actually connnects the ChAuto with the mailbox. */
uint8 ChAuto::GetMbxId() 
{
	return CH_AUTOMATE_MBX_ID;
}

uint32 ChAuto::GetObject() 
{
	return GetObjectId();
}

MessageInterface *ChAuto::GetMessageInterface(uint32 id) 
{
	return &StandardMsgCoding;
}

void ChAuto::SetDefaultHeader(uint8 infoCoding) 
{
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void ChAuto::SetDefaultFSMData() 
{
	SetDefaultHeader(StandardMessageCoding);
}

void ChAuto::NoFreeInstances() 
{
	printf("[%d] ChAuto::NoFreeInstances()\n", GetObjectId());
}

void ChAuto::Reset() 
{
	printf("[%d] ChAuto::Reset()\n", GetObjectId());
}


void ChAuto::Initialize() 
{
	SetState(FSM_Ch_Idle);	
	
	//intitialization message handlers
	InitEventProc(FSM_Ch_Idle ,MSG_Connection_Request, (PROC_FUN_PTR)&ChAuto::FSMIdle);
	InitEventProc(FSM_Ch_Connecting ,TIMER1_EXPIRED, (PROC_FUN_PTR)&ChAuto::FSMTimer1Expired );
	InitEventProc(FSM_Ch_Connecting ,MSG_Sock_Connection_Acccept, (PROC_FUN_PTR)&ChAuto::FSMSockConnectionAcccept );
	InitEventProc(FSM_Ch_Connected ,MSG_Cl_MSG, (PROC_FUN_PTR)&ChAuto::FSMConnectedClMSG );
	InitEventProc(FSM_Ch_Connected ,MSG_Sock_MSG, (PROC_FUN_PTR)&ChAuto::FSMConnectedSockMSG );

	InitTimerBlock(TIMER1_ID, TIMER1_COUNT, TIMER1_EXPIRED);
}

void ChAuto::FSMIdle()
{

	StartTimer(TIMER1_ID);
	
	SetState(FSM_Ch_Connecting);
	
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) 
	{
		return;
	} 

	/* Try to resolve the server name. */
	sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

	unsigned int addr = inet_addr(ADRESS);
	server_addr.sin_addr.s_addr	= addr;
    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	/* Create the socket. */ 
	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mySocket == INVALID_SOCKET) 
	{
		return ;
	}

	/* Try to reach the server. */
	if (connect(mySocket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) 
	{
		closesocket(mySocket);
		mySocket = INVALID_SOCKET;
		return ;
	}



	/* Then, start the thread that will listen on the the newly created socket. */
	hThread = CreateThread(NULL, 0, ClientListener, (LPVOID) this, 0, &nThreadID); 
	if (hThread == NULL) 
	{
		/* Cannot create thread.*/
		closesocket(mySocket);
		mySocket = INVALID_SOCKET;
		return ;
	}	
}

void ChAuto::FSMTimer1Expired()
{

	PrepareNewMessage(0x00, MSG_Cl_Connection_Reject);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);

	SetState(FSM_Ch_Idle);

}
void ChAuto::FSMSockConnectionAcccept()
{

	PrepareNewMessage(0x00, MSG_Cl_Connection_Accept);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);

	StopTimer(TIMER1_ID);

	SetState(FSM_Ch_Connected);

}

void ChAuto::FSMConnectedClMSG()
{

	char data[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4, size);

	data[size] = 0;

	send(mySocket, data, size, 0);

}
void ChAuto::FSMConnectedSockMSG()
{

}

void ChAuto::NetMsgToFSMMsg(const char* apBuffer, uint16 anBufferLength) 
{
	
	PrepareNewMessage(0x00, MSG_MSG);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA, anBufferLength, (uint8 *)apBuffer);
	SendMessage(CL_AUTOMATE_MBX_ID);
	
}

DWORD ChAuto::ClientListener(LPVOID param) {
	ChAuto* pParent = (ChAuto*) param;
	int nReceivedBytes;
	char buffer[255];


	nReceivedBytes = recv(pParent->mySocket, buffer, 255, 0);
	if (nReceivedBytes < 0) 
	{
			DWORD err = WSAGetLastError();
	}else
	{
		pParent->FSMSockConnectionAcccept();
		
		/* Receive data from the network until the socket is closed. */ 
		do 
		{
			nReceivedBytes = recv(pParent->mySocket, buffer, 255, 0);
			if (nReceivedBytes <= 0)
			{
				printf("\nError!");
				stop = true;
				break;
			}
			pParent->NetMsgToFSMMsg(buffer, nReceivedBytes);

			Sleep(100); 
			
			} while(1);
	}
	return 1;
}
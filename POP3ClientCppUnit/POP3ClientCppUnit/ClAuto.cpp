#include <stdio.h>

#include "const.h"
#include "ClAuto.h"
#include <stdio.h>

#define StandardMessageCoding 0x00


ClAuto::ClAuto() : FiniteStateMachine(CL_AUTOMATE_TYPE_ID, CL_AUTOMATE_MBX_ID, 0, 10, 2) 
{
}

ClAuto::~ClAuto() 
{
}


uint8 ClAuto::GetAutomate() 
{
	return CL_AUTOMATE_TYPE_ID;
}

/* This function actually connnects the ClAutoe with the mailbox. */
uint8 ClAuto::GetMbxId() 
{
	return CL_AUTOMATE_MBX_ID;
}

uint32 ClAuto::GetObject() 
{
	return GetObjectId();
}

MessageInterface *ClAuto::GetMessageInterface(uint32 id) 
{
	return &StandardMsgCoding;
}

void ClAuto::SetDefaultHeader(uint8 infoCoding) 
{
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void ClAuto::SetDefaultFSMData() 
{
	SetDefaultHeader(StandardMessageCoding);
}

void ClAuto::NoFreeInstances() 
{
	printf("[%d] ClAuto::NoFreeInstances()\n", GetObjectId());
}

void ClAuto::Reset() 
{
	printf("[%d] ClAuto::Reset()\n", GetObjectId());
}

void ClAuto::Initialize() 
{
	SetState(FSM_Cl_Ready);	
	
	//intitialization message handlers
	InitEventProc(FSM_Cl_Ready, MSG_User_Check_Mail, (PROC_FUN_PTR)&ClAuto::FSMCheckMail );
	InitEventProc(FSM_Cl_Connecting, MSG_Cl_Connection_Reject, (PROC_FUN_PTR)&ClAuto::FSMConnectionReject );
	InitEventProc(FSM_Cl_Connecting, MSG_Cl_Connection_Accept, (PROC_FUN_PTR)&ClAuto::FSMConnectionAccept );
	InitEventProc(FSM_Cl_Authorising, MSG_User_Name_Password, (PROC_FUN_PTR)&ClAuto::FSMAuthorising);
	InitEventProc(FSM_Cl_User_Check, MSG_MSG, (PROC_FUN_PTR)&ClAuto::FSMUserCheck);
	InitEventProc(FSM_Cl_Pass_Check, MSG_MSG, (PROC_FUN_PTR)&ClAuto::FSMPassCheck );
	InitEventProc(FSM_Cl_Options, MSG_Option, (PROC_FUN_PTR)&ClAuto::FSMOptionsShow);
	InitEventProc(FSM_Cl_Options, MSG_MSG, (PROC_FUN_PTR)&ClAuto::FSMReceive);
	//disconnected
	
	//InitEventProc(FSM_Cl_Options, MSG_Cl_Disconected, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Server_Disc);
	/*InitEventProc(FSM_Cl_User_Check, MSG_Cl_Disconected, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Server_Disc );
	InitEventProc(FSM_Cl_User_Check, MSG_User_Check_Mail, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Server_Disc );
	InitEventProc(FSM_Cl_Pass_Check, MSG_Cl_Disconected, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Server_Disc );
	InitEventProc(FSM_Cl_Authorising, MSG_Cl_Disconected, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Server_Disc );*/
}

void ClAuto::FSMCheckMail()
{
	
	printf("Connecting to %s ...\n",ADRESS);
	
	PrepareNewMessage(0x00, MSG_Connection_Request);
	SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CH_AUTOMATE_MBX_ID);

	SetState(FSM_Cl_Connecting);
}

void ClAuto::FSMConnectionReject()
{
	
	cout<<"Can not connect to the server!"<<endl;

	SetState(FSM_Cl_Ready);
}

void ClAuto::FSMConnectionAccept()
{

	cout<<"Connected successfully!"<<endl;

	SetState(FSM_Cl_Authorising);

	PrepareNewMessage(0x00, MSG_User_Name_Password);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

void ClAuto::FSMAuthorising()
{

	printf("Enter username:");
	scanf( "%s" , &userName);
	
	char command[20] = "user ";
	strcpy(command + 5, userName);

	PrepareNewMessage(0x00, MSG_Cl_MSG);
	SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA, strlen(command), (uint8*)command);
	SendMessage(CH_AUTOMATE_MBX_ID);
	
	SetState(FSM_Cl_User_Check);
}


void ClAuto::FSMUserCheck()
{

	char data[255]; 
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4, size);
	data[size] = 0;

	if( data[0] == '+' )
	{
		printf("Enter password: ");
		scanf("%s", &password);
		char command[20] = "pass ";
		strcpy(command + 5, password);

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA, strlen(command), (uint8*)command);
		SendMessage(CH_AUTOMATE_MBX_ID);
		
		SetState(FSM_Cl_Pass_Check);
	}
	else
	{
		SetState(FSM_Cl_Authorising);
	
		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
		
}

void ClAuto::FSMPassCheck()
{
	
	char data[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size] = 0;

	if( data[0] == '+' )
	{
		
        SetState(FSM_Cl_Options);

		msgCount = 1;
		msgNum = 0;
		checkPressed = 0;

		PrepareNewMessage(0x00, MSG_Option);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
	else
	{
		//vraca se u authorising, ako ne unese dobru sifru
		SetState(FSM_Cl_Authorising);
	
		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
}

void ClAuto::FSMOptionsShow()
{
	printf("Options: \n");
	printf("1.Check messages\n");
	printf("2.Receive message\n");
	printf("3.Send message\n");
	printf("4.Logout \n");
	char opt;
	do
	{
		printf("Enter your option[1-4]: ");
		scanf(" %c", &opt);
	}while(opt == '0' || opt > '4');

	if(opt == '4')
	{
		SetState(FSM_Cl_Authorising);

		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}else if(opt == '1')
	{
		command = 1;
		char msg[20] = "1 ";

		strcpy(msg + 2, userName);

		msgNum = 0;

		checkPressed = 1;


		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA, strlen(msg), (uint8*)msg);
		SendMessage(CH_AUTOMATE_MBX_ID);

	}else if(opt == '2')
	{
		//ako uopste i ima poruka
		if(checkPressed == 1)
		{
			if(msgNum != 0)
			{
				command = 2;
				char msg[20] = "2 ";
				long int result;

				char numb[4];
				do
				{
					printf("\nEnter the order number of the message you want to receive: ");
					scanf("%s",&numb);

					char* end;
					result = strtol(numb, &end, 10);

				}while(result > msgNum || result == 0);

				strcpy(msg + 2, numb);

				PrepareNewMessage(0x00, MSG_Cl_MSG);
				SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
				SetMsgObjectNumberTo(0);
				AddParam(PARAM_DATA, strlen(msg), (uint8*)msg);
				SendMessage(CH_AUTOMATE_MBX_ID);



				//posalji usera
				PrepareNewMessage(0x00, MSG_Cl_MSG);
				SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
				SetMsgObjectNumberTo(0);
				AddParam(PARAM_DATA, strlen(userName), (uint8*)userName);
				SendMessage(CH_AUTOMATE_MBX_ID);
			}else
			{
				printf("The are no messages for you!\n");

				PrepareNewMessage(0x00, MSG_Option);
				SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
				SetMsgObjectNumberTo(0);
				SendMessage(CL_AUTOMATE_MBX_ID);
			}
		}else
		{
			printf("You need first to check the messages!\n");
			
			PrepareNewMessage(0x00, MSG_Option);
			SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
			SetMsgObjectNumberTo(0);
			SendMessage(CL_AUTOMATE_MBX_ID);
		}

	}else 
	{
		command = 3;
		char msg[200] = "3 ";
		char contact[20];
		char text[100];
		char by[5] = " by ";


		printf("\nSend to: ");
		scanf("%s", &contact);

		printf("\nMessage: ");
		scanf(" %[^\n]s", text);

		
		strcpy(msg + 2, contact);

		//prvo posaljem samo primalaca
		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA, strlen(msg), (uint8*)msg);
		SendMessage(CH_AUTOMATE_MBX_ID);

		//posalje poruku i posiljaoca
		strcpy(text + strlen(text), by);
		strcpy(text + strlen(text), userName);
		strcpy(text + strlen(text), "\n");

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA, strlen(text), (uint8*)text);
		SendMessage(CH_AUTOMATE_MBX_ID);
	}
}


void ClAuto::FSMReceive()
{
	char data[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data, buffer + 4, size);
	data[size] = 0;

	if(command == 1)
	{
		//ako je kraj poruka onda izbaci options
		if(strncmp(data, "xxx", 3) == 0)
		{
			if(msgCount == 1)
				printf("There are no messages for you!\n");
			PrepareNewMessage(0x00, MSG_Option);
			SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
			SetMsgObjectNumberTo(0);
			SendMessage(CL_AUTOMATE_MBX_ID);
			msgCount = 1;
		}else//ako nije kraj samo printaj
		{
			msgNum++;
			msgCount++;
			printf("%d. %s", msgNum, data);
		}
	}else if(command == 2)
	{
		printf("\nMessage received: %s", data);
		FILE* fptr;

		fptr = fopen("Msg.txt", "a");
	
		if(fptr == NULL)
		{
			printf("Unable to open file\n");
		}else
		{
			fprintf(fptr, data);

			PrepareNewMessage(0x00, MSG_Option);
			SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
			SetMsgObjectNumberTo(0);
			SendMessage(CL_AUTOMATE_MBX_ID);
			fclose(fptr);
		}
	}else if(command == 3)
	{
		//isprintaj povratnu poruku i predji u opcije opet
		printf(" %s", data);
		
		PrepareNewMessage(0x00, MSG_Option);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
}

/*void ClAuto::FSM_Cl_Server_Disc()
{
	//printf("\nServer disconnected!");
	//stop = true;
}*/

void ClAuto::Start()
{
	PrepareNewMessage(0x00, MSG_User_Check_Mail);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

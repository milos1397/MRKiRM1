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

	PrepareNewMessage(0x00, MSG_User_Check_Mail);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
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

	SendToChannel(command);
	
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

		SendToChannel(command);
		
		SetState(FSM_Cl_Pass_Check);
	}
	else
	{
		printf("User does not exist!\n");
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

		//control variables init
		msgNum = 0;
		checkPressed = 0;

		SendMessOpt();
	}
	else
	{
		//vraca se u authorising, ako ne unese dobru sifru
		printf("\Incorrect password, enter login details again!\n");
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
	//logout
	if(opt == '4')
	{
		SetState(FSM_Cl_Authorising);

		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}else if(opt == '1') //checkmessages
	{
		command = 1;
		char msg[20] = "1 ";

		strcpy(msg + 2, userName);

		msgNum = 0;

		checkPressed = 1;

		SendToChannel(msg);

	}else if(opt == '2')
	{
		//you need first to check messages, and then you can receive
		if(checkPressed == 1)
		{
			//checks if you have any messages
			if(msgNum != 0)
			{
				command = 2;
				char msg[20] = "2 ";
				long int result;

				char numb[8];
				do
				{
					printf("\nEnter the order number of the message you want to receive: ");
					scanf(" %s",&numb);

					char* end;
					result = strtol(numb, &end, 10);

				}while(result > msgNum || result == 0);

				strcpy(msg + 2, numb);

				//send which message you want to receive
				SendToChannel(msg);

				//send your username, in order to know which file to open and read
				SendToChannel(userName);
			}else
			{
				printf("The are no messages for you!\n");

				SendMessOpt();
			}
		}else
		{
			printf("You need first to check the messages!\n");
			
			SendMessOpt();
		}
	}else //sending message
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

		//first send receiver
		SendToChannel(msg);

		//then send sender and message
		strcpy(text + strlen(text), by);
		strcpy(text + strlen(text), userName);
		strcpy(text + strlen(text), "\n");
		SendToChannel(text);
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
		//xxx means there no more messages, go to Options
		if(strncmp(data, "xxx", 3) == 0)
		{
			if(msgNum == 0)
				printf("There are no messages for you!\n");
			SendMessOpt();
		}else//ako nije kraj samo printaj
		{
			msgNum++;
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
		}else//save message locally
		{
			fprintf(fptr, data);

			SendMessOpt();
			fclose(fptr);
		}
	}else if(command == 3)
	{
		//whether the message is sent or not
		printf(" %s", data);
		
		SendMessOpt();
	}
}

void ClAuto::Start()
{
	PrepareNewMessage(0x00, MSG_User_Check_Mail);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
}


void ClAuto::SendToChannel(char* buffer)
{
	PrepareNewMessage(0x00, MSG_Cl_MSG);
	SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA, strlen(buffer), (uint8*)buffer);
	SendMessage(CH_AUTOMATE_MBX_ID);
}

void ClAuto::SendMessOpt()
{
	PrepareNewMessage(0x00, MSG_Option);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

void ClAuto::TestCorrPass()
{
	SetState(FSM_Cl_Pass_Check);

	char data[3];
	data[0] = '+';


	PrepareNewMessage(0x00, MSG_MSG);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA, 3, (uint8 *)data);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

void ClAuto::TestIncorrPass()
{
	SetState(FSM_Cl_Pass_Check);

	char data[3];
	data[0] = '-';


	PrepareNewMessage(0x00, MSG_MSG);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA, 3, (uint8 *)data);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

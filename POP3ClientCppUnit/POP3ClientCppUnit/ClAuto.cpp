#include <stdio.h>

#include "const.h"
#include "ClAuto.h"
#include <stdio.h>

#define StandardMessageCoding 0x00

ClAuto::ClAuto() : FiniteStateMachine(CL_AUTOMATE_TYPE_ID, CL_AUTOMATE_MBX_ID, 0, 10, 2) {
}

ClAuto::~ClAuto() {
}


uint8 ClAuto::GetAutomate() {
	return CL_AUTOMATE_TYPE_ID;
}

/* This function actually connnects the ClAutoe with the mailbox. */
uint8 ClAuto::GetMbxId() {
	return CL_AUTOMATE_MBX_ID;
}

uint32 ClAuto::GetObject() {
	return GetObjectId();
}

MessageInterface *ClAuto::GetMessageInterface(uint32 id) {
	return &StandardMsgCoding;
}

void ClAuto::SetDefaultHeader(uint8 infoCoding) {
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void ClAuto::SetDefaultFSMData() {
	SetDefaultHeader(StandardMessageCoding);
}

void ClAuto::NoFreeInstances() {
	printf("[%d] ClAuto::NoFreeInstances()\n", GetObjectId());
}

void ClAuto::Reset() {
	printf("[%d] ClAuto::Reset()\n", GetObjectId());
}

void ClAuto::Initialize() {
	SetState(FSM_Cl_Ready);	
	
	//intitialization message handlers
	InitEventProc(FSM_Cl_Ready, MSG_User_Check_Mail, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Ready_User_Check_Mail );
	InitEventProc(FSM_Cl_Connecting, MSG_Cl_Connection_Reject, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Connecting_Cl_Connection_Reject );
	InitEventProc(FSM_Cl_Connecting, MSG_Cl_Connection_Accept, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Connecting_Cl_Connectiong_Accept );
	InitEventProc(FSM_Cl_Authorising, MSG_User_Name_Password, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Authorising_User_Name_Password );
	InitEventProc(FSM_Cl_User_Check, MSG_MSG, (PROC_FUN_PTR)&ClAuto::FSM_Cl_User_Check_MSG );
	InitEventProc(FSM_Cl_Pass_Check, MSG_MSG, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Pass_Check_MSG );
	InitEventProc(FSM_Cl_Options, MSG_Option, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Options_Show);
	InitEventProc(FSM_Cl_Options, MSG_MSG, (PROC_FUN_PTR)&ClAuto::FSM_Cl_Receive);	
}

void ClAuto::FSM_Cl_Ready_User_Check_Mail(){
	
	printf("Connecting to %s ...\n",ADRESS);
	
	PrepareNewMessage(0x00, MSG_Connection_Request);
	SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CH_AUTOMATE_MBX_ID);

	SetState(FSM_Cl_Connecting);
}

void ClAuto::FSM_Cl_Connecting_Cl_Connection_Reject(){
	
	cout<<"Can not connect to the server!"<<endl;

	SetState(FSM_Cl_Ready);
}

void ClAuto::FSM_Cl_Connecting_Cl_Connectiong_Accept(){

	cout<<"Connected successfully!"<<endl;

	SetState(FSM_Cl_Authorising);

	PrepareNewMessage(0x00, MSG_User_Name_Password);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

void ClAuto::FSM_Cl_Authorising_User_Name_Password(){

	printf("Enter username:");
	scanf( "%s" , &m_UserName);
	
	char l_Command[20] = "user ";
	strcpy(l_Command+5,m_UserName);

	PrepareNewMessage(0x00, MSG_Cl_MSG);
	SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA,strlen(l_Command),(uint8*)l_Command);
	SendMessage(CH_AUTOMATE_MBX_ID);
	
	SetState(FSM_Cl_User_Check);
}


void ClAuto::FSM_Cl_User_Check_MSG(){

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size]=0;
	//printf("%s",data);
	if( data[0] == '+' )
	{
		printf("Enter password: ");
		scanf("%s", &m_Password);
		char l_Command[20] = "pass ";
		strcpy(l_Command+5,m_Password);
		//strcpy(l_Command+5+strlen(m_Password),"\r\n");

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA,strlen(l_Command),(uint8*)l_Command);
		SendMessage(CH_AUTOMATE_MBX_ID);
		
		SetState(FSM_Cl_Pass_Check);
	}
	else{
		SetState(FSM_Cl_Authorising);
	
		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
		
}

void ClAuto::FSM_Cl_Pass_Check_MSG(){
	
	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);
	data[size]=0;
	//printf("%s",data);

	if( data[0] == '+' )
	{
		/*char l_Command[20] = "stat\r\n";
		

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA,strlen(l_Command),(uint8*)l_Command);
		SendMessage(CH_AUTOMATE_MBX_ID);*/
		
		//SetState(FSM_Cl_Pass_Check);
        SetState(FSM_Cl_Options);

		PrepareNewMessage(0x00, MSG_Option);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
	else{
		//vraca se u authorising, ako ne unese dobru sifru
		SetState(FSM_Cl_Authorising);
	
		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}
}

void ClAuto::FSM_Cl_Options_Show(){
	printf("Options: \n");
	printf("1.Check messages\n");
	printf("2.Receive message\n");
	printf("3.Send message\n");
	printf("4.Logout \n");
	msg_num = 0;
	char opt;
	do{
		printf("Enter your option[1-4]: ");
		scanf(" %c", &opt);
	}while(opt=='0' || opt>'4');

	if(opt=='4')
	{
		SetState(FSM_Cl_Authorising);

		PrepareNewMessage(0x00, MSG_User_Name_Password);
		SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		SendMessage(CL_AUTOMATE_MBX_ID);
	}else if(opt=='1')
	{
		command = 1;
		char l_Command[20] = "1 ";

		strcpy(l_Command+2,m_UserName);

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA,strlen(l_Command),(uint8*)l_Command);
		SendMessage(CH_AUTOMATE_MBX_ID);

	}else if(opt=='2')
	{
		command = 2;
		char l_Command[20] = "2 ";

		strcpy(l_Command+2,m_UserName);

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA,strlen(l_Command),(uint8*)l_Command);
		SendMessage(CH_AUTOMATE_MBX_ID);
	}else 
	{
		command = 3;
		char l_Command[20] = "3 ";

		PrepareNewMessage(0x00, MSG_Cl_MSG);
		SetMsgToAutomate(CH_AUTOMATE_TYPE_ID);
		SetMsgObjectNumberTo(0);
		AddParam(PARAM_DATA,strlen(l_Command),(uint8*)l_Command);
		SendMessage(CH_AUTOMATE_MBX_ID);
	}
	//while(1);
}


void ClAuto::FSM_Cl_Receive()
{

	if(command == 1)
	{
		char data[255];
		uint8* buffer = GetParam(PARAM_DATA);
		uint16 size = buffer[2];

		memcpy(data,buffer + 4,size);
		data[size]=0;
		//ako je kraj poruka onda izbaci options
		if(strncmp(data,"xxx",3)==0)
		{
			PrepareNewMessage(0x00, MSG_Option);
			SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
			SetMsgObjectNumberTo(0);
			SendMessage(CL_AUTOMATE_MBX_ID);
		}else//ako nije kraj samo printaj
		{
			msg_num++;
			printf("%d. %s",msg_num,data);
		}
	}
}
void ClAuto::Start(){
	PrepareNewMessage(0x00, MSG_User_Check_Mail);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);
}

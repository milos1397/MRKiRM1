#ifndef _CL_AUTO_H_
#define _CL_AUTO_H_

#include "../kernel/stdMsgpc16pl16.h"
#include "NetFSM.h"
#include <stdio.h>
#include <conio.h>


class ClAuto : public FiniteStateMachine {
	
	StandardMessage StandardMsgCoding;
	
	MessageInterface *GetMessageInterface(uint32 id);
	void	SetDefaultHeader(uint8 infoCoding);
	void	SetDefaultFSMData();
	void	NoFreeInstances();
	void	Reset();
	uint8	GetMbxId();
	uint8	GetAutomate();
	uint32	GetObject();
	void	ResetData();

public:
	ClAuto();
	~ClAuto();

	void Initialize();
	
	void FSMCheckMail();
	void FSMConnectionReject();
	void FSMConnectionAccept();
	void FSMAuthorising();
	void FSMUserCheck();
	void FSMPassCheck();
	void FSMOptionsShow();
	void FSMReceive();
	//void FSM_Cl_Server_Disc();

	void Start();

protected:
	char userName[20];
	char password[20];
	int command;
	int msgNum;
};


#endif /* _Cl_AUTO_H */
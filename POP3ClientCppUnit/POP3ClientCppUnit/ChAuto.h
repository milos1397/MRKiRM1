#ifndef _CH_AUTO_H_
#define _CH_AUTO_H_

#include <fsm.h>
#include <fsmsystem.h>

#include "../kernel/stdMsgpc16pl16.h"
#include "NetFSM.h"

extern bool stop;


class ChAuto : public FiniteStateMachine {
	
	// for FSM
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
	
	// FSM States
	enum	ChStates {	FSM_Ch_Idle, 
						FSM_Ch_Connecting, 
						FSM_Ch_Connected };

	//FSM_Ch_Idle
	void	FSMIdle();
	//FSM_Ch_Connecting
	void	FSMTimer1Expired();
	void	FSMSockConnectionAcccept();
	//FSM_Ch_Connected
	void	FSMConnectedClMSG();//sending messages to server
	void	FSMConnectedSockMSG();
		
public:
	ChAuto();
	~ChAuto();
	
	//bool FSMMsg_2_NetMsg();
	void NetMsgToFSMMsg(const char* apBuffer, uint16 anBufferLength);//sending messages to client

	void Initialize();

protected:
	static DWORD WINAPI ClientListener(LPVOID);
	
	SOCKET mySocket;
	HANDLE hThread;
	DWORD nThreadID;
	uint16 nMaxMsgSize;
};

#endif /* _CH_AUTO_H */
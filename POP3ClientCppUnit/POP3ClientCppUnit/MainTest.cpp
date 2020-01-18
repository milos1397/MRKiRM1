#include "fsm.h"
#include "const.h"
#include "MainTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( MainTests );
uint8 * msg;
uint16 msgcode;


void MainTests::setUp()
{
    pSys = new FSMSystem(4, 4);

    client = new ClAuto();
	channel = new ChAuto();

   
    uint8 buffClassNo = 4;
    uint32 buffsCount[8] = { 100, 50, 50, 50 };
    uint32 buffsLength[8] = { 1025, 1025, 1025, 1025};

    pSys->InitKernel(buffClassNo, buffsCount, buffsLength, 1);

    lf = new LogFile("log.log", "log.ini");
    LogAutomateNew::SetLogInterface(lf);	

   
    pSys->Add(client, CL_AUTOMATE_TYPE_ID, 1, true);
    pSys->Add(channel, CH_AUTOMATE_TYPE_ID, 1, true);
}

void MainTests::tearDown()
{
}

void MainTests::AcceptTest()
{
	client->FSMConnectionAccept();

	msg = client->GetMsg(CL_AUTOMATE_MBX_ID);
	msgcode = client->GetState();
	client->Process(msg);//posalji sebi usercheck


	msgcode = client->GetState();
	CPPUNIT_ASSERT_EQUAL((uint16)FSM_Cl_User_Check, msgcode);
}

void MainTests::RejectTest()
{
	client->FSMConnectionReject();

	msg = client->GetMsg(CL_AUTOMATE_MBX_ID);
	msgcode = client->GetState();
	client->Process(msg);


	msgcode = client->GetState();
	CPPUNIT_ASSERT_EQUAL((uint16)FSM_Cl_Connecting, msgcode);
}

void MainTests::CorrectPassword()
{
	client->TestCorrPass();

	msg = client->GetMsg(CL_AUTOMATE_MBX_ID);
	msgcode = client->GetState();
	client->Process(msg);


	msgcode = client->GetState();
	CPPUNIT_ASSERT_EQUAL((uint16)FSM_Cl_Options, msgcode);
}

void MainTests::IncorrectPassword()
{
	client->TestIncorrPass();

	msg = client->GetMsg(CL_AUTOMATE_MBX_ID);
	msgcode = client->GetState();
	client->Process(msg);


	msgcode = client->GetState();
	CPPUNIT_ASSERT((uint16)FSM_Cl_Authorising == msgcode);
}
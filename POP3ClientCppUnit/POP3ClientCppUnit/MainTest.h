#ifndef MainTestH
#define MainTestH

#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "const.h"
#include <fsmsystem.h>
#include "../kernel/logfile.h"

#include "ClAuto.h"
#include "ChAuto.h"

class MainTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(MainTests);
	printf("\n[1]--> Client successfully connected test\n");
	CPPUNIT_TEST(AcceptTest);
	printf("\n[2]--> Client failed to connect test\n");
	CPPUNIT_TEST(RejectTest);
	printf("\n[3]--> Correct password entered\n");
	CPPUNIT_TEST(CorrectPassword);
	printf("\n[4]--> Incorrect password entered\n");
	CPPUNIT_TEST(IncorrectPassword);

    CPPUNIT_TEST_SUITE_END();
  
private:
    FSMSystem *pSys;
    ClAuto * client;
	ChAuto * channel;
    LogFile *lf;
    
public:
    void setUp();
    void tearDown();
    
protected:
	void AcceptTest();
	void RejectTest();
	void CorrectPassword();
	void IncorrectPassword();
};

#endif
/*
 * File:   testBsp.h
 * Author: Shcheblykin
 *
 * Created on May 26, 2014, 12:16:15 PM
 */

#ifndef TESTBSP_H
#define	TESTBSP_H

#include <cppunit/extensions/HelperMacros.h>
#include <inc/bsp.h>

class testBsp : public CPPUNIT_NS::TestFixture, TBsp {
    CPPUNIT_TEST_SUITE(testBsp);

    CPPUNIT_TEST(testIncError);
    CPPUNIT_TEST(testCheckConnect);
    CPPUNIT_TEST(testIsError);
    CPPUNIT_TEST(testCheckRxProtocol);
    CPPUNIT_TEST(testGetCom);
    CPPUNIT_TEST(testGetRegime);
    CPPUNIT_TEST(testIsNewData);
    CPPUNIT_TEST(testMakeTxData);

    CPPUNIT_TEST_SUITE_END();

public:
    testBsp();
    virtual ~testBsp();
    void setUp();
    void tearDown();

private:
    void testIncError();
    void testCheckConnect();
    void testIsError();
    void testCheckRxProtocol();
    void testGetCom();
    void testGetRegime();
    void testIsNewData();
    void testMakeTxData();
    
    // вывод сообщения об ошибке
    char buf[256];
    uint16_t pos;

};

#endif	/* TESTBSP_H */


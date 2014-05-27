/*
 * File:   testDR.h
 * Author: Shcheblykin
 *
 * Created on May 26, 2014, 12:18:26 PM
 */

#ifndef TESTDR_H
#define	TESTDR_H

#include <cppunit/extensions/HelperMacros.h>
#include "inc/dr.h"

class testDR : public CPPUNIT_NS::TestFixture, TDigitalRetrans {
    CPPUNIT_TEST_SUITE(testDR);

    CPPUNIT_TEST(testCodeToCom);
    CPPUNIT_TEST(testCheckByteProtocol);
    CPPUNIT_TEST(testCheckCommand);
    CPPUNIT_TEST(testClrError);
    CPPUNIT_TEST(testDecError);
    CPPUNIT_TEST(testGetCom);
    CPPUNIT_TEST(testIsConnect);
    CPPUNIT_TEST(testIsFault);
    CPPUNIT_TEST(testIsWarning);
    CPPUNIT_TEST(testSetError);

    CPPUNIT_TEST_SUITE_END();

public:
    testDR();
    virtual ~testDR();
    void setUp();
    void tearDown();

private:
    void testCodeToCom();
    void testCheckByteProtocol();
    void testCheckCommand();
    void testClrError();
    void testDecError();
    void testGetCom();
    void testIsConnect();
    void testIsFault();
    void testIsWarning();
    void testSetError();

};

#endif	/* TESTDR_H */


/*
 * File:   testBsp.cpp
 * Author: Shcheblykin
 *
 * Created on May 26, 2014, 12:16:15 PM
 */

#include "testBsp.h"
#include "inc\bsp.h"
#include <string>   
#include <stdlib.h> 
#include <vector>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(testBsp);

testBsp::testBsp()
{
}

testBsp::~testBsp()
{
}

void testBsp::setUp()
{
    tmTx = true;
    tmRx = true;
    bufRx = 0;

    newData = false;
    com = 0;
    regime = 0;
}

void testBsp::tearDown()
{
}

void testBsp::testCheckRxProtocol()
{
    bufRx = 0;
    checkRxProtocol(0x01, true);
    CPPUNIT_ASSERT_MESSAGE("Test=1", bufRx == 0xFF);
    
    struct data {
        uint8_t byte;
        uint8_t bufRx;
        uint8_t com;
        uint8_t regime;
        bool tmTx;
        bool newData;
    };
    
    data s1[] = {
        {0x01, 0x01, 0x00, 0x00,  true, false}, // 0
        {0xFF, 0xFF, 0x00, 0x00,  true, false}, // 1
        {0xC1, 0xC1, 0x00, 0x00,  true, false}, // 2 байт режима
        {0x3E, 0xFF, 0x00, 0x01,  true,  true}, // 3 инверсный байт
        {0x77, 0x77, 0x00, 0x00,  true, false}, // 4 
        {0x01, 0x01, 0x00, 0x00,  true, false}, // 5 байт команды + тм = лог.0
        {0xFE, 0xFF, 0x01, 0x00, false,  true}, // 6 инверсный байт
        {0x60, 0x60, 0x01, 0x00, false, false}, // 7 байт команды + тм = лог.1
        {0x9F, 0xFF, 0x20, 0x00,  true,  true}  // 8 инверсный байт
        
    };
    
    bufRx = 0xFF;
    for(uint8_t i = 0; i < (sizeof(s1) / sizeof(s1[0])); i++) {
        uint8_t pos = 0;
        checkRxProtocol(s1[i].byte, false);

        pos =  sprintf(&buf[0], "Test=2, step=%u: \n", i);
        pos += sprintf(&buf[pos], "\t byte=0x%.2X \n",  s1[i].byte);
        pos += sprintf(&buf[pos], "\t s.bufRx=0x%.2X, bufRx=0x%.2X \n", s1[i].bufRx, bufRx);
        pos += sprintf(&buf[pos], "\t s.com=0x%.2X, com=0x%.2X \n", s1[i].com, com);
        pos += sprintf(&buf[pos], "\t s.regime=0x%.2X, regime=0x%.2X \n", s1[i].regime, regime);
        pos += sprintf(&buf[pos], "\t s.tmTx=0x%.2X, tmTx=0x%.2X \n", s1[i].tmTx, tmTx);
        pos += sprintf(&buf[pos], "\t s.newData=0x%.2X, newData=0x%.2X", s1[i].newData, newData);
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].bufRx, bufRx);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].com, com);
        CPPUNIT_ASSERT_MESSAGE      (buf, s1[i].regime == getRegime());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].tmTx, tmTx);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].newData, isNewData());
    }
}

void testBsp::testGetCom()
{
    uint8_t out[] = {16, 0, 0, 0};
    com = out[0];
    uint8_t o = out[0];
    for(uint8_t i = 0; i < (sizeof(out) / sizeof(out[0])); i++) {
        uint8_t t = getCom();
        pos = sprintf(&buf[0], "Test=1, step=%u: \n", i);
        pos += sprintf(&buf[pos], "\t out=%u, result=%u, o=%u", out[i], t, o);
        
        CPPUNIT_ASSERT_MESSAGE(buf, t == out[i]);
    }
}

void testBsp::testGetRegime()
{
    regime = 1;
    CPPUNIT_ASSERT_MESSAGE("1", getRegime() == 1);
    CPPUNIT_ASSERT_MESSAGE("2", getRegime() == 0);
    CPPUNIT_ASSERT_MESSAGE("3", getRegime() == 0);
}

void testBsp::testIsNewData()
{
    newData = true;
    CPPUNIT_ASSERT_MESSAGE("1", isNewData() == true);
    CPPUNIT_ASSERT_MESSAGE("2", isNewData() == false);
    CPPUNIT_ASSERT_MESSAGE("3", isNewData() == false);
}

void testBsp::testMakeTxData()
{
    struct data {
        uint8_t com;
        uint8_t error;
        bool    tmRx;
        uint8_t bufTx0;
        uint8_t bufTx1;
    };
    
    data s[] = {
        {1,  0x00, false, 0x01, (unsigned) ~0x01}, // 0 команда + ТМ_RX = лог.'0'
        {1,  0x01, true,  0x41, (unsigned) ~0x41}, // 1 (команда + ошибка)  + ТМ_RX = лог.'1' - доминирует команда
        {0,  0x01, true,  0xC1, (unsigned) ~0xC1}, // 2 ошибка  + ТМ_RX = лог.'1'    
        {13, 0x00, true,  0x4D, (unsigned) ~0x4D}, // 3 команда + ТМ_RX = лог.'1'
        {32, 0x02, false, 0x20, (unsigned) ~0x20}, // 4 (команда + ошибка)  + TM_RX = лог.'0' - доминирует команда
        {0,  0x00, false, 0x00, (unsigned) ~0x00}  // 5 нет ком.+ TM_RX = лог.'0'
    };
    
    for(uint8_t i = 0; i < (sizeof(s) / sizeof(s[0])); i++) {
        tmRx = s[i].tmRx;
        makeTxData(s[i].com, s[i].error);
        
        pos = sprintf(&buf[0], "Test=1, step=%u: \n", i);
        pos += sprintf(&buf[pos], "\t s.com=0x%.2X \n", s[i].com);
        pos += sprintf(&buf[pos], "\t s.error=0x%.2X \n", s[i].error);
        pos += sprintf(&buf[pos], "\t s.tmRx=0x%.2X \n", s[i].tmRx);
        pos += sprintf(&buf[pos], "\t s.bufTx0=0x%.2X, bufTx[0]=0x%.2X \n", s[i].bufTx0, bufTx[0]);
        pos += sprintf(&buf[pos], "\t s.bufTx1=0x%.2X, bufTx[1]=0x%.2X \n", s[i].bufTx1, bufTx[1]);
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s[i].bufTx0, bufTx[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s[i].bufTx1, bufTx[1]);
    }
    
}


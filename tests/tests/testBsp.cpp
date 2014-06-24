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
    
    error = 0;
}

void testBsp::tearDown()
{
}

void testBsp::testIncError()
{
    for(uint8_t i = 0; i <= MAX_ERRORS*2; i++) {
        incError();
        if (i < MAX_ERRORS) {
            // проверка увеличения счетчика до достижения макс.значения
            CPPUNIT_ASSERT(error == (i + 1));
        } else {
            // проверка остуствия увеличения счетчика выше макс.занчения
            CPPUNIT_ASSERT(error = MAX_ERRORS);
        }
    }
}

void testBsp::testCheckConnect()
{
    for(uint8_t i = 0; i <= MAX_ERRORS*2; i++) {
        checkConnect();
        if (i < MAX_ERRORS) {
            // проверка увеличения счетчика до достижения макс.значения
            CPPUNIT_ASSERT(error == (i + 1));
        } else {
            // проверка остуствия увеличения счетчика выше макс.занчения
            CPPUNIT_ASSERT(error = MAX_ERRORS);
        }
    }
}

void testBsp::testIsError() 
{
    for(uint8_t i = 0; i <= MAX_ERRORS*2; i++) {       
        if (i < MAX_ERRORS) {
            // проверка отсутствия ошибки, если кол-во ошибок меньше порога
            CPPUNIT_ASSERT(isError() == false);
        } else {
            // проверка наличия ошибки, если кол-во ошибок больше порога
            CPPUNIT_ASSERT(isError() == true);
        }
        incError();
    }
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
        uint8_t error;
        bool tmTx;
        bool newData;
    };
    
    data s1[] = {
        {(uint8_t)  0x01, 0x01, 0x00, 0x00, 0x01,  true, false}, // 0
        {(uint8_t)  0xFF, 0xFF, 0x00, 0x00, 0x02,  true, false}, // 1
        {(uint8_t)  0xC1, 0xC1, 0x00, 0x00, 0x03,  true, false}, // 2 байт режима
        {(uint8_t)  0xC1, 0xFF, 0x00, 0x01, 0x00,  true,  true}, // 3 доп. байт
        {(uint8_t)  0x77, 0x77, 0x00, 0x00, 0x01,  true, false}, // 4 
        {(uint8_t)  0x01, 0x01, 0x00, 0x00, 0x02,  true, false}, // 5 байт команды + тм = лог.0
        {(uint8_t)  0x01, 0xFF, 0x01, 0x00, 0x00, false,  true}, // 6 доп. байт
        {(uint8_t)  0x60, 0x60, 0x01, 0x00, 0x01, false, false}, // 7 байт команды + тм = лог.1
        {(uint8_t)  0x60, 0xFF, 0x20, 0x00, 0x00,  true,  true}  // 8 доп. байт
        
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
        pos += sprintf(&buf[pos], "\t s.error=0x%.2X, error=0x%.2X", s1[i].error, error);
        
        // проверка буферного байта данных 
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].bufRx, bufRx);
        // проверка принятой команды
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].com, com);
        // проверка принятого режима
        CPPUNIT_ASSERT_MESSAGE      (buf, s1[i].regime == getRegime());
        // проверка текущего уровня выхода ТМ
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].tmTx, tmTx);
        // првоерка флага наличия достоверной посылки
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].newData, isNewData());
        // проверка установки / сброса ошибок
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].error, error);
    }
}

void testBsp::testGetCom()
{
    uint8_t out[] = {16, 0, 0, 0};
    com = out[0];
    for(uint8_t i = 0; i < (sizeof(out) / sizeof(out[0])); i++) {
        uint8_t t = getCom();
        pos = sprintf(&buf[0], "Test=1, step=%u: \n", i);
        pos += sprintf(&buf[pos], "\t out=%u, result=%u", out[i], t);
        
        // проверка возврата правильного номера команды, а также что 
        // после этого происходит сброс в 0
        CPPUNIT_ASSERT_MESSAGE(buf, t == out[i]);
    }
}

void testBsp::testGetRegime()
{
    uint8_t out[] = {2, 0, 0, 0};
    regime = out[0]; 
    // проверка возврата правильного номера команды, а также что 
       // после этого происходит сброс номера в 0
    for(uint8_t i = 0; i < (sizeof(out) / sizeof(out[0])); i++) {
        uint8_t t = getRegime();
        pos = sprintf(&buf[0], "Test=1, step=%u: \n", i);
        pos += sprintf(&buf[pos], "\t out=%u, result=%u", out[i], t);
        
        // проверка возврата правильного значения режима, а также что 
        // после этого происходит сброс в 0
        CPPUNIT_ASSERT_MESSAGE(buf, t == out[i]);
    }
}

void testBsp::testIsNewData()
{
    bool out[] = {true, false, false, false};
    newData = out[0];
    for(uint8_t i = 0; i < (sizeof(out) / sizeof(out[0])); i++) {
        bool t = isNewData();
        pos = sprintf(&buf[0], "Test=1, step=%u: \n", i);
        pos += sprintf(&buf[pos], "\t out=%u, result=%u", out[i], t);
        
        // проверка возврата правильного флага нового сообщения, а также что 
        // после этого происходит сброс н в 0
        CPPUNIT_ASSERT_MESSAGE(buf, t == out[i]);
    }
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
        {1,  0x00, false, 0x01, (uint8_t) 0x01}, // 0 команда + ТМ_RX = лог.'0'
        {1,  0x01, true,  0x41, (uint8_t) 0x41}, // 1 (команда + ошибка)  + ТМ_RX = лог.'1' - доминирует команда
        {0,  0x01, true,  0xC1, (uint8_t) 0xC1}, // 2 ошибка  + ТМ_RX = лог.'1'    
        {13, 0x00, true,  0x4D, (uint8_t) 0x4D}, // 3 команда + ТМ_RX = лог.'1'
        {32, 0x02, false, 0x20, (uint8_t) 0x20}, // 4 (команда + ошибка)  + TM_RX = лог.'0' - доминирует команда
        {0,  0x00, false, 0x00, (uint8_t) 0x00}  // 5 нет ком.+ TM_RX = лог.'0'
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


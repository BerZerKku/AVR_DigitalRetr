/*
 * File:   testDR.cpp
 * Author: Shcheblykin
 *
 * Created on May 26, 2014, 12:18:26 PM
 */

#include "testDR.h"
#include <vector>
#include <stdio.h>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(testDR);

testDR::testDR()
{
}

testDR::~testDR()
{
}

void testDR::setUp()
{
}

void testDR::tearDown()
{
}

void testDR::testCodeToCom()
{

    uint8_t t[256] = {
        0x00, 0x01, 0x02, 0xff, 0x03, 0xff, 0xff, 0xff,
        0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x05, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    // сравнение размеров массивов
    CPPUNIT_ASSERT(sizeof(t) == sizeof(mCodeToCom));
    
    // проверка содержимого массивов
    bool state = true;
    for(uint16_t i = 0; i < sizeof(t); i++) {
        if (t[i] != mCodeToCom[i]) {
            state = false;
            break;
        }
    }
    CPPUNIT_ASSERT(state);
     
}
void testDR::testCheckByteProtocol()
{
    clrError();
    stepRx = 0;
    checkByteProtocol(0 , false);
    CPPUNIT_ASSERT_MESSAGE("1", error == 0);
    
    clrError();
    stepRx = 1;
    checkByteProtocol(0xAA, true);
    CPPUNIT_ASSERT_MESSAGE("2", error != 0);
    CPPUNIT_ASSERT_MESSAGE("3", stepRx == 0);
    
    // проверка поиска синхробайт
    struct data {
        uint8_t byte;   // байт данных
        uint8_t step;   // текущее положение в протоколе
        bool sinhr;     // наличие сиинхробайт
    };
    
    data s1[] = {
        data{0x01, 0, false},
        data{0x54, 0, false},
        data{0x58, 0, false},
        data{0xAA, 1, false},
        data{0x49, 0, false},
        data{0xAA, 1, false},
        data{0x00, 0, false},
        data{0xAA, 1, false},
        data{0xAA, 2, true},
        data{0x11, 3, false}
    };
    
    clrError();
    isConnect();
    stepRx = 0;
    for(uint8_t i = 0; i < (sizeof(s1)/sizeof(s1[0])); i++) {
        checkByteProtocol(s1[i].byte, false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("4", s1[i].step, stepRx);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("5", s1[i].sinhr, isConnect());
        CPPUNIT_ASSERT_MESSAGE("6", error == 0);
        
    }
    
    
    // Проверка прямой/инверсный байт команд
    uint8_t buf3[] [8] = {
				0x01, 0xFE, 0xFE, 0x01, 0x00, 0xFF, 0x11, 0xEE,		// норм
				0x02, 0xFE, 0x01, 0xFF, 0x00, 0xFF, 0x11, 0xEE,		// 1-я восьмерка
				0x02, 0xFD, 0xFE, 0xF1, 0x00, 0xFF, 0x11, 0xEE,		// 2-я восьмерка
				0x03, 0xFC, 0xFE, 0x01, 0x50, 0xFF, 0x11, 0xEE,		// 3-я восьмерка
				0xFC, 0x03, 0xFE, 0x01, 0x00, 0xFF, 0x11, 0x1E      // 4-я восьмерка
    };	
    
    for(uint8_t i = 0; i < (sizeof(buf3) / sizeof(buf3[0])); i++) {
        clrError();
        stepRx = 2; // пропуск синхробайт
        
        // передача 8 байт данных 
        uint8_t j = 0;
        for (; j < sizeof (buf3[0]); j++) {
            checkByteProtocol(buf3[i][j], 0);
            if (error > 0)
                break;
        }
        
        if (i == 0) {
            // правильный пакет
            CPPUNIT_ASSERT_MESSAGE("7", j == sizeof(buf3[0]));
            CPPUNIT_ASSERT_MESSAGE("8", error == 0);
        } else {
            // проверка на кол-во ошибок
            CPPUNIT_ASSERT_MESSAGE("9", error == CNT_ERROR);
            // проверка на ошибку в конкретном байте
            CPPUNIT_ASSERT_MESSAGE("10", (i*2 - 1) == j);   
        }
    }
    
    
    // проверка резервных байт
    uint8_t buf4[] [5] = {
        0x00, 0x00, 0x00, 0x00, 0x00, // норм
        0x01, 0x00, 0x00, 0x00, 0x00, // 1 байт
        0x00, 0x02, 0x00, 0x00, 0x00, // 2 байт
        0x00, 0x00, 0x10, 0x00, 0x00, // 3 байт
        0x00, 0x00, 0x00, 0x20, 0x00, // 4 байт
        0x00, 0x00, 0x00, 0x00, 0xFF // 5 байт
    }; 	


	for(uint8_t i = 0; i < (sizeof(buf4) / sizeof(buf4[0])); i++) {
        clrError();
        stepRx = 10;
        uint8_t j = 0;
        for (; j < sizeof (buf4[0]); j++) {
            checkByteProtocol(buf4[i][j], 0);
            if (error > 0)
                break;
        }
        
        if (i == 0) {
            // правильный пакет
            CPPUNIT_ASSERT_MESSAGE("11", j == sizeof(buf4[0]));
            CPPUNIT_ASSERT_MESSAGE("12", error == 0);
        } else {
            // проверка на наличие ошибки
            CPPUNIT_ASSERT_MESSAGE("13", error == CNT_ERROR);
            // проверка на ошибку в конкретном байте
            CPPUNIT_ASSERT_MESSAGE("14", (i - 1) == j); 
        }
    }
    
    // проверка полной посылки с КС
    uint8_t buf5[] [16] = {
        0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0xFC, // норм
        0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x10, 0xEF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0xFC, // норм
        0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0xFA, // ошибка
        0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0xCF, // ошибка
        0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x10, 0xEF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0xEE  // ошибка
    }; 

    for (uint8_t i = 0; i < (sizeof (buf5) / sizeof (buf5[0])); i++) {
        clrError();
        stepRx = 0;
        uint8_t j = 0;
        for (; j < sizeof (buf5[0]); j++) {
            checkByteProtocol(buf5[i][j], 0);
            if (error > 0) {
                break;
            }
        }
        
        if (i <= 1) {
            // правильный пакет
            CPPUNIT_ASSERT_MESSAGE("15", j == sizeof (buf5[0]));
            CPPUNIT_ASSERT_MESSAGE("16", error == 0);
        } else {
            // проверка на наличие ошибки
            CPPUNIT_ASSERT_MESSAGE("17", error == CNT_ERROR);
            // проверка на ошибку в конкретном байте
            CPPUNIT_ASSERT_MESSAGE("18", j == (sizeof(buf5[0]) -1)); 
        }
    }
}

void testDR::testCheckCommand()
{
    struct data {
        uint64_t in;
        uint8_t com_in;
        uint8_t com_out;
        uint8_t cnt;
    };

    
    // проверка правильности определения 
    data s1[] = {
        data{0x00000000, 0, 0},
        data{0x00000001, 1, 0},
        data{0x00000002, 2, 0},
        data{0x00004000, 15, 0},
        data{0x80000000, 32, 0},
        data{0x80000000, 32, 0},
        data{0x80000000, 32, 32},
        data{0x80000000, 32, 32},
        data{0x00000001, 1, 32}
    };
    
    for(uint8_t i = 0; i < ((sizeof(s1) / sizeof(s1[0]))); i++) {
        bufRx[2] = static_cast<uint8_t> ((s1[i].in & 0x000000FF) >> 0);
        bufRx[4] = static_cast<uint8_t> ((s1[i].in & 0x0000FF00) >> 8);
        bufRx[6] = static_cast<uint8_t> ((s1[i].in & 0x00FF0000) >> 16);
        bufRx[8] = static_cast<uint8_t> ((s1[i].in & 0xFF000000) >> 24);
        
        clrError();
        checkCommand();
        
        CPPUNIT_ASSERT_EQUAL(s1[i].com_in, oldCom);
        CPPUNIT_ASSERT_EQUAL(s1[i].com_out, comRx);
        CPPUNIT_ASSERT(error == 0);
    }
    
    // проверка определения команды в случае наличия нескольких команд
     data s2[] = {
        data{0x00000014, 0, 0},
        data{0x00001200, 0, 0},
        data{0x00480000, 0, 0},
        data{0x81000000, 0, 0},
        data{0x00008001, 0, 0},
        data{0x10000400, 0, 0}
    };
     
    comRx = 0;
    oldCom = 0;
    
    for(uint8_t i = 0; i < ((sizeof(s2) / sizeof(s2[0]))); i++) {
        bufRx[2] = static_cast<uint8_t> ((s2[i].in & 0x000000FF) >> 0);
        bufRx[4] = static_cast<uint8_t> ((s2[i].in & 0x0000FF00) >> 8);
        bufRx[6] = static_cast<uint8_t> ((s2[i].in & 0x00FF0000) >> 16);
        bufRx[8] = static_cast<uint8_t> ((s2[i].in & 0xFF000000) >> 24);
        
        clrError();
        checkCommand();
        
        CPPUNIT_ASSERT_EQUAL(s2[i].com_in, oldCom);
        CPPUNIT_ASSERT_EQUAL(s2[i].com_out, comRx);
        CPPUNIT_ASSERT(error == CNT_ERROR);
    }
    
    // проверка счетчика команды
    data s3[] = {
        data{0x00000000,  0,  0, 1},
        data{0x00000001,  1,  0, 1},
        data{0x00000002,  2,  0, 1},
        data{0x00004000, 15,  0, 1},
        data{0x00004000, 15,  0, 2},
        data{0x00000001,  1,  0, 1},
        data{0x08000000, 28,  0, 1},
        data{0x08000000, 28,  0, 2},
        data{0x08000000, 28, 28, 3},
        data{0x00000001,  1, 28, 1},
        data{0x00000003,  0, 28, 0},
        data{0x08000000,  0, 28, 0},
        data{0x08000000,  0, 28, 0}
    };
    
    comRx = 0;
    oldCom = 0;
    
    clrError();
    for(uint8_t i = 0; i < ((sizeof(s3) / sizeof(s3[0]))); i++) {
        bufRx[2] = static_cast<uint8_t> ((s3[i].in & 0x000000FF) >> 0);
        bufRx[4] = static_cast<uint8_t> ((s3[i].in & 0x0000FF00) >> 8);
        bufRx[6] = static_cast<uint8_t> ((s3[i].in & 0x00FF0000) >> 16);
        bufRx[8] = static_cast<uint8_t> ((s3[i].in & 0xFF000000) >> 24);
        
        
        checkCommand();
        
//        printf("com = %u, oldCom = %u", s3[i].com_in, oldCom);
        // проверка правильности определения номера команды
        CPPUNIT_ASSERT_EQUAL_MESSAGE("1",s3[i].com_in, oldCom);
        // проверка подсчета кол-ва последовательно принятых одинаковых команд
        CPPUNIT_ASSERT_EQUAL_MESSAGE("2",s3[i].cnt, cntCom);
        // проверка утсановки номера принятой команды по ЦПП
        CPPUNIT_ASSERT_EQUAL_MESSAGE("3",s3[i].com_out, comRx);
        
        
    }
}

void testDR::testClrError()
{
    for(uint16_t i = 0; i < (MAX_ERRORS + 20); i++) {
        error = i;
        clrError();
        CPPUNIT_ASSERT(error == 0);
    }
}

void testDR::testDecError()
{
    error = 17;
    decError();
    // проверка уменьшения на 1 до 0
    for(uint8_t i = error; i > 0; i--) {
        CPPUNIT_ASSERT(error == i);
        decError();
    }
    
    // проверка уменьшения 0
    error = 0;
    decError();
    CPPUNIT_ASSERT(error == 0);
}

void testDR::testGetCom()
{
    comRx = 16;
    // проверка считывания команды и его последующего обнуления
    CPPUNIT_ASSERT(getCom() == 16);
    CPPUNIT_ASSERT(getCom() == 0);
    CPPUNIT_ASSERT(getCom() == 0);
}

void testDR::testIsConnect()
{
    connect = true;
    // проверка считывания текущего состояния и его последующего обнуления
    CPPUNIT_ASSERT(isConnect());
    CPPUNIT_ASSERT(!isConnect());
    CPPUNIT_ASSERT(!isConnect());
}

void testDR::testIsFault()
{
    // проверка наличия несиправности, при error >= ERRORS_TO_FAULT
    for(uint16_t i = 0; i < (MAX_ERRORS + 20); i++) {
        error = i;
        CPPUNIT_ASSERT(isFault() == (i >= ERRORS_TO_FAULT));
    }
}   

void testDR::testIsWarning()
{
    // проверка наличия предупреждения, при error != 0
    for(uint8_t i = 0; i < MAX_ERRORS; i++) {
        error = i;
        CPPUNIT_ASSERT(isWarning() == (i != 0));
    }
}

void testDR::testSetError()
{
    error = 0;
    
    // проверка работы функции 
    for (uint8_t i = 1; i <= (MAX_ERRORS / CNT_ERROR); i++) {
        setError();
        CPPUNIT_ASSERT(error == CNT_ERROR * i);
    }
    
    // проверка на максимальное значение переменной
    setError();
    CPPUNIT_ASSERT(error == MAX_ERRORS);
}


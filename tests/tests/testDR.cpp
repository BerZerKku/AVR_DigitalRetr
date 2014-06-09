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
    // проверка того, что при инициализации класса массив заполняется верно

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

void testDR::testComToCode()
{
    // проверка того, что при инициализации класса массив заполняется верно

    uint8_t t[MAX_NUM_COM + 1][8] = {
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},

        {0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x02, 0xFD, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x04, 0xFB, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x10, 0xEF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x20, 0xDF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x40, 0xBF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},
        {0x80, 0x7F, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF},

        {0x00, 0xFF, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x02, 0xFD, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x04, 0xFB, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x10, 0xEF, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x20, 0xDF, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x40, 0xBF, 0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0x80, 0x7F, 0x00, 0xFF, 0x00, 0xFF},

        {0x00, 0xFF, 0x00, 0xFF, 0x01, 0xFE, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x02, 0xFD, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x04, 0xFB, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x08, 0xF7, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x10, 0xEF, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x20, 0xDF, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x40, 0xBF, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF, 0x80, 0x7F, 0x00, 0xFF},

        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x01, 0xFE},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x02, 0xFD},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x04, 0xFB},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x08, 0xF7},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x10, 0xEF},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x20, 0xDF},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x40, 0xBF},
        {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x80, 0x7F}
    };

    // сравнение размеров массивов
    pos  = 0;
    pos += sprintf(&buf[pos], "Test=1 \n");
    pos += sprintf(&buf[pos], "\t sizeof(t)=%u, sizeof(mComToCode)=%u)",
            sizeof(t), sizeof(mComToCode));
    CPPUNIT_ASSERT_MESSAGE(buf, sizeof(t) == sizeof(mComToCode));

    // проверка содержимого массивов
    uint16_t step = 0;
    for(uint8_t i = 0; i <= MAX_NUM_COM; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if (t[i][j] != mComToCode[i][j]) {
                break;
            }
            step++;
        }
    }

    pos  = 0;
    pos += sprintf(&buf[pos], "Test=2 \n");
    pos += sprintf(&buf[pos], "t[%u]=0x%.2X, mComToCode[%u]=0x%.2X.",
            step, t[step], step, mComToCode[step]);
    CPPUNIT_ASSERT_MESSAGE(buf, step == sizeof(t));
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
        {0x01, 0, false},
        {0x54, 0, false},
        {0x58, 0, false},
        {0xAA, 1, false},
        {0x49, 0, false},
        {0xAA, 1, false},
        {0x00, 0, false},
        {0xAA, 1, false},
        {0xAA, 2, true},
        {0x11, 3, false}
    };
    
    clrError();
    stepRx = 0;
    connect = false;
    for(uint8_t i = 0; i < (sizeof(s1)/sizeof(s1[0])); i++) {
        checkByteProtocol(s1[i].byte, false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("4", s1[i].step, stepRx);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("5", s1[i].sinhr, connect);
        CPPUNIT_ASSERT_MESSAGE("6", error == 0);
        connect = false;
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
        0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50, // норм
        0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x10, 0xEF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50, // норм
        0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x51, // ошибка
        0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x49, // ошибка
        0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x10, 0xEF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x52  // ошибка
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
        {0x00000000, 0, 0},
        {0x00000001, 1, 0},
        {0x00000002, 2, 0},
        {0x00004000, 15, 0},
        {0x80000000, 32, 0},
        {0x80000000, 32, 0},
        {0x80000000, 32, 32},
        {0x80000000, 32, 32},
        {0x00000001, 1, 32}
    };

    for(uint8_t i = 0; i < ((sizeof(s1) / sizeof(s1[0]))); i++) {
        bufRx[2] = static_cast<uint8_t> ((s1[i].in & 0x000000FF) >> 0);
        bufRx[4] = static_cast<uint8_t> ((s1[i].in & 0x0000FF00) >> 8);
        bufRx[6] = static_cast<uint8_t> ((s1[i].in & 0x00FF0000) >> 16);
        bufRx[8] = static_cast<uint8_t> ((s1[i].in & 0xFF000000) >> 24);

        clrError();
        checkCommand();
        
        pos  = 0;
        pos  = sprintf(&buf[pos], "Test=1, Step=%u", i);
        pos += sprintf(&buf[pos], "\t s.com_in=0x%.2X \n", s1[i].com_in);
        pos += sprintf(&buf[pos], "\t s.com_out=0x%.2X, com=0x%.2X \n", s1[i].com_out, comRx);
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].com_in, oldCom);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].com_out, comRx);
        CPPUNIT_ASSERT_MESSAGE(buf, error == 0);
    }

    // проверка определения команды в случае наличия нескольких команд
     data s2[] = {
        {0x00000014, 0, 0},
        {0x00001200, 0, 0},
        {0x00480000, 0, 0},
        {0x81000000, 0, 0},
        {0x00008001, 0, 0},
        {0x10000400, 0, 0}
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
        
        pos  = 0;
        pos  = sprintf(&buf[pos], "Test=2, Step=%u", i);
        pos += sprintf(&buf[pos], "\t s.com_in=0x%.2X \n", s2[i].com_in);
        pos += sprintf(&buf[pos], "\t s.com_out=0x%.2X, com=0x%.2X \n", s2[i].com_out, comRx);
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s2[i].com_in, oldCom);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s2[i].com_out, comRx);
        CPPUNIT_ASSERT_MESSAGE(buf, error == CNT_ERROR);
    }

    // проверка счетчика команды
    data s3[] = {
        {0x00000000,  0,  0, 1},    // 0
        {0x00000001,  1,  0, 1},    // 1
        {0x00000002,  2,  0, 1},    // 2
        {0x00004000, 15,  0, 1},    // 3
        {0x00004000, 15,  0, 2},    // 4
        {0x00000001,  1,  0, 1},    // 5
        {0x08000000, 28,  0, 1},    // 6
        {0x08000000, 28,  0, 2},    // 7
        {0x08000000, 28, 28, 3},    // 8
        {0x00000001,  1, 28, 1},    // 9
        {0x00000003,  0, 28, 0},    // 10
        {0x08000000,  0, 28, 0},    // 11
        {0x08000000,  0, 28, 0}     // 12
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
        
        pos  = 0;
        pos  = sprintf(&buf[pos], "Test=3, Step=%u", i);
        pos += sprintf(&buf[pos], "\t s.com_in=0x%.2X \n", s3[i].com_in);
        pos += sprintf(&buf[pos], "\t s.com_out=0x%.2X, com=0x%.2X \n", s3[i].com_out, comRx);

//        printf("com = %u, oldCom = %u", s3[i].com_in, oldCom);
        // проверка правильности определения номера команды
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf,s3[i].com_in, oldCom);
        // проверка подсчета кол-ва последовательно принятых одинаковых команд
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf,s3[i].cnt, cntPckgRx);
        // проверка утсановки номера принятой команды по ЦПП
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf,s3[i].com_out, comRx);
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
        decError();
        CPPUNIT_ASSERT(error == (i - 1));
    }

    // проверка уменьшения 0
    error = 0;
    decError();
    CPPUNIT_ASSERT(error == 0);
}

void testDR::testGetCom()
{
     struct data {
        REG_DR regime;
        uint8_t inCom;
        uint8_t  outCom;
        
    };
    
    data s1[] = {
        {REG_OFF,       0,  0},     // 0
        {REG_OFF,       16, 0},     // 1
        {REG_TX_KEDR,   0,  0},     // 2
        {REG_TX_KEDR,   1,  1},     // 3
        {REG_TX_KEDR,   32, 32},    // 4
        {REG_RX_KEDR,   0,  0},     // 8
        {REG_RX_KEDR,   15, 0},     // 9
        {REG_RX_KEDR,   31, 0},     // 10
    };

    for(uint8_t i = 0; i < (sizeof(s1) / sizeof(s1[0])); i++) {
        setRegime(s1[i].regime);
        comRx = s1[i].inCom;

        uint8_t val = getCom();

        pos  = 0;
        pos  = sprintf(&buf[pos], "Test=1, Step=%u", i);
        pos += sprintf(&buf[pos], "\t s.com=0x%.2X, com=0x%.2X \n", s1[i].outCom, val);

        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].outCom, val);
    }

    // проверка на то, что команда не обнуляется после того как ее забрали
    setRegime(REG_TX_KEDR);  
    comRx = 16;
    CPPUNIT_ASSERT_MESSAGE("Test=2.1", getCom() == 16);
    CPPUNIT_ASSERT_MESSAGE("Test=2.2", getCom() == 16);
    CPPUNIT_ASSERT_MESSAGE("Test=2.3", getCom() == 16);
    
    // проверка на то, что команда обнуляется при ошибке в работе ЦПП
    setRegime(REG_TX_KEDR);
    comRx = 13;
    CPPUNIT_ASSERT_MESSAGE("Test=3.1", getCom() == 13);
    error = CNT_ERROR;
    CPPUNIT_ASSERT_MESSAGE("Test=3.2", getCom() == 13);
    error = ERRORS_TO_FAULT;
    CPPUNIT_ASSERT_MESSAGE("Test=3.3", getCom() == 0);
}

void testDR::testCheckConnect()
{
    error = 0;
    connect = true;
    checkConnect();
    CPPUNIT_ASSERT_MESSAGE("Test=1.1", error == 0);
    checkConnect();
    CPPUNIT_ASSERT_MESSAGE("Test=1.2", error == CNT_ERROR);
    checkConnect();
    CPPUNIT_ASSERT_MESSAGE("Test=1.3", error == 2*CNT_ERROR);
    connect = true;
    checkConnect();
    CPPUNIT_ASSERT_MESSAGE("Test=1.4", error == 2*CNT_ERROR);
}

void testDR::testGetError()
{
    struct data {
        REG_DR regime;
        uint8_t numErrors;
        uint8_t  error;
    };

    data s1[] = {
        {REG_OFF,       0,                      ERR_OFF},   // 0
        {REG_OFF,       ERRORS_TO_FAULT,        ERR_OFF},   // 1
        {REG_RX_KEDR,   0,                      ERR_NO},    // 2
        {REG_RX_KEDR,   1,                      ERR_NO},    // 3
        {REG_RX_KEDR,   67,                     ERR_NO},    // 4
        {REG_RX_KEDR,   ERRORS_TO_FAULT - 1,    ERR_NO},    // 5
        {REG_RX_KEDR,   ERRORS_TO_FAULT,        ERR_RX},    // 6
        {REG_RX_KEDR,   ERRORS_TO_FAULT + 1,    ERR_RX},    // 7
        {REG_TX_KEDR,   0,                      ERR_NO},    // 8
        {REG_TX_KEDR,   1,                      ERR_NO},    // 9
        {REG_TX_KEDR,   67,                     ERR_NO},    // 10
        {REG_TX_KEDR,   ERRORS_TO_FAULT - 1,    ERR_NO},    // 11
        {REG_TX_KEDR,   ERRORS_TO_FAULT,        ERR_TX},    // 12
        {REG_TX_KEDR,   ERRORS_TO_FAULT + 1,    ERR_TX},    // 13
        {REG_OFF,       0,                      ERR_OFF},   // 14
    };

    for(uint8_t i = 0; i < (sizeof(s1) / sizeof(s1[0])); i++) {
        setRegime(s1[i].regime);
        error = s1[i].numErrors;
        uint8_t val = getError();

        pos  = 0;
        pos  = sprintf(&buf[pos], "Test1=1, Step=%u", i);
        pos += sprintf(&buf[pos], "\t s.error=0x%.2X, error=0x%.2X \n", s1[i].error, val);

        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, s1[i].error, val);
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

void testDR::testIsError()
{
    // проверка наличия ошибки, при error >= ERROR_TO_FAULT
    for(uint8_t i = 0; i < MAX_ERRORS; i++) {
        error = i;
        pos = 0;
        pos += sprintf(&buf[pos], "error=%d", i);
        CPPUNIT_ASSERT_MESSAGE(buf, isError() == (i >= ERRORS_TO_FAULT));
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

void testDR::testSetRegime()
{
    setRegime(static_cast<uint8_t> (REG_OFF));
    CPPUNIT_ASSERT_MESSAGE("Test=1", regime == REG_OFF);

    setRegime(static_cast<uint8_t> (REG_RX_KEDR));
    CPPUNIT_ASSERT_MESSAGE("Test=2", regime == REG_RX_KEDR);

    setRegime(static_cast<uint8_t> (REG_TX_KEDR));
    CPPUNIT_ASSERT_MESSAGE("Test=3", regime == REG_TX_KEDR);

    // выход за диапазон значений
    setRegime(static_cast<uint8_t> (REG_TX_KEDR) + 1);
    CPPUNIT_ASSERT_MESSAGE("Test=3", regime == REG_TX_KEDR);

    // нулевое значение
    setRegime(0);
    CPPUNIT_ASSERT_MESSAGE("Test=3", regime == REG_TX_KEDR);
}

void testDR::testCrtTxNewData()
{
    uint8_t nullCom[16] = { 0xAA, 0xAA,
                            0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
                            0x00, 0x00, 0x00, 0x00, 0x00,
                            0x50
    };


    // проверка правильности формирования команды врежиме REG_RX_KEDR
    setRegime(REG_RX_KEDR);
    for(uint8_t i = 0; i <= MAX_NUM_COM; i++) {

        pos  = 0;
        pos += sprintf(&buf[pos], "Test=1, com=%u.", i);

        comTx = i;
        crtTxNewData();

        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[0] == nullCom[0]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[1] == nullCom[1]);

        uint8_t byte1 = ((i >= 1 ) && ( i <= 8 )) ? (1 << ((i - 1)%8)) : 0x00;
        uint8_t nbyte1 = ~byte1;
        uint8_t byte2 = ((i >= 9 ) && ( i <= 16)) ? (1 << ((i - 1)%8)) : 0x00;
        uint8_t nbyte2 = ~byte2;
        uint8_t byte3 = ((i >= 17) && ( i <= 24)) ? (1 << ((i - 1)%8)) : 0x00;
        uint8_t nbyte3 = ~byte3;
        uint8_t byte4 = ((i >= 25) && ( i <= 32)) ? (1 << ((i - 1)%8)) : 0x00;
        uint8_t nbyte4 = ~byte4;

//        pos += sprintf(&buf[pos], "buf=%.2X ~byte=%.2X", bufTx[2], byte1);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[2] == byte1);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[3] == nbyte1);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[4] == byte2);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[5] == nbyte2);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[6] == byte3);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[7] == nbyte3);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[8] == byte4);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[9] == nbyte4);

        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[10] == nullCom[10]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[11] == nullCom[11]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[12] == nullCom[12]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[13] == nullCom[13]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[14] == nullCom[14]);

        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[15] == nullCom[15]);

        // проверка сброса команды, после формирования пакета для нее
        CPPUNIT_ASSERT_MESSAGE(buf, comTx == 0);
    }

    // Проверка правильности установки команды в режиме REG_OFF
    comTx = 1;
    setRegime(REG_OFF);
    crtTxNewData();
    for(uint8_t i = 0; i < MAX_STEP; i++) {
        pos  = 0;
        pos += sprintf(&buf[pos], "Test=2, step=%u.", i);
//        pos += sprintf(&buf[pos], "buf=%.2X ~byte=%.2X", bufTx[i], nullCom[i]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[i] == nullCom[i]);
    }

     // Проверка правильности установки команды в режиме REG_TX_KEDR
    comTx = 3;
    setRegime(REG_TX_KEDR);
    crtTxNewData();
    for(uint8_t i = 0; i < MAX_STEP; i++) {
        pos  = 0;
        pos += sprintf(&buf[pos], "Test=2, step=%u.", i);
//        pos += sprintf(&buf[pos], "buf=%.2X ~byte=%.2X", bufTx[i], nullCom[i]);
        CPPUNIT_ASSERT_MESSAGE(buf, bufTx[i] == nullCom[i]);
    }
}

void testDR::testGetTxByte()
{
    struct data {
        uint8_t com;
        uint8_t buf[MAX_STEP];
    };

    data s1[] = {
        {0x01, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 0
        {0x02, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 1
        {0x03, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 2
        {0x04, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 3
        {0x05, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 4
        {0x06, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 5
        {0x07, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 6
        {0x08, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 7
        {0x09, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 8
        {0x0A, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 9
        {0x04, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 10
        {0x05, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 11
        {0x06, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 12
        {0x07, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 13
        {0x08, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 14
        {0x09, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 15
        {0x0A, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 16
        {0x0B, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 17
        {0x0C, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 18
        {0x0D, {0xAA, 0xAA, 0x08, 0xF7, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 19
        {0x00, {0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 20
        {0x01, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 21
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 22
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 23
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 24
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 25
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 26
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 27
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 28
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 29
        {0x00, {0xAA, 0xAA, 0x01, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 30
        {0xFF, {0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0, 0, 0, 0, 0, 0x50}},  // 31
        {0x1A, {0xAA, 0xAA, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x02, 0xFD, 0, 0, 0, 0, 0, 0x50}}   // 32
    };

    setRegime(REG_RX_KEDR);
    stepTx = MAX_STEP;
    cntPackgTx = 0;

    for(uint8_t i = 0; i < (sizeof(s1) / sizeof(s1[0])); i++) {
        comTx = s1[i].com;
        for(uint8_t j = 0; j < MAX_STEP; j++) {
            uint8_t byte = getTxByte();

            pos  = 0;
            pos += sprintf(&buf[pos], "Test=1, step=%u \n", i);
            pos += sprintf(&buf[pos], "\t s.buf[%u]=0x%.2X, byte=0x%.2X.", j, s1[i].buf[j], byte);

            CPPUNIT_ASSERT_MESSAGE(buf, s1[i].buf[j] == byte);
        }
    }

}

void testDR::testResetProtocol()
{
    comRx = 1;
    cntPckgRx = 2;
    stepRx = 3;
    oldCom = 4;
    connect = true;
    error = 7;
    
    comTx = 5;
    stepTx = 6;
    cntPackgTx = 8;
    
    resetProtocol();

    CPPUNIT_ASSERT_MESSAGE(buf, comRx == 0);
    CPPUNIT_ASSERT_MESSAGE(buf, cntPckgRx == 0);
    CPPUNIT_ASSERT_MESSAGE(buf, stepRx == 0);
    CPPUNIT_ASSERT_MESSAGE(buf, oldCom == 0);
    CPPUNIT_ASSERT_MESSAGE(buf, connect == false);
    CPPUNIT_ASSERT_MESSAGE(buf, error == CNT_ERROR);
    
    CPPUNIT_ASSERT_MESSAGE(buf, comTx == 0);
    CPPUNIT_ASSERT_MESSAGE(buf, stepTx == MAX_STEP);
    CPPUNIT_ASSERT_MESSAGE(buf, cntPackgTx == 0);
    
}

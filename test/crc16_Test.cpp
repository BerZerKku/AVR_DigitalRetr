#include <cstdio>
#include <iostream>

#include "gtest\gtest.h"
#include "protocolModbus.h"
#include "crc16.h"
using namespace std;

class CRC16_Test: public ::testing::Test {
public:
	TCrc16 crc16;
};

TEST_F(CRC16_Test, test) {
	// начальное состояние
	ASSERT_TRUE(crc16.get() == 0xFFFF) << crc16.get();
	ASSERT_TRUE(crc16.getLow() == 0xFF) << hex << crc16.getLow();
	ASSERT_TRUE(crc16.getHigh() == 0xFF) << hex << crc16.getHigh();

	// эталонные значения взяты с калькулятора
	// https://www.lammertbies.nl/comm/info/crc-calculation.html

	crc16.add(0x01);
	ASSERT_TRUE(crc16.get() == 0x807E) << hex << crc16.get();
	ASSERT_TRUE(crc16.getLow() == 0x7E) << hex << crc16.getLow();
	ASSERT_TRUE(crc16.getHigh() == 0x80) << hex << crc16.getHigh();

	crc16.add(0x02);
	ASSERT_TRUE(crc16.get() == 0xE181) << hex << crc16.get();
	ASSERT_TRUE(crc16.getLow() == 0x81) << hex << crc16.getLow();
	ASSERT_TRUE(crc16.getHigh() == 0xE1) << hex << crc16.getHigh();

	crc16.add(0xFF);
	ASSERT_TRUE(crc16.get() == 0x2061) << hex << crc16.get();
	ASSERT_TRUE(crc16.getLow() == 0x61) << hex << crc16.getLow();
	ASSERT_TRUE(crc16.getHigh() == 0x20) << hex << crc16.getHigh();

	// сброс
	crc16.reset();
	ASSERT_TRUE(crc16.get() == 0xFFFF) << crc16.get();
	ASSERT_TRUE(crc16.getLow() == 0xFF) << hex << crc16.getLow();
	ASSERT_TRUE(crc16.getHigh() == 0xFF) << hex << crc16.getHigh();
}

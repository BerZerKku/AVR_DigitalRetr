#include <cstdio>
#include <iostream>

#include "gtest\gtest.h"
#include "protocolS.h"
using namespace std;

// размер массива
#define SIZE_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))

#define TPS TProtocolS

class ProtocolS_Test: public ::testing::Test {
public:
	ProtocolS *mb;		// тестируемый класс
	char msg[1000];		// буфер сообщений
	uint16_t cnt_msg; 	// кол-во данных в буфере

	// конструктор
	ProtocolS_Test() {
		cnt_msg = 0;
	};

	// деструктор
	virtual ~ProtocolS_Test() {};

	bool sendData(uint8_t buf[], uint8_t num) {
		for(uint8_t i = 0; i < num; i++) {
			if (mb->isReadData()) {
				cnt_msg += sprintf(&msg[cnt_msg], " Error on step %i\n", num);
				return false;
			}
			mb->push(buf[i], false);
		}

		cnt_msg += sprintf(&msg[cnt_msg], " State is not IDLE -> %02X\n", mb->getState());
		cnt_msg += sprintf(&msg[cnt_msg], " State check -> %02X\n", mb->checkState(mb->STATE_READ_OK));


		if (!mb->isReadData()) {
			cnt_msg += sprintf(&msg[cnt_msg], " Frame reception ERROR\n");
			return false;
		}

		if (!mb->readData()) {
			cnt_msg += sprintf(&msg[cnt_msg], " Frame proccesing ERROR\n");
			return false;
		}

		cnt_msg += sprintf(&msg[cnt_msg], " STATE_OFF -> %d\n", (uint16_t) mb->STATE_OFF);
		cnt_msg += sprintf(&msg[cnt_msg], " STATE_IDLE -> %d\n", (uint16_t) mb->STATE_IDLE);
		cnt_msg += sprintf(&msg[cnt_msg], " STATE_READ -> %d\n", (uint16_t) mb->STATE_READ);
		cnt_msg += sprintf(&msg[cnt_msg], " STATE_READ_OK -> %d\n", (uint16_t) mb->STATE_READ_OK);
		cnt_msg += sprintf(&msg[cnt_msg], " STATE_WRITE -> %d\n", (uint16_t) mb->STATE_WRITE);

		return false;

		return true;
	}

	bool readData(uint8_t buf[], uint8_t num) {
		bool state = true;
		uint8_t byte = 0;

		if (!mb->isIdle()) {
			cnt_msg += sprintf(&msg[cnt_msg], " State is not IDLE\n -> %d", mb->state);
		}

		state &= mb->pull(byte);
		state &= mb->isSendData();

		cnt_msg += sprintf(&msg[cnt_msg], " buf[%d] = ", num);

		for(uint8_t i = 0; i < num; i++) {
			if (!mb->pull(byte)) {
				state = false;
				break;
			}

			if (byte != buf[num]) {
				state = false;
				break;
			}

			cnt_msg += sprintf(&msg[cnt_msg], "0x%02X ", byte);
		}
		cnt_msg += sprintf(&msg[cnt_msg], "\n");

		return state;
	}

private:
	virtual void SetUp() {
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		mb = new ProtocolS;
	};

	virtual void TearDown() {
		delete mb;
	};
};

TEST_F(ProtocolS_Test, test) {

	mb->setEnable();

	ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
	ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));

	{	// Корректное сообщение с ненулевыми командами
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x11, 0x04, 0xF1, 0x1F, 0xF2, 0x2F, 0x46};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req))) << msg;
		ASSERT_EQ(0x1FF1, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x2FF2, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Корректное сообщение с ненулевыми командами
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x11, 0x04, 0x12, 0x34, 0x56, 0x78, 0x29};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req))) << msg;
		ASSERT_EQ(0x3412, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x7856, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Корректное сообщенеие с нулевыми командами
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0x15};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req))) << msg;
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Ошибка, команда 0x12
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x12, 0x04, 0xF1, 0x1F, 0xF2, 0x2F, 0x47};
		EXPECT_FALSE(sendData(req, SIZE_ARRAY(req))) << msg;
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Ошибка, на 1 байт данных меньше.
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x11, 0x03, 0xF1, 0x1F, 0xF2, 0x16};
		EXPECT_FALSE(sendData(req, SIZE_ARRAY(req))) << msg;
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Корректное сообщение с ненулевыми командами
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x11, 0x04, 0x12, 0x34, 0x56, 0x78, 0x29};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req))) << msg;
		ASSERT_EQ(0x3412, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x7856, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		uint8_t req[] = {0x55, 0xAA, 0x12, 0x04, 0x00, 0x00, 0x00, 0x00, 0x16};
		EXPECT_TRUE(readData(req, SIZE_ARRAY(req))) << msg;
	}

}


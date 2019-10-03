#include <cstdio>
#include <iostream>

#define TEST_FRIENDS \
    friend class ProtocolS_Test;

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
		mb = nullptr;
		cnt_msg = 0;
	};

	// деструктор
	virtual ~ProtocolS_Test() {};

	bool sendData(uint8_t buf[], uint8_t num) {
		mb->setRead();
		for(uint8_t i = 0; i < num; i++) {
			if (mb->isReadData()) {
				EXPECT_FALSE(true) << "step " << (uint16_t) i << endl;
				return false;
			}
			mb->push(buf[i], false);
		}

		// Используется для подсчета КС сообщения
//		if (mb->calcCRC() != mb->getCRC()) {
//			EXPECT_EQ(mb->getCRC(), mb->calcCRC());
//			return false;
//		}

		return true;
	}

	bool readData(uint8_t buf[], uint8_t num) {
		uint8_t byte;

		mb->setIdle();

		if (!mb->sendData()) {
			EXPECT_TRUE(false);
			return false;
		}

		if (!mb->isSendData()) {
			EXPECT_TRUE(false);
			return false;
		}

		if (num != mb->nTx) {
			EXPECT_EQ(num, mb->nTx);
			return false;
		}

		for(uint8_t i = 0; i < num; i++) {
			if (!mb->pull(byte)) {
				EXPECT_TRUE(false) << " step " << (uint16_t) i;
				return false;
			}

			if (buf[i] != byte) {
				EXPECT_EQ(buf[i], byte) << " step " << (uint16_t) i;
				return false;
			}
		}

		if (mb->pull(byte)) {
			EXPECT_FALSE(true);
		}

		if (!mb->checkState(mb->STATE_READ)) {
			EXPECT_TRUE(false);
			return false;
		}

		return true;
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

TEST_F(ProtocolS_Test, testRead) {

	mb->setEnable();

	ASSERT_EQ(0x00, mb->getCOut());
	ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
	ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));

	{	// Корректное сообщение с ненулевыми командами
		uint8_t req[] = {0x55, 0xAA, 0x12, 0x05, 0xF1, 0x1F, 0xF2, 0x2F, 0x03, 0x4B};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_TRUE(mb->isReadData());
		ASSERT_TRUE(mb->readData());

		ASSERT_EQ(0x03,   mb->getCOut());
		ASSERT_EQ(0x1FF1, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x2FF2, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Корректное сообщение с ненулевыми командами
		uint8_t req[] = {0x55, 0xAA, 0x12, 0x05, 0x12, 0x34, 0x56, 0x78, 0x18, 0x43};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_TRUE(mb->isReadData());
		ASSERT_TRUE(mb->readData());

		ASSERT_EQ(0x18,   mb->getCOut());
		ASSERT_EQ(0x3412, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x7856, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Корректное сообщенеие с нулевыми командами
		uint8_t req[] = {0x55, 0xAA, 0x12, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_TRUE(mb->isReadData());
		ASSERT_TRUE(mb->readData());

		ASSERT_EQ(0x00,   mb->getCOut());
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Ошибка, корректное сообщение, но 0x11
		uint8_t req[] = {0x55, 0xAA, 0x11, 0x05, 0x12, 0x34, 0x56, 0x78, 0x18, 0x42};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_TRUE(mb->isReadData());
		ASSERT_FALSE(mb->readData());

		ASSERT_EQ(0x00,   mb->getCOut());
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Ошибка, корректное сообщение. но на 1 байт данных меньше.
		uint8_t req[] = {0x55, 0xAA, 0x12, 0x04, 0xF1, 0x1F, 0xF2, 0x2F, 0x47};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_TRUE(mb->isReadData());
		ASSERT_FALSE(mb->readData());

		ASSERT_EQ(0x00,   mb->getCOut());
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{	// Корректное сообщение с ненулевыми командами
		uint8_t req[] = {0x55, 0xAA, 0x12, 0x05, 0x12, 0x34, 0x56, 0x78, 0x27, 0x52};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_TRUE(mb->isReadData());
		ASSERT_TRUE(mb->readData());

		ASSERT_EQ(0x27,   mb->getCOut());
		ASSERT_EQ(0x3412, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x7856, mb->getDOut(mb->D_OUTPUT_32_17));
	}

	{
		// Ошибочная контрольная сумма в сообщении.
		uint8_t req[] = {0x55, 0xAA, 0x12, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15};
		ASSERT_TRUE(sendData(req, SIZE_ARRAY(req)));
		ASSERT_FALSE(mb->isReadData());
		ASSERT_FALSE(mb->readData());

		ASSERT_EQ(0x27,   mb->getCOut());
		ASSERT_EQ(0x3412, mb->getDOut(mb->D_OUTPUT_16_01));
		ASSERT_EQ(0x7856, mb->getDOut(mb->D_OUTPUT_32_17));
	}
}

TEST_F(ProtocolS_Test, testSend) {

	mb->setEnable();

	{
		// Проверка начального состояния дискретных входов.
		uint8_t req[] = {0x55, 0xAA, 0x11, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16};
		ASSERT_TRUE(readData(req, SIZE_ARRAY(req)));
	}

	{
		// Проверка корректности установки команд.
		uint8_t req[] = {0x55, 0xAA, 0x11, 0x05, 0x17, 0x86, 0x34, 0x59, 0x37, 0x77};

		mb->setCInput(0x37);
		mb->setDInput(mb->D_INPUT_16_01, 0x8617);
		mb->setDInput(mb->D_INPUT_32_17, 0x5934);

		ASSERT_TRUE(readData(req, SIZE_ARRAY(req)));
	}
}

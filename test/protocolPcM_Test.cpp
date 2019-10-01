#include <cstdio>
#include <iostream>

#include "gtest\gtest.h"
#include "protocolPcM.h"
#include "crc16.h"
using namespace std;

// размер массива
#define SIZE_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))

#define TPM TProtocolModbus

class ProtocolPcM_Test: public ::testing::Test {
public:
	TProtocolPcM *mb;					// тестируемый класс

	char msg[1000];		// буфер сообщений
	uint16_t cnt_msg; 	// кол-во данных в буфере

	// конструктор
	ProtocolPcM_Test() {
		cnt_msg = 0;
		mb = nullptr;
	};

	// деструктор
	virtual ~ProtocolPcM_Test() {};

	// формирует процесс получения посылки
	bool readCom(uint8_t *buf, uint16_t size, uint8_t adr) {
		mb->setAddressLan(adr);
		mb->setReadState();

		// пересылка данных в буфер
		TCrc16 crc16;

		for (uint8_t i = 0; i < size; i++) {
			crc16.add(buf[i]);
			mb->push(buf[i]);
		}

		mb->push(crc16.getLow());
		mb->push(crc16.getHigh());

		// формирование сигнала окончания посылки
		for(uint8_t k = 0; k < 50; k++) {
			mb->tick();
		}

		if (!mb->readData()) {
			cnt_msg += sprintf(&msg[cnt_msg], "Message CRC = 0x%04X.\n", crc16.get());
			return false;
		}

		return true;
	}

	// сравнение массивов, true - совпадают
	bool checkArray(uint8_t *response, uint16_t size) {
		uint8_t i = 0;
		uint16_t crc = 0;
		uint8_t byte = 0;

		TCrc16 crc16;
		cnt_msg += sprintf(&msg[cnt_msg], "Receive message: ");
		if (size > 0) {
			for(i = 0; i < size; i++) {
				if (!mb->isSendData()) {
					cnt_msg += sprintf(&msg[cnt_msg], "\nNo send byte on step %d.", i);
					return false;
				}

				byte = mb->pull();
				cnt_msg += sprintf(&msg[cnt_msg], "%02X ", byte);


				if (response[i] != byte) {
					cnt_msg += sprintf(&msg[cnt_msg], "\nBUF[%d] 0x%02X != ", i, response[i]);
					cnt_msg += sprintf(&msg[cnt_msg], "0x%02X\n", byte);
					return false;
				}
				crc16.add(byte);
			}

			if (!mb->isSendData()) {
				cnt_msg += sprintf(&msg[cnt_msg], "\nNo send byte on CRC low.\n");
				return false;
			}

			byte = mb->pull();
			cnt_msg += sprintf(&msg[cnt_msg], "%02X ", byte);
			crc += byte;

			if (!mb->isSendData()) {
				cnt_msg += sprintf(&msg[cnt_msg], "\nNo send byte on CRC high.\n");
				return false;
			}

			byte = mb->pull();
			cnt_msg += sprintf(&msg[cnt_msg], "%02X ", byte);
			crc += (byte << 8);


			if (crc16.get() != crc) {
				cnt_msg += sprintf(&msg[cnt_msg], "\ncrc       = 0x%04X\n", crc);
				cnt_msg += sprintf(&msg[cnt_msg], "crc16.get = 0x%04X\n", crc16.get());
				return false;
			}
		}

		if (mb->isSendData()) {
			cnt_msg += sprintf(&msg[cnt_msg], "\nToo more byte for send (%d).\n",  mb->getNumOfBytes());
			cnt_msg += sprintf(&msg[cnt_msg], "BYTE[%d] = 0x%02X\n", i, mb->pull());
			return false;
		}

		return true;
	}

	char* printArray(uint8_t *arr, uint16_t size) {
		uint16_t cnt = 0;
		for(uint8_t i = 0; i < size; i++) {
			cnt += sprintf(&msg[cnt], "0x%02X ", arr[i]);
		}
		return (char *) arr;
	}

private:
	virtual void SetUp() {
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");

		mb = new TProtocolPcM;
		mb->setTick(57600, 100);
	};

	virtual void TearDown() {
		delete mb;
	};
};

// проверка команды работы с дискретными входами
TEST_F(ProtocolPcM_Test, input) {
	{	// проверка адресов дискретных входов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		ASSERT_EQ(0, mb->D_INPUT_16_01);
		ASSERT_EQ(1, mb->D_INPUT_32_17);
//		ASSERT_TRUE(mb->D_OUTPUT_16_01 == 0) << msg;
//		ASSERT_TRUE(mb->D_OUTPUT_32_17 == 1) << msg;
	}

	{	// проверка начального состояния входов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_32_17));
		ASSERT_EQ(0x0000, mb->getCI());

		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x06};	// CRC 0xC5, 0xC8
		uint8_t res[] = {
				0x01, 0x03, // адрес + команда
				0x0C, 		// количество байт данных
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00
		};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка установки командой записи дискретных входов с 16 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x00, 0x51, 0x37};	// CRC 0xF5, 0x8C
		uint8_t res[] = {0x01, 0x06, 0x00, 0x00, 0x51, 0x37};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x5137, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_32_17));
		ASSERT_EQ(0x0000, mb->getCI());
	}

	{	// проверка считывания дискретных входов с 16 по 1 (по первому адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};	// CRC 0x84, 0x0A
		uint8_t res[] = {0x01, 0x03, 0x02, 0x51, 0x37};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка считывания дискретных входов с 16 по 1 (по второму адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x01};	// CRC 0x25, 0xCA
		uint8_t res[] = {0x01, 0x03, 0x02, 0x51, 0x37};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка установки командой записи дискретных входов с 32 по 17
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x01, 0x83, 0x61};	// CRC 0x78, 0xD2
		uint8_t res[] = {0x01, 0x06, 0x00, 0x01, 0x83, 0x61};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x5137, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x8361, mb->getDI(mb->D_INPUT_32_17));
		ASSERT_EQ(0x0000, mb->getCI());
	}

	{	// проверка считывания дискретных входов с 32 по 17 (по первому адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01};	// CRC 0xD5, 0xCA
		uint8_t res[] = {0x01, 0x03, 0x02, 0x83, 0x61};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка считывания дискретных входов с 32 по 17 (по второму адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x03, 0x00, 0x01};	// CRC 0x74, 0x0A
		uint8_t res[] = {0x01, 0x03, 0x02, 0x83, 0x61};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка установки командой записи дискретных входов с 32 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0xA5, 0x97, 0x48, 0x84};	// CRC 0x56, 0xEC
		uint8_t res[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x02};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0xF5B7, mb->getDI(mb->D_INPUT_16_01)); // 0x5137 | 0xA597
		ASSERT_EQ(0xCBE5, mb->getDI(mb->D_INPUT_32_17)); // 0x8361 | 0x4884
		ASSERT_EQ(0x0000, mb->getCI());
	}

	{	// проверка считывания дискретных входов с 32 по 1 (по обоим адресам)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04};	// CRC 0x44 0x09
		uint8_t res[] = {0x01, 0x03, 0x08, 0xF5, 0xB7, 0xCB, 0xE5, 0xF5, 0xB7, 0xCB, 0xE5};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка сброса командой записи дискретных входов с 16 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x02, 0x73, 0xDF};	// CRC 0x4C, 0xA2
		uint8_t res[] = {0x01, 0x06, 0x00, 0x02, 0x73, 0xDF};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x8420, mb->getDI(mb->D_INPUT_16_01)); // 0xF5B7 & ~0x73DF
		ASSERT_EQ(0xCBE5, mb->getDI(mb->D_INPUT_32_17));
		ASSERT_EQ(0x0000, mb->getCI());
	}

	{	// проверка считывания дискретных входов с 32 по 1 (по обоим адресам)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04};	// CRC 0x44 0x09
		uint8_t res[] = {0x01, 0x03, 0x08, 0x84, 0x20, 0xCB, 0xE5, 0x84, 0x20, 0xCB, 0xE5};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка сброса командой записи дискретных входов с 32 по 17
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x03, 0x17, 0x97};	// CRC 0x37, 0x94
		uint8_t res[] = {0x01, 0x06, 0x00, 0x03, 0x17, 0x97};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x8420, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0xC860, mb->getDI(mb->D_INPUT_32_17)); // 0xCBE5 & ~0x1797
		ASSERT_EQ(0x0000, mb->getCI());
	}

	{	// проверка считывания дискретных входов с 32 по 1 (по обоим адресам)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04};	// CRC 0x44 0x09
		uint8_t res[] = {0x01, 0x03, 0x08, 0x84, 0x20, 0xC8, 0x60, 0x84, 0x20, 0xC8, 0x60};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка сброса записи дискретных входов с 32 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x02, 0x00, 0x02, 0x04, 0xF1, 0x22, 0x3F, 0x44};	// CRC 0xF1 0x43
		uint8_t res[] = {0x01, 0x10, 0x00, 0x02, 0x00, 0x02};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x0400, mb->getDI(mb->D_INPUT_16_01)); // 0x8420 & ~0xF122
		ASSERT_EQ(0xC020, mb->getDI(mb->D_INPUT_32_17)); // 0xC860 & ~0x3F44
		ASSERT_EQ(0x0000, mb->getCI());
	}

	{	// проверка считывания дискретных входов с 32 по 1 (по обоим адресам)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04};	// CRC 0x44 0x09
		uint8_t res[] = {0x01, 0x03, 0x08, 0x04, 0x00, 0xC0, 0x20, 0x04, 0x00, 0xC0, 0x20};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка установки командой записи контрольных частот
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x04, 0x51, 0x37};	// CRC 0xС4, 0xBF
		uint8_t res[] = {0x01, 0x06, 0x00, 0x04, 0x51, 0x37};

		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;

		ASSERT_EQ(0x0400, mb->getDI(mb->D_INPUT_16_01));	// Команды измениться не должны
		ASSERT_EQ(0xC020, mb->getDI(mb->D_INPUT_32_17));	// Команды измениться не должны
		ASSERT_EQ(0x5137, mb->getCI());
	}

	{	// проверка считывания контрольных частот (по первому адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x01};	// CRC 0xC5, 0xCB
		uint8_t res[] = {0x01, 0x03, 0x02, 0x51, 0x37};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка считывания контрольных частот (по второму адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x05, 0x00, 0x01};	// CRC 0x94, 0x0B
		uint8_t res[] = {0x01, 0x03, 0x02, 0x51, 0x37};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка сброса командой записи дискретных входов с 16 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x05, 0x73, 0xDF};	// CRC 0xFD, 0x63
		uint8_t res[] = {0x01, 0x06, 0x00, 0x05, 0x73, 0xDF};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;

		ASSERT_EQ(0x0400, mb->getDI(mb->D_INPUT_16_01));	// Команды измениться не должны
		ASSERT_EQ(0xC020, mb->getDI(mb->D_INPUT_32_17));	// Команды измениться не должны
		ASSERT_EQ(0x0020, mb->getCI());	// 0xF137 & ~0x73DF
	}

	{	// проверка считывания контрольных частот (по первому адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x01};	// CRC 0xC5, 0xCB
		uint8_t res[] = {0x01, 0x03, 0x02, 0x00, 0x20};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка считывания контрольных частот (по второму адресу)
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x05, 0x00, 0x01};	// CRC 0x94, 0x0B
		uint8_t res[] = {0x01, 0x03, 0x02, 0x00, 0x20};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка сброса
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		mb->reset();
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_32_17));
		ASSERT_EQ(0x0000, mb->getCI());

		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x06};	// CRC 0x44 0x09
		uint8_t res[] = {
				0x01, 0x03,	// адрес + команда
				0x0C,		// количество байт данных
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00
		};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// проверка команды работы с дискретными выходами
TEST_F(ProtocolPcM_Test, discreteOutput) {

	{	// проверка адресов дискретных выходов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		ASSERT_TRUE(mb->D_OUTPUT_16_01 == 0) << msg;
		ASSERT_TRUE(mb->D_OUTPUT_32_17 == 1) << msg;
	}

	{	// проверка начального состояния дискретных выходов c 32 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};	// CRC 0x31, 0xCA
		uint8_t res[] = {0x01, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка установки дискретных выходов с 16 по 1
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		mb->setDO(mb->D_OUTPUT_16_01, 0x3782);

		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};	// CRC 0x31, 0xCA
		uint8_t res[] = {0x01, 0x04, 0x04, 0x37, 0x82, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка установки дискретных выходов с 32 по 17
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		mb->setDO(mb->D_OUTPUT_32_17, 0x6495);

		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};	// CRC 0x31, 0xCA
		uint8_t res[] = {0x01, 0x04, 0x04, 0x37, 0x82, 0x64, 0x95};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка сброса
		mb->reset();

		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};	// CRC 0x31, 0xCA
		uint8_t res[] = {0x01, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// проверка взаимного влияния дискретных входов и выхода
TEST_F(ProtocolPcM_Test, discreteIO) {

	{	// установка дискретных выходов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		mb->setDO(mb->D_OUTPUT_16_01, 0x3782);
		mb->setDO(mb->D_OUTPUT_32_17, 0x8916);

		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};
		uint8_t res[] = {0x01, 0x04, 0x04, 0x37, 0x82, 0x89, 0x16};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка состояния дискретных входов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
		uint8_t res[] = {0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x0000, mb->getDI(mb->D_INPUT_32_17));
	}

	{	// установка дискретных входов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0xA5, 0x97, 0x48, 0x84};
		uint8_t res[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x02};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0xA597, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x4884, mb->getDI(mb->D_INPUT_32_17));
	}

	{	// проверка дискретных выходов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};
		uint8_t res[] = {0x01, 0x04, 0x04, 0x37, 0x82, 0x89, 0x16};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// установка дискретных выходов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		mb->setDO(mb->D_OUTPUT_16_01, 0x1234);
		mb->setDO(mb->D_OUTPUT_32_17, 0x5678);

		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};
		uint8_t res[] = {0x01, 0x04, 0x04, 0x12, 0x34, 0x56, 0x78};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// проверка состояния дискретных входов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
		uint8_t res[] = {0x01, 0x03, 0x04, 0xA5, 0x97, 0x48, 0x84};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0xA597, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x4884, mb->getDI(mb->D_INPUT_32_17));
	}

	{	// сброс дискретных входов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x02, 0x00, 0x02, 0x04, 0xF1, 0x11, 0x3F, 0x44};
		uint8_t res[] = {0x01, 0x10, 0x00, 0x02, 0x00, 0x02};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
		ASSERT_EQ(0x0486, mb->getDI(mb->D_INPUT_16_01));
		ASSERT_EQ(0x4080, mb->getDI(mb->D_INPUT_32_17));
	}

	{	// проверка дискретных выходов
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};
		uint8_t res[] = {0x01, 0x04, 0x04, 0x12, 0x34, 0x56, 0x78};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

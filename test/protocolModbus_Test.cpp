#include <cstdio>
#include <iostream>

#include "gtest\gtest.h"
#include "protocolModbus.h"
#include "crc16.h"
using namespace std;


// ������ �������
#define SIZE_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))

#define TPM TProtocolModbus

class ProtocolModbusTest: public ::testing::Test {
public:
	TProtocolModbus *mb;	// ����������� �����
	char msg[1000];			// ����� ���������
	uint16_t cnt_msg; 		// ���-�� ������ � ������

	// �����������
	ProtocolModbusTest() {
		cnt_msg = 0;
		mb = nullptr;
	};

	// ����������
	virtual ~ProtocolModbusTest() {};

	// ��������� ������� ��������� �������
	bool readCom(uint8_t *buf, uint16_t size, uint8_t adr) {
		mb->setAddressLan(adr);
		mb->setReadState();

		// ��������� ������ � �����
		TCrc16 crc16;

		for (uint8_t i = 0; i < size; i++) {
			crc16.add(buf[i]);
			mb->push(buf[i]);
		}

		mb->push(crc16.getLow());
		mb->push(crc16.getHigh());

		// ������������ ������� ��������� �������
		for(uint8_t k = 0; k < 50; k++) {
			mb->tick();
		}

		if (!mb->readData()) {
			cnt_msg += sprintf(&msg[cnt_msg], "Message CRC = 0x%04X.\n", crc16.get());
			return false;
		}

		return true;
	}

	// ��������� ��������, true - ���������
	bool checkArray(uint8_t *response, uint16_t size) {
		uint8_t i = 0;
		uint16_t crc = 0;

		TCrc16 crc16;
		if (size > 0) {
			for(i = 0; i < size; i++) {
				uint8_t byte = 0;
				if (!mb->isSendData()) {
					cnt_msg += sprintf(&msg[cnt_msg], "No send byte on step %d.", i);
					return false;
				}

				byte = mb->pull();

				if (response[i] != byte) {
					cnt_msg += sprintf(&msg[cnt_msg], "BUF[%d] 0x%02X != ", i, response[i]);
					cnt_msg += sprintf(&msg[cnt_msg], "0x%02X\n", byte);
					return false;
				}
				crc16.add(byte);
			}

			if (!mb->isSendData()) {
				cnt_msg += sprintf(&msg[cnt_msg], "No send byte on CRC low.\n");
				return false;
			}

			crc = mb->pull();

			if (!mb->isSendData()) {
				cnt_msg += sprintf(&msg[cnt_msg], "No send byte on CRC high.\n");
				return false;
			}

			crc += (mb->pull() << 8);

			if (crc16.get() != crc) {
				cnt_msg += sprintf(&msg[cnt_msg], "crc       = 0x%04X\n", crc);
				cnt_msg += sprintf(&msg[cnt_msg], "crc16.get = 0x%04X\n", crc16.get());
				return false;
			}
		}

		if (mb->isSendData()) {
			cnt_msg += sprintf(&msg[cnt_msg], "Too more byte for send (%d).\n",  mb->getNumOfBytes());
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

		mb = new TProtocolModbus;
		mb->setTick(57600, 100);
	};

	virtual void TearDown() {
		delete mb;
	};
};

// �������� �������� � ���������� ���������
TEST_F(ProtocolModbusTest, constants) {
	EXPECT_EQ(6, mb->getMaxNumRegisters());
	EXPECT_EQ(32, mb->getMaxNumCoils());
	EXPECT_EQ(255, mb->getAddressError());
	EXPECT_EQ(1, mb->getAddressMin());
	EXPECT_EQ(247, mb->getAddressMax());

	ASSERT_EQ(0, mb->getNumOfBytes());
}

// �������� ������ �������� � ����
TEST_F(ProtocolModbusTest, addressLan) {
	uint8_t adrError = mb->getAddressError();
	uint8_t adrMin = mb->getAddressMin();
	uint8_t adrMax = mb->getAddressMax();

	// 1. �������� ������ ��-���������.
	if (mb->getAddressLan() != adrError) {
		sprintf(msg, "  >>> ����� �� ��������� �� ��������� � ADDRESS_ERR.");
		ASSERT_TRUE(false) << msg;
	}

	// 2. �������� ���������/���������� ������.
	for (uint16_t i = 0; i <= 255; i++) {
		bool t = mb->setAddressLan(i);

		// �������� ������������� �������� �������� serAddress())
		if (((i < adrMin) || (i > adrMax)) == t) {
			sprintf(msg, "  >>> setAddress() ���������� ��������� ��������� �� ���� %d", i);
			ASSERT_TRUE(false) << msg;
		}

		// �������� �������������� ��������
		if (i < adrMin) {
			if (mb->getAddressLan() != adrError) {
				sprintf(msg, "  >>> �������� ������ ADDRESS_MIN �� ����� � ADDRESS_ERR �� ���� %d", i);
				ASSERT_TRUE(false) << msg;
			}
		} else if (i > adrMax) {
			if (mb->getAddressLan() != adrMax) {
				sprintf(msg, "  >>> �������� ������ ADDRESS_MAX �� ����� � ADDRESS_MAX �� ���� %d", i);
				ASSERT_TRUE(false) << msg;
			}
		} else if (i != mb->getAddressLan()) {
			sprintf(msg, "  >>> ������������� ����� �� ��������� �� ��������� �� ���� %d", i);
			ASSERT_TRUE(false) << msg;
		}
	}
}

// �������� ���������
TEST_F(ProtocolModbusTest, state) {
	// ��������� ��� ������������� ������
	EXPECT_TRUE(mb->checkState(mb->STATE_OFF));

	// ������������ ������� ��������� � �������� �������� ���������.
	for(uint8_t i = mb->STATE_OFF; i < mb->STATE_ERROR; i++) {
		TPM::STATE t = static_cast<TPM::STATE> (i);
		mb->setState(t);
		EXPECT_TRUE(mb->checkState(t)) << " >>> ������ �� ���� " << t;
		EXPECT_TRUE(t == mb->getState()) << " >>> ������ �� ���� " << t;
	}

	mb->setEnable();
	EXPECT_TRUE(mb->checkState(TPM::STATE_READ));
	EXPECT_TRUE(mb->isEnable());
	EXPECT_FALSE(mb->isReadData());

	mb->setDisable();
	EXPECT_TRUE(mb->checkState(TPM::STATE_OFF));
	EXPECT_FALSE(mb->isEnable());

	mb->setReadState();
	EXPECT_TRUE(mb->checkState(TPM::STATE_READ));
	EXPECT_FALSE(mb->isReadData());

	for(uint8_t i = TPM::STATE_OFF; i < TPM::STATE_ERROR; i++) {
		TPM::STATE t = static_cast<TPM::STATE> (i);
		mb->setState(t);
		if ((t == TPM::STATE_READ_OK) != mb->isReadData()) {
			EXPECT_TRUE(false) << " >>> ������ �� ���� " << t;
		}
	}
}

// �������� ������ ��������� ����������
TEST_F(ProtocolModbusTest, tick) {
	// ������ ���� ���� ������� �������� ���� �� �����
	// ������� ��� ����������� � ����

	ASSERT_EQ(1000, mb->setTick(115200, 50));
	ASSERT_EQ(1000, mb->setTick(57600, 50));
	ASSERT_EQ(2000, mb->setTick(57600, 100));
	ASSERT_EQ(872, mb->setTick(19200, 50));
	ASSERT_EQ(1745, mb->setTick(19200, 100));
	ASSERT_EQ(13, mb->setTick(300, 50));
	ASSERT_EQ(27, mb->setTick(300, 100));

	struct sData {
		TPM::STATE startState;	// ��������� ��������� ���������
		TPM::STATE stopState;	// �������� ��������� ���������
		uint8_t	 numBytes;		// ���-�� ���� ������ � ������
		uint16_t baudrate;		// �������� ������ �����
		uint8_t  period; 		// ������ ������ ������� tick()
		uint16_t numTicks; 		// ���-�� �����

	};

	sData data[] = {
			{TPM::STATE_OFF, 		TPM::STATE_OFF, 		8, 19200, 50, 50},
			{TPM::STATE_READ, 		TPM::STATE_READ, 		3, 57600, 50, 35},
			{TPM::STATE_READ, 		TPM::STATE_READ_OK, 	4, 57600, 50, 35},
			{TPM::STATE_READ, 		TPM::STATE_READ_OK, 	5, 57600, 50, 35},
			{TPM::STATE_READ, 		TPM::STATE_READ_OK, 	6, 19200, 50, 41},
			{TPM::STATE_READ, 		TPM::STATE_READ, 		8, 19200, 50, 40},
			{TPM::STATE_READ, 		TPM::STATE_READ_OK, 	8, 19200, 50, 41},
			{TPM::STATE_READ_ERROR, TPM::STATE_READ, 		8, 19200, 50, 41},
			{TPM::STATE_READ_OK, 	TPM::STATE_READ_OK, 	8, 19200, 50, 50},
			{TPM::STATE_WRITE, 		TPM::STATE_WRITE, 		8, 19200, 50, 50}
	};

	for (uint16_t i = 0; i < (sizeof (data) / sizeof (data[0])); i++) {

		uint16_t step = mb->setTick(data[i].baudrate, data[i].period);
		mb->setState(TPM::STATE_READ);	// ��� ������ ��������
		mb->setState(data[i].startState);	// ������ ��� ����� ���������

		// ������� � ����� �������� ���-�� ���� ������
		for(uint8_t j = 0; j < data[i].numBytes; j++) {
			mb->push(0x00);
		}

		for (uint8_t j = 0; j < data[i].numTicks; j++) {
			mb->tick();
		}

		TPM::STATE state = mb->getState();
		if (state != data[i].stopState) {
			uint8_t cnt = sprintf(msg, "  >>> ������ �� ���� %d", i);
			cnt += sprintf(&msg[cnt], "\n start = %d,  finish = %d, need = %d",
					data[i].startState, state, data[i].stopState);
			cnt += sprintf(&msg[cnt], "\n step tick = %d", step);
			ASSERT_TRUE(false) << msg;
		}
	}
}

// �������� ����������� �������� ������
TEST_F(ProtocolModbusTest, push) {
	mb->setState(TPM::STATE_READ);

	// 1. �������� ������ ����������� ���������� ���-�� ���� ������
	// ��� �������� ������������ tick
	for (uint16_t i = 1; i < 60; i++) {
		uint16_t t = mb->push(i);
		if (i <= 30) {
			if (t != i) {
				sprintf(msg, "  <<< �������� ���������� �������� ���� �� ���� %d", i);
				ASSERT_TRUE(false) << msg;
			}
		} else {
			if (t != 30) {
				sprintf(msg, "  <<< ������������ ������ �� ���� %d", i);
				ASSERT_TRUE(false) << msg;
			}
		}
	}

	// 2. �������� ������ ������� ��� ����� 1.5 ���������.
	struct sData {
		TPM::STATE stateStart;	// ��������� ��������� � ������
		TPM::STATE stateStop;	// ��������� ��������� � �����
		uint8_t numOfPush;	// ���-�� �������� ������
		uint8_t stepToReset;	// ���, �� ������� ����������� ����� ������
		uint8_t numOfByte;	// ���-�� �������� ���� �� ���������
	};

	sData data[] = {
			{TPM::STATE_OFF, 		TPM::STATE_OFF, 		20, 21,	0},
			{TPM::STATE_OFF, 		TPM::STATE_OFF, 		20, 5, 	0},
			{TPM::STATE_READ, 		TPM::STATE_READ, 		20, 20,	20},
			{TPM::STATE_READ, 		TPM::STATE_READ_ERROR, 	20, 18,	18},
			{TPM::STATE_READ, 		TPM::STATE_READ_ERROR, 	20, 2,	2},
			{TPM::STATE_READ_ERROR,	TPM::STATE_READ_ERROR, 	20, 18,	0},
			{TPM::STATE_READ_OK, 	TPM::STATE_READ_OK, 	20, 18,	0},
			{TPM::STATE_WRITE, 		TPM::STATE_WRITE, 		20, 21,	0}
	};

	for (uint16_t i = 0; i < (sizeof (data) / sizeof (data[0])); i++) {
		mb->setTick(57600, 100);
		mb->setState(TPM::STATE_READ);	// ��� ������ ��������
		mb->setState(data[i].stateStart);	// ������ ��� ����� ���������

		uint8_t num = 0;
		for (uint8_t j = 0; j < data[i].numOfPush; j++) {
			// ������������ ����� ������
			if (j == data[i].stepToReset) {
				for(uint8_t k = 0; k < 15; k++) {
					mb->tick();
				}
			}
			num = mb->push(0x00);
		}

		TPM::STATE state = mb->getState();
		if (state != data[i].stateStop) {
			uint8_t cnt = sprintf(msg, "  >>> ��������� ��������� �� ���� %d", i);
			cnt += sprintf(&msg[cnt], "\n start = %d,  finish = %d, need = %d", data[i].stateStart, state, data[i].stateStop);
			cnt += sprintf(&msg[cnt], "\n num = %d", num);
			ASSERT_TRUE(false) << msg;
		}

		if (num != data[i].numOfByte) {
			uint8_t cnt = sprintf(msg, "  >>> ��������� ���-�� �������� ���� �� ���� %d", i);
			cnt += sprintf(&msg[cnt], "\n start = %d,  finish = %d", data[i].stateStart, data[i].stateStop);
			cnt += sprintf(&msg[cnt], "\n num = %d, need = %d", num, data[i].numOfByte);
			ASSERT_TRUE(false) << msg;
		}
	}
}

TEST_F(ProtocolModbusTest, trResponse) {

	// ��������� ������������ ������ � ��������� STATE_WRITE_READY
	uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
	uint8_t min = TPM::STATE_OFF;
	uint8_t max = TPM::STATE_ERROR;


	for(uint8_t i = min; i < max; i++) {
		readCom(req, SIZE_ARRAY(req), req[0]);


		TPM::STATE t = static_cast<TPM::STATE> (i);
		mb->setState(t);

		if ((t == TPM::STATE_WRITE) != (mb->isSendData())) {
			sprintf(msg, "  >>> ������ �� ���� %d", t);
			ASSERT_TRUE(false) << msg;
		}
	}
}

// �������� ������ � �������� ����������� ������
TEST_F(ProtocolModbusTest, com_crc) {

	{	// ���������� CRC
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
		mb->setAddressLan(req[0]);
		mb->setReadState();

		for (uint8_t i = 0; i < SIZE_ARRAY(req); i++) {
			mb->push(req[i]);
		}

		// ������������ ������� ��������� �������
		for(uint8_t k = 0; k < 50; k++) {
			mb->tick();
		}

		ASSERT_TRUE(mb->readData());
	}

	{	// ������ �������� ����� CRC
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xED, 0xCA};
		mb->setAddressLan(req[0]);
		mb->setReadState();

		for (uint8_t i = 0; i < SIZE_ARRAY(req); i++) {
			mb->push(req[i]);
		}

		// ������������ ������� ��������� �������
		for(uint8_t k = 0; k < 50; k++) {
			mb->tick();
		}

		ASSERT_FALSE(mb->readData());
	}

	{	// ������ �������� ����� CRC
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0x1A};
		mb->setAddressLan(req[0]);
		mb->setReadState();

		for (uint8_t i = 0; i < SIZE_ARRAY(req); i++) {
			mb->push(req[i]);
		}

		// ������������ ������� ��������� �������
		for(uint8_t k = 0; k < 50; k++) {
			mb->tick();
		}

		ASSERT_FALSE(mb->readData());
	}
}

// �������� ������� ������ ������
TEST_F(ProtocolModbusTest, com_0x01_read_coil) {

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01}; 	// CRC 0xFD, 0xCA
		uint8_t res[] = {0x01, 0x01, 0x01, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01};	// CRC 0xFD, 0xCA
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00};
		uint8_t res[] = {0x01, 0x81, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
		uint8_t res[] = {0x01, 0x81, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� �������������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x65, 0x00, 0x01};	// CRC 0xED, 0xD5
		uint8_t res[] = {0x01, 0x01, 0x01, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x01, 0x2B, 0x00, 0x01};	// CRC 0x8C, 0x3E
		uint8_t res[] = {0x01, 0x01, 0x01, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ������ �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x11, 0x01, 0x00, 0x61, 0x00, 0x0A};	// CRC 0xEF, 0x43
		uint8_t res[] = {0x11, 0x01, 0x02, 0x07, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ����������� ���������� ���-�� �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x11, 0x01, 0x00, 0x50, 0x00, 0x20};	// CRC 0x3F, 0x53
		uint8_t res[] = {0x11, 0x01, 0x04, 0xFF, 0xFF, 0x0F, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������� 0
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00};	// CRC 0x3C, 0x0A
		uint8_t res[] = {0x01, 0x81, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ���-�� ������������� ������� 33
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x21};	// CRC 0xFC, 0x12
		uint8_t res[] = {0x01, 0x81, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ���������� ���������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x01, 0x21, 0x00, 0x0C};	// CRC 0x6D, 0xF9
		uint8_t res[] = {0x01, 0x81, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ������ ������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x01, 0x01, 0x2C, 0x00, 0x01};	// CRC 0x3D, 0xFF
		uint8_t res[] = {0x01, 0x81, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}


}

// �������� ������� ������ ���������� ������
TEST_F(ProtocolModbusTest, com_0x02_discrete_inputs) {

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x01};	// CRC 0xB9, 0xCA
		uint8_t res[] = {0x01, 0x02, 0x01, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x01};	// CRC 0xB9, 0xCA
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x00, 0x00};
		uint8_t res[] = {0x01, 0x82, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00};
		uint8_t res[] = {0x01, 0x82, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� �������������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x65, 0x00, 0x01};	// CRC 0xA9, 0xD5
		uint8_t res[] = {0x01, 0x02, 0x01, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x01, 0x2B, 0x00, 0x01};	// CRC 0xC8, 0x3E
		uint8_t res[] = {0x01, 0x02, 0x01, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ������ �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x11, 0x02, 0x00, 0x61, 0x00, 0x0A};	// CRC 0xAB, 0x43
		uint8_t res[] = {0x11, 0x02, 0x02, 0x07, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ����������� ���������� ���-�� �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x11, 0x02, 0x00, 0x50, 0x00, 0x20};	// CRC 0x7B, 0x53
		uint8_t res[] = {0x11, 0x02, 0x04, 0xFF, 0xFF, 0x0F, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������� 0
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00};	// CRC 0x78, 0x0A
		uint8_t res[] = {0x01, 0x82, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ���-�� ������������� ������� 33
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x21};	// CRC 0xB8, 0x12
		uint8_t res[] = {0x01, 0x82, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ���������� ���������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x01, 0x21, 0x00, 0x0C};	// CRC 0x29, 0xF9
		uint8_t res[] = {0x01, 0x82, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ������ ������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x02, 0x01, 0x2C, 0x00, 0x01};	// CRC 0x79, 0xFF
		uint8_t res[] = {0x01, 0x82, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}


}

// �������� ������� ������ ��������� ���������
TEST_F(ProtocolModbusTest, com_0x03_read_holding_register) {

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};	// CRC 0x84, 0x0A
		uint8_t res[] = {0x01, 0x03, 0x02, 0x00, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};	// CRC 0x84, 0x0A
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		// FIXME CRC �������� � ���������� ��������� � �������� ������ ������
//		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
//		uint8_t req[] = {0x01, 0x03, 0x00, 0x01, 0x00};
//		uint8_t res[] = {0x01, 0x83, 0x04};
//		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
//		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00};
		uint8_t res[] = {0x01, 0x83, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� �������������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x63, 0x00, 0x01};	// CRC 0x74, 0x14
		uint8_t res[] = {0x01, 0x03, 0x02, 0x00, 0x64};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x01, 0x2B, 0x00, 0x01};	// CRC 0xF5, 0xFE
		uint8_t res[] = {0x01, 0x03, 0x02, 0x01, 0x2C};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ������ �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x62, 0x00, 0x03};	// CRC 0xA4, 0x15
		uint8_t res[] = {0x01, 0x03, 0x06, 0x00, 0x63, 0x00, 0x64, 0xFF, 0xFF};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ������������� ���������� �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0xC7, 0x00, 0x04};	// CRC 0xF5, 0xF4
		uint8_t res[] = {0x01, 0x03, 0x08, 0xFF, 0xFF, 0x0, 0xC9, 0x00, 0xCA, 0x00, 0xCB};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������� 0
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x00};	// CRC 0x45, 0xCA
		uint8_t res[] = {0x01, 0x83, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ���-�� ������������� ������� 7
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07};	// CRC 0x85, 0xC9
		uint8_t res[] = {0x01, 0x83, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ���������� ���������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x01, 0x2A, 0x00, 0x03};	// CRC 0x25, 0xFF
		uint8_t res[] = {0x01, 0x83, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ������ ������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x03, 0x01, 0x2C, 0x00, 0x01};	// CRC 0x44, 0x3F
		uint8_t res[] = {0x01, 0x83, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// �������� ������� ������ ������� ���������
TEST_F(ProtocolModbusTest, com_0x04_read_input_register) {

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x01};	// CRC 0x31, 0xCA
		uint8_t res[] = {0x01, 0x04, 0x02, 0x00, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x01};	// CRC 0x31, 0xCA
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		// FIXME CRC �������� � ���������� ��������� � �������� ������ ������
//		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
//		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00};
//		uint8_t res[] = {0x01, 0x84, 0x04};
//		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
//		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00};
		uint8_t res[] = {0x01, 0x84, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� �������������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x63, 0x00, 0x01};	// CRC 0x74, 0x14
		uint8_t res[] = {0x01, 0x04, 0x02, 0x00, 0x64};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ���������� ������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x01, 0x2B, 0x00, 0x01};	// CRC 0xF5, 0xFE
		uint8_t res[] = {0x01, 0x04, 0x02, 0x01, 0x2C};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ������ �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x62, 0x00, 0x03};	// CRC 0xA4, 0x15
		uint8_t res[] = {0x01, 0x04, 0x06, 0x00, 0x63, 0x00, 0x64, 0xFF, 0xFF};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ���������� ������������� ���������� �������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0xC7, 0x00, 0x04};	// CRC 0xF5, 0xF4
		uint8_t res[] = {0x01, 0x04, 0x08, 0xFF, 0xFF, 0x0, 0xC9, 0x00, 0xCA, 0x00, 0xCB};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������� 0
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x00};	// CRC 0x45, 0xCA
		uint8_t res[] = {0x01, 0x84, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ���-�� ������������� ������� 7
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x07};	// CRC 0x85, 0xC9
		uint8_t res[] = {0x01, 0x84, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ���������� ���������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x01, 0x2A, 0x00, 0x03};	// CRC 0x25, 0xFF
		uint8_t res[] = {0x01, 0x84, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� �� �������� ��������� �������, ��� ������ ������
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x04, 0x01, 0x2C, 0x00, 0x01};	// CRC 0x44, 0x3F
		uint8_t res[] = {0x01, 0x84, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// �������� ������� ������ ������ �����
TEST_F(ProtocolModbusTest, com_0x05_write_single_coil) {

	{	// �������� ������ � ������ �����
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00};	// CRC 0x8C, 0x3A
		uint8_t res[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00};	// CRC 0x8C, 0x3A
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		// FIXME CRC �������� � ���������� ��������� � �������� ������ ������
//		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
//		uint8_t req[] = {0x01, 0x05, 0x00, 0x00, 0xFF};
//		uint8_t res[] = {0x01, 0x85, 0x04};
//		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
//		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x00};
		uint8_t res[] = {0x01, 0x85, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ � ��������� �����
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x01, 0x2B, 0x00, 0x00};	// CRC 0xBC, 0x3E
		uint8_t res[] = {0x01, 0x05, 0x01, 0x2B, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ � ������������� ����� �������� true
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0x98, 0xFF, 0x00};	// CRC 0x0D, 0xD5
		uint8_t res[] = {0x01, 0x05, 0x00, 0x98, 0xFF, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ � ������������� ����� �������� false
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0xEE, 0x00, 0x00};	// CRC 0xAD, 0xFF
		uint8_t res[] = {0x01, 0x05, 0x00, 0xEE, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������ � �������������� �����
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x01, 0x2C, 0x00, 0x00};	// CRC 0x0D, 0xFF
		uint8_t res[] = {0x01, 0x85, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������������ �������� ��� ������ true
		//  ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0xC8, 0xFF, 0x00};	// CRC 0x0D, 0xC4
		uint8_t res[] = {0x01, 0x85, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������������ �������� ��� ������ false
		//  ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x05, 0x00, 0x64, 0x00, 0x00};	// CRC 0x8C, 0x15
		uint8_t res[] = {0x01, 0x85, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: �������� �������� �����
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x88, 0x05, 0x01, 0x60, 0x00, 0xFF};	// CRC 0x93, 0x31
		uint8_t res[] = {0x88, 0x85, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// �������� ������ ������ ��������
TEST_F(ProtocolModbusTest, com_0x06_write_single_register) {

	{	// �������� ������ � ������ �����
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x00, 0xFF, 0x00};	// CRC 0xC8, 0x3A
		uint8_t res[] = {0x01, 0x06, 0x00, 0x00, 0xFF, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x00, 0xFF, 0x00};	// CRC 0xEF, 0x57
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x00, 0xFF};
		uint8_t res[] = {0x01, 0x86, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x00, 0xFF, 0x00, 0x00};
		uint8_t res[] = {0x01, 0x86, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ � ��������� �����
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x01, 0x2B, 0x11, 0x17};	// CRC 0xB4, 0x60
		uint8_t res[] = {0x01, 0x06, 0x01, 0x2B, 0x11, 0x17};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ � ������������� ����� 0x0000
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0x66, 0x00, 0x00};	// CRC 0x69, 0xD5
		uint8_t res[] = {0x01, 0x06, 0x00, 0x66, 0x00, 0x00};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ � ������������� ����� 0xFFFF
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0xEE, 0xFF, 0xFF};	// CRC 0xE8, 0x4F
		uint8_t res[] = {0x01, 0x06, 0x00, 0xEE, 0xFF, 0xFF};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������ � �������������� �����
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x01, 0x2C, 0x00, 0x00};	// CRC 0x49, 0xFF
		uint8_t res[] = {0x01, 0x86, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������������ �������� ��� ������ � �������� "�� ������ 100"
		//  ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x00, 0xC5, 0x80, 0x65};	// CRC 0x38, 0x1C
		uint8_t res[] = {0x01, 0x86, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������������ �������� ��� ������ � �������� "�� ������ 1000"
		//  ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x06, 0x01, 0x21, 0x00, 0x01};	// CRC 0x19, 0xFC
		uint8_t res[] = {0x01, 0x86, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// �������� ������� ������ ������ ������
TEST_F(ProtocolModbusTest, com_0x0F_write_multiplie_coils) {

	{	// �������� ������ ������ �����
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01};	// CRC 0xEF, 0x57
		uint8_t res[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01};	// CRC 0xEF, 0x57
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01, 0x01};
		uint8_t res[] = {0x01, 0x8F, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00};
		uint8_t res[] = {0x01, 0x8F, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������ ������, ������ 8
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x07, 0x01, 0x15};	// CRC 0x0F, 0x59
		uint8_t res[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x07};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������ ������, ������ 8
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x0B, 0x02, 0x15, 0x01};	// CRC 0x2B, 0x94
		uint8_t res[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x0B};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������ ������, ������ 8
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x18, 0x03, 0x15, 0x11, 0x28};	// CRC 0x5D, 0xFE
		uint8_t res[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x18};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������������� ���-�� ������, 32
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {	0x01, 0x0F, 0x00, 0x00, 0x00, 0x20, 0x04,
							0x0F, 0x00, 0x17, 0x5A};	//
		uint8_t res[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x20};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ��������� ����� ��������� ������������ �����
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x01, 0x2C, 0x00, 0x02, 0x01, 0x03};	// CRC 0x0E, 0x81
		uint8_t res[] = {0x01, 0x8F, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ��������� ����� + ���������� ������ ��������� ������������ �����
		//  ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x0F, 0x01, 0x22, 0x00, 0x10, 0x02, 0x00, 0x15};	// CRC 0xA2, 0x48
		uint8_t res[] = {0x01, 0x8F, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������� ���������� ���� ��������������� ������ ��� ������
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x88, 0x0F, 0x00, 0xFC, 0x00, 0x02, 0x01, 0x02};	// CRC 0xC7, 0x48
		uint8_t res[] = {0x88, 0x8F, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������� �������� ���� ��������������� ������ ��� ���������
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x88, 0x0F, 0x00, 0x71, 0x00, 0x02, 0x01, 0x00};	// CRC 0x6A, 0x96
		uint8_t res[] = {0x88, 0x8F, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ �������� ���-�� ������
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x11, 0x0F, 0x00, 0x13, 0x00, 0x00, 0x00};	// CRC 0x1E, 0x7A
		uint8_t res[] = {0x11, 0x8F, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ���-�� ������� ������������ ��������, 257
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {	0x11, 0x0F, 0x00, 0x00, 0x00, 0x21, 0x05,
							0x01, 0x02, 0x03, 0x04, 0x05};	// CRC
		uint8_t res[] = {0x11, 0x8F, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// �������� ������� ������ ������ ���������
TEST_F(ProtocolModbusTest, com_0x10_write_multiplie_registers) {

	{	// �������� ������ ������ ��������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00};	// CRC 0xA6, 0x50
		uint8_t res[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ����� ����������
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00};	// CRC 0xA6, 0x50
		uint8_t res[] = {};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0] + 1)) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00};
		uint8_t res[] = {0x01, 0x90, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ���������� ������ ������ �� 1 ����
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00};
		uint8_t res[] = {0x01, 0x90, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������ ��������� ������ ���������, 2
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {	0x01, 0x10, 0x00, 0x10, 0x00, 0x02, 0x04,
							0x15, 0x0F, 0x14, 0x88};
		uint8_t res[] = {0x01, 0x10, 0x00, 0x10, 0x00, 0x02};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������������� ���-�� ���������, 4
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {0x11, 0x10, 0x00, 0x01, 0x00, 0x04, 0x08,
				0x15, 0x0F, 0x14, 0x88, 0x15, 0x0F, 0x14, 0x88};
		uint8_t res[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x04};
		ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ��������� ����� ��������� ������������ �����
		// ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {	0x01, 0x10, 0x01, 0x2C, 0x00, 0x02, 0x04,
							0xFF, 0xFF, 0xAA, 0xBB}; // CRC	0xC2, 0x85
		uint8_t res[] = {0x01, 0x90, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ��������� ����� + ���������� ��������� ��������� ������������ �����
		//  ���������� EXCEPTION_02H_ILLEGAL_DATA_ADR
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {0x01, 0x10, 0x01, 0x2A, 0x00, 0x03, 0x06,
							0xFF, 0xFF, 0x10, 0x00, 0x51, 0x98};
		uint8_t res[] = {0x01, 0x90, 0x02};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// ������: ������� ������ � ������� "�� ������ 100" ��������� ��������
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {0x88, 0x10, 0x00, 0x7D, 0x00, 0x02, 0x04,
							0x00, 0x01, 0x00, 0x65};
		uint8_t res[] = {0x88, 0x90, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	//  ������: ������� ������ � ������� "�� ������ 1000" ��������� ��������
		// ���������� EXCEPTION_04H_DEVICE_FAILURE
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {0x88, 0x10, 0x00, 0xC8, 0x00, 0x02, 0x04,
							0x03, 0xEE, 0x03, 0xE7};
		uint8_t res[] = {0x88, 0x90, 0x04};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ �������� ���-�� ������
		// ���������� EXCEPTION_03H_ILLEGAL_DATA_VAL
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00};
		uint8_t res[] = {0x11, 0x90, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}

	{	// �������� ������ ������������� ���-�� ���������, 7
		cnt_msg = sprintf(&msg[0], " <<< Error <<< \n");
		uint8_t req[] =  {
				0x11, 0x10, // ����� + ��� �������
				0x00, 0x01, // ����� ������� ��������
				0x00, 0x07, // ���������� ���������
				0x0E, 		// ���������� ���� ������
				0x15, 0x0F, 0x14, 0x88, 0x15, 0x0F, 0x14, 0x88,
			    0x15, 0x0F, 0x14, 0x88, 0x15, 0x0F
		};
		uint8_t res[] = {0x11, 0x90, 0x03};
		ASSERT_FALSE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
		ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
	}
}

// �������� ������� ������ ���������� �� ����������
TEST_F(ProtocolModbusTest, com_0x11_slave_ID) {

	uint8_t req[] = {0x03, 0x11, 0xC1, 0x4C};
	uint8_t res[] = {0x03, 0x11, 0x09, 'V','i','r','t','u','a','l', '\0', 0x00};
	ASSERT_TRUE(readCom(req, SIZE_ARRAY(req), req[0])) << msg;
	ASSERT_TRUE(checkArray(res, SIZE_ARRAY(res))) << msg;
}

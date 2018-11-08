#include "gtest\gtest.h"
#include "protocolModbus.h"
#include <cstdio>
#include <iostream>
using namespace std;

// размер массива
#define SIZE_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))

#define TPM TProtocolModbus

char msg[1000];		// буфер сообщений
uint16_t cnt_msg = 0; 	// кол-во данных в буфере

class TCrc16 {
public:
	TCrc16() {
		reset();
	}

	void reset() {
		crc = 0xFFFF;
	}

	void add(uint8_t val) {
		crc ^= val;

		for (int i = 8; i != 0; i--) {
			if ((crc & 0x0001) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
			} else {
				crc >>= 1;
			}
		}
	}

	uint16_t get() const {
		return crc;
	}

	uint8_t getLow() const {
		return crc;
	}

	uint8_t getHigh() const {
		return crc >> 8;
	}

private:
	uint16_t crc;
};

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

class ProtocolModbusTest_Constants: public ::testing::Test {
public:
	TProtocolModbus mb;

private:

	virtual void SetUp() {};
	virtual void TearDown() {};
};

TEST_F(ProtocolModbusTest_Constants, getMaxNumRegisters) {
	ASSERT_EQ(8, mb.getMaxNumRegisters());
}

TEST_F(ProtocolModbusTest_Constants, getMaxNumCoils) {
	ASSERT_EQ(32, mb.getMaxNumCoils());
}

TEST_F(ProtocolModbusTest_Constants, getAddressError) {
	ASSERT_EQ(255, mb.getAddressError());
}

TEST_F(ProtocolModbusTest_Constants, getAddressMin) {
	ASSERT_EQ(1, mb.getAddressMin());
}

TEST_F(ProtocolModbusTest_Constants, getAddressMax) {
	ASSERT_EQ(247, mb.getAddressMax());
}

class ProtocolModbusTest_State: public ::testing::Test {
public:
	TProtocolModbus mb;

private:
	virtual void SetUp() {};
	virtual void TearDown() {};
};

// Проверка состояния при инициализации класса
TEST_F(ProtocolModbusTest_State, protocolModbus) {
	ASSERT_TRUE(mb.checkState(mb.STATE_OFF));
}
//
TEST_F(ProtocolModbusTest_State, setState_and_checkState) {
	// Тестирование функций установки и проверки текущего состояния.

	uint8_t min = mb.STATE_OFF;
	uint8_t max = mb.STATE_ERROR;
	for(uint8_t i = min; i < max; i++) {
		TPM::STATE t = static_cast<TPM::STATE> (i);
		mb.setState(t);
		ASSERT_TRUE(mb.checkState(t)) << " >>> Ошибка на шаге " << t;
	}
}

TEST_F(ProtocolModbusTest_State, getState) {
	uint8_t min = TPM::STATE_OFF;
	uint8_t max = TPM::STATE_ERROR;
	for(uint8_t i = min; i < max; i++) {
		TPM::STATE t = static_cast<TPM::STATE> (i);
		mb.setState(t);
		ASSERT_TRUE(t == mb.getState()) << " >>> Ошибка на шаге " << t;
	}
}

TEST_F(ProtocolModbusTest_State, setEnable) {
	mb.setEnable();
	ASSERT_TRUE(mb.checkState(TPM::STATE_READ));
}

TEST_F(ProtocolModbusTest_State, setDisable) {
	mb.setEnable();
	mb.setDisable();
	ASSERT_TRUE(mb.checkState(TPM::STATE_OFF));
}

TEST_F(ProtocolModbusTest_State, isEnable) {
	mb.setEnable();
	ASSERT_TRUE(mb.isEnable());
	mb.setDisable();
	ASSERT_TRUE(!mb.isEnable());
}

TEST_F(ProtocolModbusTest_State, setReadState) {
	mb.setReadState();
	ASSERT_TRUE(mb.checkState(TPM::STATE_READ));
}

TEST_F(ProtocolModbusTest_State, isReadData) {
	uint8_t min = TPM::STATE_OFF;
	uint8_t max = TPM::STATE_ERROR;
	for(uint8_t i = min; i < max; i++) {
		TPM::STATE t = static_cast<TPM::STATE> (i);
		mb.setState(t);
		if ((t == TPM::STATE_READ_OK) != mb.isReadData()) {
			ASSERT_TRUE(false) << " >>> Ошибка на шаге " << t;
		}
	}

	mb.setDisable();
	ASSERT_TRUE(!mb.isReadData());
	mb.setReadState();
	ASSERT_TRUE(!mb.isReadData());
}

// Проверка проводится при помощи "вспомогательной" функции checkState.
class ProtocolModbusTest_Push_and_Tick: public ::testing::Test {
public:
	TProtocolModbus mb;

private:
	virtual void SetUp() {};
	virtual void TearDown() {};
};

TEST_F(ProtocolModbusTest_Push_and_Tick, setTick) {
	ASSERT_EQ(1000, mb.setTick(115200, 50));
	ASSERT_EQ(1000, mb.setTick(57600, 50));
	ASSERT_EQ(2000, mb.setTick(57600, 100));
	ASSERT_EQ(872, mb.setTick(19200, 50));
	ASSERT_EQ(1745, mb.setTick(19200, 100));
	ASSERT_EQ(13, mb.setTick(300, 50));
	ASSERT_EQ(27, mb.setTick(300, 100));
}

TEST_F(ProtocolModbusTest_Push_and_Tick, tick) {
	// Работа этих двух функций завязана друг на друге
	// поэтому они тестируются в паре

	struct sData {
		TPM::STATE startState;	// начальное состояние протокола
		TPM::STATE stopState;	// конечное состояние протокола
		uint8_t	 numBytes;		// кол-во байт данных в буфере
		uint16_t baudrate;		// скорость работы порта
		uint8_t  period; 		// период вызова функции tick()
		uint16_t numTicks; 		// кол-во тиков

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
			{TPM::STATE_WRITE_WAIT, TPM::STATE_WRITE_WAIT, 	8, 19200, 50, 50},
			{TPM::STATE_WRITE, 		TPM::STATE_WRITE, 		8, 19200, 50, 50}
	};

	for (uint16_t i = 0; i < (sizeof (data) / sizeof (data[0])); i++) {

		uint16_t step = mb.setTick(data[i].baudrate, data[i].period);
		mb.setState(TPM::STATE_READ);	// для сброса счетчика
		mb.setState(data[i].startState);	// нужное для теста состояние

		// закинем в буфер заданное кол-во байт данных
		for(uint8_t j = 0; j < data[i].numBytes; j++) {
			mb.push(0x00);
		}

		for (uint8_t j = 0; j < data[i].numTicks; j++) {
			mb.tick();
		}

		TPM::STATE state = mb.getState();
		if (state != data[i].stopState) {
			uint8_t cnt = sprintf(msg, "  >>> Ошибка на шаге %d", i);
			cnt += sprintf(&msg[cnt], "\n start = %d,  finish = %d, need = %d",
					data[i].startState, state, data[i].stopState);
			cnt += sprintf(&msg[cnt], "\n step tick = %d", step);
			ASSERT_TRUE(false) << msg;
		}
	}
}

TEST_F(ProtocolModbusTest_Push_and_Tick, push) {
	mb.setState(TPM::STATE_READ);

	// 1. Проверка записи максимально возможного кол-ва байт данных
	// без проверки срабатывания tick
	for (uint16_t i = 1; i < 60; i++) {
		uint16_t t = mb.push(i);
		if (i <= 30) {
			if (t != i) {
				sprintf(msg, "  <<< Неверное количество принятых байт на шаге %d", i);
				ASSERT_TRUE(false) << msg;
			}
		} else {
			if (t != 30) {
				sprintf(msg, "  <<< Переполнение буфера на шаге %d", i);
				ASSERT_TRUE(false) << msg;
			}
		}
	}

	// 2. Проверка сброса посылки при паузе 1.5 интервала.
	struct sData {
		TPM::STATE stateStart;	// состояние протокола в начале
		TPM::STATE stateStop;	// состояние протокола в конце
		uint8_t numOfPush;	// кол-во принятых данных
		uint8_t stepToReset;	// шаг, на котором формируется пауза сброса
		uint8_t numOfByte;	// кол-во принятых байт по окончанию
	};

	sData data[] = {
			{TPM::STATE_OFF, 		TPM::STATE_OFF, 		20, 21,	0},
			{TPM::STATE_OFF, 		TPM::STATE_OFF, 		20, 5, 	0},
			{TPM::STATE_READ, 		TPM::STATE_READ, 		20, 20,	20},
			{TPM::STATE_READ, 		TPM::STATE_READ_ERROR, 	20, 18,	18},
			{TPM::STATE_READ, 		TPM::STATE_READ_ERROR, 	20, 2,	2},
			{TPM::STATE_READ_ERROR,	TPM::STATE_READ_ERROR, 	20, 18,	0},
			{TPM::STATE_READ_OK, 	TPM::STATE_READ_OK, 	20, 18,	0},
			{TPM::STATE_WRITE_WAIT,	TPM::STATE_WRITE_WAIT, 	20, 18,	0},
			{TPM::STATE_WRITE, 		TPM::STATE_WRITE, 		20, 21,	0}
	};

	for (uint16_t i = 0; i < (sizeof (data) / sizeof (data[0])); i++) {
		mb.setTick(57600, 100);
		mb.setState(TPM::STATE_READ);	// для сброса счетчика
		mb.setState(data[i].stateStart);	// нужное для теста состояние

		uint8_t num = 0;
		for (uint8_t j = 0; j < data[i].numOfPush; j++) {
			// формирование паузы сброса
			if (j == data[i].stepToReset) {
				for(uint8_t k = 0; k < 15; k++) {
					mb.tick();
				}
			}
			num = mb.push(0x00);
		}

		TPM::STATE state = mb.getState();
		if (state != data[i].stateStop) {
			uint8_t cnt = sprintf(msg, "  >>> Ошибочное состояние на шаге %d", i);
			cnt += sprintf(&msg[cnt], "\n start = %d,  finish = %d, need = %d", data[i].stateStart, state, data[i].stateStop);
			cnt += sprintf(&msg[cnt], "\n num = %d", num);
			ASSERT_TRUE(false) << msg;
		}

		if (num != data[i].numOfByte) {
			uint8_t cnt = sprintf(msg, "  >>> Ошибочное кол-во принятых байт на шаге %d", i);
			cnt += sprintf(&msg[cnt], "\n start = %d,  finish = %d", data[i].stateStart, data[i].stateStop);
			cnt += sprintf(&msg[cnt], "\n num = %d, need = %d", num, data[i].numOfByte);
			ASSERT_TRUE(false) << msg;
		}
	}
}

// Проверка проводится при помощи "вспомогательной" функции checkState.
class ProtocolModbusTest_AddressLan: public ::testing::Test {
public:

	TProtocolModbus mb;

private:
	virtual void SetUp() {};
	virtual void TearDown() {};
};

TEST_F(ProtocolModbusTest_AddressLan, get_and_set) {
	uint8_t adrError = mb.getAddressError();
	uint8_t adrMin = mb.getAddressMin();
	uint8_t adrMax = mb.getAddressMax();

	// 1. Проверка адреса по-умолчанию.
	if (mb.getAddressLan() != adrError) {
		sprintf(msg, "  >>> Адрес по умолчанию не совпадает с ADDRESS_ERR.");
		ASSERT_TRUE(false) << msg;
	}

	// 2. Проверка установки/считывания адреса.
	for (uint16_t i = 0; i <= 255; i++) {
		bool t = mb.setAddressLan(i);

		// проверка возвращаемого значения функцией serAddress())
		if (((i < adrMin) || (i > adrMax)) == t) {
			sprintf(msg, "  >>> setAddress() возвращает ошибочное состояние на шаге %d", i);
			ASSERT_TRUE(false) << msg;
		}

		// проверка установленного значения
		if (i < adrMin) {
			if (mb.getAddressLan() != adrError) {
				sprintf(msg, "  >>> Значение меньше ADDRESS_MIN не ведет к ADDRESS_ERR на шаге %d", i);
				ASSERT_TRUE(false) << msg;
			}
		} else if (i > adrMax) {
			if (mb.getAddressLan() != adrMax) {
				sprintf(msg, "  >>> Значение больше ADDRESS_MAX не ведет к ADDRESS_MAX на шаге %d", i);
				ASSERT_TRUE(false) << msg;
			}
		} else if (i != mb.getAddressLan()) {
			sprintf(msg, "  >>> Установленный адрес не совпадает со считанным на шаге %d", i);
			ASSERT_TRUE(false) << msg;
		}
	}
}

class ProtocolModbusTest_readData: public ::testing::Test {
public:
	TProtocolModbus *mb;					// тестируемый класс

	// конструктор
	ProtocolModbusTest_readData() {
		mb = new TProtocolModbus;
		mb->setTick(57600, 100);
	}

	TCrc16 crc16;

	// деструктор
	virtual ~ProtocolModbusTest_readData() {
		delete mb;
	}

	// формирует процесс получения посылки
	bool readCom(uint8_t *buf, uint16_t size) {
		mb->setAddressLan(buf[0]);
		mb->setReadState();

		// пересылка данных в буфер
		crc16.reset();
		for (uint8_t i = 0; i < size; i++) {
			if (i < (size - 2)) {
				crc16.add(buf[i]);
			}

			mb->push(buf[i]);
		}

		uint16_t crc = *((uint16_t *) (&buf[size - 2]));
		if (crc != crc16.get()) {
			cnt_msg += sprintf(&msg[cnt_msg], "crc       = 0x%04X\n", crc);
			cnt_msg += sprintf(&msg[cnt_msg], "crc16.get = 0x%04X\n", crc16.get());
			return false;
		}

		// формирование сигнала окончания посылки
		for(uint8_t k = 0; k < 50; k++) {
			mb->tick();
		}

		if (!mb->isReadData()) {

			return false;
		}

		if (!mb->readData()) {
			cnt_msg += sprintf(&msg[cnt_msg], "Received message is error.");
			return false;
		}

		return true;
	}

	// сравнение массивов, true - совпадают
	bool checkArray(uint8_t *response, uint16_t size) {
		uint8_t i = 0;
		uint16_t crc = 0;


		crc16.reset();
		for(i = 0; i < size; i++) {
			uint8_t byte = 0;
			if (!mb->isSendData()) {
				cnt_msg += sprintf(&msg[cnt_msg], "No send byte on step %d.", i);
				return false;
			}

			byte = mb->pull();

			if ( i < (size - 2)) {
				if (response[i] != byte) {
					cnt_msg += sprintf(&msg[cnt_msg], "BUF[%d] 0x%02X != ", i, response[i]);
					cnt_msg += sprintf(&msg[cnt_msg], "0x%02X\n", byte);
					return false;
				}
				crc16.add(byte);
			} else if (i == (size - 2)) {
				crc = byte;
			} else if (i == (size - 1)) {
				crc += (byte << 8);
			}
		}

		if (crc16.get() != crc) {
			cnt_msg += sprintf(&msg[cnt_msg], "crc       = 0x%04X\n", crc);
			cnt_msg += sprintf(&msg[cnt_msg], "crc16.get = 0x%04X\n", crc16.get());
			return false;
		}

		if (mb->isSendData()) {
			cnt_msg += sprintf(&msg[cnt_msg], "Too more byte for send (%d).\n",  mb->getNumOfBytes());
			cnt_msg += sprintf(&msg[cnt_msg], "BYTE[%d] = 0x%02X\n", i, mb->pull());
			return false;
		}

		return true;
	}

	// тестирование
	bool test(uint8_t *request, uint16_t size1, uint8_t *response, uint16_t size2) {
		readCom(request, size1);
		return checkArray(response, size2);
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
		cnt_msg = sprintf(&msg[cnt_msg], " <<< Error <<< \n");
	};
	virtual void TearDown() {};
};

TEST_F(ProtocolModbusTest_readData, getNumOfBytes) {
	// проверяется только значение по умолчанию
	// дополнительная проверка в readData
	ASSERT_EQ(0, mb->getNumOfBytes());
}

// проверка команд с неверной контрольной суммой
TEST_F(ProtocolModbusTest_readData, com_crc_error) {

	uint8_t req1[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0x1A};
	uint8_t res1[] = {};
	ASSERT_FALSE(readCom(req1, SIZE_ARRAY(req1))) << msg;
	ASSERT_TRUE(checkArray(res1, SIZE_ARRAY(res1))) << msg;

//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;

	uint8_t req2[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xED, 0xCA};
	uint8_t res2[] = {};
	ASSERT_FALSE(readCom(req2, SIZE_ARRAY(req2))) << msg;
	ASSERT_TRUE(checkArray(res2, SIZE_ARRAY(res2))) << msg;
}

// проверка команды чтения флагов
TEST_F(ProtocolModbusTest_readData, com_0x01_read_coil) {

	// считывание начального адреса
	uint8_t req1[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
	uint8_t res1[] = {0x01, 0x01, 0x01, 0x01};
	ASSERT_TRUE(readCom(req1, SIZE_ARRAY(req1))) << msg;
	ASSERT_TRUE(checkArray(res1, SIZE_ARRAY(res1))) << msg;

	// считывание промежуточного адреса
	uint8_t req2[] = {0x01, 0x01, 0x00, 0x65, 0x00, 0x01, 0xED, 0xD5};
	uint8_t res2[] = {0x01, 0x01, 0x01, 0x00};
	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;

	// считывание последнего адреса
	uint8_t req3[] = {0x01, 0x01, 0x01, 0x2B, 0x00, 0x01, 0x8C, 0x3E};
	uint8_t res3[] = {0x01, 0x01, 0x01, 0x01, 0x90, 0x48};
	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;

	// считывание группы адресов
	uint8_t req4[] = {0x11, 0x01, 0x00, 0x50, 0x00, 0x29, 0xFF, 0x55};
	uint8_t res4[] = {0x11, 0x01, 0x06, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00};
	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;

	// cчитывание максимально возможного кол-ва адресов
	uint8_t req5[] = {0x11, 0x01, 0x00, 0x00, 0x01, 0x00, 0x3F, 0x0A};
	uint8_t res5[] = {0x11, 0x01, 0x20,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;

	// ошибка: количество адресов 0
	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
	uint8_t req6[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x0A};
	uint8_t res6[] = {0x01, 0x81, 0x03};
	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;

	// ошибка: превышение кол-ва запрашиваемых адресов 247
	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
	uint8_t req7[] = {0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0xFC, 0x5A};
	uint8_t res7[] = {0x01, 0x81, 0x03};
	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;

	// ошибка: выход за диапазон доступных адресов, при корректном стартовом
	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
	uint8_t req8[] = {0x01, 0x01, 0x01, 0x21, 0x00, 0x0C, 0x6D, 0xF9};
	uint8_t res8[] = {0x01, 0x81, 0x02};
	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;

	// ошибка: выход за диапазон доступных адресов, при ошибке адреса
	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
	uint8_t req9[] = {0x01, 0x01, 0x01, 0x2C, 0x00, 0x01, 0x3D, 0xFF};
	uint8_t res9[] = {0x01, 0x81, 0x02};
	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
}

//// проверка команды чтения дискретных входов
//TEST_F(ProtocolModbusTest_readData, com_0x02_discrete_inputs) {
//
//	// считывание начального адреса
//	uint8_t req1[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0xB9, 0xCA};
//	uint8_t res1[] = {0x01, 0x02, 0x01, 0x01};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// считывание промежуточного адреса
//	uint8_t req2[] = {0x01, 0x02, 0x00, 0x65, 0x00, 0x01, 0xA9, 0xD5};
//	uint8_t res2[] = {0x01, 0x02, 0x01, 0x00};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// считывание последнего адреса
//	uint8_t req3[] = {0x01, 0x02, 0x01, 0x2B, 0x00, 0x01, 0xC8, 0x3E};
//	uint8_t res3[] = {0x01, 0x02, 0x01, 0x01};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// считывание группы адресов
//	uint8_t req4[] = {0x11, 0x02, 0x00, 0x50, 0x00, 0x29, 0xBB, 0x55};
//	uint8_t res4[] = {0x11, 0x02, 0x06, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// cчитывание максимально возможного кол-ва адресов
//	uint8_t req5[] = {0x11, 0x02, 0x00, 0x00, 0x01, 0x00, 0x7B, 0x0A};
//	uint8_t res5[] = {0x11, 0x02, 0x20,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
//	};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: количество адресов 0
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req6[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0A};
//	uint8_t res6[] = {0x01, 0x82, 0x03};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: превышение кол-ва запрашиваемых адресов 247
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req7[] = {0x01, 0x02, 0x00, 0x00, 0x01, 0x01, 0xB8, 0x5A};
//	uint8_t res7[] = {0x01, 0x82, 0x03};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// ошибка: выход за диапазон доступных адресов, при корректном стартовом
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req8[] = {0x01, 0x02, 0x01, 0x21, 0x00, 0x0C, 0x29, 0xF9};
//	uint8_t res8[] = {0x01, 0x82, 0x02};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//
//	// ошибка: выход за диапазон доступных адресов, при ошибке адреса
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req9[] = {0x01, 0x02, 0x01, 0x2C, 0x00, 0x01, 0x79, 0xFF};
//	uint8_t res9[] = {0x01, 0x82, 0x02};
//	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
//}
//
//// проверка команды чтения внутрених регистров
//TEST_F(ProtocolModbusTest_readData, com_0x03_read_holding_register) {
//
//	// считывание первого
//	uint8_t req1[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
//	uint8_t res1[] = {0x01, 0x03, 0x02, 0x00, 0x01};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// считывание промежуточного адреса
//	uint8_t req2[] = {0x01, 0x03, 0x00, 0x63, 0x00, 0x01, 0x74, 0x14};
//	uint8_t res2[] = {0x01, 0x03, 0x02, 0x00, 0x64};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// считывание последнего адреса
//	uint8_t req3[] = {0x01, 0x03, 0x01, 0x2B, 0x00, 0x01, 0xF5, 0xFE};
//	uint8_t res3[] = {0x01, 0x03, 0x02, 0x01, 0x2C};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// считывание группы адресов
//	uint8_t req4[] = {0x01, 0x03, 0x00, 0x60, 0x00, 0x05, 0x85, 0xD7};
//	uint8_t res4[] = {0x01, 0x03, 0x0A, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63,
//			0x00, 0x64, 0xFF, 0xFF};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// считывание максимального количества адресов
//	uint8_t req5[] = {0x01, 0x03, 0x00, 0x50, 0x00, 0x20, 0x44, 0x03};
//	uint8_t res5[] = {0x01, 0x03, 0x40,
//			0x00, 0x51, 0x00, 0x52, 0x00, 0x53, 0x00, 0x54,
//			0x00, 0x55, 0x00, 0x56, 0x00, 0x57, 0x00, 0x58,
//			0x00, 0x59, 0x00, 0x5A, 0x00, 0x5B, 0x00, 0x5C,
//			0x00, 0x5D, 0x00, 0x5E, 0x00, 0x5F, 0x00, 0x60,
//			0x00, 0x61, 0x00, 0x62,	0x00, 0x63, 0x00, 0x64,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
//	};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: количество адресов 0
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req6[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x45, 0xCA};
//	uint8_t res6[] = {0x01, 0x83, 0x03};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: превышение кол-ва запрашиваемых адресов 33
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req7[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x21, 0x85, 0xD2};
//	uint8_t res7[] = {0x01, 0x83, 0x03};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// ошибка: выход за диапазон доступных адресов, при корректном стартовом
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req8[] = {0x01, 0x03, 0x01, 0x2A, 0x00, 0x03, 0x25, 0xFF};
//	uint8_t res8[] = {0x01, 0x83, 0x02};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//
//	// ошибка: выход за диапазон доступных адресов, при ошибке адреса
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req9[] = {0x01, 0x03, 0x01, 0x2C, 0x00, 0x01, 0x44, 0x3F};
//	uint8_t res9[] = {0x01, 0x83, 0x02};
//	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
//}
//
//// проверка команды чтения входных регистров
//TEST_F(ProtocolModbusTest_readData, com_0x04_read_input_register) {
//
//	// считывание первого
//	uint8_t req1[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA};
//	uint8_t res1[] = {0x01, 0x04, 0x02, 0x00, 0x01};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// считывание промежуточного адреса
//	uint8_t req2[] = {0x01, 0x04, 0x00, 0x63, 0x00, 0x01, 0xC1, 0xD4};
//	uint8_t res2[] = {0x01, 0x04, 0x02, 0x00, 0x64};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// считывание последнего адреса
//	uint8_t req3[] = {0x01, 0x04, 0x01, 0x2B, 0x00, 0x01, 0x40, 0x3E};
//	uint8_t res3[] = {0x01, 0x04, 0x02, 0x01, 0x2C};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// считывание группы адресов
//	uint8_t req4[] = {0x01, 0x04, 0x00, 0x60, 0x00, 0x05, 0x30, 0x17};
//	uint8_t res4[] = {0x01, 0x04, 0x0A, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63,
//			0x00, 0x64, 0xFF, 0xFF};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// считывание максимального количества адресов
//	uint8_t req5[] = {0x01, 0x04, 0x00, 0x50, 0x00, 0x20, 0xF1, 0xC3};
//	uint8_t res5[] = {0x01, 0x04, 0x40,
//			0x00, 0x51, 0x00, 0x52, 0x00, 0x53, 0x00, 0x54,
//			0x00, 0x55, 0x00, 0x56, 0x00, 0x57, 0x00, 0x58,
//			0x00, 0x59, 0x00, 0x5A, 0x00, 0x5B, 0x00, 0x5C,
//			0x00, 0x5D, 0x00, 0x5E, 0x00, 0x5F, 0x00, 0x60,
//			0x00, 0x61, 0x00, 0x62,	0x00, 0x63, 0x00, 0x64,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
//	};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: количество адресов 0
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req6[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x0A};
//	uint8_t res6[] = {0x01, 0x84, 0x03};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: превышение кол-ва запрашиваемых адресов 33
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req7[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x21, 0x30, 0x12};
//	uint8_t res7[] = {0x01, 0x84, 0x03};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// ошибка: выход за диапазон доступных адресов, при корректном стартовом
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req8[] = {0x01, 0x04, 0x01, 0x2A, 0x00, 0x03, 0x90, 0x3F};
//	uint8_t res8[] = {0x01, 0x84, 0x02};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//
//	// ошибка: выход за диапазон доступных адресов, при ошибке адреса
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req9[] = {0x01, 0x04, 0x01, 0x2C, 0x00, 0x01, 0xF1, 0xFF};
//	uint8_t res9[] = {0x01, 0x84, 0x02};
//	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
//}
//
//// проверка команды записи одного флага
//TEST_F(ProtocolModbusTest_readData, com_0x05_write_single_coil) {
//
//	// проверка записи в первый адрес
//	uint8_t req1[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A};
//	uint8_t res1[] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// проверка записи в последний адрес
//	uint8_t req2[] = {0x01, 0x05, 0x01, 0x2B, 0x00, 0x00, 0xBC, 0x3E};
//	uint8_t res2[] = {0x01, 0x05, 0x01, 0x2B, 0x00, 0x00};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// проверка записи в промежуточный адрес значения true
//	uint8_t req3[] = {0x01, 0x05, 0x00, 0x98, 0xFF, 0x00, 0x0D, 0xD5};
//	uint8_t res3[] = {0x01, 0x05, 0x00, 0x98, 0xFF, 0x00};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// проверка записи в промежуточный адрес значения false
//	uint8_t req4[] = {0x01, 0x05, 0x00, 0xEE, 0x00, 0x00, 0xAD, 0xFF};
//	uint8_t res4[] = {0x01, 0x05, 0x00, 0xEE, 0x00, 0x00};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// ошибка: запись в несуществующий адрес
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req5[] = {0x01, 0x05, 0x01, 0x2C, 0x00, 0x00, 0x0D, 0xFF};
//	uint8_t res5[] = {0x01, 0x85, 0x02};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: недопустимое значение для записи true
//	//  исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req6[] = {0x01, 0x05, 0x00, 0xC8, 0xFF, 0x00, 0x0D, 0xC4};
//	uint8_t res6[] = {0x01, 0x85, 0x04};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: недопустимое значение для записи false
//	//  исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req7[] = {0x01, 0x05, 0x00, 0x64, 0x00, 0x00, 0x8C, 0x15};
//	uint8_t res7[] = {0x01, 0x85, 0x04};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// ошибка: неверное значение флага
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req8[] = {0x88, 0x05, 0x01, 0x60, 0x00, 0xFF, 0x93, 0x31};
//	uint8_t res8[] = {0x88, 0x85, 0x03};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//}
//
//// проверка команды записи группы флагов
//TEST_F(ProtocolModbusTest_readData, com_0x0F_write_multiplie_coils) {
//
//	// проверка записи одного флага
//	uint8_t req1[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0xEF, 0x57};
//	uint8_t res1[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x01};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// проверка записи группы флагов, меньше 8
//	uint8_t req2[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x07, 0x01, 0x15, 0x0F, 0x59};
//	uint8_t res2[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x07};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// проверка записи группы флагов, больше 8
//	uint8_t req3[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x0B, 0x02, 0x15, 0x01, 0x2B, 0x94};
//	uint8_t res3[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x0B};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// проверка записи группы флагов, кратно 8
//	uint8_t req4[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x18, 0x03, 0x15, 0x11, 0x28, 0x5D, 0xFE};
//	uint8_t res4[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x18};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// проверка записи максимального кол-ва флагов, 256
//	uint8_t req5[] = {0x01, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x20,
//			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
//			0x09, 0x0A, 0x0B, 0x0C, 0xF0, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x8B, 0x8A };
//	uint8_t res5[] = {0x01, 0x0F, 0x00, 0x00, 0x01, 0x00};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: стартовый адрес преывшает максимальный адрес
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req6[] = {0x01, 0x0F, 0x01, 0x2C, 0x00, 0x02, 0x01, 0x03, 0x0E, 0x81};
//	uint8_t res6[] = {0x01, 0x8F, 0x02};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: стартовый адрес + количество флагов превышают максимальный адрес
//	//  исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req7[] = {0x01, 0x0F, 0x01, 0x22, 0x00, 0x10, 0x02, 0x00, 0x00, 0x15, 0xA2, 0x48};
//	uint8_t res7[] = {0x01, 0x8F, 0x02};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// ошибка: попытка установить флаг предназначенный только для сброса
//	// исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req8[] = {0x88, 0x0F, 0x00, 0xFC, 0x00, 0x02, 0x01, 0x02, 0xC7, 0x48};
//	uint8_t res8[] = {0x88, 0x8F, 0x04};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//
//	// ошибка: попытка сбросить флаг предназначенный только для установки
//	// исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req9[] = {0x88, 0x0F, 0x00, 0x71, 0x00, 0x02, 0x01, 0x00, 0x6A, 0x96};
//	uint8_t res9[] = {0x88, 0x8F, 0x04};
//	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
//
//	// проверка записи нулевого кол-ва флагов
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req10[] = {0x11, 0x0F, 0x00, 0x13, 0x00, 0x00, 0x00, 0x1E, 0x7A};
//	uint8_t res10[] = {0x11, 0x8F, 0x03};
//	ASSERT_TRUE(test(req10, SIZE_ARRAY(req10), res10, SIZE_ARRAY(res10))) << msg;
//
//	// проверка записи кол-ва адресов превышающего максимум, 257
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req11[] = {0x11, 0x0F, 0x00, 0x13, 0x01, 0x00, 0x21,
//			0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
//			0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
//			0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
//			0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
//			0x05,
//			0xA7, 0x9E};
//	uint8_t res11[] = {0x11, 0x8F, 0x03};
//	ASSERT_TRUE(test(req11, SIZE_ARRAY(req11), res11, SIZE_ARRAY(res11))) << msg;
//}
//
//// проверка записи одного регистра
//TEST_F(ProtocolModbusTest_readData, com_0x06_write_single_register) {
//
//	// проверка записи в первый адрес
//	uint8_t req1[] = {0x01, 0x06, 0x00, 0x00, 0xFF, 0x00, 0xC8, 0x3A};
//	uint8_t res1[] = {0x01, 0x06, 0x00, 0x00, 0xFF, 0x00};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// проверка записи в последний адрес
//	uint8_t req2[] = {0x01, 0x06, 0x01, 0x2B, 0x11, 0x17, 0xB4, 0x60};
//	uint8_t res2[] = {0x01, 0x06, 0x01, 0x2B, 0x11, 0x17};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// проверка записи в промежуточный адрес 0x0000
//	uint8_t req3[] = {0x01, 0x06, 0x00, 0x66, 0x00, 0x00, 0x69, 0xD5};
//	uint8_t res3[] = {0x01, 0x06, 0x00, 0x66, 0x00, 0x00};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// проверка записи в промежуточный адрес 0xFFFF
//	uint8_t req4[] = {0x01, 0x06, 0x00, 0xEE, 0xFF, 0xFF, 0xE8, 0x4F};
//	uint8_t res4[] = {0x01, 0x06, 0x00, 0xEE, 0xFF, 0xFF};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// ошибка: запись в несуществующий адрес
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req5[] = {0x01, 0x06, 0x01, 0x2C, 0x00, 0x00, 0x49, 0xFF};
//	uint8_t res5[] = {0x01, 0x86, 0x02};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: недопустимое значение для записи в регистры "не больше 100"
//	//  исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req6[] = {0x01, 0x06, 0x00, 0xC5, 0x80, 0x65, 0x38, 0x1C};
//	uint8_t res6[] = {0x01, 0x86, 0x04};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: недопустимое значение для записи в регистры "не меньше 1000"
//	//  исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req7[] = {0x01, 0x06, 0x01, 0x21, 0x00, 0x01, 0x19, 0xFC};
//	uint8_t res7[] = {0x01, 0x86, 0x04};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//}
//
//// проверка команды записи группы регистров
//TEST_F(ProtocolModbusTest_readData, com_0x10_write_multiplie_registers) {
//
//	// проверка записи одного регистра
//	uint8_t req1[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0xA6, 0x50};
//	uint8_t res1[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01};
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//
//	// проверка записи группы регистров, меньше 8
//	uint8_t req2[] = {0x01, 0x10, 0x00, 0x10, 0x00, 0x07, 0x0E,
//			0x15, 0x0F, 0x14, 0x88, 0x91, 0x94, 0x71, 0x98,
//			0x12, 0xFF, 0xE1, 0x12, 0x17, 0x71,
//			0xEA, 0x36};
//	uint8_t res2[] = {0x01, 0x10, 0x00, 0x10, 0x00, 0x07};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// проверка записи группы регистров, больше 8
//	uint8_t req3[] = {0x01, 0x10, 0x00, 0x25, 0x00, 0x09, 0x12,
//			0x15, 0x0F, 0x14, 0x88, 0x91, 0x94, 0x71, 0x98,
//			0x12, 0xFF, 0xE1, 0x12, 0x17, 0x71, 0x77, 0x99,
//			0x13, 0x14,
//			0x0A, 0x27};
//	uint8_t res3[] = {0x01, 0x10, 0x00, 0x25, 0x00, 0x09};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// проверка записи группы регистров, кратно 8
//	uint8_t req4[] = {0x01, 0x10, 0x00, 0xC6, 0x00, 0x08, 0x10,
//			0x00, 0x11, 0x00, 0x62, 0x91, 0x94, 0x71, 0x98,
//			0x12, 0xFF, 0xE1, 0x12, 0x17, 0x71, 0x77, 0x99,
//			0x0B, 0x51};
//	uint8_t res4[] = {0x01, 0x10, 0x00, 0xC6, 0x00, 0x08};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// проверка записи максимального кол-ва регистров, 32
//	uint8_t req5[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x20, 0x40,
//			 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
//			 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
//			 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
//			 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
//			 0xED, 0xDE};
//	uint8_t res5[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x20};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// ошибка: стартовый адрес преывшает максимальный адрес
//	// исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req6[] = {0x01, 0x10, 0x01, 0x2C, 0x00, 0x02, 0x04,
//			0xFF, 0xFF, 0xAA, 0xBB,
//			0xC2, 0x85};
//	uint8_t res6[] = {0x01, 0x90, 0x02};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// ошибка: стартовый адрес + количество регистров превышают максимальный адрес
//	//  исключение EXCEPTION_02H_ILLEGAL_DATA_ADR
//	uint8_t req7[] = {0x01, 0x10, 0x01, 0x2A, 0x00, 0x04, 0x08,
//			0xFF, 0xFF, 0x10, 0x00, 0x51, 0x98, 0x78, 0xFF,
//			0x9C, 0x94};
//	uint8_t res7[] = {0x01, 0x90, 0x02};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// ошибка: попытка записи в регистр "не больше 100" неверного значения
//	// исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req8[] = {0x88, 0x10, 0x00, 0x7D, 0x00, 0x02, 0x04,
//			0x00, 0x01, 0x00, 0x65, 0x23, 0xA7};
//	uint8_t res8[] = {0x88, 0x90, 0x04};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//
//	//  ошибка: попытка записи в регистр "не меньше 1000" неверного значения
//	// исключение EXCEPTION_04H_DEVICE_FAILURE
//	uint8_t req9[] = {0x88, 0x10, 0x00, 0xC8, 0x00, 0x02, 0x04,
//			0x03, 0xEE, 0x03, 0xE7, 0x59, 0xCC};
//	uint8_t res9[] = {0x88, 0x90, 0x04};
//	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
//
//	// проверка записи нулевого кол-ва флагов
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req10[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x19, 0x6D};
//	uint8_t res10[] = {0x11, 0x90, 0x03};
//	ASSERT_TRUE(test(req10, SIZE_ARRAY(req10), res10, SIZE_ARRAY(res10))) << msg;
//
//	// проверка записи кол-ва адресов превышающего максимум, 33
//	// исключение EXCEPTION_03H_ILLEGAL_DATA_VAL
//	uint8_t req11[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x21, 0x42,
//			 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
//			 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
//			 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
//			 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
//			 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
//			 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
//			 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
//			 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
//			 0x09, 0x09, 0x19, 0xEC};
//	uint8_t res11[] = {0x11, 0x90, 0x03};
//	ASSERT_TRUE(test(req11, SIZE_ARRAY(req11), res11, SIZE_ARRAY(res11))) << msg;
//}
//
//// проверка команды чтения инфорамции об устройстве
//TEST_F(ProtocolModbusTest_readData, com_0x11_slave_ID) {
//
//	uint8_t req1[] = {0x03, 0x11, 0xC1, 0x4C};
//	uint8_t res1[] = {0x03, 0x11, 0x09, 'V','i','r','t','u','a','l', '\0', 0x00};
//
//	ASSERT_TRUE(test(req1, SIZE_ARRAY(req1), res1, SIZE_ARRAY(res1))) << msg;
//}
//
//class ProtocolModbusTest_trResponse: public ::testing::Test {
//public:
//	static const uint16_t SIZE_BUF = 255; 	// размер буфера данных
//	uint8_t buf[SIZE_BUF];					// буфер данных
//	TPM *mb;								// тестируемый класс
//
//	// конструктор
//	ProtocolModbusTest_trResponse() {
//		mb = new TPM(buf, SIZE_BUF);
//		mb->setTick(57600, 100);
//	}
//
//	// деструктор
//	virtual ~ProtocolModbusTest_trResponse() {
//		delete mb;
//	}
//
//	// формирует процесс получения посылки
//	void readCom(uint8_t *buf, uint16_t size) {
//
//		mb->setReadState();
//
//		// пересылка данных в буфер
//		for (uint8_t i = 0; i < size; i++) {
//			mb->push(*buf++);
//		}
//
//		// формирование сигнала окончания посылки
//		for(uint8_t k = 0; k < 50; k++) {
//			mb->tick();
//		}
//	}
//
//	// сравнение массивов, true - совпадают
//	bool checkArray(uint8_t *buf1, uint16_t size1, uint8_t *buf2, uint16_t size2) {
//		bool state = true;
//
//		if (size1 != size2) {
//			cnt_msg += sprintf(&msg[cnt_msg], "(sizeof(expected) = %d) != ", size1);
//			cnt_msg += sprintf(&msg[cnt_msg], "(sizeof(response) = %d) \n", size2);
//			state = false;
//		}
//
//
//		for (uint16_t i = 0; i < size1; i++) {
//			if (buf1[i] != buf2[i]) {
//				cnt_msg += sprintf(&msg[cnt_msg], "(buf1[%d] = 0x%02X) != ", i, buf1[i]);
//				cnt_msg += sprintf(&msg[cnt_msg], "(buf2[%d] = 0x%02X) \n", i, buf2[i]);
//				state = false;
//				break;
//			}
//		}
//
//		if (!state) {
//			for(uint16_t i = 0; i < size1; i++) {
//				cnt_msg += sprintf(&msg[cnt_msg], "0x%02X ", buf1[i]);
//			}
//
//			cnt_msg +=  sprintf(&msg[cnt_msg], "\n");
//
//			for(uint16_t i = 0; i < size2; i++) {
//				cnt_msg += sprintf(&msg[cnt_msg], "0x%02X ", buf2[i]);
//			}
//		}
//
//		return state;
//	}
//
//	// тестирование
//	bool test(uint8_t *request, uint16_t size1, uint8_t *response, uint16_t size2) {
//		cnt_msg = 0;
//		readCom(request, size1);
//		mb->setAddressLan(request[0]);
//		mb->readData();
//		mb->sendData();
//		return checkArray(response, size2, buf, mb->getNumOfBytes());
//	}
//
//	char* printArray(uint8_t *arr, uint16_t size) {
//		uint16_t cnt = 0;
//		for(uint8_t i = 0; i < size; i++) {
//			cnt += sprintf(&msg[cnt], "0x%02X ", arr[i]);
//		}
//		return (char *) arr;
//	}
//
//private:
//	virtual void SetUp() {};
//	virtual void TearDown() {};
//};
//
//TEST_F(ProtocolModbusTest_trResponse, trResponse) {
//
//	// сообщение отправляется только в состоянии STATE_WRITE_READY
//	uint8_t req1[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
//	uint8_t min = TPM::STATE_OFF;
//	uint8_t max = TPM::STATE_ERROR;
//	for(uint8_t i = min; i < max; i++) {
//		readCom(req1, SIZE_ARRAY(req1));
//		mb->setAddressLan(req1[0]);
//		mb->readData();
//
//		TPM::STATE t = static_cast<TPM::STATE> (i);
//		mb->setState(t);
//
//		if ((t == TPM::STATE_WRITE_READY) != (mb->sendData() != 0)) {
//			sprintf(msg, "  >>> Ошибка на шаге %d", t);
//			ASSERT_TRUE(false) << msg;
//		}
//	}
//
//	// проверка подсчета контрольной суммы в ответе команды 0x01
//	uint8_t req2[] = {0x11, 0x01, 0x00, 0x50, 0x00, 0x29, 0xFF, 0x55};
//	uint8_t res2[] = {0x11, 0x01, 0x06, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x6E, 0x63};
//	ASSERT_TRUE(test(req2, SIZE_ARRAY(req2), res2, SIZE_ARRAY(res2))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x03
//	uint8_t req3[] = {0x01, 0x03, 0x00, 0x63, 0x00, 0x01, 0x74, 0x14};
//	uint8_t res3[] = {0x01, 0x03, 0x02, 0x00, 0x64, 0xB9, 0xAF};
//	ASSERT_TRUE(test(req3, SIZE_ARRAY(req3), res3, SIZE_ARRAY(res3))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x05
//	uint8_t req4[] = {0x01, 0x05, 0x00, 0xEE, 0x00, 0x00, 0xAD, 0xFF};
//	uint8_t res4[] = {0x01, 0x05, 0x00, 0xEE, 0x00, 0x00, 0xAD, 0xFF};
//	ASSERT_TRUE(test(req4, SIZE_ARRAY(req4), res4, SIZE_ARRAY(res4))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x06
//	uint8_t req5[] = {0x01, 0x06, 0x00, 0xEE, 0xFF, 0xFF, 0xE8, 0x4F};
//	uint8_t res5[] = {0x01, 0x06, 0x00, 0xEE, 0xFF, 0xFF, 0xE8, 0x4F};
//	ASSERT_TRUE(test(req5, SIZE_ARRAY(req5), res5, SIZE_ARRAY(res5))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x0F
//	uint8_t req6[] = {0x01, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x20,
//			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
//			0x09, 0x0A, 0x0B, 0x0C, 0xF0, 0xFF, 0xFF, 0xFF,
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//			0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x8B, 0x8A };
//	uint8_t res6[] = {0x01, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x54, 0x5B};
//	ASSERT_TRUE(test(req6, SIZE_ARRAY(req6), res6, SIZE_ARRAY(res6))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x10
//	uint8_t req7[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0xA6, 0x50};
//	uint8_t res7[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0xC9};
//	ASSERT_TRUE(test(req7, SIZE_ARRAY(req7), res7, SIZE_ARRAY(res7))) << msg;
//
//	// првоерки подсчета контрольной суммы в ответе команды 0x11
//	uint8_t req8[] = {0x03, 0x11, 0xC1, 0x4C};
//	uint8_t res8[] = {0x03, 0x11, 0x09, 'V','i','r','t','u','a','l', '\0', 0x00, 0xFB, 0x6D};
//	ASSERT_TRUE(test(req8, SIZE_ARRAY(req8), res8, SIZE_ARRAY(res8))) << msg;
//
//	// проверка подсчета контрольной суммы в исключении
//	uint8_t req9[] = {0x11, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x19, 0x6D};
//	uint8_t res9[] = {0x11, 0x90, 0x03, 0x0D, 0xC4};
//	ASSERT_TRUE(test(req9, SIZE_ARRAY(req9), res9, SIZE_ARRAY(res9))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x02
//	uint8_t req10[] = {0x01, 0x02, 0x01, 0x2B, 0x00, 0x01, 0xC8, 0x3E};
//	uint8_t res10[] = {0x01, 0x02, 0x01, 0x01, 0x60, 0x48};
//	ASSERT_TRUE(test(req10, SIZE_ARRAY(req10), res10, SIZE_ARRAY(res10))) << msg;
//
//	// проверка подсчета контрольной суммы в ответе команды 0x04
//	uint8_t req11[] = {0x01, 0x04, 0x00, 0x60, 0x00, 0x05, 0x30, 0x17};
//	uint8_t res11[] = {0x01, 0x04, 0x0A, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63,
//			0x00, 0x64, 0xFF, 0xFF, 0xB3, 0x4E};
//	ASSERT_TRUE(test(req11, SIZE_ARRAY(req11), res11, SIZE_ARRAY(res11))) << msg;
//}

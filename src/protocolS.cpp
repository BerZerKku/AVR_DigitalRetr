/*
 * protocolS.cpp
 *
 *  Created on: 25.09.2019
 *      Author: Shcheblykin
 */

#include <assert.h>
#include "protocolS.h"


// Конструктор.
ProtocolS::ProtocolS() {
	cnt = 0;
	nTx = 0;
	buf[0] = 0x00;
	buf[1] = 0x00;
	state = STATE_OFF;

	for(uint8_t i = 0; i < D_INPUT_MAX; i++) {
		dIn[i] = 0x00;
	}

	for(uint8_t i = 0; i < D_OUTPUT_MAX; i++) {
		dOut[i] = 0x00;
	}

	cIn = 0;
	cOut = 0;
}

// Обработка принятых данных.
bool ProtocolS::readData() {
	bool error = true;

	if (state == STATE_READ_OK) {
		if (buf[COM] == COM_FROM_BSP) {
			if (buf[NUM] == 5) {
				dOut[D_OUTPUT_16_01] = *((uint16_t *) &buf[BYTE(0)]);
				dOut[D_OUTPUT_32_17] = *((uint16_t *) &buf[BYTE(2)]);
				cOut = buf[BYTE(4)];
				error = false;
			}
		}

		setIdle();
	}

	return !error;
}

//	Подготовка данных для передачи.
bool ProtocolS::sendData() {
	bool error = true;

	if (state == STATE_IDLE) {
		buf[COM] = COM_TO_BSP;
		buf[NUM] = 5;
		*((uint16_t *) &buf[BYTE(0)]) = dIn[D_INPUT_16_01];
		*((uint16_t *) &buf[BYTE(2)]) = dIn[D_INPUT_32_17];
		buf[BYTE(4)] = cIn;

		nTx = buf[NUM];
		error = false;
	}

	return !error;
}

// Проверка наличия принятого пакета.
bool ProtocolS::isReadData() {
	if (state == STATE_READ_OK) {
		if (getCRC() != calcCRC()) {
			setIdle();
		}
	}

	return (state == STATE_READ_OK);
}

// Проверка наличия данных для передачи.
bool ProtocolS::isSendData() {
	if ((state == STATE_IDLE) && (nTx > 0)) {
		if (addADU()) {
			state = STATE_WRITE;
		}
	}

	return (state == STATE_WRITE);
}

// Прием байта данных.
void ProtocolS::push(uint8_t byte, bool err) {
	uint8_t cnt = this->cnt;

	if (state != STATE_READ) {
		return;
	}

//	if (state == STATE_IDLE) {
//		cnt = 0;
//		if ((byte = 0x55) &&  !err) {
//			state = STATE_READ;
//		}
//	}

	if (err) {
		cnt = 0;
		return;
	}

	if (state == STATE_READ) {
		buf[cnt] = byte;

		switch(cnt) {
			case 0: {	// sync character 1
				if (byte == PREAMBLE_CHAR_1) {
					cnt++;
				} else {
					cnt = 0;
				}
			} break;

			case 1: {	// sync character 2
				if (byte == PREAMBLE_CHAR_2) {
					cnt++;
				} else {
					cnt = 0;
				}
			} break;

			default: {	// databytes + crc
				cnt++;
				if (cnt < BUF_SIZE_MAX) {
					if (cnt >= (MIN_SIZE_ADU + buf[NUM])) {
						state = STATE_READ_OK;
					}
				}
			}
		}
	}

	this->cnt = cnt;
}

// Чтение контрольных сигналов для передачи.
uint8_t ProtocolS::getCOut() const {
	return cOut;
}

// Чтение состояний дискретных выходов.
uint16_t ProtocolS::getDOut(dOutput_t dout) const {
	uint16_t val = 0;
	if ((dout >= D_OUTPUT_16_01) && (dout < D_OUTPUT_MAX)) {
		val = dOut[dout];
	}
	return val;
}

// Запись контрольных сигналов на приеме.
void ProtocolS::setCInput(uint8_t val) {
	cIn = val;
}

//	Запись состояний дискретных входов.
void ProtocolS::setDInput(dInput_t din, uint16_t val) {
	if ((din >= D_INPUT_16_01) && (din < D_INPUT_MAX)) {
		dIn[din] = val;
	}
}


// Передача байта данных.
bool ProtocolS::pull(uint8_t &byte) {
	bool tr = false;

	if (state == STATE_WRITE) {
		if (nTx > 0) {
			nTx--;
			byte = buf[cnt++];
			tr = true;
		} else {
			setRead();
		}
	}

	return tr;
}

//	Запуск работы протокола.
void ProtocolS::setEnable() {
	for(uint8_t i = 0; i < D_INPUT_MAX; i++) {
		dIn[i] = 0x00;
	}

	for(uint8_t i = 0; i < D_OUTPUT_MAX; i++) {
		dOut[i] = 0x00;
	}

	cnt = 0;
	nTx= 0;
	state = STATE_IDLE;
}

// Возвращает контрольную сумму ADU.
uint8_t ProtocolS::calcCRC() const {
	uint8_t crc = 0;
	uint8_t len = buf[3] + MIN_SIZE_ADU;

	for(uint8_t i = 2; i < (len - 1); i++) {
		crc += buf[i];
	}

	return crc;
}

///	Возвращает контрольную сумму ADU.
uint8_t ProtocolS::getCRC() const {
	return buf[MIN_SIZE_ADU + buf[NUM] - 1];
}

/// Устанавливает состояние ожидания.
void ProtocolS::setIdle() {
	if (state != STATE_OFF) {
		cnt = 0;
		nTx= 0;
		state = STATE_IDLE;
	}
}

// Установить состояние чтения.
void ProtocolS::setRead() {
	if (state != STATE_OFF) {
		cnt = 0;
		nTx= 0;
		state = STATE_READ;
	}
}

// Добавляет информацию для ADU.
bool ProtocolS::addADU() {
	nTx += MIN_SIZE_ADU;

	if (nTx <= BUF_SIZE_MAX) {
		buf[0] = PREAMBLE_CHAR_1;
		buf[1] = PREAMBLE_CHAR_2;
		buf[nTx - 1] = calcCRC();
	} else {
		nTx = 0;
	}

	return (nTx != 0);
}

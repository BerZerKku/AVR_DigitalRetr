///*
// * protocolS.cpp
// *
// *  Created on: 28.06.2012
// *      Author: Shcheblykin
// */
//#include "protocolS.h"
//
//clProtocolS::clProtocolS(uint8_t *buf, uint8_t size, stGBparam *sParam) :
//		buf(buf), size_(size), sParam_(sParam) {
//	cnt_ = 0;
//	maxLen_ = 0;
//	old_ = PRTS_STATUS_OFF;
//	cntCycle_ = 0;
//	stat_ = PRTS_STATUS_OFF;
//	statDef_ = PRTS_STATUS_OFF;
//}
//
///**	Опрос флага наличия принятой посылки.
// *
// * 	Если посылка принята, но контрольная сумма неверная - флаг сбрасывается.
// *
// * 	@param Нет
// * 	@return Нет
// */
//bool clProtocolS::checkReadData() {
//	bool stat = false;
//
//	// Т.к. обработка посылки уже началась, сбросим счетчик принятых байт
////	cnt_ = 0;
//
//	if (!(stat = checkCRC()))
//		setCurrentStatus(PRTS_STATUS_NO);
//
//	return stat;
//}
//
///** Отправка сообщения лежащего в буфере.
// *
// *  @param Нет
// *  @return Кол-во байт данных приготовленных для передачи
// */
//uint8_t clProtocolS::trCom() {
//	setCurrentStatus(PRTS_STATUS_WRITE);
//
//	return maxLen_;
//}
//
///**	Копирование имеющейся команды в свой буфер.
// *
// * 	Копирование произойдет только если будут найдены синхробайты и размер
// * 	посылки не превышает размер своего буфера.
// *
// * 	@param bufSource Адрес массива с командой
// * 	@return True - в случае удачного копирования, инчае - False
// */
//bool clProtocolS::copyCommandFrom(uint8_t * const bufSource) {
//	bool stat = true;
//	uint8_t cnt = 0;
//
//	if (bufSource[0] != 0x55)
//		stat = false;
//	else if (bufSource[1] != 0xAA)
//		stat = false;
//	else {
//		cnt = bufSource[3] + 5;
//
//		if (cnt > size_)
//			stat = false;
//		else {
//			for (uint_fast8_t i = 0; i < cnt; i++) {
//				if (i < size_)
//					buf[i] = bufSource[i];
//			}
//		}
//	}
//
//	// в случае какой-либо ошибки, сообщенеие игнорируется
//	if (!stat) {
//		setCurrentStatus(PRTS_STATUS_NO);
//	} else {
//		setCurrentStatus(PRTS_STATUS_WRITE_PC);
//		maxLen_ = cnt;
//	}
//
//	return stat;
//}
//
///**	Проверка текущего состояния и сброс его в значение по умолчанию
// * 	при необходимости (когда слишком долго висит одно состояние)
// *
// * 	@param Нет
// * 	@return Нет
// */
//void clProtocolS::checkStat() {
//	if (old_ == stat_) {
//		if (stat_ != statDef_) {
//			cntCycle_++;
//			if (cntCycle_ >= MAX_CYCLE_TO_REST_SOST) {
//				setCurrentStatus(statDef_);
//				old_ = statDef_;
//				cntCycle_ = 0;
//			}
//		}
//	} else {
//		old_ = stat_;
//		cntCycle_ = 0;
//	}
//}
//
///**	Подготовка к отправке команды (сама команда, кол-во данных и данные
// * 	уже должны лежать в буфере)
// *
// * 	@param Нет
// * 	@return Кол-во отправляемых байт данных
// */
//uint8_t clProtocolS::addCom() {
//	uint8_t cnt = 0;
//
//	buf[0] = 0x55;
//	buf[1] = 0xAA;
//	// команда будет отправлена если лежит не нулевая команда и
//	// под заявленное кол-во данных хватает размера буфера
//	if (buf[2] != 0) {
//		uint8_t len = buf[3] + 5;
//		if (len <= (size_ - 5)) {
//			buf[len - 1] = getCRC();
//			cnt = len;
//			maxLen_ = len;
//			setCurrentStatus(PRTS_STATUS_WRITE);
//		}
//	}
//
//	return cnt;
//}
//
///**	Подготовка к отправке команды с данными (заполнение буфера)
// * 	@param com Команда
// * 	@param size Кол-во байт данных
// * 	@param b[] Массив данных
// * 	@return Кол-во отправляемых байт данных
// */
//uint8_t clProtocolS::addCom(uint8_t com, uint8_t size, uint8_t b[]) {
//	uint8_t cnt = 0;
//
//	if (size < (this->size_ - 5)) {
//		buf[cnt++] = 0x55;
//		buf[cnt++] = 0xAA;
//		buf[cnt++] = com;
//		buf[cnt++] = size;
//		// Скопируем данные в буфер передатчика
//		for (uint8_t i = 0; i < size; i++, cnt++)
//			buf[cnt] = b[i];
//		buf[cnt++] = getCRC();
//
//		maxLen_ = cnt;
//		setCurrentStatus(PRTS_STATUS_WRITE);
//	}
//
//	return cnt;
//}
//
///**	Подготовка к отправке команды с 2 байтами данных (заполнение буфера)
// * 	@param com Команда
// * 	@param byte1 Первый байт данных
// * 	@param byte2 Второй байт данных
// * 	@return Кол-во отправляемых байт данных
// */
//uint8_t clProtocolS::addCom(uint8_t com, uint8_t byte1, uint8_t byte2) {
//	uint8_t cnt = 0;
//
//	buf[cnt++] = 0x55;
//	buf[cnt++] = 0xAA;
//	buf[cnt++] = com;
//	buf[cnt++] = 0x02;
//	buf[cnt++] = byte1;
//	buf[cnt++] = byte2;
//	buf[cnt++] = com + 0x02 + byte1 + byte2;
//
//	maxLen_ = cnt;
//	setCurrentStatus(PRTS_STATUS_WRITE);
//
//	return cnt;
//}
//
///**	Подготовка к отправке команды с 1 байтом данных (заполнение буфера)
// * 	@param com Команда
// * 	@param byte Данные
// * 	@return Кол-во отправляемых байт данных
// */
//uint8_t clProtocolS::addCom(uint8_t com, uint8_t byte) {
//	uint8_t cnt = 0;
//
//	buf[cnt++] = 0x55;
//	buf[cnt++] = 0xAA;
//	buf[cnt++] = com;
//	buf[cnt++] = 0x01;
//	buf[cnt++] = byte;
//	buf[cnt++] = com + 0x01 + byte;
//
//	maxLen_ = cnt;
//	setCurrentStatus(PRTS_STATUS_WRITE);
//
//	return cnt;
//}
//
///**	Подготовка к отправке команды без данных (заполнение буфера)
// * 	@param com Передаваемая команда
// * 	@return Кол-во отправляемых байт данных
// */
//uint8_t clProtocolS::addCom(uint8_t com) {
//	uint8_t cnt = 0;
//
//	buf[cnt++] = 0x55;
//	buf[cnt++] = 0xAA;
//	buf[cnt++] = com;
//	buf[cnt++] = 00;
//	buf[cnt++] = com;
//
//	maxLen_ = cnt;
//	setCurrentStatus(PRTS_STATUS_WRITE);
//
//	return cnt;
//}
//
///**	Проверка принятой контрольной суммы
// * 	@param Нет
// * 	@return true - если верная контрольная сумма
// */
//bool clProtocolS::checkCRC() const {
//	bool stat = false;
//	uint8_t crc = 0;
//	uint8_t len = maxLen_ - 1;
//
//	for (uint8_t i = 2; i < len; i++)
//		crc += buf[i];
//
//	if (crc == buf[len])
//		stat = true;
//
//	return stat;
//}
//

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
}

// Обработка принятых данных.
bool ProtocolS::readData() {
	bool error = true;

	if (state == STATE_READ_OK) {
		if ((buf[COM] == 0x12) && (buf[NUM] == 4)) {
			dOut[D_OUTPUT_16_01] = *((uint16_t *) &buf[BYTE(0)]);
			dOut[D_OUTPUT_32_17] = *((uint16_t *) &buf[BYTE(2)]);

			error = false;
		}
		setIdle();
	}

	return !error;
}

//	Подготовка данных для передачи.
bool ProtocolS::sendData() {
	bool error = true;

	if (state == STATE_IDLE) {

		buf[COM] = 0x11;
		buf[NUM] = 4;
		*((uint16_t *) &buf[BYTE(0)]) = dIn[D_INPUT_16_01];
		*((uint16_t *) &buf[BYTE(2)]) = dIn[D_INPUT_32_17];

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

	if ((state != STATE_IDLE) && (state != STATE_READ)) {
		return;
	}

	if (state == STATE_IDLE) {
		cnt = 0;
		if ((byte = 0x55) &&  !err) {
			state = STATE_READ;
		}
	}

	if (state == STATE_READ) {
		buf[cnt] = byte;

		switch(cnt) {
			case 0: {	// sync character 1
				if (byte == 0x55) {
					cnt++;
				} else {
					state = STATE_IDLE;
				}
			} break;

			case 1: {	// sync character 2
				if (byte == 0xAA) {
					cnt++;
				} else {
					state = STATE_IDLE;
				}
			} break;

			case 2: 	// function code
			case 3: {	// quantity of data bytes
				cnt++;
			} break;

			default: {	// databytes + crc
				cnt++;
				if (cnt >= (MIN_SIZE_ADU + buf[NUM])) {
					state = STATE_READ_OK;
				}
			}
		}
	}

	this->cnt = cnt;
}

// Чтение состояний дискретных выходов.
uint16_t ProtocolS::getDOut(dOutput_t dout) const {
	uint16_t val = 0;
	if ((dout >= D_OUTPUT_16_01) && (dout < D_OUTPUT_MAX)) {
		val = dOut[dout];
	}
	return val;
}

//	Запись состояний дискретных входов.
void ProtocolS::setDInput(dInput_t din, uint16_t val) {
	if ((din >= D_INPUT_16_01) && (din < D_INPUT_MAX)) {
		dIn[din] = val;
	}
}


// Передача байта данных.
bool ProtocolS::pull(uint8_t &byte) {

	if (state == STATE_WRITE) {
		if (nTx > 0) {
			nTx--;
			byte = buf[cnt++];
		} else {
			state = STATE_IDLE;
		}
	}

	return (state == STATE_WRITE);
}

//	Запуск работы протокола.
void ProtocolS::setEnable() {
	for(uint8_t i = 0; i < D_INPUT_MAX; i++) {
		dIn[i] = 0x00;
	}

	for(uint8_t i = 0; i < D_OUTPUT_MAX; i++) {
		dOut[i] = 0x00;
	}

	setIdle();
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
	cnt = 0;
	nTx= 0;
	state = STATE_IDLE;
}

// Добавляет информацию для ADU.
bool ProtocolS::addADU() {
	nTx += MIN_SIZE_ADU;

	if (nTx <= BUF_SIZE_MAX) {
		buf[0] = 0x55;
		buf[1] = 0xAA;
		buf[nTx - 1] = calcCRC();
	} else {
		nTx = 0;
	}

	return (nTx != 0);
}

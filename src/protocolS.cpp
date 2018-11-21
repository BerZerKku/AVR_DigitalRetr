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
///**	����� ����� ������� �������� �������.
// *
// * 	���� ������� �������, �� ����������� ����� �������� - ���� ������������.
// *
// * 	@param ���
// * 	@return ���
// */
//bool clProtocolS::checkReadData() {
//	bool stat = false;
//
//	// �.�. ��������� ������� ��� ��������, ������� ������� �������� ����
////	cnt_ = 0;
//
//	if (!(stat = checkCRC()))
//		setCurrentStatus(PRTS_STATUS_NO);
//
//	return stat;
//}
//
///** �������� ��������� �������� � ������.
// *
// *  @param ���
// *  @return ���-�� ���� ������ �������������� ��� ��������
// */
//uint8_t clProtocolS::trCom() {
//	setCurrentStatus(PRTS_STATUS_WRITE);
//
//	return maxLen_;
//}
//
///**	����������� ��������� ������� � ���� �����.
// *
// * 	����������� ���������� ������ ���� ����� ������� ����������� � ������
// * 	������� �� ��������� ������ ������ ������.
// *
// * 	@param bufSource ����� ������� � ��������
// * 	@return True - � ������ �������� �����������, ����� - False
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
//	// � ������ �����-���� ������, ���������� ������������
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
///**	�������� �������� ��������� � ����� ��� � �������� �� ���������
// * 	��� ������������� (����� ������� ����� ����� ���� ���������)
// *
// * 	@param ���
// * 	@return ���
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
///**	���������� � �������� ������� (���� �������, ���-�� ������ � ������
// * 	��� ������ ������ � ������)
// *
// * 	@param ���
// * 	@return ���-�� ������������ ���� ������
// */
//uint8_t clProtocolS::addCom() {
//	uint8_t cnt = 0;
//
//	buf[0] = 0x55;
//	buf[1] = 0xAA;
//	// ������� ����� ���������� ���� ����� �� ������� ������� �
//	// ��� ���������� ���-�� ������ ������� ������� ������
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
///**	���������� � �������� ������� � ������� (���������� ������)
// * 	@param com �������
// * 	@param size ���-�� ���� ������
// * 	@param b[] ������ ������
// * 	@return ���-�� ������������ ���� ������
// */
//uint8_t clProtocolS::addCom(uint8_t com, uint8_t size, uint8_t b[]) {
//	uint8_t cnt = 0;
//
//	if (size < (this->size_ - 5)) {
//		buf[cnt++] = 0x55;
//		buf[cnt++] = 0xAA;
//		buf[cnt++] = com;
//		buf[cnt++] = size;
//		// ��������� ������ � ����� �����������
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
///**	���������� � �������� ������� � 2 ������� ������ (���������� ������)
// * 	@param com �������
// * 	@param byte1 ������ ���� ������
// * 	@param byte2 ������ ���� ������
// * 	@return ���-�� ������������ ���� ������
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
///**	���������� � �������� ������� � 1 ������ ������ (���������� ������)
// * 	@param com �������
// * 	@param byte ������
// * 	@return ���-�� ������������ ���� ������
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
///**	���������� � �������� ������� ��� ������ (���������� ������)
// * 	@param com ������������ �������
// * 	@return ���-�� ������������ ���� ������
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
///**	�������� �������� ����������� �����
// * 	@param ���
// * 	@return true - ���� ������ ����������� �����
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

// �����������.
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

// ��������� �������� ������.
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

//	���������� ������ ��� ��������.
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

// �������� ������� ��������� ������.
bool ProtocolS::isReadData() {
	if (state == STATE_READ_OK) {
		if (getCRC() != calcCRC()) {
			setIdle();
		}
	}

	return (state == STATE_READ_OK);
}

// �������� ������� ������ ��� ��������.
bool ProtocolS::isSendData() {
	if ((state == STATE_IDLE) && (nTx > 0)) {
		if (addADU()) {
			state = STATE_WRITE;
		}
	}

	return (state == STATE_WRITE);
}

// ����� ����� ������.
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

// ������ ��������� ���������� �������.
uint16_t ProtocolS::getDOut(dOutput_t dout) const {
	uint16_t val = 0;
	if ((dout >= D_OUTPUT_16_01) && (dout < D_OUTPUT_MAX)) {
		val = dOut[dout];
	}
	return val;
}

//	������ ��������� ���������� ������.
void ProtocolS::setDInput(dInput_t din, uint16_t val) {
	if ((din >= D_INPUT_16_01) && (din < D_INPUT_MAX)) {
		dIn[din] = val;
	}
}


// �������� ����� ������.
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

//	������ ������ ���������.
void ProtocolS::setEnable() {
	for(uint8_t i = 0; i < D_INPUT_MAX; i++) {
		dIn[i] = 0x00;
	}

	for(uint8_t i = 0; i < D_OUTPUT_MAX; i++) {
		dOut[i] = 0x00;
	}

	setIdle();
}

// ���������� ����������� ����� ADU.
uint8_t ProtocolS::calcCRC() const {
	uint8_t crc = 0;
	uint8_t len = buf[3] + MIN_SIZE_ADU;

	for(uint8_t i = 2; i < (len - 1); i++) {
		crc += buf[i];
	}

	return crc;
}

///	���������� ����������� ����� ADU.
uint8_t ProtocolS::getCRC() const {
	return buf[MIN_SIZE_ADU + buf[NUM] - 1];
}

/// ������������� ��������� ��������.
void ProtocolS::setIdle() {
	cnt = 0;
	nTx= 0;
	state = STATE_IDLE;
}

// ��������� ���������� ��� ADU.
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

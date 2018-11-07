
#include "protocolModbus.h"
#ifdef DEBUG
#include "debug.h"
#endif

const uint8_t TProtocolModbus::CRC_HI[256] PROGMEM = {
		0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
		0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
		0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
		0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
		0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
		0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
		0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
		0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
		0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 };

const uint8_t TProtocolModbus::CRC_LOW[256]  PROGMEM = {
		0x00, 0xC0, 0xC1, 0x01, 0xC3,
		0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC,
		0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9,
		0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E,
		0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7,
		0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0,
		0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5,
		0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA,
		0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB,
		0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4,
		0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1,
		0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66,
		0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF,
		0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78,
		0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D,
		0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72,
		0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93,
		0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C,
		0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99,
		0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E,
		0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87,
		0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40 };

// Конструктор
TProtocolModbus::TProtocolModbus() {
	state_ = STATE_OFF;
	address_ = ADDRESS_ERR;

	cnt_ = 0;
	cntTx_ = 0;

	tick_ = 0;
	tickStep_ = 0;
}

// принятый байт данных
uint8_t TProtocolModbus::push(uint8_t byte) {
	uint8_t cnt = cnt_;

	if (checkState(STATE_READ)) {
		// сброс счетчика принятых байт, если обнаружено начало новой посылки
		if ((cnt != 0) && (tick_ >= DELAY_RESET)) {
			setState(STATE_READ_ERROR);
		} else {
			if (cnt < BUF_SIZE_MAX) {
				buf_[cnt++] = byte;
			}
		}
	}
	tick_ = 0;	// сброс счетчика времени

	return (cnt_ = cnt);
}

// бай на передачу
uint8_t TProtocolModbus::pull() {
	uint8_t byte = 0;

	if (checkState(STATE_WRITE) && (cntTx_ < cnt_)) {
		byte = buf_[cntTx_++];
	}

	return byte;
}

// Настройка счетчика времени
uint16_t TProtocolModbus::setTick(uint32_t baudrate, uint8_t period) {
	uint16_t step = 0;

	if (baudrate > 19200) {
		step = (1UL * DELAY_RESET * period) / 750;
	} else {
		step = (((1UL * baudrate * period) / 1100) * DELAY_RESET) / 15000;
	}

	return (tickStep_ = step);
}

// Счет времени прошедшего с момента прихода последнего байта.
void TProtocolModbus::tick() {
	uint16_t tick = tick_;

	if (checkState(STATE_READ) || checkState(STATE_READ_ERROR)) {
		if (tick < DELAY_READ) {
			tick += tickStep_;
			if (tick >= DELAY_READ) {
				if (checkState(STATE_READ) && (cnt_ >= 4)) {
					setState(STATE_READ_OK);
				} else {
					setState(STATE_READ);
				}
			}
		}
		tick_ = tick;
	}
}

// Возвращает адрес устройства в сети.
uint8_t TProtocolModbus::getAddressLan() const {
	return address_;
}

// Установка адреса устройства в сети.
bool TProtocolModbus::setAddressLan(uint8_t adr) {
	bool state = false;

	if ((adr >= ADDRESS_MIN) && (adr <= ADDRESS_MAX)) {
		address_ = adr;
		state = true;
	}

	return state;
}

// Обработка принятых данных
bool TProtocolModbus::readData() {
	bool state = false;

	if (checkReadPackage() == CHECK_ERR_NO) {
		TProtocolModbus::COM com = static_cast<TProtocolModbus::COM> (buf_[1]);

		switch(com) {
			case COM_01H_READ_COILS:
				state = comReadCoils();
				break;
			case COM_02H_READ_DISCRETE_INPUTS:
				state = comReadDInputs();
				break;
			case COM_03H_READ_HOLDING_REGISTERS:
				state = comReadRegisters();
				break;
			case COM_04H_READ_INPUT_REGISTERS:
				state = comReadIRegisters();
				break;
			case COM_05H_WRITE_SINGLE_COIL :
				state = comWriteCoil();
				break;
			case COM_06H_WRITE_SINGLE_REGISTER:
				state = comWriteRegister();
				break;
			case COM_0FH_WRITE_MULTIPLIE_COILS:
				state = comWriteCoils();
				break;
			case COM_10H_WRITE_MULITPLIE_REGISTERS:
				state = comWriteRegisters();
				break;
			case COM_11H_REPORT_SLAVE_ID:
				state = comReadID();
				break;
			default:
				// остальные команды не поддерживаются
				setException(EXCEPTION_01H_ILLEGAL_FUNCTION);
		}

		addCRC();
		setState(STATE_WRITE);
	}

	return state;
}

/**	Проверка адреса устройства на совпадение с установленным
 *
 *	Дополнительно проводится проверка на ошибочный адрес \a ADDRESS_ERR.
 *
 *	@see ADDRESS_ERR
 * 	@adr Адрес, который будет сравниваться с установленным.
 */
bool TProtocolModbus::checkAddressLan(uint8_t adr) {
	return ((adr != ADDRESS_ERR) && (address_ == adr));
}

/**	Рассчет контрольной суммы для заданного кол-ва байт данных в буфере
 *
 * 	@param num Кол-во байт данных.
 * 	@return Рассчитанная контрольная сумма.
 */
uint16_t TProtocolModbus::calcCRC(uint8_t num) {
	uint8_t low = 0xFF;
	uint8_t hi = 0xFF;
	for (uint8_t i = 0, tmp; i < num; i++) {
		tmp = hi ^ buf_[i];
		hi = low ^ pgm_read_byte(&CRC_HI[tmp]);
		low = pgm_read_byte(&CRC_LOW[tmp]);
	}
	return ((hi << 8) | low);
}

/**	Возвращает контрольную сумму переданную в посылке.
 *
 *	Рассчет CRC осуществляется если в посылке есть как минимум 4 байта (адрес,
 *	код команды, и 2 байта контрольной суммы).
 *
 * 	@return Контрольная сумма посылки.
 */
uint16_t TProtocolModbus::getCRC() const {
	uint16_t crc = 0xFFFF;
	if (cnt_ >= 4) {
		crc = (((uint16_t) buf_[cnt_ - 2] << 8) + buf_[cnt_ - 1]);
	}
	return crc;
}

/**	Добавляет к имеющейся в буфере послылке контрольную сумму.
 *
 * 	Рассчитывается контрольная сумма для имеющегося количества данных в буфере.
 *	При этом увеличиваетс счетчик байт в посылке.
 */
void TProtocolModbus::addCRC() {
	uint16_t crc = calcCRC(cnt_);
	buf_[cnt_++] = crc >> 8;
	buf_[cnt_++] = crc;
}

/**	Проверка полученной посылки на соответствие протоколу.
 *
 * 	Проверяются:
 * 	- адрес устройства;
 * 	- контрольная сумма;
 *
 * 	@retval CHECK_ERR_NO Ошибок в принятой посылке нет.
 * 	@retval CHECK_ERR_ADR_LAN Неверный адрес устройства.
 * 	@retval CHECK_ERR_CRC Неверная контрольная сумма.
 */
TProtocolModbus::CHECK_ERR TProtocolModbus::checkReadPackage() {
	CHECK_ERR state = CHECK_ERR_NO;

	if (!checkAddressLan(buf_[0])) {
		// Не совпал адрес устройства.
		setState(STATE_READ);
		state = CHECK_ERR_ADR_LAN;
	} else if (getCRC() != calcCRC(cnt_ - 2)) {
		// не совпала контрольная сумма
		setState(STATE_READ);
		state = CHECK_ERR_CRC;
	}

	return state;
}

/**	Ответ на запрос с кодом исключения.
 *
 *	К коду принятой команды добавляется 0х80.
 *	Добавляется один байт данных - код исключения.
 *	Подсчитывается контрольная сумма.
 *	Флаг состояния протокола устанавливаетс \a STATE_WRITE, т.е. готовность
 *	к отправке данных.
 *
 *	@param code Код исключения.
 */
void TProtocolModbus::setException(TProtocolModbus::EXCEPTION code) {
	cnt_ = 1;	// адрес остается прежним
	buf_[cnt_++] |= 0x80;
	buf_[cnt_++]  = code;
}

// 	Смена состояния работы протокола.
void TProtocolModbus::setState(TProtocolModbus::STATE state) {
	if ((state >= STATE_OFF) && (state < STATE_ERROR)) {
		if (state == STATE_READ) {
			// при смене режима работы на "Чтение", обнулим счетчик принятых
			// байт
			cnt_ = 0;
			tick_ = 0;
		} else if (state == STATE_WRITE) {
			cntTx_ = 0;
		}
		state_ = state;
	}
}

/** Обработка команды чтения флагов 0x01.
 *
 * 	В случае обнаружения ошибок в принятом пакете, будет сформировано
 *	исключение.
 *
 *	Если ошибок в принятой посылке нет, будет сформирован ответ без КС.
 *
 *	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comReadCoils() {
	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t num = getNumOfAddress();	// кол-во адресов

	// проверка количества регистров
	if ((num == 0) || (num > MAX_NUM_COILS)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// подготовка ответа
	uint8_t cnt = 2;						// адрес + команда
	buf_[cnt++] = (num + 7) / 8;			// кол-во передаваемых байт данных
	for(uint8_t i = 0; i < buf_[2]; i++) {
		buf_[cnt++] = 0;					// обнуление передаваемых данных
	}
	cnt_ = cnt;

	// заполнение ответа теущими сотстояниями флагов
	// если встретится хотя бы один недопустимый адрес, будет сформировано
	// исключение
	for(uint16_t i = 0; i < num; i++, adr++) {
		bool val = false;

		TProtocolModbus::CHECK_ERR err = readCoil(adr, val);

		if (err == CHECK_ERR_ADR) {
			setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
			return false;
		}

		// При необходимости устанавливаем текущий флаг. Обнулять не надо
		// т.к. в ответ изначально записаны 0х00.
		if (val) {
			buf_[3 + i/8] |= (1 << i%8);
		}
	}

	return true;
}


/** Обработка команды чтения флагов 0x01.
 *
 * 	В случае обнаружения ошибок в принятом пакете, будет сформировано
 *	исключение.
 *
 *	Если ошибок в принятой посылке нет, будет сформирован ответ без КС.
 *
 *	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comReadDInputs() {
	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t num = getNumOfAddress();	// кол-во адресов

	// проверка количества регистров
	if ((num == 0) || (num > MAX_NUM_COILS)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// подготовка ответа
	uint8_t cnt = 2;						// адрес + команда
	buf_[cnt++] = (num + 7) / 8;			// кол-во передаваемых байт данных
	for(uint8_t i = 0; i < buf_[2]; i++) {
		buf_[cnt++] = 0;					// обнуление передаваемых данных
	}
	cnt_ = cnt;

	// заполнение ответа теущими сотстояниями флагов
	// если встретится хотя бы один недопустимый адрес, будет сформировано
	// исключение
	for(uint16_t i = 0; i < num; i++, adr++) {
		bool val = false;

		TProtocolModbus::CHECK_ERR err = readDInput(adr, val);

		if (err == CHECK_ERR_ADR) {
			setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
			return false;
		}

		// При необходимости устанавливаем текущий флаг. Обнулять не надо
		// т.к. в ответ изначально записаны 0х00.
		if (val) {
			buf_[3 + i/8] |= (1 << i%8);
		}
	}

	return true;
}


/** Обработка команды чтения внутренних регистров 0x03.
 *
 * 	В случае обнаружения ошибок в принятом пакете, будет сформировано
 *	исключение.
 *
 *	Если ошибок в принятой посылке нет, будет сформирован ответ без КС.
 *
 *	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comReadRegisters() {
	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t num = getNumOfAddress();	// кол-во адресов

	// проверка количества регистров
	if ((num == 0) || (num > MAX_NUM_REGISTERS)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// подготовка ответа, изначально все данные заполнены нулями
	uint8_t cnt = 2; 						// адрес + команда
	buf_[cnt++] = num * 2; 			// кол-во передаваемых байт данных
	for(uint8_t i = 0; i < buf_[2]; i++) {
		buf_[cnt++] = 0x00;					// обнуление данных
	}
	cnt_ = cnt;

	// заполнение ответа теущими значениями регистров
	// если встретится хотя бы один недопустимый адрес, будет сформировано
	// исключение
	for(uint16_t i = 0; i < num; i++, adr++) {
		uint16_t val = 0xFFFF;

		TProtocolModbus::CHECK_ERR err = readRegister(adr, val);

		if (err == CHECK_ERR_ADR) {
			setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
			return false;
		}

		buf_[i*2 + 3] = val >> 8;
		buf_[i*2 + 4] = val;

	}

	return true;
}


/** Обработка команды чтения входных регистров 0x04.
 *
 * 	В случае обнаружения ошибок в принятом пакете, будет сформировано
 *	исключение.
 *
 *	Если ошибок в принятой посылке нет, будет сформирован ответ без КС.
 *
 *	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comReadIRegisters() {
	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t num = getNumOfAddress();	// кол-во адресов

	// проверка количества регистров
	if ((num == 0) || (num > MAX_NUM_REGISTERS)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// подготовка ответа, изначально все данные заполнены нулями
	uint8_t cnt = 2; 						// адрес + команда
	buf_[cnt++] = num * 2; 			// кол-во передаваемых байт данных
	for(uint8_t i = 0; i < buf_[2]; i++) {
		buf_[cnt++] = 0x00;					// обнуление данных
	}
	cnt_ = cnt;

	// заполнение ответа теущими значениями регистров
	// если встретится хотя бы один недопустимый адрес, будет сформировано
	// исключение
	for(uint16_t i = 0; i < num; i++, adr++) {
		uint16_t val = 0xFFFF;

		TProtocolModbus::CHECK_ERR err = readIRegister(adr, val);

		if (err == CHECK_ERR_ADR) {
			setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
			return false;
		}

		buf_[i*2 + 3] = val >> 8;
		buf_[i*2 + 4] = val;

	}

	return true;
}


/**	Обработка команды записи флага 0x05.
 *
 * 	В случае обнаружения ошибок в принятой посылке будет сформировано
 * 	исключение. Если ошибок нет, будет записан треуемый флаг и сформирован
 * 	ответ без КС.
 *
 * 	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comWriteCoil() {

	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t val = getNumOfAddress();	// в данном случае это значение флага

	if ((val != 0x0000) && (val != 0xFF00)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	TProtocolModbus::CHECK_ERR err = writeCoil(adr, (val == 0xFF00));

	if (err != CHECK_ERR_NO) {
		if (err == CHECK_ERR_ADR) {
			setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
			return false;
		} else if (err == CHECK_ERR_DATA) {
			setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
			return false;
		} else if (err == CHECK_ERR_DEVICE) {
			setException(EXCEPTION_04H_DEVICE_FAILURE);
			return false;
		}
	}

	cnt_ = 6; // ответ совпадает с запросом

	return true;
}


/**	Обработка команды записи регистра 0x06.
 *
 * 	В случае обнаружения ошибок в принятой посылке будет сформировано
 * 	исключение. Если ошибок нет, будет записан треуемый регистр и сформирован
 * 	ответ без КС.
 *
 * 	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comWriteRegister() {

	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t val = getNumOfAddress();	// в данном случае это значение регистра

	// по сути лишняя проверка, т.к. используется весь диапазон значений
	if ((val < 0x0000) || (val > 0xFFFF)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	TProtocolModbus::CHECK_ERR err = writeRegister(adr, val);

	if (err != CHECK_ERR_NO) {
		if (err == CHECK_ERR_ADR) {
			setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
			return false;
		} else if (err == CHECK_ERR_DATA) {
			setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
			return false;
		} else if (err == CHECK_ERR_DEVICE) {
			setException(EXCEPTION_04H_DEVICE_FAILURE);
			return false;
		}
	}

	cnt_ = 6; // ответ совпадает с запросом

	return true;
}


/**	Обработка команды записи группы флагов 0x0F.
 *
 * 	В случае обнаружения ошибок в принятой посылке будет сформировано
 * 	исключение. Если ошибок нет, будет записаны треуемые флаги и сформирован
 * 	ответ без КС.
 *
 * 	Запись будет производится до первой встреченной ошибки адреса. Т.е.
 * 	если доступно для записи 4 флага, а требуется записать 8. Будут записаны
 * 	4 и затем сформировано исключение. При ошибках устройства, сначала будут
 * 	записаны все адреса и только потом сформировано исключение.
 *
 * 	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comWriteCoils() {
	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t num = getNumOfAddress();	// кол-во флагов

	// ограничение на минимум/максимум устанавливаемых флагов
	if ((num == 0) || (num > MAX_NUM_COILS)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// сравнение кол-ва флагов с заявленным кол-вом байт данных в посылке
	if (((num + 7) / 8) != buf_[6]) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// заполнение ответа теущими сотстояниями флагов
	// если встретится хотя бы один недопустимый адрес, будет сформировано
	// исключение
	TProtocolModbus::CHECK_ERR err_t = CHECK_ERR_NO;
	for(uint16_t i = 0; i < num; i++, adr++) {

		// состояние текущего флага для записи
		bool val = ((buf_[7 + i/8] & (1 << i%8)) != 0);

		// FIXME Работа записи группы флагов зависит от записи одного флага.
		TProtocolModbus::CHECK_ERR err = writeCoil(adr, val);

		if (err != CHECK_ERR_NO) {
			// если в процессе записи обнаружена ошибка адреса, сформируем
			// исключение
			if (err == CHECK_ERR_ADR) {
				setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
				return false;
			} else if (err == CHECK_ERR_DEVICE) {
				err_t = CHECK_ERR_DEVICE;
			}
		}
	}

	// если в процессе записи возникли ошибки устройства, сформируем исключение
	if (err_t == CHECK_ERR_DEVICE) {
		setException(EXCEPTION_04H_DEVICE_FAILURE);
		return false;
	}

	cnt_ = 6; // возвращаем первые 6 байт посылки

	return true;
}

/**	Обработка команды записи группы регистров 0x10.
 *
 * 	В случае обнаружения ошибок в принятой посылке будет сформировано
 * 	исключение. Если ошибок нет, будут записаны требуемые регистры и сформирован
 * 	ответ без КС.
 *
 * 	Запись будет производится до первой встреченной ошибки адреса. Т.е.
 * 	если доступно для записи 4 регистра, а требуется записать 8. Будут записаны
 * 	4 и затем сформировано исключение. При ошибках устройства, сначала будут
 * 	записаны все адреса и только потом сформировано исключение.
 *
 * 	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comWriteRegisters() {
	uint16_t adr = getStartAddress();	// начальный адрес
	uint16_t num = getNumOfAddress();	// кол-во регистров

	// ограничение на минимум/максимум устанавливаемых флагов
	if ((num == 0) || (num > MAX_NUM_REGISTERS)) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// сравнение кол-ва флагов с заявленным кол-вом байт данных в посылке
	if (2*num != buf_[6]) {
		setException(EXCEPTION_03H_ILLEGAL_DATA_VAL);
		return false;
	}

	// заполнение ответа теущими сотстояниями флагов
	// если встретится хотя бы один недопустимый адрес, будет сформировано
	// исключение
	TProtocolModbus::CHECK_ERR err_t = CHECK_ERR_NO;
	for (uint16_t i = 0; i < num; i++, adr++) {

		// значение текущего регистра
		uint16_t val = (((uint16_t) buf_[7 + i*2]) << 8) + buf_[8 + i*2];

		// FIXME Работа записи группы регистров зависит от записи одного регистра.
		TProtocolModbus::CHECK_ERR err = writeRegister(adr, val);

		if (err != CHECK_ERR_NO) {
			// если в процессе записи обнаружена ошибка адреса, сформируем
			// исключение
			if (err == CHECK_ERR_ADR) {
				setException(EXCEPTION_02H_ILLEGAL_DATA_ADR);
				return false;
			} else if (err == CHECK_ERR_DEVICE) {
				err_t = CHECK_ERR_DEVICE;
			}
		}
	}

	// если в процессе записи возникли ошибки устройства, сформируем исключение
	if (err_t == CHECK_ERR_DEVICE) {
		setException(EXCEPTION_04H_DEVICE_FAILURE);
		return false;
	}

	cnt_ = 6; // возвращаем первые 6 байт посылки

	return true;
}


/** Обработка команды чтения инфорации об устройстве 0x11.
 *
 * 	В случае обнаружения ошибок в принятой посылке, будет сформировано
 *	исключение. Если ошибок  нет, будет сформирован ответ без КС.
 *
 *	@retval True - если посылка обработана без ошибок.
 *	@retval False - если при обработке посылки возникли ошбки.
 */
bool TProtocolModbus::comReadID() {

	uint8_t num = MAX_ID_CHARS; // максимальное кол-во байт данных на передачу

	TProtocolModbus::CHECK_ERR err = readID((char *) &buf_[3], num);

	if (err == CHECK_ERR_DATA) {
		setException(EXCEPTION_04H_DEVICE_FAILURE);
		return false;
	}

	buf_[2] = num;
	cnt_ = 3 + num; // 3 = адрес + команда + кол-во байт данных

	return true;
}


// Чтение флагов.
TProtocolModbus::CHECK_ERR TProtocolModbus::readCoil(uint16_t adr, bool &val) {
	// диапазон допустимых адресов
	if ((adr == 0) || (adr > 300))
		return CHECK_ERR_ADR;

	// считывание текущего состояния флага
	if (adr <= 100) {
		val = true;
	} else if (adr <= 200) {
		val = false;
	} else if (adr <= 300) {
		val = true;
	}

	return CHECK_ERR_NO;
}


// Чтение дискретных входов.
TProtocolModbus::CHECK_ERR TProtocolModbus::readDInput(uint16_t adr, bool &val) {

	// диапазон допустимых адресов
	if ((adr == 0) || (adr > 300))
		return CHECK_ERR_ADR;

	// считывание текущего состояния флага
	if (adr <= 100) {
		val = true;
	} else if (adr <= 200) {
		val = false;
	} else if (adr <= 300) {
		val = true;
	}

	return CHECK_ERR_NO;
}



// Чтение внутренних регистров.
TProtocolModbus::CHECK_ERR TProtocolModbus::readRegister(uint16_t adr, uint16_t &val) {

	if ((adr == 0) || (adr > 300))
		return CHECK_ERR_ADR;

	if (adr <= 100) {
		val = adr;
	} else if (adr <= 200) {
		val = 0xFFFF;
	} else if (adr <= 300) {
		val = adr;
	}

	return CHECK_ERR_NO;
}


// Чтение входных регистров.
TProtocolModbus::CHECK_ERR TProtocolModbus::readIRegister(uint16_t adr, uint16_t &val) {

	if ((adr == 0) || (adr > 300))
		return CHECK_ERR_ADR;

	if (adr <= 100) {
		val = adr;
	} else if (adr <= 200) {
		val = 0xFFFF;
	} else if (adr <= 300) {
		val = adr;
	}

	return CHECK_ERR_NO;
}


// Запись флагов.
TProtocolModbus::CHECK_ERR TProtocolModbus::writeCoil(uint16_t adr, bool val) {

	if ((adr == 0) || (adr > 300))
		return CHECK_ERR_ADR;

	if (adr <= 100) {
		// можно записывать любое значение
	} else if (adr <= 200) {
		if (!val)
			return CHECK_ERR_DEVICE;
	} else if (adr <= 300) {
		if (val)
			return CHECK_ERR_DEVICE;
	}

	return CHECK_ERR_NO;
}


// Запись регистров.
TProtocolModbus::CHECK_ERR TProtocolModbus::writeRegister(uint16_t adr, uint16_t val) {

	if ((adr == 0) || (adr > 300))
		return CHECK_ERR_ADR;

	if (adr <= 100) {
		// можно записывать любое значение
	} else if (adr <= 200) {
		if (val > 100)
			return CHECK_ERR_DEVICE;
	} else if (adr <= 300) {
		if (val < 1000)
			return CHECK_ERR_DEVICE;
	}

	return CHECK_ERR_NO;
}


//	Чтение ID.
TProtocolModbus::CHECK_ERR TProtocolModbus::readID(char *buf, uint8_t &size) {

	static const char ID[] PROGMEM = "Virtual";

	uint8_t cnt = 0;
	for(; cnt < (sizeof(ID) / sizeof(ID[0])); cnt++) {
		if (cnt >= size)
			return CHECK_ERR_DEVICE;	// невозможно передать всю информацию
		buf[cnt] = pgm_read_byte(&ID[cnt]);
	}

	// последний байт данных - индикатор состояния.
	buf[cnt++] = 0x00;

	// количество передаваемых данных
	size = cnt;

	return CHECK_ERR_NO;
}


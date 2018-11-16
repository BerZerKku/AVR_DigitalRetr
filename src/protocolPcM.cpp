/*
 * protocolBspS.cpp
 *
 *  Created on: 15.07.2013
 *      Author: Shcheblykin
 */
#include "protocolPcM.h"

// Сброс состояний дискретных входов и выходов.
void TProtocolPcM::reset() {
	for(uint8_t i = 0; i < D_INPUT_MAX; i++) {
		dIn[i] = 0x0000;
	}

	for(uint8_t i = 0; i < D_OUTPUT_MAX; i++) {
		dOut[i] = 0x0000;
	}
};

// Чтение состояний дискретных входов.
uint16_t TProtocolPcM::getDI(dInput_t din) const {
	uint16_t val = 0;
	if ((din >= D_INPUT_16_01) && (din < D_INPUT_MAX)) {
		val = dIn[din];
	}
	return val;
}

// Запись состояний дискретных выходов.
void TProtocolPcM::setDO(dOutput_t dout, uint16_t val) {
	if ((dout >= D_OUTPUT_16_01) && (dout < D_OUTPUT_MAX)) {
		dOut[dout] = val;
	}
}

/**	Чтение внутренних регистров.
 *
 *	Проводится проверка корректности адреса. Если он находится в допустимом
 *	диапазоне, возвращается текущее значение регистра.
 *
 *	Доступные адреса для считывания:
 *	- ADR_HOLDING_REG_DI_16_01_ON	Состояние дискретных входов с 16 по 1.
 *	- ADR_HOLDING_REG_DI_32_17_ON	Состояние дискретных входов с 32 по 17.
 *	- ADR_HOLDING_REG_DI_16_01_OFF	Состояние дискретных входов с 16 по 1.
 *	- ADR_HOLDING_REG_DI_32_17_OFF	Состояние дискретных входов с 32 по 17.
 *	Т.е. состояние одних и тех же дискретных входов можно считать с разных
 *	адресов.
 *
 *	C неиспользуемых адресов будет считано 0x0000.
 *
 *	@param adr Адрес регистра.
 *	@param val [out] Значение регистра.
 *	@retval CHECK_ERR_NO Ошибок при считывании регистра не возникло.
 *	@retval CHECK_ERR_ADR Недопустимый адрес регистра.
 */
TProtocolModbus::CHECK_ERR TProtocolPcM::readRegister(uint16_t adr, uint16_t &val) {
	//  проверка адресов
	if ((adr <= ADR_HOLDING_REG_MIN) || (adr >= ADR_HOLDING_REG_MAX))
		return CHECK_ERR_ADR;

	val = 0x0000;
	switch((adrHoldingReg_t) adr) {
		case ADR_HOLDING_REG_DI_16_01_ON:	// DOWN
		case ADR_HOLDING_REG_DI_16_01_OFF:
			val = dIn[D_INPUT_16_01];
			break;

		case ADR_HOLDING_REG_DI_32_17_ON:	// DOWN
		case ADR_HOLDING_REG_DI_32_17_OFF:
			val = dIn[D_INPUT_32_17];
			break;

		case ADR_HOLDING_REG_MIN:	// DOWN
		case ADR_HOLDING_REG_MAX:
			break;
	}

	return CHECK_ERR_NO;
}

/**	Чтение внешних регистров.
 *
 *	Проводится проверка корректности адреса. Если он находится в допустимом
 *	диапазоне, возвращается текущее значение регистра.
 *
 *	Доступные адреса для считывания:
 *	- ADR_INPUT_REG_DI_16_01 Состояние дискретных выходов с 16 по 1.
 *	- ADR_INPUT_REG_DI_32_17 Состояние дискретных выходов с 32 по 17.
 *
 *	C неиспользуемых адресов будет считано 0x0000.
 *
 *	@param adr Адрес регистра.
 *	@param val [out] Значение регистра.
 *	@retval CHECK_ERR_NO Ошибок при считывании регистра не возникло.
 *	@retval CHECK_ERR_ADR Недопустимый адрес регистра.
 */
TProtocolModbus::CHECK_ERR TProtocolPcM::readIRegister(uint16_t adr, uint16_t &val) {
	//  проверка адресов
	if ((adr <= ADR_INPUT_REG_MIN) || (adr >= ADR_INPUT_REG_MAX))
		return CHECK_ERR_ADR;

	// по умолчанию будет возвращено 0x0000
	val = 0x0000;
	switch((adrInputReg_t) adr) {
		case ADR_INPUT_REG_DO_16_01:
			val = dOut[D_OUTPUT_16_01];
			break;

		case ADR_INPUT_REG_DO_32_17:
			val = dOut[D_OUTPUT_32_17];
			break;

		case ADR_INPUT_REG_MIN:	// DOWN
		case ADR_INPUT_REG_MAX:
			break;
	}

	return CHECK_ERR_NO;
}

/**	Запись внутренних регистров.
 *
 * 	Проводится проверка корректности адреса. Выход за максимальный или
 *	минимальный адреса считается ошибкой.
 *
 *	Доступные адреса для записи:
 *	- ADR_COM_16_01 Команды с 16 по 1.
 *	- ADR_COM_32_17 Команды с 32 по 17.
 *
 *	@param adr Адрес регистра.
 *	@param val Состояние регистра.
 * 	@retval CHECK_ERR_NO Ошибок при считывании регистра не возникло.
 * 	@retval CHECK_ERR_ADR Недопустимый адрес регистра.
 *	@retval CHECK_ERR_DEVICE Возникла внутренняя ошибка.
 */
TProtocolModbus::CHECK_ERR TProtocolPcM::writeRegister(uint16_t adr, uint16_t val) {

	if ((adr <= ADR_HOLDING_REG_MIN) || (adr >= ADR_HOLDING_REG_MAX))
		return CHECK_ERR_ADR;

	switch((adrHoldingReg_t) adr) {
		case ADR_HOLDING_REG_DI_16_01_ON:
			dIn[D_INPUT_16_01] |= val;
			break;

		case ADR_HOLDING_REG_DI_32_17_ON:
			dIn[D_INPUT_32_17] |= val;
			break;

		case ADR_HOLDING_REG_DI_16_01_OFF:
			dIn[D_INPUT_16_01] &= ~val;
			break;

		case ADR_HOLDING_REG_DI_32_17_OFF:
			dIn[D_INPUT_32_17] &= ~val;
			break;

		case ADR_HOLDING_REG_MIN:	// DOWN
		case ADR_HOLDING_REG_MAX:
			break;
	}

	return CHECK_ERR_NO;
}

/**	Чтение ID.
 *
 * 	В посылку для ответа помещаются необходимые данные. Последним байтом
 * 	помещается индикатор состояния: ON - 0xFF, OFF- 0x00.
 *
 * 	Будет передано назавание изделия и последним байтом ON если ЦПП в работе,
 * 	OFF - если нет.
 *
 * 	Пример названия изделия: "ABAHT".
 *
 *	@param buf Указатель на буфер, для записи данных.
 *	@param size [out] Передается максимальный размер сообщения, не включая
 *	байт для индикатора состояния. Возвращается кол-во байт данных
 *	записанных в буфер.
 * 	@retval CHECK_ERR_NO Ошибок при считывании  не возникло.
 *	@retval CHECK_ERR_DEVICE Возникла внутренняя ошибка.
 */
TProtocolModbus::CHECK_ERR TProtocolPcM::readID(char *buf, uint8_t &size) {

	static const char ID[] PROGMEM = "ABAHT";

	uint8_t cnt = 0;
	for(; cnt < (sizeof(ID) / sizeof(ID[0])); cnt++) {
		if (cnt >= size)
			return CHECK_ERR_DEVICE;	// невозможно передать всю информацию
		buf[cnt] = pgm_read_byte(&ID[cnt]);
	}

	// FIXME - добавить текущее состояние протокола
		// По сути ерунда. Если протокол не в работе, то до этого этапа не дойдет.
	buf[cnt++] = isEnable() ? 0x00 : 0xFF;

	// количество передаваемых данных
	size = cnt;

	return CHECK_ERR_NO;
}

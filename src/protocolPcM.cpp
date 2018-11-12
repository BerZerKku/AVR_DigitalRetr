/*
 * protocolBspS.cpp
 *
 *  Created on: 15.07.2013
 *      Author: Shcheblykin
 */
#include "protocolPcM.h"

/**	Чтение внутренних регистров.
 *
 *	Проводится проверка корректности адреса. Если он находится в допустимом
 *	диапазоне, возвращается текущее значение регистра.
 *
 *	C неиспользуемых адресов будет считано 0xFFFF.
 *
 *	@param adr Адрес регистра.
 *	@param val [out] Значение регистра.
 *	@retval CHECK_ERR_NO Ошибок при считывании регистра не возникло.
 *	@retval CHECK_ERR_ADR Недопустимый адрес регистра.
 */
TProtocolModbus::CHECK_ERR TProtocolPcM::readRegister(uint16_t adr, uint16_t &val) {
	//  проверка адресов
	if ((adr <= ADR_REG_MIN) || (adr >= ADR_REG_MAX))
		return CHECK_ERR_ADR;

	// по умолчанию будет возвращено 0xFFFF
	val = 0xFFFF;
	if (adr == ADR_COM_16_01) {
		val = comTx[ADR_COM_16_01];
	}  else if (adr == ADR_COM_32_17) {
		val = comTx[ADR_COM_32_17];
	}

	return CHECK_ERR_NO;
}

/**	Чтение внешних регистров.
 *
 *	Проводится проверка корректности адреса. Если он находится в допустимом
 *	диапазоне, возвращается текущее значение регистра.
 *
 *	C неиспользуемых адресов будет считано 0xFFFF.
 *
 *	@param adr Адрес регистра.
 *	@param val [out] Значение регистра.
 *	@retval CHECK_ERR_NO Ошибок при считывании регистра не возникло.
 *	@retval CHECK_ERR_ADR Недопустимый адрес регистра.
 */
TProtocolModbus::CHECK_ERR TProtocolPcM::readIRegister(uint16_t adr, uint16_t &val) {
	//  проверка адресов
	if ((adr <= ADR_REG_MIN) || (adr >= ADR_REG_MAX))
		return CHECK_ERR_ADR;

	// по умолчанию будет возвращено 0xFFFF
	val = 0xFFFF;
	if (adr == ADR_COM_16_01) {
//		val = comRx[ADR_COM_16_01]; FIXME
		val = comTx[ADR_COM_16_01];
	}  else if (adr == ADR_COM_32_17) {
//		val = comRx[ADR_COM_32_17]; FIXME
		val = comTx[ADR_COM_32_17];
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

	if ((adr <= ADR_REG_MIN	) || (adr >= ADR_REG_MAX))
		return CHECK_ERR_ADR;

	if (adr == ADR_COM_16_01) {
		comTx[ADR_COM_16_01] = val;
	} else if (adr == ADR_COM_32_17) {
		comTx[ADR_COM_32_17] = val;
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

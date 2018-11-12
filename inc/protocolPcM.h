/*
 * protocolBspM.h
 *
 *  Created on: 05.09.2014
 *      Author: Shcheblykin
 */

#ifndef PROTOCOLPCM_H_
#define PROTOCOLPCM_H_

#include "protocolModbus.h"

class TProtocolPcM : public TProtocolModbus
{
	/// Адреса регистров и флагов устройства
	enum ADR_REG{
		ADR_REG_MIN			= 0,
		// Адреса состояния команд.
		ADR_COM_16_01		= 1,
		ADR_COM_32_17,
		// Максимальное кол-во используемых адресов регистров
		ADR_REG_MAX
	};

public:
	/**	Конструктор.
	 */
	TProtocolPcM() {};

	/**	Деструктор.
	 *
	 */
//	~TProtocolPcM() {};

private:
	/// Внутренние регистры.
	uint16_t comTx[ADR_REG_MAX];

	/// Входные регистры.
	uint16_t comRx[ADR_REG_MAX];

	// Чтение внутренних регистров.
	TProtocolModbus::CHECK_ERR readRegister(uint16_t adr, uint16_t &val);

	// Чтение входных регистров.
	TProtocolModbus::CHECK_ERR readIRegister(uint16_t adr, uint16_t &val);

	// Запись регистров
	TProtocolModbus::CHECK_ERR writeRegister(uint16_t adr, uint16_t val);

	// Обработка принятой команды чтения ID.
	TProtocolModbus::CHECK_ERR readID(char *buf, uint8_t &size);
};


#endif /* PROTOCOLPM_H_ */

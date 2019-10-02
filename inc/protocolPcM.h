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
	typedef enum {
		ADR_HOLDING_REG_MIN	= 0,		///< Минимальный номер регистра.
		//
		ADR_HOLDING_REG_DI_16_01_ON, 	///< Регистр установки дискретного входа c 16 по 1.
		ADR_HOLDING_REG_DI_32_17_ON,	///< Регистр установки дискретного входа c 32 по 17.
		ADR_HOLDING_REG_DI_16_01_OFF,	///< Регистр сброса дискретного входа с 16 по 1.
		ADR_HOLDING_REG_DI_32_17_OFF,	///< Регистр сброса дискретного входа с 32 по 17.
		ADR_HOLDING_REG_CI_ON,			///< Регистр установки контрольных сигналов на приеме.
		ADR_HOLDING_REG_CI_OFF,			///< Регистр сброса контрольных сигналов на приеме.
		//
		ADR_HOLDING_REG_MAX				///< Максимальный номер регистра.
	} adrHoldingReg_t;

	typedef enum {
		ADR_INPUT_REG_MIN = 0,	///< Минимальный номер регистра.
		//
		ADR_INPUT_REG_DO_16_01,	///< Регистр считывания дискретного выхода с 16 по 1.
		ADR_INPUT_REG_DO_32_17,	///< Регистр считывания дискретного выхода с 32 по 17.
		ADR_INPUT_REG_CO,		///< Регистр считывания контрольных сигналов для передачи.
		//
		ADR_INPUT_REG_MAX		///< Максимальный номер регистра.
	} adrInputReg_t;

public:

	/// Дискретные входы (клеммы передатчика).
	typedef enum D_INPUT {
		D_INPUT_16_01 = 0,	///< Дискретные входы с 16 по 1 (1 - установлен).
		D_INPUT_32_17,		///< Дискретные входы с 32 по 17 (1 - установлен).
		//
		D_INPUT_MAX			///< Количество элементов в списке.
	} dInput_t;

	/// Дискретные выходы (клеммы приемника).
	typedef enum {
		D_OUTPUT_16_01 = 0,	///< Дискретные выходы с 16 по 1 (1 - установлен).
		D_OUTPUT_32_17,		///< Дискретные выходы с 32 по 17 (1 - установлен).
		//
		D_OUTPUT_MAX		///< Количество элементов в списке.
	} dOutput_t;

	//	Конструктор.
	TProtocolPcM() {
		reset();
	}

	//	Деструктор.
//	virtual ~TProtocolPcM() {};


	/** Сброс состояний дискретных входов и выходов.
	 *
	 *	Дискретные входы и выходы сбрасываются в 0.
	 */
	void reset();

	/**	Чтение состояний контрльных сигналов на приеме.
	 *
	 * 	@return Текущее состояние контрольных сигналов на приеме.
	 */
	uint16_t getCI() const;

	/**	Чтение состояний дискретных входов.
	 *
	 * 	В случае ошибочного номера дискретных входов будет возвращен 0.
	 *
	 * 	@param[in] din Дискрентные входы.
	 * 	@return Текущее состояние указанных дискретных входов.
	 *
	 */
	uint16_t getDI(dInput_t din) const;

	/**	Запись состояний контрольных сигналов для передачи.
	 *
	 * 	@param[in] val Состояние.
	 */
	void setCO(uint16_t val);

	/**	Запись состояний дискретных выходов.
	 *
	 *	@param[in] dout Дискретные выходы.
	 *	@param[in] val Состояние.
	 */
	void setDO(dOutput_t dout, uint16_t val);

private:
	/// Контрольные сигналы на приеме (принять по Modbus).
	uint16_t cIn;

	/// Контрольные сигналы для передачи (передать по Modbus).
	uint16_t cOut;

	/// Дисктреные входы (клеммники передатчика).
	uint16_t dIn[D_INPUT_MAX];

	/// Дискретные выходы (клеммники приемника).
	uint16_t dOut[D_OUTPUT_MAX];

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

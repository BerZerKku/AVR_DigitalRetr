/*
 * protocolS.h
 *
 *  Created on: 28.06.2012
 *      Author: Shcheblykin
 */

#ifndef __PROTOCOL_S_H_
#define __PROTOCOL_S_H_

#include <stdint.h>

#ifdef AVR
	#include <avr/io.h>
	#include <avr/pgmspace.h>
#else
	#define PROGMEM
	#define pgm_read_byte(a) (*a)
#endif

/// Позиция команды в посылке протокола.
#define COM (2)

/// Позиция количества байт данных в посылке протокола.
#define NUM (3)

/// Позиция указанного байта данных в посылке протокола начиная с 0.
#define BYTE(n) (n + 4)

#ifndef TEST_FRIENDS
/*
  Дефайн необходимый для тестирования private и protected методов.
  Определяется в файле с тестом.
  Например:
  #define TEST_FRIENDS \
    friend class IEC61850goose_Test_decodeLength_Test; \
    friend class IEC61850goose_Test_getEthernetHdr_Test;
 */
#define TEST_FRIENDS
#endif

/**	\brief Класс работы с протоколом СТАНДАРТНЫЙ.
 *
 * 	--- Glossary ----
 *
 *	ADU - application data unit - элемент данных приложения.
 * 	PDU - protocol data unit - элемент данных протокола.
 *
 *	--- Frame description ---
 *
 * 	Frame:
 * 	- preambule, 2 bytes:
 * 		- 0x55, sync character 1
 * 		- 0xAA, sync character 2
 * 	- PDU:
 * 		- funcion code, 1 byte
 * 		- quantity of data bytes = N, 1 byte
 * 		- data bytes, N bytes
 * 	- checksum, 1 byte:
 * 		- least significant 8 bits of the sum all bytes of PDU
 *
 */
class ProtocolS {
	TEST_FRIENDS;

	/// Maximum rx/tx buffer size.
	static const uint8_t BUF_SIZE_MAX = 12;

	/// Minimum ADU size.
	static const uint8_t MIN_SIZE_ADU = 5;

	/// First character preambule.
	static const uint8_t PREAMBLE_CHAR_1 = 0x55;

	/// Second character preambule.
	static const uint8_t PREAMBLE_CHAR_2 = 0xAA;

	/// Command transmitted to the BSP.
	static const uint8_t COM_TO_BSP = 0x11;

	/// Comand received from the BSP.
	static const uint8_t COM_FROM_BSP = 0x12;

public:

	enum state_t {
		STATE_OFF 	= 0,
		STATE_IDLE 	= 1,
		STATE_READ 	= 2,
		STATE_READ_OK = 3,
		STATE_WRITE = 4
	};

	/// Дискретные входы (клеммы передатчика).
	enum dInput_t {
		D_INPUT_16_01 = 0,	///< Дискретные входы с 16 по 1 (1 - установлен).
		D_INPUT_32_17,		///< Дискретные входы с 32 по 17 (1 - установлен).
		//
		D_INPUT_MAX			///< Количество элементов в списке.
	};

	/// Дискретные выходы (клеммы приемника).
	enum dOutput_t{
		D_OUTPUT_16_01 = 0,	///< Дискретные выходы с 16 по 1 (1 - установлен).
		D_OUTPUT_32_17,		///< Дискретные выходы с 32 по 17 (1 - установлен).
		//
		D_OUTPUT_MAX		///< Количество элементов в списке.
	};

	/// Конструктор.
	ProtocolS();

	/// Деструктор.
	~ProtocolS() {};

	/**	Обработка принятых данных.
	 *
	 *	По завершению обработки состояние изменится на Idle.
	 *
	 * 	@retval true если посылка обработана, инача false.
	 */
	bool readData();

	/**	Подготовка данных для передачи.
	 *
	 * 	@retval true если данные готовы, инача false.
	 */
	bool sendData();

	/** Проверка наличия принятого пакета.
	 *
	 *	Проводится проверка корректности принятной КС. Если не совпадает
	 *	то производится сброс состояния в Idle.
	 *
	 * 	@return true если принят пакет, иначе false.
	 */
	bool isReadData();

	/** Проверка наличия данных для передачи.
	 *
	 *	Проверяется наличие данных для передачи и если они есть добавляется
	 *	контрольная сумма ADU.
	 *
	 *	@return true если есть данные для передачи, иначе false.
	 */
	bool isSendData();

	/**	Прием байта данных.
	 *
	 *	Принятый байт данных помещается в буфер.
	 *	При возникновении ошибки прием данных прекращается и протокол
	 *	переходит в режим ожидания.
	 *
	 * 	@param[in] byte Байт данных.
	 * 	@err[in] err true если время приема произошла ошибка, иначае false.
	 */
	void push(uint8_t byte, bool err);

	/**	Передача байта данных.
	 *
	 *	После передачи последнего байта данных протокол переходит в режим
	 *	ожидания.
	 *
	 * 	@param[out] byte Байт данных.
	 * 	@return true если есть данные для передачи, иначе false.	 *
	 */
	bool pull(uint8_t &byte);

	///	Запуск работы протокола.
	void setEnable();

	///	Остановка работы протокола.
	void setDisable() { state = STATE_OFF; }

	/**	Чтение контрольных сигналов для передачи.
	 *
	 * 	@return Текущее состояние контрольных сигналов для передачи.
	 */
	uint8_t getCOut() const;

	/**	Чтение состояний дискретных выходов.
	 *
	 * 	@param[in] dout Дискрентные выходы.
	 * 	@return Текущее состояние указанных дискретных входов.
	 * 	@retval 0 для ошибочного номера дискретных выходов.
	 *
	 */
	uint16_t getDOut(dOutput_t dout) const;

	/**	Запись контрольных сигналов на приеме.
	 *
	 * 	@param[in] Контрольные сигналы на приеме.
	 */
	void setCInput(uint8_t val);

	/**	Запись состояний дискретных входов.
	 *
	 *	@param[in] din Дискретные входы.
	 *	@param[in] val Состояние.
	 */
	void setDInput(dInput_t din, uint16_t val);

	/// Установка состояние ожидания.
	void setIdle();

	/// Установить состояние чтения.
	void setRead();

private:

	/// Дисктреные входы (клеммники передатчика).
	uint16_t dIn[D_INPUT_MAX];

	/// Дискретные выходы (клеммники пприемника).
	uint16_t dOut[D_OUTPUT_MAX];

	/// Контрольные сигналы для передачи.
	uint8_t cOut;

	/// Контрольные сигналы на приеме.
	uint8_t cIn;

	/// Счетчик количества байт в буфере.
	uint8_t cnt;

	/// Количество байт данных на передачу.
	uint8_t nTx;

	/// Текущее состояние работы протокола.
	volatile state_t state;

	uint8_t buf[BUF_SIZE_MAX];	///> Буфер принятых/передаваемых данных.

	/**	Подсчет контрольной суммы ADU.
	 *
	 * 	@return Контрольная сумма.
	 */
	uint8_t calcCRC() const;

	///	Возвращает контрольную сумму ADU.
	uint8_t getCRC() const;

	/**	Проверка текущего состояния.
	 *
	 * 	@param[in] state Состояние.
	 * 	@return True если текущее состояние совпадает с полученным, иначе false.
	 */
	bool checkState(state_t state) { return state == this->state; }

	/**	Добавляет информацию для ADU.
	 *
	 * 	Добавляются синхробайты и контрольная сумма.
	 *
	 * 	@return true если инфорация добавлена, иначе false.
	 */
	bool addADU();
};

#endif /* __TPROTOCOL_S_H_ */

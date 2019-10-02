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

/// ������� ������� � ������� ���������.
#define COM (2)

/// ������� ���������� ���� ������ � ������� ���������.
#define NUM (3)

/// ������� ���������� ����� ������ � ������� ��������� ������� � 0.
#define BYTE(n) (n + 4)

#ifndef TEST_FRIENDS
/*
  ������ ����������� ��� ������������ private � protected �������.
  ������������ � ����� � ������.
  ��������:
  #define TEST_FRIENDS \
    friend class IEC61850goose_Test_decodeLength_Test; \
    friend class IEC61850goose_Test_getEthernetHdr_Test;
 */
#define TEST_FRIENDS
#endif

/**	\brief ����� ������ � ���������� �����������.
 *
 * 	--- Glossary ----
 *
 *	ADU - application data unit - ������� ������ ����������.
 * 	PDU - protocol data unit - ������� ������ ���������.
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

	/// ���������� ����� (������ �����������).
	enum dInput_t {
		D_INPUT_16_01 = 0,	///< ���������� ����� � 16 �� 1 (1 - ����������).
		D_INPUT_32_17,		///< ���������� ����� � 32 �� 17 (1 - ����������).
		//
		D_INPUT_MAX			///< ���������� ��������� � ������.
	};

	/// ���������� ������ (������ ���������).
	enum dOutput_t{
		D_OUTPUT_16_01 = 0,	///< ���������� ������ � 16 �� 1 (1 - ����������).
		D_OUTPUT_32_17,		///< ���������� ������ � 32 �� 17 (1 - ����������).
		//
		D_OUTPUT_MAX		///< ���������� ��������� � ������.
	};

	/// �����������.
	ProtocolS();

	/// ����������.
	~ProtocolS() {};

	/**	��������� �������� ������.
	 *
	 *	�� ���������� ��������� ��������� ��������� �� Idle.
	 *
	 * 	@retval true ���� ������� ����������, ����� false.
	 */
	bool readData();

	/**	���������� ������ ��� ��������.
	 *
	 * 	@retval true ���� ������ ������, ����� false.
	 */
	bool sendData();

	/** �������� ������� ��������� ������.
	 *
	 *	���������� �������� ������������ ��������� ��. ���� �� ���������
	 *	�� ������������ ����� ��������� � Idle.
	 *
	 * 	@return true ���� ������ �����, ����� false.
	 */
	bool isReadData();

	/** �������� ������� ������ ��� ��������.
	 *
	 *	����������� ������� ������ ��� �������� � ���� ��� ���� �����������
	 *	����������� ����� ADU.
	 *
	 *	@return true ���� ���� ������ ��� ��������, ����� false.
	 */
	bool isSendData();

	/**	����� ����� ������.
	 *
	 *	�������� ���� ������ ���������� � �����.
	 *	��� ������������� ������ ����� ������ ������������ � ��������
	 *	��������� � ����� ��������.
	 *
	 * 	@param[in] byte ���� ������.
	 * 	@err[in] err true ���� ����� ������ ��������� ������, ������ false.
	 */
	void push(uint8_t byte, bool err);

	/**	�������� ����� ������.
	 *
	 *	����� �������� ���������� ����� ������ �������� ��������� � �����
	 *	��������.
	 *
	 * 	@param[out] byte ���� ������.
	 * 	@return true ���� ���� ������ ��� ��������, ����� false.	 *
	 */
	bool pull(uint8_t &byte);

	///	������ ������ ���������.
	void setEnable();

	///	��������� ������ ���������.
	void setDisable() { state = STATE_OFF; }

	/**	������ ����������� �������� ��� ��������.
	 *
	 * 	@return ������� ��������� ����������� �������� ��� ��������.
	 */
	uint8_t getCOut() const;

	/**	������ ��������� ���������� �������.
	 *
	 * 	@param[in] dout ����������� ������.
	 * 	@return ������� ��������� ��������� ���������� ������.
	 * 	@retval 0 ��� ���������� ������ ���������� �������.
	 *
	 */
	uint16_t getDOut(dOutput_t dout) const;

	/**	������ ����������� �������� �� ������.
	 *
	 * 	@param[in] ����������� ������� �� ������.
	 */
	void setCInput(uint8_t val);

	/**	������ ��������� ���������� ������.
	 *
	 *	@param[in] din ���������� �����.
	 *	@param[in] val ���������.
	 */
	void setDInput(dInput_t din, uint16_t val);

	/// ��������� ��������� ��������.
	void setIdle();

	/// ���������� ��������� ������.
	void setRead();

private:

	/// ���������� ����� (��������� �����������).
	uint16_t dIn[D_INPUT_MAX];

	/// ���������� ������ (��������� ����������).
	uint16_t dOut[D_OUTPUT_MAX];

	/// ����������� ������� ��� ��������.
	uint8_t cOut;

	/// ����������� ������� �� ������.
	uint8_t cIn;

	/// ������� ���������� ���� � ������.
	uint8_t cnt;

	/// ���������� ���� ������ �� ��������.
	uint8_t nTx;

	/// ������� ��������� ������ ���������.
	volatile state_t state;

	uint8_t buf[BUF_SIZE_MAX];	///> ����� ��������/������������ ������.

	/**	������� ����������� ����� ADU.
	 *
	 * 	@return ����������� �����.
	 */
	uint8_t calcCRC() const;

	///	���������� ����������� ����� ADU.
	uint8_t getCRC() const;

	/**	�������� �������� ���������.
	 *
	 * 	@param[in] state ���������.
	 * 	@return True ���� ������� ��������� ��������� � ����������, ����� false.
	 */
	bool checkState(state_t state) { return state == this->state; }

	/**	��������� ���������� ��� ADU.
	 *
	 * 	����������� ����������� � ����������� �����.
	 *
	 * 	@return true ���� ��������� ���������, ����� false.
	 */
	bool addADU();
};

#endif /* __TPROTOCOL_S_H_ */

/*
 * bsp.h
 *
 *  Created on: 23.05.2014
 *      Author: Shcheblykin
 */

#ifndef __BSP_H_
#define __BSP_H_

#define INLINE __attribute__((always_inline))

class TBsp{
	/// ������ ������ �������� ��� ��������� ����������.
	const static uint8_t BUF_MAX = 2;

	bool newData;					///< ���� ������� ����� ������

	uint8_t bufRx;					///< ����� ������.

	uint8_t com;					///< ������� ���������� � ���
	uint8_t regime;					///< ����� ������ ��� ���������� � ���

public:
	/**	�����������.
	 *
	 */
	uint8_t bufTx[BUF_MAX];			///< ����� ��������.
	bool tmTx;						///< ���. ��������� ������ ��
	bool tmRx;						///< ���. ��������� ����� ��

	TBsp() {
		tmTx = true;
		tmRx = true;
		bufRx = 0;

		newData = false;
		com = 0;
		regime = 0;
	}

	/**	�������� ������� ����� ������ � ���.
	 *
	 * 	��� ������ ������� ���� ������������.
	 *	@return True ���� ���� ����� ������, ����� False.
	 */
	INLINE bool isNewData() {
		bool t = newData;
		newData = false;
		return t;
	}

	/**	���������� ����� ���� ������ ��� ���.
	 *
	 * 	��� ������ ������� �������� ������ ������������ � 0.
	 * 	@retun ����� ������ ���. 0 ������ ��� ������ �������� ���.
	 */
	INLINE bool getRegime() {
		uint8_t t = regime;
		regime = 0;
		return t;
	}

	/**	���������� ������� ���������� � ���.
	 *
	 * 	��� ������ ������� �������� ������� ������������ � 0.
	 * 	@retun ����� ������ ���. 0 ������ ��� ������ �������� ���.
	 */
	INLINE bool getCom() {
		uint8_t t = com;
		com = 0;
		return t;
	}



	/**	�������� ��������� ����� ������ �� ������������ ���������.
	 *
	 *	�������� ����� � ���:
	 *	������ ��� ����� ������, ��� ������� ������ ����� ���������� �������.
	 *	� ������ ������ ��������� ������ \a status, ���������� ������
	 *	��������������� � 0xFF.
	 *
	 *	- @b b1 - ������ ���� = abcccccc
	 *		-# a=0 , ����������� �������
	 *	@n b - ���������� ������� ��������������� �� ����� ��
	 *	@n cccccc - ����� ������� �� ��������
	 *		-# a=1 , ����������� ���������
	 *	@n b=1, cccccc=1 - ������� ���
	 *	@n b=1, cccccc=2 - ������ � �������� ��������� ����
	 *	@n b=1, cccccc=3 - ������ � �������� ����������� ����
	 *	- @b b2 - ��������� ���� = ~b1
	 *
	 *	@param byte ���� ������.
	 *	@param status ������ ��������� ������, True - ������.
	 *	@return �������
	 */
	INLINE void checkRxProtocol(uint8_t byte, bool status) {
		if (status) {
			 bufRx = 0xFF;
		} else if ((byte ^ bufRx) == 0xFF) {
			newData = true;
			byte = bufRx;
			bufRx = 0xFF;
			// ������� ����������� ������
			if (byte & 0x80) {
				if (byte & 0x40) {
					byte = byte & 0x3F;
					if (byte <= 3) {
						regime = byte;
					}
				}
			} else {
				tmTx = byte & 0x40;
				com = byte & 0x3F;
			}
		} else {
			bufRx = byte;
		}
	}

	/**	������������ ������� �������� ���������.
	 *
	 *	��� ������ � ������� ����������� ����:
	 *	�������� ������� �� ��� ������������ � ���, ���� ��� ������� ������
	 *	����� ���.
	 *
	 *	�������� ����� � ���:
	 *	���������� 2 ����� ������. ������ ��������� �������.
	 *
	 *	- @b b1 - ������ ���� = abcccccc.
	 *		-# a=0 , ���������� �������.
	 *	@n b - ���������� ������� �� ����� ��.
	 *	@n cccccc - ����� ������� �������� � ���.
	 *		-# a=1 , ���������� ������.
	 *	@n b - ���������� ������� �� ����� ��.
	 *	@n cccccc=1 - ��� �� �������.
	 *	@n cccccc=2 - ������ � ������ ���.
	 *	- @b b2 - ��������� ���� = ~b1.
	 *
	 *	@param byte ������� �� ��������, 0..32.
	 *	@param status ������ ��������� ������, True - ������.
	 */
	INLINE void makeTxData(uint8_t com, uint8_t error) {
		uint8_t byte = 0;
		if ((com != 0) || (error = 0)) {
			byte = (tmRx) ? 0x40 : 0x00;
			byte |= (com & 0x3F);
		} else {
			byte = (tmRx) ? 0xC0 : 0x80;
			byte |= (error & 0x3F);
		}
		bufTx[0] = byte;
		bufTx[1] = ~byte;
	}
};


#endif /* __BSP_H_ */

/*
 * dr.h
 *
 *  Created on: 19.05.2014
 *      Author: Shcheblykin
 */

/// ����� ������ � ���.
/** ��� ����������� ����� ������ � �������� ������� ��� � �������� ������
 * 	����������� �������� \a CNT_ERROR. C��������� ������� ��������
 * 	������������� ���-�� ������� � ��, �� ������� ����������� ������ ���.
 * 	���� �������� \a error ��������� ����� \a ERRORS_TO_FAULT, �� �������
 * 	��� ������ ��� ����������. ����������� ��������� �������� ������ ����� ����
 * 	\a MAX_ERRORS.
 * 	@see CNT_ERROR
 * 	@see ERRORS_TO_FAULT
 * 	@see MAX_ERRORS
 */
class TDigitalRetrans {

private:
	/// ������ ������ ������/�������� ��� ��������� ����������.
	const static uint8_t BUF_MAX = 20;
	/// ���-�� ����������� ������ ��� �������� �� �������������� � ������
	const static uint8_t ERRORS_TO_FAULT = 100;
	/// ������������ ���-�� ������
	const static uint8_t MAX_ERRORS = 200;
	/// ���-�� �� ����������� ��� ����������� ������
	const static uint8_t CNT_ERROR = 5;

	uint8_t mCodeToCom[256];		///< ������� ��� ��� -> �������
	uint8_t error;					///< ���� ������ ���.
	uint8_t com;					///< ������� �� ������/��������.
	uint8_t cntCom;					///< ���-�� ��������(����������) �������.

	/**	���������� ������� ����������� ������ ������� � ���� ���.
	 *
	 *	������������ ��� ���������� ������� �������������� ��� <-> �������.
	 *	��� ������ ������� ������������� ����� �������������� ����.
	 */
	void createTableCodeToCom() {
		// ���������� ������� ���������� 0xFF, �.�. �������
		for (uint8_t i = 0, temp = 0x01; i < 32; i++) {
			mCodeToCom[i] = temp;
			temp  = (temp & 0x80) ? 0x01 : temp << 1;
		}
		// ���������� ������� ������� ����������
		// �.�. ������[��� ������� ���] = ����� �������
		for(uint16_t i = 1; i <= 8; i++) {
			mCodeToCom[(1 << i)] = i;
		}
	}

public:
	/**	�����������.
	 *
	 * 	������������� ���-�� ������ \a CNT_ERROR.
	 * 	@see CNT_ERROR
	 */
	TDigitalRetrans() {
		error = CNT_ERROR;
		com = 0;
		cntCom = 0;
	}

	/**	�������� ������� ������ � �������� ������� ���.
	 * 	@retval True - ���� ������.
	 * 	@retval False - ������ ���.
	 */
	bool isWarning() const __attribute__((always_inline)) {
		return (error > 0);
	 }


	/**	�������� ������� ������ � ������ ���.
	 *
	 * 	���������������, ���� ������ � �������� ������� ���������� �������
	 * 	�����. � ������ >= \a MAX_ERROR.
	 * 	@see MAX_ERROR
	 * 	@retval True - ���� ������.
	 * 	@retval False - ������ ���.
	 */
	bool isFault() const __attribute__((always_inline)) {
		return (error > ERRORS_TO_FAULT);
	}

	/** ��������� ������� ������ � �������� ������� ���.
	 *
	 * 	���� �������� �������� ������ �� ��������� \a MAX_ERROR, � ��������
	 * 	����������� \a CNT_ERROR ������.
	 * 	@see MAX_ERRORS
	 * 	@see CNT_ERROR
	 */
	void setError() __attribute__((always_inline)) {
		if (error < MAX_ERRORS)
			error += CNT_ERROR;
	}

	/** ���������� ���-�� ������ � �������� ������� ���.
	 *
	 * 	���� �������� �������� ������ ����, �� ��� ����������� �� �������.
	 */
	void decError() __attribute__((always_inline)) {
		if (error > 0)
			error--;
	}

	/**	�������� ������� ������� � �������� ������ ���.
	 *
	 * 	����������� ������ ����� ������ �� ������� � ��� �������. � ������
	 * 	������ ���� �� ����� ����� �������.
	 */
	void checkCommand() __attribute__((always_inline)) {
		uint8_t com = 0;

		for (unsigned char i = 2, tCom = 0; i < 10; i += 2, tCom += 8) {
			uint8_t temp = mCodeToCom[i];
			// ���� ����� �� ���������� - ������� ��������� ���
			if (temp > 8) {
				// � ������� ����� ���������� ����� ����� �������
				com = 0xFF;
				break;
			} else if (temp != 0) {
				if (com == 0) {
					com = tCom + temp;
				} else {
					// � ������� ���������� ����� ����� �������
					com = 0xFF;
					break;
				}
			}
		}

		if (com > 32) {
			// ������ ������ � �������� ������ ���
			setError();
		} else {
			// ������� �������
			if (this->com == com) {
				// ���������� �������� �������� �������� ������� � ���� ��������
				this->cntCom = (this->cntCom < 3) ? this->cntCom + 1 : 3;
			} else {
				// ������� ����� �������
				this->com = com;
				this->cntCom = 1;
			}
		}
	}

	uint8_t buf[BUF_MAX];			///< ����� ������/��������.
};


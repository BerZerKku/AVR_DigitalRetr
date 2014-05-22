/*
 * dr.h
 *
 *  Created on: 19.05.2014
 *      Author: Shcheblykin
 */

/// Класс работы с ЦПП.
/** При обнаружении любой ошибки в принятых пакетах ЦПП к счетчику ошибок
 * 	добавляется значение \a CNT_ERROR. Cодержимое данного счетчика
 * 	соответствует кол-ву времени в мс, на которое блокируется работа ЦПП.
 * 	Если значение \a error превысило порог \a ERRORS_TO_FAULT, то считаем
 * 	что работа ЦПП невозможна. Максимально возможное значение ошибки может быть
 * 	\a MAX_ERRORS.
 * 	@see CNT_ERROR
 * 	@see ERRORS_TO_FAULT
 * 	@see MAX_ERRORS
 */
class TDigitalRetrans {

private:
	/// Размер буфера приема/передачи для цифрового переприема.
	const static uint8_t BUF_MAX = 20;
	/// Кол-во накопленных ошибок для перехода из предупреждения в аварию
	const static uint8_t ERRORS_TO_FAULT = 100;
	/// Максимальное кол-во ошибок
	const static uint8_t MAX_ERRORS = 200;
	/// Кол-во мс добавляемых при определении ошибки
	const static uint8_t CNT_ERROR = 5;

	uint8_t mCodeToCom[256];		///< Таблица код ЦПП -> команда
	uint8_t error;					///< Флаг ошибки ЦПП.
	uint8_t com;					///< Команда на приеме/передаче.
	uint8_t cntCom;					///< Кол-во принятых(переданных) посылок.

	/**	Заполнение таблицы сответствия номера команды и кода ЦПП.
	 *
	 *	Используется для уменьшения времени преобразования код <-> команда.
	 *	Где номеру команды соответствует номер установленного бита.
	 */
	void createTableCodeToCom() {
		// заполнение массива значениями 0xFF, т.е. ошибкой
		for (uint8_t i = 0, temp = 0x01; i < 32; i++) {
			mCodeToCom[i] = temp;
			temp  = (temp & 0x80) ? 0x01 : temp << 1;
		}
		// заполнение массива нужными значениями
		// т.е. массив[код команды ЦПП] = номер команды
		for(uint16_t i = 1; i <= 8; i++) {
			mCodeToCom[(1 << i)] = i;
		}
	}

public:
	/**	Конструктор.
	 *
	 * 	Устанавливаем кол-во ошибок \a CNT_ERROR.
	 * 	@see CNT_ERROR
	 */
	TDigitalRetrans() {
		error = CNT_ERROR;
		com = 0;
		cntCom = 0;
	}

	/**	Проверка наличия ошибок в принятых пакетах ЦПП.
	 * 	@retval True - есть ошибки.
	 * 	@retval False - ошибок нет.
	 */
	bool isWarning() const __attribute__((always_inline)) {
		return (error > 0);
	 }


	/**	Проверка наличия ошибки в работе ЦПП.
	 *
	 * 	Устанавливается, если ошибок в принятых пакетах накопилось слишком
	 * 	много. А точнее >= \a MAX_ERROR.
	 * 	@see MAX_ERROR
	 * 	@retval True - есть ошибка.
	 * 	@retval False - ошибок нет.
	 */
	bool isFault() const __attribute__((always_inline)) {
		return (error > ERRORS_TO_FAULT);
	}

	/** Установка наличия ошибки в принятых пакетах ЦПП.
	 *
	 * 	Если значение счетчика ошибок не превышает \a MAX_ERROR, к счетчику
	 * 	добавляется \a CNT_ERROR ошибок.
	 * 	@see MAX_ERRORS
	 * 	@see CNT_ERROR
	 */
	void setError() __attribute__((always_inline)) {
		if (error < MAX_ERRORS)
			error += CNT_ERROR;
	}

	/** Уменьшение кол-ва ошибок в принятых пакетах ЦПП.
	 *
	 * 	Если значение счетчика больше нуля, то оно уменьшается на единицу.
	 */
	void decError() __attribute__((always_inline)) {
		if (error > 0)
			error--;
	}

	/**	Проверка наличия команды в принятом пакете ЦПП.
	 *
	 * 	Проверяются прямые байты команд на наличие в них команды. В пакете
	 * 	должно быть не более одной команды.
	 */
	void checkCommand() __attribute__((always_inline)) {
		uint8_t com = 0;

		for (unsigned char i = 2, tCom = 0; i < 10; i += 2, tCom += 8) {
			uint8_t temp = mCodeToCom[i];
			// если число не побходящее - вернуть ошибочный код
			if (temp > 8) {
				// в текущем байте обнаружено более одной команды
				com = 0xFF;
				break;
			} else if (temp != 0) {
				if (com == 0) {
					com = tCom + temp;
				} else {
					// в посылке обнаружено более одной команды
					com = 0xFF;
					break;
				}
			}
		}

		if (com > 32) {
			// ошибка данных в принятом пакете ЦПП
			setError();
		} else {
			// принята команда
			if (this->com == com) {
				// увеличение счетчика повторно принятых посылок с этой командой
				this->cntCom = (this->cntCom < 3) ? this->cntCom + 1 : 3;
			} else {
				// принята новая команда
				this->com = com;
				this->cntCom = 1;
			}
		}
	}

	uint8_t buf[BUF_MAX];			///< Буфер приема/передачи.
};


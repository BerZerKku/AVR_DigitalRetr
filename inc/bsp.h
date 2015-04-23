/*
 * bsp.h
 *
 *  Created on: 23.05.2014
 *      Author: Shcheblykin
 */

#ifndef __BSP_H_
#define __BSP_H_

#include <stdint.h> 

#define INLINE __attribute__((always_inline))

#ifdef AVR
#include <avr/io.h>
#define VOLATILE volatile
#else
#define VOLATILE
#endif

/**	\brief Класс работы с БСП.
 *
 *
 * 	От БСП ожидаеются два байта данных, причем второй равен первому,
 * 	при их нахождении происходит распознование содержимого: настройка или
 * 	команда + выход ТМ.
 * 	После получения посылки формируется ответ в БСП. Если имеется команда на
 * 	передачу, она будет отправлена в БСП даже при наличии ошибки работы
 * 	устройства.
 *
 *	Для проверки связи c БСП используется счетчик ошибок. Который увеличивается
 *	при приеме байта не совпавшего с предыдущем или при периодическом вызове
 *	функции \a checkConnect и обнуляется в случае принятия нового сообщения.
 *	Если счетчик ошибок достигнет значения \a MAX_ERRORS, то это означает что
 *	связи с БСП нет.
 *
 * 	@see MAX_ERRORS
 */
class TBsp{
protected:
	/// Размер буфера передачи для цифрового переприема.
	const static uint8_t BUF_MAX = 2;
	/// Критическое кол-во ошибок при приеме данных с БСП
	const static uint8_t MAX_ERRORS = 10;

	VOLATILE bool newData;			///< флаг наличия новых данных
	VOLATILE uint8_t com;			///< команда полученная с БСП
	VOLATILE uint8_t regime;		///< режим работы ЦПП полученный с БСП
	VOLATILE uint8_t error;			///< Счетчик ошибок
	uint8_t bufRx;					///< Буфер приема.


public:
	/**	Конструктор.
	 *
	 *	Количество ошибок устанавливается максимальным.  Уровни сигналов ТМ
	 *	равны лог.1.
	 */
	uint8_t bufTx[BUF_MAX];			///< Буфер передачи.
	volatile bool tmTx;				///< лог. состояние выхода ТМ
	volatile bool tmRx;				///< лог. состояние входа ТМ

	TBsp() {
		tmTx = true;
		tmRx = true;
		bufRx = 0;
		error = MAX_ERRORS;

		newData = false;
		com = 0;
		regime = 0;
	}

	/**	Проверка наличия новых данных с БСП.
	 *
	 * 	При вызове функции флаг сбрасывается.
	 *	@return True если есть новые данные, иначе False.
	 */
	bool isNewData() {
		bool t = false;
		if (newData) {
			t = newData;
			newData = false;
		}
		return t;
	}

	/**	Увеличение счетчика ошибок.
	 *
	 */
	INLINE void incError() {
		if (error < MAX_ERRORS) {
			error++;
		}
	}

	/**	Проверка на наличие ошибок связи с БСП.
	 *
	 * 	@return True - в случае ошибки, False - иначе.
	 */
	INLINE bool isError() {
		return (error >= MAX_ERRORS);
	}

	/**	Проверка наличия связи с БСП.
	 *
	 * 	Увеличивается счетчик ошибок. Если связи нет, то со временем
	 * 	счетчик достигнет предельного значения.
	 *
	 * 	Необходимо выбирать период вызова данной функции с учетом периода
	 * 	следования посылок с БСП, а точнее не чаще 1-2 раз за это время.
	 */
	INLINE void checkConnect() {
		incError();
	}

	/**	Возвращает новый режим работы для ЦПП.
	 *
	 * 	При вызове функции значение режима сбрасывается в 0.
	 * 	@return Режим работы ЦПП. 0 значит что нового значения нет.
	 */
	uint8_t getRegime() {
		uint8_t t = 0;
		if (regime) {
			t = regime;
			regime = 0;
		}
		return t;
	}

	/**	Возвращает команду полученную с БСП.
	 *
	 * 	При вызове функции значение команды сбрасывается в 0.
	 * 	@return Режим работы ЦПП. 0 значит что нового значения нет.
	 */
	uint8_t getCom() {
		uint8_t t = 0;
		if (com) {
			t = com;
			com = 0;
		}
		return t;
	}

	/**	Проверка принятого байта данных на соответствие протоколу.
	 *
	 *	Протокол связи с БСП:
	 *	Ищется два одинаковых байта подряд.
	 *	В случае ошибки источника данных \a status, содержимое буфера
	 *	устанавливается в 0xFF.
	 *
	 *	При обнаружении достоверной посылки буфер устанавливается в 0xFF.
	 *
	 *	При обнаружении первого байта увеличивается счетчик ошибок, при
	 *	обнаружении второго (достоверная посылка) счетчик сбрасывается.
	 *
	 *	- @b b1 - прямой байт = abcccccc
	 *		-# a=0 , принимается команда
	 *	@n b - логический уровень устанавливаемый на выход ТМ
	 *	@n cccccc - номер команды на передачу
	 *		-# a=1 , принимаются настройки
	 *	@n b=1, cccccc=1 - останов ЦПП
	 *	@n b=1, cccccc=2 - работа в качестве Приемника КЕДР
	 *	@n b=1, cccccc=3 - рбаота в качестве Передатчика КЕДР
	 *	- @b b2 - доп.байт = b1
	 *
	 *	@param byte Байт данных.
	 *	@param status Статус источника данных, True - ошибка.
	 *	@return Команда
	 */
	INLINE void checkRxProtocol(uint8_t byte, bool status) {
		if (status) {
			 bufRx = 0xFF;
		} else {
			if (byte == bufRx) {
				newData = true;
				bufRx = 0xFF;
				error = 0;
				// приняты достоверные данные
				if (byte & 0x80) {
					if (byte & 0x40) {
						byte = byte & 0x3F;
						if (byte <= 3) {
							regime = byte & 0x3F ;
						}
					}
				} else {
					tmTx = byte & 0x40;
					com = byte & 0x3F;
				}
			} else {
				bufRx = byte;
				incError();
			}
		}
	}

	/**	Формирование посылки согласно протоколу.
	 *
	 *	При работе в качесте Передатчика КЕДР:
	 *	принятые команды по ЦПП отправляются в БСП, даже при наличии ошибок
	 *	связи ЦПП.
	 *
	 *	Протокол связи с БПС:
	 *	Передаются 2 байта данных. Второй совпадает с первым.
	 *
	 *	- @b b1 - прямой байт = abcccccc.
	 *		-# a=0 , передается команда.
	 *	@n b - логический уровень на входе ТМ.
	 *	@n cccccc - номер команды принятый с ЦПП.
	 *		-# a=1 , передается ошибка.
	 *	@n b - логический уровень на входе ТМ.
	 *	@n cccccc=1 - ЦПП не запущен.
	 *	@n cccccc=2 - ошибки в работе ЦПП для режима Приемник КЕДР.
	 *	@n cccccc=3 - ошибки в работе ЦПП для режима Передатчик КЕДР.
	 *	- @b b2 - доп.байт = b1
	 *
	 *	@param com Команда на передачу, 0..32.
	 *	@param error Статус источника данных, True - ошибка.
	 */
	INLINE void makeTxData(uint8_t com, uint8_t error) {
		uint8_t byte = 0;
		if ((com != 0) || (error == 0)) {
			byte = (tmRx) ? 0x40 : 0x00;
			byte |= (com & 0x3F);
		} else {
			byte = (tmRx) ? 0xC0 : 0x80;
			byte |= (error & 0x3F);
		}
		bufTx[0] = byte;
		bufTx[1] = byte;
	}
};


#endif /* __BSP_H_ */

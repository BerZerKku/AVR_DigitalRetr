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

class TBsp{
protected:
	/// Размер буфера передачи для цифрового переприема.
	const static uint8_t BUF_MAX = 2;

	bool newData;					///< флаг наличия новых данных

	uint8_t bufRx;					///< Буфер приема.

	uint8_t com;					///< команда полученная с БСП
	uint8_t regime;					///< режим работы ЦПП полученный с БСП

public:
	/**	Конструктор.
	 *
	 */
	uint8_t bufTx[BUF_MAX];			///< Буфер передачи.
	bool tmTx;						///< лог. состояние выхода ТМ
	bool tmRx;						///< лог. состояние входа ТМ

	TBsp() {
		tmTx = true;
		tmRx = true;
		bufRx = 0;

		newData = false;
		com = 0;
		regime = 0;
	}

	/**	Проверка наличия новых данных с БСП.
	 *
	 * 	При вызове функции флаг сбрасывается.
	 *	@return True если есть новые данные, иначе False.
	 */
	INLINE bool isNewData() {
		bool t = newData;
		newData = false;
		return t;
	}

	/**	Возвращает новый режим работы для ЦПП.
	 *
	 * 	При вызове функции значение режима сбрасывается в 0.
	 * 	@retun Режим работы ЦПП. 0 значит что нового значения нет.
	 */
	INLINE bool getRegime() {
		uint8_t t = regime;
		regime = 0;
		return t;
	}

	/**	Возвращает команду полученную с БСП.
	 *
	 * 	При вызове функции значение команды сбрасывается в 0.
	 * 	@retun Режим работы ЦПП. 0 значит что нового значения нет.
	 */
	INLINE uint8_t getCom() {
		uint8_t t = com;
		com = 0;
		return t;
	}



	/**	Проверка принятого байта данных на соответствие протоколу.
	 *
	 *	Протокол связи с БСП:
	 *	Ищется два байта подряд, для которых первый равен инверсному второму.
	 *	В случае ошибки источника данных \a status, содержимое буфера
	 *	устанавливается в 0xFF.
	 *
	 *	- @b b1 - прямой байт = abcccccc
	 *		-# a=0 , принимается команда
	 *	@n b - логический уровень устанавливаемый на выход ТМ
	 *	@n cccccc - номер команды на передачу
	 *		-# a=1 , принимаются настройки
	 *	@n b=1, cccccc=1 - останов ЦПП
	 *	@n b=1, cccccc=2 - работа в качестве Приемника КЕДР
	 *	@n b=1, cccccc=3 - рбаота в качестве Передатчика КЕДР
	 *	- @b b2 - инверсный байт = ~b1
	 *
	 *	@param byte Байт данных.
	 *	@param status Статус источника данных, True - ошибка.
	 *	@return Команда
	 */
	INLINE void checkRxProtocol(uint8_t byte, bool status) {
		if (status) {
			 bufRx = 0xFF;
		} else if ((byte ^ bufRx) == 0xFF) {
			newData = true;
			byte = bufRx;
			bufRx = 0xFF;
			// приняты достоверные данные
			if (byte & 0x80) {
				if (byte & 0x40) {
					byte = byte & 0x3F;
					if (byte <= 3) {
						regime = 0x3F & byte;
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

	/**	Формирование посылки согласно протоколу.
	 *
	 *	При работе в качесте Передатчика КЕДР:
	 *	принятые команды по ЦПП отправляются в БСП, даже при наличии ошибок
	 *	связи ЦПП.
	 *
	 *	Протокол связи с БПС:
	 *	Передаются 2 байта данных. Второй инверсный первому.
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
	 *	- @b b2 - инверсный байт = ~b1.
	 *
	 *	@param byte Команда на передачу, 0..32.
	 *	@param status Статус источника данных, True - ошибка.
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
		bufTx[1] = ~byte;
	}
};


#endif /* __BSP_H_ */

/*
 * 	main.cpp
 *
 *  Created on: 15.05.2014
 *      Author: Shcheblykin
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "inc/dr.h"
#include "inc/bsp.h"

/**
 *	@mainpage MainPage
 *
 *  @section brief_sec Описание
 *
 *  Программа предназначена для прошивки МК блока БВП аппаратуры АВАНТ РЗСК или
 *  К400. И предназначена для осуществления цифрового переприема. На данный
 *  момент возможна работа в качестве "Приемник КЕДР" или "Передатчик КЕДР".
 *
 *  При включеннии аппаратуры (перезагрузке МК) ЦПП выключен. О чем сообщает
 *  посылая данный код ошибки в БСП. При этом в канал ЦПП постоянно посылаются
 *  пустые посылки.
 *
 *  При работе в режиме "Приемник КЕДР" команды получаемые от БСП, пересылаются
 *  в канал ЦПП, при их отсутствии передаются пустые посылки. По приему
 *  контролируется только наличие посылок, но их содержимое игнорируется.
 *  В случае ошибки работы ЦПП в данном режиме, будет сформирована ошибка.
 *  Проверка приема контролирует только наличие обратнойсвязи и на передачу
 *  команд не влияет.
 *
 *	При работе в режиме "Передатчик КЕДР" команды получаемые от БСП
 *	игнорируются. Команды полученные по ЦПП пересылаются в БСП, даже при
 *	появлении ошибки работы ЦПП. В канал ЦПП постоянно посылаются пустые
 *	посылки. В случае ошибки работы ЦПП в данном режиме, будет сформирована
 *	ошибка. До окончания дейтсвия данной ошибки, прием команд по ЦПП
 *	прекращается.
 */

//---МАКРОПОДСТАНОВКИ-----------------------------------------------------------

//-----	Тестовые сигналы
#define LED_VD20 PA0	///< Пин подключения отладочного светодиода VD20.
#define LED_VD19 PA1	///< Пин подключения отладочного светодиода VD19.
#define TP3 PA2			///< Пин подключения отладочной тестовой точки TP3.
#define TP4 PA3			///< Пин подключения отладочной тестовой точки TP4.

//-----	Телемеханика
#define TM_RX PC3		///< Пин подключения входа Телемеханики.
#define TM_TX PC2		///< Пин подключения выхода Телемеханики.

//----- Цифровая ретрансляция
#define DR_RX PD2 		///< Пин подключения входа ЦПП.
#define DR_TX PD3 		///< Пин подключения выхода ЦПП.
#define DR_EN PD4 		///< Пин управления ЦПП.

//----- Связь с БСП
#define BSP_RX PD0		///< Пин подключения входа БСП.
#define BSP_TX PD1		///< Пин подключения выхода БСП.

/// Размещение в области начальной инициализации
#define INITSECTION __attribute__((section(".init3")))


//---ОПИСАНИЕ СТРУКТУР----------------------------------------------------------



//---ОБЪЯВЛЕНИЯ ФУНКЦИЙ---------------------------------------------------------

INITSECTION __attribute__((__naked__)) void low_level_init();
static inline void enableDrIO();
static inline void disableDrIO();

//---ПЕРЕМЕННЫЕ-----------------------------------------------------------------


TDigitalRetrans dr;	///< Класс работы с ЦПП
TBsp bsp;			///< Класс работы с БСП.


//---ОПРЕДЕЛЕНИЯ ФУНКЦИЙ--------------------------------------------------------

/**	Разрешение работы преобразователя UART <-> RS422.
 *
 */
void enableDrIO() {
	PORTD |= (1 << DR_EN);
}

/** Запрет работы преобразователя UART <-> RS422.
 *
 */
void disableDrIO() {
	PORTD &= ~(1 << DR_EN);
}


/** Main.
 *
 * 	При получении посылки от БСП, данные передаются в модуль ЦПП. Далее
 * 	формируется ответ для БСП.
 *
 * 	@return Нет.
 */
__attribute__ ((OS_main)) int main(void) {

	enableDrIO();
	sei();

	dr.setRegime(TDigitalRetrans::REG_RX_KEDR);

	while(1) {
		PINA |= (1 << LED_VD19);

		// получена посылка с БСП, надо обработать и ответить
		if (bsp.isNewData()) {

			// установка режима работы
			dr.setRegime(bsp.getRegime());
			// запись команды на передачу
			dr.setCom(bsp.getCom());

			// подготовка данных для отправки в БСП
			bsp.tmRx = (PINC & (1 << TM_RX));	//текущий уровень входа ТМ
			bsp.makeTxData(dr.getCom(), dr.getError());

			// передача двух байт данных в БСП
//			while(!(UCSR0A | (1 << UDRE0)));
			UDR0 = bsp.bufTx[0];
//			while(!(UCSR0A | (1 << UDRE0)));
			UDR0 = bsp.bufTx[1];

			// установка значения на выходе ТМ
			if (bsp.tmTx) {
				PORTC = (1 << TM_TX);
			} else {
				PORTC &= ~(1 << TM_TX);
			}
		}



		wdt_reset();
	}
}

/**	Прерывание по свопадению таймера 1.
 *
 *	Уменьшается кол-во текщих ошибок.
 *	Проверяется наличие полученной посылки по ЦПП.
 */
ISR(TIMER1_COMPA_vect) {
	static uint8_t delay = 0;
	dr.decError();
	dr.checkConnect();
}

/** Прерывание по приему UART0.
 *
 * 	Прием байта данных от БСП.
 */
ISR(USART0_RX_vect) {
	uint8_t status = UCSR0A;	// региcтр состояния
	uint8_t byte = UDR0;		// регистр данных

	// Обработка данных принятого байта
	bsp.checkRxProtocol(byte, status & ((1 << FE0) | (1 << DOR0)));
}


/** Прерывание по приему UART1.
 */
ISR(USART1_RX_vect) {
	uint8_t status = UCSR1A;	// регистр состояния
	uint8_t byte = UDR1;		// регистр данных

	// Обработка принятого байта
	dr.checkByteProtocol(byte, status & ((1 << FE1) | (1 << DOR1)));
}


/** Прерывание по опустошению буфера передачи UART1.
 *
 * 	В случае отсутствия байта на передачу
 */
ISR(USART1_UDRE_vect) {
	UDR1 = dr.getTxByte();
}


/**	Инициализация периферии.
 *
 * 	Функция помещается в начальный загрузчик по адресу ".init3".
 * 	@n Неиспользуемые порты по умолчанию настроены на вход с подтяжкой к +.
 */
void low_level_init() {
	// включение сторожевого таймера
	wdt_enable(WDTO_15MS);

	// порт А
	// тестовые выходы
	DDRA = (1 << LED_VD20) | (1 << LED_VD19) | (1 << TP3) | (1 << TP4);
	PORTA = 0x00;

	// порт C
	// телемеханика
	DDRC = (1 << TM_TX) | (0 << TM_RX);
	PORTC = 0x00;

	// PORTD
	// ЦПП + связь с БСП
	DDRD = (1 << DR_TX) | (1 << DR_EN) | (0 << DR_RX);
	DDRD |= (1 << BSP_TX) | (0 << BSP_RX);
	PORTD = 0;

	// UART0
	// связь с БСП
	// 38.4бит/с при кварце 20Мгц
	// включено удвоение скорости работы
	// 1 стоп бит, 8 бит данных, контроль четности отключен
	// включено прерывание по приему
	UBRR0   = 64;
	UCSR0A  = (1 << U2X0);
	UCSR0B  = (1 << RXCIE0) | (0 << UCSZ02);
	UCSR0C  = (1 << UCSZ00) | (1 << UCSZ01);	//  по умолчанию
	UCSR0B  |= (1 << RXEN0) | (1 << TXEN0);


	// UART1
	// ЦПП
	// 500бит/с при кварце 20Мгц
	// включено удвоение скорости работы
	// 1 стоп бит, 8 бит данных, контроль четности отключен
	// включены прерывания по приему и опустошению буфера
	UBRR1   = 4;
	UCSR1A  = (1 << U2X1);
	UCSR1B  = (1 << RXCIE1) | (1 << UDRIE1) | (0 << UCSZ12);
	UCSR1C  = (1 << UCSZ10) | (1 << UCSZ11);	//  по умолчанию
	UCSR1B  |= (1 << RXEN1) | (1 << TXEN1);



	// Таймер 1
	// режим CTC
	// делитель N = 8, счет до OCR = 2499
	// частота = F_CPU / (2 * N * (1 + OCR)
	// при F_CPU = 20МГц получим 500Гц
	TCCR1A 	= (0 << WGM11) | (0 << WGM10);
	TCCR1B 	= (0 << WGM13) | (1 << WGM12);
	TCCR1C 	= 0;	// по умолчанию
	TCNT1 	= 0;
	OCR1A 	= 2499;
	TIMSK1 	= (1 << OCIE1A);
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10); // запуск с делителем 8
}

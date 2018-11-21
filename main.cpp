/*
 * 	main.cpp
 *
 *  Created on: 15.05.2014
 *      Author: Shcheblykin
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "protocolPcM.h"
#include "protocolS.h"

#define VERS 0x0103

/**
 *	@mainpage MainPage
 *
 *  @section brief_sec Описание
 *
 *  Программа предназначена для работы фифрового переприема и прошивается в МК
 *  блока БВП аппаратуры АВАНТ РЗСК или К400. На данный момент возможна работа
 *  в качестве "Приемник КЕДР" или "Передатчик КЕДР".
 *
 *  При включеннии аппаратуры (перезагрузке МК) ЦПП выключен. О чем сообщает
 *  посылая данный код ошибки в БСП.
 *
 *  При работе в режиме "Приемник КЕДР" команды получаемые от БСП, пересылаются
 *  в канал ЦПП, при их отсутствии передаются пустые посылки. По приему
 *  контролируется только наличие посылок, но их содержимое игнорируется.
 *  В случае ошибки работы ЦПП в данном режиме, будет сформирована ошибка.
 *  Проверка приема контролирует только наличие обратной связи и на передачу
 *  команд не влияет.
 *
 *	При работе в режиме "Передатчик КЕДР" команды получаемые от БСП
 *	игнорируются. Команды полученные по ЦПП пересылаются в БСП, даже при
 *	появлении ошибки работы ЦПП. В канал ЦПП постоянно посылаются пустые
 *	посылки. В случае ошибки работы ЦПП в данном режиме, будет сформирована
 *	ошибка. До окончания дейтсвия данной ошибки, прием команд по ЦПП
 *	прекращается.
 *
 *	В случае нарушения связи с БСП ЦПП выключается.
 *
 *	Если ЦПП выключен, сигналы в линию не передаются и с нее не считыаются (
 *	запрет на уровне железа).
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

TProtocolPcM drModbus;	///< Класс работы с ЦПП Modbus
ProtocolS bspS;
volatile static uint8_t num_1ms = 0;

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
	uint16_t dout = 0;
	uint16_t din = 0;

	enableDrIO();
	drModbus.setTick(115200, 150);
	drModbus.setAddressLan(1);
	drModbus.setEnable();

	bspS.setEnable();

	sei();

	while(1) {

		if (num_1ms >= 10) {
			bspS.setIdle();
			num_1ms = 0;
		}

		if (bspS.isReadData()) {

			bspS.readData();

			dout = bspS.getDOut(bspS.D_OUTPUT_16_01);
			drModbus.setDO(drModbus.D_OUTPUT_16_01, dout);

			dout = bspS.getDOut(bspS.D_OUTPUT_32_17);
			drModbus.setDO(drModbus.D_OUTPUT_32_17, dout);


			PINA = (1 << LED_VD20);
			num_1ms = 3;
		}

		if (num_1ms > 2) {
			if (bspS.sendData()) {
				if (bspS.isSendData()) {
					uint8_t byte;
					if (bspS.pull(byte)) {
						PINA = (1 << LED_VD19);
						UCSR0B &= ~(1 << RXCIE0);
						UDR0 = byte;
						UCSR0B |= (1 << UDRIE0) | (1 << TXCIE0);
					}
				}
			}
		}

		if (drModbus.isReadData()) {
			PORTA |= (1 << TP4);
			if (drModbus.readData()) {

				din = drModbus.getDI(drModbus.D_INPUT_16_01);
				bspS.setDInput(bspS.D_INPUT_16_01, din);

				din = drModbus.getDI(drModbus.D_INPUT_32_17);
				bspS.setDInput(bspS.D_INPUT_32_17, din);
			}

			if (drModbus.isSendData()) {
				// отключение прерывания приемника
				UCSR1B &= ~(1 << RXCIE1);
				// включение прерывания передатчика
				UDR1 = drModbus.pull();
				UCSR1B |= (1 << UDRIE1) | (1 << TXCIE1);
			}
			PORTA &= ~(1 << TP4);
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
	static uint8_t cnt = 0;

	if (++cnt > 6) {
		if (num_1ms < 10) {
			num_1ms++;
		}
		cnt = 0;
	}

	drModbus.tick();	// выполняется около 2мкс
}

/** Прерывание по приему UART0.
 *
 * 	Прием байта данных от БСП.
 */
ISR(USART0_RX_vect) {
	uint8_t status = UCSR0A;	// региcтр состояния
	uint8_t byte = UDR0;		// регистр данных


	// Обработка данных принятого байта
	bspS.push(byte, status & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)));
	num_1ms = 0;
}

/**
 *
 */
ISR(USART0_UDRE_vect) {
	uint8_t byte = 0;
	if (bspS.pull(byte)) {
		UDR0 = byte;
		num_1ms = 0;
	} else {
		UCSR0B &= ~(1 << UDRIE0);
	}
}

/**
 *
 */
ISR(USART0_TX_vect) {
	UCSR0B  &= ~(1 << TXCIE0);
	UCSR0B  |= (1 << RXCIE0);

	bspS.setIdle();
}


/** Прерывание по приему UART1.
 *
 * 	Прием байт данных по ЦПП.
 */
ISR(USART1_RX_vect) {

	uint8_t status = UCSR1A;	// регистр состояния
	uint8_t byte = UDR1;		// регистр данных

	// Обработка принятого байта
	if (status & ((1 << FE1) | (1 << DOR1) | (1 << UPE1))) {
		drModbus.setReadState();
	} else {

		drModbus.push(byte);
	}
}

/** Прерывание по опустошению буфера передачи UART1.
 *
 * 	Передача байт данных по ЦПП.
 */
ISR(USART1_UDRE_vect) {
	if (drModbus.isSendData()) {
		UDR1 = drModbus.pull();
	} else {
		UCSR1B &= ~(1 << UDRIE1);
	}
}

/**	Прерывание по завершению передачи UART1.
 *
 * 	Отключаются прервания по завершению передачи.
 * 	Включается прерывание по приему данных.
 *
 * 	Устанавливается режим приема данных.
 */
ISR(USART1_TX_vect) {
	UCSR1B  &= ~(1 << TXCIE1);
	UCSR1B  |= (1 << RXCIE1);

	drModbus.setReadState();
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
	UCSR0B  = (1 << RXCIE0) | (0 << UDRIE0) | (0 << TXCIE0) | (0 << UCSZ02);
	UCSR0C  = (1 << UCSZ00) | (1 << UCSZ01);	//  по умолчанию
	UCSR0B  |= (1 << RXEN0) | (1 << TXEN0);

	// UART1
	// ЦПП
	// включено удвоение скорости работы
	// 1 стоп бит, 8 бит данных, контроль четности отключен
	// включен приемник и прерывание по приему
	// UBRR(U2X1 = 0) = (F_CPU / (16 * BAUD)) - 1
	// UBRR(U2X1 = 1) = (F_CPU / (8 * BAUD)) - 1
	// UBRR1 = 4,  U2X1 = 1 -> 500кбит/с при кварце 20МГц
	// UBRR1 = 21, U2X1 = 1 -> 115.2кбит/с при кварце 20МГц
	UBRR1   = 21;
	UCSR1A  = (1 << U2X1);
	UCSR1B  = (1 << RXCIE1) | (0 << UDRIE1) | (0 << TXCIE1) | (0 << UCSZ12);
	UCSR1C  = (1 << UCSZ11) | (1 << UCSZ10);
	UCSR1B  |= (1 << RXEN1) | (1 << TXEN1);

	// Таймер 1
	// режим CTC
	// частота = F_CPU / (2 * N * (1 + OCR)
	// F_CPU = 20MHz, N = 8, OCR = 2499 ->  500 Hz (или раз в 1мс)
	// F_CPU = 20MHz, N = 8, OCR = 124 -> 10000 HZ (или раз в 50мкс)
	// F_CPU = 20MHz, N = 8, OCR = 374 -> 3333.3 Hz (Или раз в 150мкс)
	TCCR1A 	= (0 << WGM11) | (0 << WGM10);
	TCCR1B 	= (0 << WGM13) | (1 << WGM12);
	TCCR1C 	= 0;	// по умолчанию
	TCNT1 	= 0;
	OCR1A 	= 374;
	TIMSK1 	= (1 << OCIE1A);
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10); // запуск с делителем 8
}

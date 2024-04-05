/*
 * dr.h
 *
 *  Created on: 19.05.2014
 *      Author: Shcheblykin
 */

#ifndef __DR_H_
#define __DR_H_

#include <stdint.h>

#define INLINE __attribute__((always_inline))

#ifdef AVR
#include <avr/io.h>
#define VOLATILE volatile
#else
#define VOLATILE
#endif

/** \brief Класс работы с ЦПП.
 *
 * При обнаружении любой ошибки в принятых пакетах ЦПП к счетчику ошибок
 * добавляется значение \a CNT_ERROR. Cодержимое данного счетчика
 * соответствует кол-ву времени в мс, на которое блокируется работа ЦПП.
 * Если значение \a error превысило порог \a ERRORS_TO_FAULT, то считаем
 * что работа ЦПП невозможна. Максимально возможное значение ошибки может
 * быть \a MAX_ERRORS.
 *
 * Изначально работа ЦПП отключена. Для запуска необходимо сменить режим
 * работы, на один из возможных: Приемник КЕДР или Передатчик КЕДР.
 *
 * @see CNT_ERROR
 * @see ERRORS_TO_FAULT
 * @see MAX_ERRORS
 */
class TDigitalRetrans
{
public:
    /// Режимы работы цифрового переприема
    enum REG_DR
    {
        REG_OFF     = 1,  ///< Выключен.
        REG_RX_KEDR = 2,  ///< Приемник КЕДР
        REG_TX_KEDR = 3   ///< Передатчик КЕДР
    };

    /// Ошибки в работе модуля ЦПП
    enum ERR_DR
    {
        ERR_NO  = 0,  ///< Ошибок нет
        ERR_OFF = 1,  ///< Модуль ЦПП выключен
        ERR_RX  = 2,  ///< Предупреждение, об ошибках в связи ЦПП
        ERR_TX  = 3   ///< Ошибка в связи ЦПП
    };

protected:
    /// Максимальное кол-во байт в послыке протокола КЕДР.
    const static uint8_t MAX_STEP = 16;
    /// Кол-во накопленных ошибок для перехода из предупреждения в аварию.
    const static uint8_t ERRORS_TO_FAULT = 100;
    /// Максимальное кол-во ошибок
    const static uint8_t MAX_ERRORS = 200;
    /// Кол-во мс добавляемых при определении ошибки.
    const static uint8_t CNT_ERROR = 5;
    /// Кол-во последовательных посылок, необходимое для принятия команды.
    const static uint8_t CNT_COM = 3;
    /// Кол-во пакетов необходимое передать для новой (не нулевой) команды
    const static uint8_t MAX_NUM_PACKG = 10;
    /// Мксимальное значение команды
    const static uint8_t MAX_NUM_COM = 32;


    VOLATILE bool   connect;  ///< Флаг наличия связи по ЦПП.
    VOLATILE REG_DR regime;   ///< Режим работы модуля ЦПП.

    // переменные необходимые для приема
    uint8_t          mCodeToCom[256];  ///< Таблица код ЦПП -> команда.
    VOLATILE uint8_t error;            ///< Флаг ошибки ЦПП.
    VOLATILE uint8_t comRx;            ///< Команда считанная по ЦПП.
    uint8_t          oldCom;           ///< Команда в последней посылке.
    uint8_t          cntPckgRx;        ///< Кол-во принятых(переданных) посылок.
    uint8_t          stepRx;           ///< Текущий шаг приема по протоколу.
    uint8_t          crcRx;            ///< Контрольная сумма полученной посылки.

    // переменные необходимые для передачи
    uint8_t          mComToCode[MAX_NUM_COM + 1][8];  ///< Таблица команда -> код ЦПП.
    VOLATILE uint8_t comTx;                           ///< Команда передваемая по ЦПП.
    uint8_t          stepTx;                          ///< Текущий шаг передачи по протоколу.
    uint8_t          cntPackgTx;                      ///< Счетчик переданных пакетов

    /**	Заполнение таблицы сответствия кода ЦПП - номер команды.
     *
     *	Используется для уменьшения времени преобразования код -> команда.
     *	Где номеру установленного бита соответствует номеру команды.
     */
    void createTableCodeToCom()
    {
        mCodeToCom[0] = 0;
        // заполнение массива значениями 0xFF, т.е. ошибкой
        for (uint16_t i = 1, temp = 0x01; i < 256; i++)
        {
            mCodeToCom[i] = 0xFF;
            temp          = (temp & 0x80) ? 0x01 : temp << 1;
        }
        // заполнение массива нужными значениями
        // т.е. массив[код команды ЦПП] = номер команды
        for (uint8_t i = 0; i < 8; i++)
        {
            mCodeToCom[(1 << i)] = i + 1;
        }
    }

    /**	Заполнение таблицы соответствиями номер команд - код ЦПП.
     *
     * 	Используется для уменьшения времени преобразования команда -> код.
     * 	Где номеру команды соответствует массив данных для передачи
     */
    void createTableComToCode()
    {
        // заполнение всего массива кодами для 0-ой команды
        for (uint8_t i = 0; i <= MAX_NUM_COM; i++)
        {
            mComToCode[i][0] = 0;
            mComToCode[i][1] = 0xFF;
            mComToCode[i][2] = 0;
            mComToCode[i][3] = 0xFF;
            mComToCode[i][4] = 0;
            mComToCode[i][5] = 0xFF;
            mComToCode[i][6] = 0;
            mComToCode[i][7] = 0xFF;
        }

        // заполнение байт команд
        for (uint8_t i = 0; i < MAX_NUM_COM; i++)
        {
            uint8_t data                       = (1 << (i % 8));
            mComToCode[i + 1][(i / 8) * 2]     = data;
            mComToCode[i + 1][(i / 8) * 2 + 1] = ~data;
        }
    }

    /**	Формирование новой посылки для передачи по ЦПП.
     *
     * 	В случае если ЦПП выключен или работает в режиме Передатчик КЕДР,
     * 	передаваемая команда всегда считается 0. Если команда есть, то
     *  необходимо передать MAX_NUM_PACKG, иначе только 1.
     *
     *  После формирования пакета номер команды на передачу сбрасывается в 0.
     *
     *  @see MAX_NUM_PACKG
     *
     *  @return Кол-во повторов на передачу для данного пакета.
     *  @retval MAX_NUM_PACKG - в случае наличия команды.
     *  @retval 1 - в случае нулевой команды или ошибки.
     */
    INLINE uint8_t crtTxNewData()
    {
        uint8_t num = 1;
        uint8_t com = comTx;

        if (com != 0)
        {
            comTx = 0;

            // проверка на допустимый диапазон значений для команды
            if (com > MAX_NUM_COM)
                com = 0;

            // команда передается по ЦПП только в режиме Приемник КЕДР
            if (regime != REG_RX_KEDR)
                com = 0;

            // для команды формируется MAX_NUM_PACKG кол-во пакетов передаваемых
            // в ЦПП, иначе только 1
            if (com != 0)
                num = MAX_NUM_PACKG;
        }

        uint8_t size = sizeof(mComToCode[0]) / sizeof(mComToCode[0][0]);
        for (uint8_t i = 0; i < size; i++)
        {
            bufTx[i + 2] = mComToCode[com][i];
        }

        comTx = 0;
        return num;
    }

    /**	Сброс работы протокола по приему и передаче.
     *
     *	Переменные для работы протокола по приему и передаче устанавливаются
     *	в исходное состояние.
     *
     *	Принятые команды по ЦПП и команда на передачу по ЦПП сбрасывается.
     *	Если в момент сброса посылка принималась или(и) передвалась, она будет
     *	прервана.
     *
     *	Для работы приемника будет установлено мимнимальное время ошибки \a
     *	CNT_ERROR.
     *
     *	@see CNT_ERROR
     */
    INLINE void resetProtocol()
    {
        connect = false;

        error     = CNT_ERROR;
        comRx     = 0;
        cntPckgRx = 0;
        stepRx    = 0;
        crcRx     = 0;
        oldCom    = 0;


        comTx      = 0;
        stepTx     = MAX_STEP;
        cntPackgTx = 0;
    }

    uint8_t bufRx[MAX_STEP + 5];  ///< Буфер приемника.
    uint8_t bufTx[MAX_STEP + 5];  ///< Буфер передатчика.

public:

    /**	Конструктор.
     *
     * 	Устанавливается исходное состояние работы протокола приема и передачи.
     * 	Массивы приема/передачи заполняются константными значениями.
     * 	Заполняются массивы преобразования код <-> команда.
     * 	Режим работы по умолчанию - \a REG_OFF.
     *
     *	@see REG_OFF
     */
    TDigitalRetrans()
    {
        regime = REG_OFF;

        resetProtocol();

        // заполнение буфера приема синхробайтами
        bufRx[0] = 0xAA;
        bufRx[1] = 0xAA;

        // заполнение буфера для передачи
        bufTx[0] = 0xAA;  // синхробайты
        bufTx[1] = 0xAA;
        // ... 8 байт данных
        bufTx[10] = 0x00;  // резервный байты
        bufTx[11] = 0x00;
        bufTx[12] = 0x00;
        bufTx[13] = 0x00;
        bufTx[14] = 0x00;
        bufTx[15] = 0x50;  // контрольная сумма

        createTableComToCode();
        createTableCodeToCom();
    }

    /**	Проверка наличия связи по ЦПП в текущий момент.
     *
     * 	При отсутствии полученных посылок по ЦПП, с момента последнего вызова
     * 	данной функции, будет установлена ошибка приема ЦПП.
     *
     * 	Необходимо выбрать период вызова данной функции с запасом, например
     * 	такой чтобы за это время могло приняться пара посылок по ЦПП.
     *
     */
    INLINE void checkConnect()
    {
        if (!connect)
        {
            setError();
        }
        connect = false;
    }

    /**	Проверка наличия ошибок в принятых пакетах ЦПП.
     *
     * 	Проверяет наличие хотя бы одной ошибки в работе ЦПП.
     *
     * 	@retval True - есть ошибки.
     * 	@retval False - ошибок нет.
     */
    INLINE bool isWarning() const { return (error > 0); }

    /** Проверка ошибки работы ЦПП.
     *
     * 	Проверяет наличие критического кол-ва ошибок в работе ЦПП.
     *
     * 	@retval True - ЦПП работает с ошибками, False - иначе.
     */
    INLINE bool isError() const { return (error >= ERRORS_TO_FAULT); }

    /** Возвращает текущее значение неисправности ЦПП.
     *
     *	Если модуль ЦПП отключен, возвращается ошибка \a ERR_OFF.
     *
     *	В зависимости от текущего режима работы ЦПП при превышении допустимого
     *	количества ошибок в работе ЦПП возвращаются разные коды ошибок ЦПП. Для
     *	работы в качестве Приемника КЕДР это будет предупреждение \a ERR_RX, а
     *	для Передатчика КЕДР - ошибка \a ERR_TX.
     *
     * 	@see ERR_DR
     * 	@see REG_DR
     *
     * 	@return Текущий код ошибки или ERR_NO при их отсутствии.
     */
    INLINE uint8_t getError()
    {
        ERR_DR err = ERR_NO;

        switch (regime)
        {
        case REG_OFF: err = ERR_OFF; break;
        case REG_TX_KEDR:
            if (isError())
            {
                err = ERR_TX;
            }
            break;
        case REG_RX_KEDR:
            if (isError())
            {
                err = ERR_RX;
            }
            break;
        }

        return static_cast<uint8_t>(err);
    }

    /** Установка наличия ошибки в принятых пакетах ЦПП.
     *
     * 	Если значение счетчика ошибок не превышает \a (MAX_ERROR - CNT_ERROR),
     * 	к счетчику добавляется \a CNT_ERROR ошибок. При этом сбрасывается
     * 	счетчик принятых команд и предыдущая команда.
     *
     * 	@see MAX_ERRORS
     * 	@see CNT_ERROR
     */
    INLINE void setError()
    {
        if (error <= (MAX_ERRORS - CNT_ERROR))
        {
            error += CNT_ERROR;
        }
        cntPckgRx = 0;
        oldCom    = 0;
    }

    /** Уменьшение кол-ва ошибок в принятых пакетах ЦПП.
     *
     * 	Если значение счетчика больше нуля, то оно уменьшается на единицу.
     */
    INLINE void decError()
    {
        if (error > 0)
        {
            error--;
        }
    }

    /** Сбрасывает кол-во ошибок в принятых пакетах ЦПП.
     *
     */
    INLINE void clrError() { error = 0; }

    /**	Установка нового режима работы.
     *
     *  В случае ошибочного значения, посылка будет проигнорирована.
     *  В случае смены режима, работа протокола по приему и передаче будет
     *  сброшена в исходное состояние.
     *
     *	@see REG_DR
     *  @param val Новый режим работы.
     */
    INLINE void setRegime(uint8_t val)
    {
        if (val != 0)
        {
            REG_DR reg = static_cast<REG_DR>(val);
            switch (reg)
            {
            case REG_OFF:
            case REG_RX_KEDR:
            case REG_TX_KEDR:
                {
                    if (reg != regime)
                    {
                        regime = reg;
                        resetProtocol();
                    }
                    break;
                }
            }
        }
    }

    /** Отключение ЦПП.
     *
     * 	Режим работы изменяется на \a REG_OFF.
     */
    void disable() { setRegime(REG_OFF); }

    /** Запись команды для передачи по ЦПП.
     *
     *	В случае если еще не передана предыдущая команда или данная команда
     *	выходит за допустимый диапазон, т.е. больше \a MAX_NUM_COM, запись
     *	новой команды не произойдет.
     *
     *	@see MAX_NUM_COM
     *	@param val Номер команды для передачи по ЦПП.
     *	@return True - успешная запись команды, False - иначе.
     */
    INLINE bool setCom(uint8_t val)
    {
        bool state = false;
        if (val != 0)
        {
            switch (regime)
            {
            case REG_RX_KEDR:
                if ((comTx == 0) && (val <= MAX_NUM_COM))
                {
                    comTx = val;
                    state = true;
                }
                break;
            case REG_OFF:
            case REG_TX_KEDR: break;
            }
        }
        return state;
    }

    /**	Возвращает номер принятой команды.
     *
     * 	Номер команды может быть от 0 до \a MAX_NUM_COM.
     * 	Если ЦПП выключен или работает в режиме Приемник КЕДР, то возвращается
     * 	всегда 0.
     *	Если ЦПП работает с ошибкой, команда на приеме сбрасываетя в 0.
     *
     *	@see REG_DR
     *	@see MAX_NUM_COM
     *
     * 	@return Номер команды [0, MAX_NUM_COM].
     */
    INLINE uint8_t getCom()
    {
        uint8_t com = 0;

        if (isError())
        {
            comRx = 0;
        }

        switch (regime)
        {
        case REG_TX_KEDR: com = comRx; break;
        case REG_OFF:
        case REG_RX_KEDR: break;
        }

        return com;
    }

    /**	Проверка наличия команды в принятом пакете ЦПП.
     *
     * 	Проверяются прямые байты команд на наличие в них команды. В пакете
     * 	должно быть не более одной команды.
     * 	Если команда обнаружена (в том числе и нулевая), ее номер будет записан
     * 	в \a comRx.
     *
     * 	@see comRx
     */
    INLINE void checkCommand()
    {
        uint8_t newcom = 0;

        for (unsigned char i = 2, tCom = 0; i < 10; i += 2, tCom += 8)
        {
            uint8_t temp = mCodeToCom[bufRx[i]];
            // если число не подходящее - вернуть ошибочный код
            if (temp > 8)
            {
                // в текущем байте обнаружено более одной команды
                newcom = 0xFF;
                break;
            }
            else if (temp != 0)
            {
                if (newcom == 0)
                {
                    newcom = tCom + temp;
                }
                else
                {
                    // в посылке обнаружено более одной команды
                    newcom = 0xFF;
                    break;
                }
            }
        }

        // проверка номера команды на максимум
        if (newcom > MAX_NUM_COM)
        {
            setError();
        }

        // запись принятой команды, в случае если отсутствуют ошибки в работе
        // ЦПП и кол-во принятых пакетов с этой командой не менее CNT_COM.
        if (!isWarning())
        {
            if (oldCom == newcom)
            {
                if (cntPckgRx < CNT_COM)
                {
                    cntPckgRx++;
                    if (cntPckgRx >= CNT_COM)
                    {
                        comRx = newcom;
                    }
                }
            }
            else
            {
                oldCom    = newcom;
                cntPckgRx = 1;
            }
        }
    }

    /**	Проверка принятого байта данных на соответствие протоколу.
     *
     *	Если установлен флаг наличия ошибки в \a status, протокол
     *	сбрасывается и начинается поиск новой посылки. В случае обнаружения
     *	ошибки в самом протоколе, также происходит сброс протокола.
     *
     *	Если пакет принят полностью и ошибок нет, вызывается функция поиска
     *	команды в принятых данных.
     *
     * 	Протокол КЕДР:
     *	-# 	@b 0хАА - первый синхробайт
     *	-# 	@b 0хАА - второй синхробайт
     *	-# 	@b b1 - прямой байт, 		команды 1..8
     *	-# 	@b i1 - инверсный байт, 	команды 1..8
     *	-#  @b b2 - прямоо байт, 		команды 9..16
     *	-#  @b i2 - инверсный байт,		команды 9..16
     *	-#  @b b3 - прямоо байт, 		команды 17..24
     *	-#  @b i3 - инверсный байт,		команды 17..24
     *	-#	@b b4 - прямоо байт, 		команды 25..32
     *	-#	@b i4 - инверсный байт, 	команды 25..32
     *	-# 	@b 0x00 - резерв
     *	-# 	@b 0x00 - резерв
     *	-# 	@b 0x00 - резерв
     *	-# 	@b 0x00 - резерв
     *	-# 	@b 0x00 - резерв
     *	-#	@b crc - контрольная сумма, равная младшему байту суммы всех данных,
     *	c учетом синхробайт.
     *
     *	@param byte Байт данных.
     *	@param status Статус источника данных, True - ошибка.
     */
    INLINE void checkByteProtocol(uint8_t byte, bool status)
    {
        // проверка полученного байта по протоколу
        if (status)
        {
            stepRx = MAX_STEP;
        }
        else
        {
            bufRx[stepRx] = byte;
            if (stepRx == 0)
            {
                // первый синхробайт 0хAA
                if (byte == 0xAA)
                {
                    stepRx = 1;
                    crcRx  = byte;
                }
            }
            else if (stepRx == 1)
            {
                // второй синхробайт 0хАА
                // при этом устанавливается флаг наличия связи
                if (byte == 0xAA)
                {
                    stepRx  = 2;
                    connect = true;
                    crcRx += byte;
                }
                else
                {
                    stepRx = 0;
                }
            }
            else if (stepRx <= 9)
            {
                // 4 прямой + инверсный байта команд
                // прямой байт записывается в буфер
                // инверсный сравнивается с ним
                crcRx += byte;
                if ((stepRx % 2) == 0)
                {
                    stepRx++;
                }
                else
                {
                    if ((byte ^ bufRx[stepRx - 1]) == 0xFF)
                    {
                        bufRx[stepRx++] = byte;
                    }
                    else
                    {
                        // ошибка, прямой байт команды не равен инверсному
                        stepRx = MAX_STEP;
                    }
                }
            }
            else if (stepRx <= 14)
            {
                // 5 резервных байт данных
                crcRx += byte;
                stepRx++;
            }
            else if (stepRx == 15)
            {
                // проверка контрольной суммы
                if (byte == crcRx)
                {
                    stepRx = 0;
                    checkCommand();
                }
                else
                {
                    // ошибка контрольной суммы
                    stepRx = MAX_STEP;
                }
            }
        }

        // в случае ошибки обработки протокола
        // возвращаемся к поиску синхробайт и устанавливаем ошибку
        if (stepRx >= MAX_STEP)
        {
            stepRx = 0;
            setError();
        }
    }

    /**	Передача следующего байта по ЦПП.
     *
     *	После передачи \a MAX_NUM_PACKG посылок для команды, формируется посылка
     *	для следующей команды.
     *
     *	@see MAX_NUM_PACKG
     *
     * 	@return Следующий байт на передачу.
     */
    INLINE uint8_t getTxByte()
    {
        if (stepTx >= MAX_STEP)
        {
            if (cntPackgTx == 0)
            {
                // формирование новой посылки на передачу
                cntPackgTx = crtTxNewData();
            }
            stepTx = 0;
            cntPackgTx--;
        }

        return bufTx[stepTx++];
    }
};

#endif /* __DR_H_ */

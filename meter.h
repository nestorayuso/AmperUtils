#ifndef METER_H
#define METER_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TARIFF_MAX (1+4)
#define PHASE_MAX (1+3)
#define ARCH_DATA_MAX (2)
#define ARCH_DATA_PARAM_COUT_MAX (16)
#define ARCH_DATA_PROFILE_PARAM_COUT_MAX (4)

typedef enum {
    MDT_P_p             = 0x01,// активная + итератор
    MDT_P_p_d           = 0x11,// активная + дельта
    
    MDT_P_n             = 0x02,// активная - итератор
    MDT_P_n_d           = 0x12,// активная - дельта
    
    MDT_Q_p             = 0x03,// реактивная + итератор
    MDT_Q_p_d           = 0x13,// реактивная + дельта
    
    MDT_Q_n             = 0x04,// реактивная - итератор
    MDT_Q_n_d           = 0x14,// реактивная - дельта
    
    MDT_S               = 0x05,// суммарная итератор
    MDT_S_d             = 0x15,// суммарная дельта
    
    MDT_Q1              = 0x25,// реактивная квандранта 1 итератор
    MDT_Q2              = 0x26,// реактивная квандранта 2 итератор
    MDT_Q3              = 0x27,// реактивная квандранта 3 итератор
    MDT_Q4              = 0x28,// реактивная квандранта 4 итератор
    
    MDT_U               = 0x33,// напряжение межфазное
    MDT_Ulin            = 0x34,// напряжение между линиями 1-2 2-3 3-1
    MDT_I               = 0x35,// ток
    MDT_F               = 0x36,// частота
    MDT_cosFi           = 0x37,// cosFi
} METER_DATA_TYPE;

typedef enum {
    ME_update_fw        = 0x01, // обновление прошивки
    ME_back_fw          = 0x02, // возврат к заводской прошивки
    
    ME_update_rts       = 0x09, // установка времени на
    ME_update_rts_old   = 0x0A, // перевод времени с
    
    ME_tariff_new       = 0x11, // смена тарифного расписания
    ME_k_new            = 0x12, // смена коэффициента трансформации
    ME_quality_set_new  = 0x13, // смена величины параметров качества электрической энергии

    ME_btn_cover_up     = 0x21, // вскрытие клемной крышки
    ME_btn_cover_down   = 0x22, // закрытие клемной крышки
    ME_btn_body_up      = 0x23, // вскрытие корпуса
    ME_btn_body_down    = 0x24, // закрытие корпуса
    ME_over_magnetic_field_start = 0x25, // начало    воздействия сверхнормативного магнитного воздействия (цитата:) )
    ME_over_magnetic_field_stop  = 0x26, // окончание воздействия сверхнормативного магнитного воздействия (цитата:) )

    ME_over_load        = 0x31, // превышение нагрузки (отключение реле - разрыв цепи)
    ME_under_load       = 0x32, // возвращение нагрузки после вревышения (включение реле - замыкание цепи)

    ME_clear_values     = 0x39, // сброс показаний (интеграторов)
    ME_clear_arch_data1 = 0x3A, // очистка архива данных 1
    ME_clear_arch_data2 = 0x3B, // очистка архива данных 2
    ME_clear_arch_event = 0x3C, // очистка журнала событий
    
    ME_set_arch_data1   = 0x41, // новые настройки для журнала данных 1
    ME_set_arch_data2   = 0x42, // новые настройки для журнала данных 2
    
    //качество сети
    ME_max_U1_start     = 0x50, // начало     превышения максимума напряжения фазы 1
    ME_max_U1_stop      = 0x51, // завершение превышения максимума напряжения фазы 1
    ME_min_U1_start     = 0x52, // начало     превышения минимума  напряжения фазы 1
    ME_min_U1_stop      = 0x53, // завершение превышения минимума  напряжения фазы 1
    ME_max_U2_start     = 0x54, // начало     превышения максимума напряжения фазы 2
    ME_max_U2_stop      = 0x55, // завершение превышения максимума напряжения фазы 2
    ME_min_U2_start     = 0x56, // начало     превышения минимума  напряжения фазы 2
    ME_min_U2_stop      = 0x57, // завершение превышения минимума  напряжения фазы 2
    ME_max_U3_start     = 0x58, // начало     превышения максимума напряжения фазы 3
    ME_max_U3_stop      = 0x59, // завершение превышения максимума напряжения фазы 3
    ME_min_U3_start     = 0x5A, // начало     превышения минимума  напряжения фазы 3
    ME_min_U3_stop      = 0x5B, // завершение превышения минимума  напряжения фазы 3
    ME_max_F_start      = 0x5C, // начало     превышения максимума частоты
    ME_max_F_stop       = 0x5D, // завершение превышения максимума частоты
    ME_min_F_start      = 0x5E, // начало     превышения минимума  частоты
    ME_min_F_stop       = 0x5F, // завершение превышения минимума  частоты
    
    ME_power_on         = 0x71, // включение счетчика
    ME_power_off        = 0x72, // отключение счетчика
    ME_power_up_f1      = 0x73, // подключена фаза 1 отсутствие напряжения при наличии тока в измерительных цепях
    ME_power_down_f1    = 0x74, // отключена  фаза 1 отсутствие напряжения при наличии тока в измерительных цепях
    ME_power_up_f2      = 0x75, // подключена фаза 2 отсутствие напряжения при наличии тока в измерительных цепях
    ME_power_down_f2    = 0x76, // отключена  фаза 2 отсутствие напряжения при наличии тока в измерительных цепях
    ME_power_up_f3      = 0x77, // подключена фаза 3 отсутствие напряжения при наличии тока в измерительных цепях
    ME_power_down_f3    = 0x78, // отключена  фаза 3 отсутствие напряжения при наличии тока в измерительных цепях
    
    ME_direction_I1     = 0x79, // смена направления тока фазы 1
    ME_direction_I2     = 0x7A, // смена направления тока фазы 2
    ME_direction_I3     = 0x7B, // смена направления тока фазы 3
} METER_EVENT;

typedef enum {
    METER_TIME_ALIGMENT_not = 0,
    METER_TIME_ALIGMENT_min = 1,
    METER_TIME_ALIGMENT_hour= 2,
    METER_TIME_ALIGMENT_day = 3,
} METER_TIME_ALIGMENT;      // для настроек выравнивания архивов

#pragma pack(push, 1)
typedef struct {
    uint32_t timestamp;                         // время начала отсчета данных для записи
    union {                                     // данные для записи в архив
//        uint32_t u32;
        float    f32;
    } value [ARCH_DATA_PARAM_COUT_MAX];         // значение
    uint8_t  reserve[3];
    uint8_t  crc;                               // контрольная сумма всегда последний байт
} METER_ARCH_DATA;    // архив с данными

typedef struct {
    uint32_t timestamp;                         // время начала отсчета данных для записи
    union {                                     // данные для записи в архив
//        uint32_t u32;
        float    f32;
    } value [ARCH_DATA_PROFILE_PARAM_COUT_MAX]; // значение
    uint8_t  reserve[3];
    uint8_t  crc;                               // контрольная сумма всегда последний байт
} METER_ARCH_PROFILE_DATA;    // архив с профилями

typedef struct {
    uint32_t timestamp;                         // время события
    uint8_t  event;                             // id события
    uint8_t  reserve[2];
    uint8_t  crc;                               // контрольная сумма всегда последний байт
} METER_ARCH_EVENT;   // архив с событиями
#pragma pack(pop)

//#pragma pack(push, 1)

typedef struct {
    uint64_t P_p;   // A+ активная энергия прямого направления
    uint64_t P_n;   // A- активная энергия обратного направления
    uint64_t Q_p;   // R+ реактивная энергия прямого направления
    uint64_t Q_n;   // R- реактивная энергия обратного направления
    uint64_t S;     //    полная энергия
    
    uint64_t Q1;    // R1 реактивная энергия 1-го квадранта
    uint64_t Q2;    // R2 реактивная энергия 2-го квадранта
    uint64_t Q3;    // R3 реактивная энергия 3-го квадранта
    uint64_t Q4;    // R4 реактивная энергия 4-го квадранта
} METER_INTEGRATORS;// интеграторы

typedef struct {
    int32_t P;      // активная энергия
    int32_t Q;      // реактивная энергия
    int32_t S;      // полная энергия
    float   U;      // напряжение фазное RMS
    float   Ulin;   // напряжение линейное RMS
    float   I;      // ток RMS
    float   F;      // частота сети
    float   cosFi;  // cosFi
} METER_INSTANTS;   // текущие значения

typedef struct {
    uint8_t MDT;    // METER_DATA_TYPE
    uint8_t tariff; // тариф, 0-сумма
    uint8_t phase;  // фаза, 0-сумма
    uint8_t reserv;
} METER_ARCH_PARAM_CFG; // настройки параметра архива

typedef struct {
    uint32_t timeout;                   // время за которое происходит измерение
    uint8_t  start_at_time_alignment;   // флаг необходтмости выравнивания времени
    uint8_t  count_param;               // кол-во параметров
    METER_ARCH_PARAM_CFG param_id[ARCH_DATA_PARAM_COUT_MAX];
} METER_ARCH_DATA_SETTINGS;   // настройки архива с данными

typedef struct {
    float    nominal;       // номинальное значение // = 50 Hz // 220 V
    uint32_t time_integral;// время усреднения сек
    float    max;           // верхняя граница
    float    min;           // нижняя граница
} METER_QUALITY_SETTINGS; // настройки архива с качеством

typedef struct {
    float    value_to_aver; // усредняемое значение (сумма)
    uint32_t count_to_aver; // количество сумм
    float    value_d;       // отклонение
    uint32_t flag;          // флаг лимита 0=норма 1=превышение 2=провал
    float    value_limit;   // максимально достигнутое значение параметра
} METER_QUALITY;    // текущие значения качества

typedef struct {
    METER_INSTANTS phase[PHASE_MAX];
    struct {
        float Pi;  // активная   мощность 
        float Qi;  // реактивная мощность
        float Si;  // полная     мощность
    } to_display[PHASE_MAX]; // для отображения на дисплее
} METER_NEW_DATA;

typedef struct {
    uint32_t timestamp;                                 // время последнего вычисления данных
    METER_INTEGRATORS itgr[TARIFF_MAX][PHASE_MAX];      // интеграторы по сумме тарифов + тарифам
    METER_NEW_DATA curr;                                // текущие значения (для отображения на экране)
    METER_ARCH_DATA_SETTINGS aDataSet[ARCH_DATA_MAX];   // настройки архивов с данными
    METER_ARCH_DATA aData[ARCH_DATA_MAX];               // архив с данными
    METER_QUALITY_SETTINGS mQualitySet_F;               // настройки качества // частота
    METER_QUALITY_SETTINGS mQualitySet_U;               // настройки качества // напряжение фазное
    METER_QUALITY mQuality_F;                           // архив с качеством // частота
    METER_QUALITY mQuality_U[PHASE_MAX];                // архив с качеством // напряжение фазное
} METER_DATA;

//#pragma pack(pop)

typedef void (*METER_update)(uint32_t timestamp);
//typedef float (*METER_i32_to_f32)(int32_t data);
typedef uint8_t (*METER_tariff_from_time)(uint32_t timestamp);
typedef void (*METER_event_data)(uint32_t data);
typedef void (*METER_event)(METER_EVENT me, uint32_t timestamp, uint16_t val);
typedef struct {
    METER_update update;                // пересчитывает показания из curr
    float K;
//    METER_i32_to_f32 conv_energ;        // преобразует энергию
    METER_tariff_from_time tariff_conv; // получает тариф из времени
    METER_event_data event_arch_data;   // добавление записи в архив
    METER_event event_arch;             // добавление записи в журнал событий
    METER_DATA * data;
} METER;

extern METER * meter;

void METER_init();

#ifdef __cplusplus
}
#endif

#endif // METER_H

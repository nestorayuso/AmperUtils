#ifndef AMPER_DEV_H
#define AMPER_DEV_H

#include "dti_device.h"
#include "meter.h"


//Energy type defines
#define ACTIVE_ENERGY           0
#define REACTIVE_ENERGY         1
#define APPARENT_ENERGY         2

typedef struct {
  quint8             DTI_address;
  quint8             Relay_OFF;
  float              Power_limit;
  quint8             Pulse0_energy_type;
  quint8             Pulse1_energy_type;
  quint16            Pulse0_freq;
  quint16            Pulse1_freq;
  quint32            ProtC_ID;
  quint8             Password[10];
}settings_s;


typedef struct {
  quint8       TarifTitleTotal;
  quint8       TarifZoneTotal;

  quint8      crc;
}tarif_settings_s;


typedef enum {
  EMPTY_RATE = 0
  , NIGHT_ZONE_RATE = 0x10000001            //T1
  , PEAK_ZONE_RATE = 0x10000002             //T2
  , HALF_PEAK_ZONE_RATE = 0x10000003        //T3
  , TOTAL_RATE = 0x10000004                 //Total
}RateZoneType_e;

#define TITLE_NAME_LEN  8
typedef struct {
  RateZoneType_e    type;
  quint8       name[TITLE_NAME_LEN];
}TarifTitle_s;

typedef struct {
  time_t	start;	// Time of start, two elements in case one tarif happens twice a day
  time_t	end;	// End time
}TimeZone_s;


typedef struct {
  RateZoneType_e    title;
  TimeZone_s    time_zone;
}Tarif_s;


typedef struct {
  quint32      EnGain;
  float         WattH;

  quint8      crc;
}calibrate_settings_s;


//#pragma pack(push, 1)
typedef struct {
  quint8    name[TITLE_NAME_LEN];		// Name tag

  float 	Active_P;
  float 	Active_N;
  float 	Reactive_P;
  float 	Reactive_N;
  float     Apparent;
}Adder_s;
//#pragma pack(pop)

typedef struct {
  RateZoneType_e        type;
  Adder_s               Adders;
}BindAdder_s;


typedef struct {
  float         apower_calc;    // active power
  float         rpower_calc;    // reactive power
  float         aparpower_calc; //apparent power
  float         vrms;		// Voltage RMS
  float         irms;		// Current RMS
  float         pfreq;          //Регистр периода
  float         angle;          // Angle between I and U
  int32_t       aenergy;
  int32_t       renergy;
  int32_t       apenergy;
}ADE_Inst_Reg;

typedef struct {
  quint32   aigain;
  quint32   avgain;
  quint32   awgain;
  quint32   avargain;
  quint32   avagain;
  quint32   airmsos;
  quint32   vrmsos;
  quint32   awattos;
  quint32   avaros;
  quint32   avaos;
//  quint32   reseved[4];
}ADE_Cal_Reg;

typedef struct {
  quint16       ver;
  quint16      start_add;
  quint16      end_add;
  quint16      crc;
  quint16      reserved[2];
  quint16      crc_of_this_struct;
}soft_update;




#define UPDATE_ADD  0x70000

typedef enum {
  NO_EVENT                      = 0
  , RTC_SECOND_EVENT            = 1
  , SCHEDULE_EVENT              = 2
  , MODEM_RECV_EVENT            = 3
  , TIME_SYNC_EVENT             = 4
  , SET_PWR_LIMIT_EVENT         = 5
  , LIMITATION_EVENT            = 6

  , SEND_DAY_REGULAR_EVENT      = 7
  , SEND_CURR_PARAM_EVENT       = 8
  , SEND_POST_MSG_EVENT         = 9

  , PARSE_RECV_EVENT            = 10
  , DAILY_STATISTIC_EVENT       = 11
  , QUALITY_STATISTIC_EVENT     = 12
  , LD_PROFILE_STATISTIC_EVENT  = 13

  , PWR_ONLINE_EVENT            = 14

  , DISPLAY_EVENT               = 15
  , DISPLAY_NEXT_EVENT          = 16
  , DISPLAY_DRAW_EVENT          = 17
  , DISPLAY_OFF_EVENT           = 18
  , DISPLAY_ON_EVENT            = 19
  , DISPLAY_FULL_DISPLAY_EVENT  = 20
  , DISPLAY_NORMAL_DISPLAY_EVENT= 21

  , RED_BUTTON_EVENT            = 22
  , BLUE_BUTTON_EVENT           = 23
  , BLUE_BUTTON_RELEASE_EVENT   = 24

  , INIT_COVER_EVENT            = 25
  , INIT_MAGNET_EVENT           = 26
  , DEINIT_COVER_EVENT          = 27
  , DEINIT_MAGNET_EVENT         = 28
  , DEINIT_MAGNET_COVER_EVENT   = 29

  , STRIJ_LED_ON_EVENT          = 30
  , STRIJ_LED_SHORT_ON_EVENT    = 31
  , STRIJ_LED_OFF_EVENT         = 32

  , MODEM_DROP_STATUS_EVENT     = 33

  , AXEM_BUSY_EVENT             = 34
  , AXEM_ASK_EVENT              = 35
  , AXEM_NACK_EVENT             = 36
  , PULSE_CALIBRATION_EVENT     = 37

  , WATCH_DOG_EVENT             = 38

  , LEUART_RECV_EVENT           = 39
  , RS_485_CMD_EVENT            = 40

  , STORE_NEW_TARIF             = 41
  , SEND_TARIFS_EVENT           = 42
  , RELAY_ON                    = 43
  , RELAY_OFF                   = 44
  , SAVE_SETTINGS               = 45
  , REQUEST_ID                  = 46
  , RED_BUTTON_RELEASE_EVENT    = 47
  , SET_FAST_DL                 = 48
  , RESET_FAST_DL               = 49
  , UPDATE_CHECK                = 50
  , CLEAR_UPDATE_AREA           = 51

} mQueueType_e;



#define MAX_TARIF_TITLES    4
#define MAX_TARIF_ZONES     12


class amper_dev : public DTI_device
{
public:
    amper_dev(DTI_interface *interface = 0);

    bool getTarifData();
    bool setTarifData();
    bool runEvent(mQueueType_e event, bool broadcast = false);
    bool erase4kblock(quint32 add) {return runMethod("Erase_4k", add);}
    bool erase64kblock(quint32 add) {return runMethod("Erase_64k", add);}
    bool getAmperDateTime(QDateTime *time);
    bool setAmperDateTime(QDateTime time);
    bool saveAllSettings(const char *filename);
    bool restoreAllSettings(const char *filename);
    bool setSettings();
    bool getSettings();
    bool getAddersData();
    bool getCurrentData();
    bool getADECals(int phase);
    bool setADECals(int phase);
    bool setDTI_address(quint8 add);
    bool setProtC_address(quint32 add);
    bool changePassword(QByteArray password);
    //bool writeFirmware(soft_update* update_struct, const char *filename);
    quint16 CRC_calc(char *data, quint32 size);



//private:
    quint8 N_of_Titles , N_of_Zones;
    settings_s  Settings;
    TarifTitle_s pTitle[MAX_TARIF_TITLES];
    Tarif_s pTarif[MAX_TARIF_ZONES];
    BindAdder_s pBindAdder[3];
    bool is_pBindAdder;
    ADE_Inst_Reg ADE_Inst;
    ADE_Cal_Reg ADE_Cal;

    METER_DATA meter_data;
};

#endif // AMPER_DEV_H

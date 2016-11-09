#ifndef DTI_H
#define DTI_H

#include <QSerialPort>
#include <QSettings>
#include <QDateTime>
#include <QMutex>
#include <stdint.h>

#include "DTI_url.h"

typedef enum
{
  ERR_SUCCESS = 0,
  ERR_QSERIAL = 1,
  ERR_BAD_PAKET = 2,
  ERR_READ_TIMOUT = 3,
  ERR_BAD_LRC = 4,
  ERR_REMOTE_UNKN_COMM = 5,
  ERR_REMOTE_WRITE_PROTECT = 6,
  ERR_REMOTE_NO_PROPERTY = 7,
  ERR_PORT_LOCKED = 8,
  ERR_REMOTE_OUT_OF_MEMORY = 9,
  ERR_REMOTE_NOT_SUPPORTED = 10,
  ERR_INVALID_PASSWORD = 11,
  ERR_ACCESS_DENIED = 12

} DTIError;

typedef struct
{
  quint8 name[10];
  quint16 id;
  quint8 type;
  quint32 address;
  quint16 parent_id;
}  DTI_Property;

//#pragma pack(push, 1)
//typedef struct {
//  uint32_t address;
//  uint16_t id;
//  uint16_t parent_id;
//  uint8_t type;
//  uint8_t name[11];
//}  DTI_Property; // 20 bytes
//#pragma pack(pop)


#define  UINT8_TYPE             0
#define  INT8_TYPE              1
#define  UINT16_TYPE            2
#define  INT16_TYPE             3
#define  UINT32_TYPE            4
#define  INT32_TYPE             5
#define  UINT8_MAS_TYPE         6
#define  INT8_MAS_TYPE          7
#define  UINT16_MAS_TYPE        8
#define  INT16_MAS_TYPE         9
#define  UINT32_MAS_TYPE        10
#define  INT32_MAS_TYPE         11
#define  STRUCT_TYPE            12
#define  FUNCTION_TYPE          13
#define  TIME_TYPE              14
#define  FLOAT_TYPE             15

#define  SEGMENT_RAM_RO         0
#define  SEGMENT_RAM_RW         (1<<6)
#define  SEGMENT_EE_RO          (2<<6)
#define  SEGMENT_EE_RW          (3<<6)


#define SLIP_START  0xDD
#define SLIP_END    0xDE
#define SLIP_ESC    0xDF

#define SLOW_DL 0x03
#define FAST_DL 0x43

#define SLIP_DRX 0x0
#define SLIP_CRX 0x1
#define SLIP_FASTDL 0x2

#define PASSWORD_MAX_LEN    9


class DTI_interface
{
public:
    DTI_interface(DTI_url * dti_url);
    ~DTI_interface();

    QSerialPort::SerialPortError openport();
    void closeport();
    bool checkAdd(quint8 dti_add);
    bool readData(quint8 dti_add, quint32 mem_add, quint16 len, void* Data);
    bool readInfo(quint8 dti_add, quint32 mem_add, quint16 len, void* Data);
    bool writeData(quint8 dti_add, quint32 mem_add, quint16 len, void* Data);
    bool getPropertyByName(quint8 dti_add, const char *name, DTI_Property *prop);
    bool runMethod(quint8 dti_add, quint16 method_id, quint32 param);
    DTIError error() {return last_error;}
    QString error_message();
    void setSLIP(bool slip = true) {isSLIP = slip;}
    bool SLIP_Send(quint8 cmd, quint8 *payload, quint8 len);
    bool SLIP_SetRxMode(quint8 DL_type, quint8 mode);
    bool isReady() {return ready;}
    bool logIn(quint8 dti_add, const char* password);
    bool logOut(quint8 dti_add);

    bool url_set_last_timestamp();
    QVariantList url_get_last_UL();
    bool url_send_DL(QByteArray arr_send);
protected:
    quint8 HexAsciiToChar(quint8 a, quint8 b);
    void CharToHexAscii(quint8 c, quint8 *h1,  quint8 *h2);
    quint8 Tele_CalcLRC(quint8* Mas, qint32 Num);
    bool MakeTry(quint8* SendString, quint16 SendNum, quint8* ReceiveString, quint16 *ReceiveNum);
    bool MakeTry_old(quint8* SendString, quint16 SendNum, quint8* ReceiveString, quint16 *ReceiveNum);
    bool MakeTry_new(quint8* SendString, quint16 SendNum, quint8* ReceiveString, quint16 *ReceiveNum);
    quint8 SLIP_Receive(QByteArray read);
public:
    bool no_wait;
    quint16 ReadTimeout;
    quint16 TryNum;
    quint16 StrWriteDelay;

    QString url_api_key;
    quint32 dev_id;
    quint32 server_last_timestamp;
private:
    DTI_url * dti_url;
    QSerialPort *serial;
    QSettings *pSettings;


    DTIError last_error;
    //QMutex	m_mutex;
    bool locker_state;
    bool locker_lock() {if (locker_state) return false; else {locker_state = true; return true;} }
    void locker_unlock() {locker_state = false;}

    bool isSLIP;
    bool ready;
};

#endif // DTI_H

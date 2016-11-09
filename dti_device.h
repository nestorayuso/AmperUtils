#ifndef DTI_DEVICE_H
#define DTI_DEVICE_H

#include <QMap>
#include "dti.h"



typedef struct
{
  quint8 name[16];
  quint16 hardware_ID;
  quint8  hardware_rev;
  quint8  software_rev;
  quint8  software_subrev;
  quint16 packet_size;
  quint32 RAM_RO_startadd;
  quint32 RAM_RW_startadd;
  quint32 EE_RO_startadd;
  quint32 EE_RW_startadd;
  quint32 End_Of_Mem;
} DTI_info;


typedef enum
{
    RAM_RO = 0,
    RAM_RW = 1,
    EE_RO = 2,
    EE_RW = 3
} Seg_Type;




class DTI_device
{
public:

    DTI_device(DTI_interface *interface = 0);
    ~DTI_device();
    DTI_interface * interface() {return inter;}
    void setInterface(DTI_interface *interface) {inter = interface;}
    bool connect(quint8 address = 0);
    void disconnect();
    bool isConnected() {return connected;}
    bool logIn(const char* password);// {return logged = inter->logIn(dev_add, password);}
    bool logOut() {return logged = !inter->logOut(dev_add);}
    bool isLogged() {return logged;}
    void setDeviceAddress(quint8 add){dev_add = add;}
    quint8 deviceAddress() {return dev_add;}
    bool readData(Seg_Type seg, quint32 mem_add, quint16 len, void* Data);
    bool writeData(Seg_Type seg, quint32 mem_add, quint16 len, void* Data, bool broadcast = false);
    bool getChar(Seg_Type seg, quint32 mem_add, quint8* Data){ return readData(seg, mem_add, 1, Data);}
    bool getInt16(Seg_Type seg, quint32 mem_add, quint16* Data){ return readData(seg, mem_add, 2, Data);}
    bool getInt32(Seg_Type seg, quint32 mem_add, quint32* Data){return readData(seg, mem_add, 4, Data);}
    bool getFloat(Seg_Type seg, quint32 mem_add, float* Data){return readData(seg, mem_add, 4, Data);}
    bool setChar(Seg_Type seg, quint32 mem_add, quint8* Data, bool broadcast = false){ return writeData(seg, mem_add, 1, Data, broadcast);}
    bool setInt16(Seg_Type seg, quint32 mem_add, quint16* Data, bool broadcast = false){return writeData(seg, mem_add, 2, Data, broadcast);}
    bool setInt32(Seg_Type seg, quint32 mem_add, quint32* Data, bool broadcast = false){return writeData(seg, mem_add, 4, Data, broadcast);}
    bool setFloat(Seg_Type seg, quint32 mem_add, float* Data, bool broadcast = false){return writeData(seg, mem_add, 4, Data, broadcast);}
    bool getDateTime(Seg_Type seg, quint32 mem_add, QDateTime *time);
    bool setDateTime(Seg_Type seg, quint32 mem_add, QDateTime *time, bool broadcast = false);
    bool getPropertyByName(const char *name, DTI_Property *prop);
    bool getValueOfProperty_byName(const char *name, QVariant *tag, quint16 parent = 0);
    bool setValueOfProperty_byName(const char *name, QVariant *tag, quint16 parent = 0, bool broadcast = false);
    bool runMethod(quint16 method_id, quint32 param, bool broadcast = false) {return inter->runMethod(broadcast?255:dev_add, method_id, param);}
    bool runMethod(const char *name, quint32 param, bool broadcast = false);
    bool exploreProperties();
    bool isReady() {return inter->isReady();}
    quint16 propertyID(const char *name, quint16 parent);
    Seg_Type  propertySeg(DTI_Property *prop) {return (Seg_Type)(prop->type>>6);}
    Seg_Type  propertySeg(const char *name, quint16 parent = 0);

    DTIError error() {return inter->error();}
    QString error_message() {return inter->error_message();}
private:
    DTI_interface *inter;
    quint8  dev_add;
    DTI_info info;
    bool connected;
    bool logged;
protected:
    quint32 getSegOffset(Seg_Type seg);

    QMap<quint16, DTI_Property> DTI_list;
};

#endif // DTI_DEVICE_H

#include "dti_device.h"
#include "logger.h"

DTI_device::DTI_device(DTI_interface *interface)
{
    inter = interface;
}

DTI_device::~DTI_device()
{
    disconnect();
}

bool DTI_device::connect(quint8 address)
{
    if(!inter) return false;
    //if(!inter->SLIP_SetRxMode(SLOW_DL, SLIP_FASTDL)) return false;
    bool result = inter->readInfo(address, 0, sizeof(DTI_info), &info);
    //bool result = true;
    connected = result;
    dev_add = address;
    log_1 << "dev_add" << dev_add << "connect" << connected;
    return result;
}

void DTI_device::disconnect()
{
    connected = 0;
    DTI_list.clear();
}

bool DTI_device::logIn(const char *password)
{
    bool log_in = inter->logIn(dev_add, password);
    log_1 << "dev_add" << dev_add << "logIn" << (logged = log_in);
    return logged = log_in;
}


bool DTI_device::readData(Seg_Type seg, quint32 mem_add, quint16 len, void* Data)
{
    if(!connected) return false;
    mem_add += getSegOffset(seg);
    if(mem_add + len >= getSegOffset(Seg_Type((uint)seg + 1))) return false;
    quint8 *ptr = (quint8 *)Data;
    bool result;
    while(len)
    {
        result = inter->readData(dev_add, mem_add, (len>info.packet_size)?info.packet_size:len, ptr);
        if(!result) break;
        if(len >= info.packet_size)
        {
            len -= info.packet_size;
            mem_add += info.packet_size;
            ptr += info.packet_size;
        }
        else len = 0;
    }
    return result;
}

bool DTI_device::writeData(Seg_Type seg, quint32 mem_add, quint16 len, void* Data, bool broadcast)
{
    if(!connected) return false;
    mem_add += getSegOffset(seg);
    if(mem_add + len >= getSegOffset(Seg_Type((uint)seg + 1))) return false;
    quint8 *ptr = (quint8 *)Data;
    bool result;
    while(len)
    {   if(broadcast) inter->no_wait = 1;
        result = inter->writeData(broadcast?255:dev_add, mem_add, (len>info.packet_size)?info.packet_size:len, ptr);
        inter->no_wait = 0;
        if(!result) break;
        if(len >= info.packet_size)
        {
            len -= info.packet_size;
            mem_add += info.packet_size;
            ptr += info.packet_size;
        }
        else len = 0;
    }
    return result;
}


quint32 DTI_device::getSegOffset(Seg_Type seg)
{
    switch(seg)
    {
    case RAM_RO:
        return info.RAM_RO_startadd;
    case RAM_RW:
        return info.RAM_RW_startadd;
    case EE_RO:
        return info.EE_RO_startadd;
    case EE_RW:
        return info.EE_RW_startadd;
    default:
        return info.End_Of_Mem;
    }
}
/*
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
#define  FLOAT_TYPE */



bool DTI_device::getPropertyByName(const char *name, DTI_Property *prop)
{
    DTI_Property p;

    quint16 id = propertyID(name, 0);

    if(id) p = DTI_list[id];
    else
    {
        if(!inter->getPropertyByName(dev_add, name, prop)) return false;
        else
        {
            DTI_list[prop->id] = *prop;
            return true;
        }

    }

    memcpy(prop, &p, sizeof(DTI_Property));

    return true;

}

bool DTI_device::getValueOfProperty_byName(const char *name, QVariant *tag, quint16 parent)
{
    DTI_Property p;
    Seg_Type seg;
    quint16 id = propertyID(name, parent);
    if(id) p = DTI_list[id];
    else if(!getPropertyByName(name, &p)) return false;
    seg = (Seg_Type)(p.type>>6);
    switch(p.type&0x3f)
    {
        case UINT8_TYPE:
            quint8 uc;
            if(getChar(seg, p.address, &uc))
            {
                tag->setValue(uc);
                return true;
            }
            break;
        case INT8_TYPE:
            qint8 c;
            if(getChar(seg, p.address, (quint8*)&c))
            {
                tag->setValue(c);
                return true;
            }
            break;
        case UINT16_TYPE:
            quint16 ui;
            if(getInt16(seg, p.address, &ui))
            {
                tag->setValue(ui);
                return true;
            }
            break;
        case INT16_TYPE:
            qint16 i;
            if(getInt16(seg, p.address, (quint16*)&i))
            {
                tag->setValue(i);
                return true;
            }
            break;
        case UINT32_TYPE:
            quint32 UI;
            if(getInt32(seg, p.address, &UI))
            {
                tag->setValue(UI);
                return true;
            }
            break;
        case INT32_TYPE:
            qint32 I;
            if(getInt32(seg, p.address, (quint32*)&I))
            {
                tag->setValue(I);
                return true;
            }
            break;
        case FLOAT_TYPE:
            float f;
            if(getFloat(seg, p.address, &f))
            {
                tag->setValue(f);
                return true;
            }
            break;
        case TIME_TYPE:
            QDateTime dt;
            if(getDateTime(seg, p.address, &dt))
            {
                tag->setValue(dt);
                return true;
            }
            break;
    }
    return false;
}


bool DTI_device::setValueOfProperty_byName(const char *name, QVariant *tag, quint16 parent, bool broadcast)
{
    DTI_Property p;
    Seg_Type seg;
    quint16 id = propertyID(name, parent);
    if(id) p = DTI_list[id];
    else if(!getPropertyByName(name, &p)) return false;
    seg = (Seg_Type)(p.type>>6);
    quint8 uc = tag->toUInt();
    qint8 c = tag->toInt();
    quint16 ui = tag->toUInt();
    qint16 i = tag->toInt();
    quint32 UI = tag->toUInt();
    qint32 I = tag->toInt();
    float f = tag->toFloat();
    QDateTime dt = tag->toDateTime();
    switch(p.type&0x3f)
    {
        case UINT8_TYPE:
            if(setChar(seg, p.address, &uc, broadcast))
            {
                return true;
            }
            break;
        case INT8_TYPE:
            if(setChar(seg, p.address, (quint8*)&c, broadcast))
            {
                return true;
            }
            break;
        case UINT16_TYPE:
            if(setInt16(seg, p.address, &ui, broadcast))
            {
                return true;
            }
            break;
        case INT16_TYPE:
            if(setInt16(seg, p.address, (quint16*)&i, broadcast))
            {
                return true;
            }
            break;
        case UINT32_TYPE:
            if(setInt32(seg, p.address, &UI, broadcast))
            {
                return true;
            }
            break;
        case INT32_TYPE:
            if(setInt32(seg, p.address, (quint32*)&I, broadcast))
            {
                return true;
            }
            break;
        case FLOAT_TYPE:
            if(setFloat(seg, p.address, &f, broadcast))
            {
                return true;
            }
            break;
        case TIME_TYPE:
            if(setDateTime(seg, p.address, &dt, broadcast))
            {
                return true;
            }
            break;
    }
    return false;
}



bool DTI_device::getDateTime(Seg_Type seg, quint32 mem_add, QDateTime *time)
{
    time_t tm;

    if(readData(seg, mem_add, 4, &tm) == 0) return false;
    time->setTimeSpec(Qt::UTC);
    time->setTime_t(tm);

    return true;

}

bool DTI_device::setDateTime(Seg_Type seg, quint32 mem_add, QDateTime *time, bool broadcast)
{
    time_t tm;
    time->setTimeSpec(Qt::UTC);
    tm = time->toTime_t();
    if(writeData(seg, mem_add, 4, &tm, broadcast) == 0) return false;

    return true;
}

bool DTI_device::exploreProperties()
{

    DTI_Property dti_list, dti_num;

    if(!getPropertyByName("DTI_list", &dti_list)) return false;
    if(!getPropertyByName("DTI_num", &dti_num)) return false;

    quint8 num;
    if(!getChar(RAM_RO, dti_num.address, &num)) return false;

    DTI_Property prop;
    for(int i = 0; i != num; i++)
    {
        quint32 add;
        if(!getInt32(RAM_RO, dti_list.address + 4*i, &add)) {DTI_list.clear(); return false;}
        add &= 0xfffffff;
        if(!readData(RAM_RO, add, sizeof(DTI_Property), &prop)) {DTI_list.clear();return false;}
        prop.address &= 0xfffffff;
        DTI_list[prop.id] = prop;

    }

    return true;
}

quint16 DTI_device::propertyID(const char *name, quint16 parent)
{
    QMapIterator<quint16, DTI_Property> i(DTI_list);

    while (i.hasNext())
    {
        i.next();
        if(!strcmp(((const char*)i.value().name), name) && i.value().parent_id == parent) return i.key();
    }
    return 0;
}

bool DTI_device::runMethod(const char *name, quint32 param, bool broadcast)
{
    DTI_Property p;
    quint16 id = propertyID(name, 0);
    if(id) p = DTI_list[id];
    else if(!getPropertyByName(name, &p)) return false;
    runMethod(p.id, param, broadcast);
    return true;
}


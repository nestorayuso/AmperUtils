#include "amper_dev.h"
#include <QTimeZone>
#include <QFile>
#include "crc.h"

#include "logger.h"

amper_dev::amper_dev(DTI_interface *interface) : DTI_device(interface)
{
    is_pBindAdder = false;

}


bool amper_dev::getSettings()
{
    DTI_Property prop;
    if(!getPropertyByName("Settings", &prop)) return false;
    return readData(propertySeg(&prop), prop.address, sizeof(settings_s), &Settings);
}

bool amper_dev::setSettings()
{
    DTI_Property prop;
    if(!getPropertyByName("Settings", &prop)) return false;
    return writeData(propertySeg(&prop), prop.address, sizeof(settings_s), &Settings);
}

bool amper_dev::setDTI_address(quint8 add)
{
    if(!getSettings()) return false;
    Settings.DTI_address = add;
    if(!setSettings()) return false;
    disconnect();
    if(!connect(add)) return false;
    return runEvent(SAVE_SETTINGS);
}

bool amper_dev::setProtC_address(quint32 add)
{
    if(!getSettings()) return false;
    Settings.ProtC_ID = add;
    if(!setSettings()) return false;
    return runEvent(SAVE_SETTINGS);
}

bool amper_dev::changePassword(QByteArray password)
{
    if(!getSettings()) return false;
    for(uint i = 0; i != 10; i++) Settings.Password[i] = password[i];
    if(!setSettings()) return false;
    return runEvent(SAVE_SETTINGS);
}

 bool amper_dev::getTarifData()
 {
     DTI_Property prop;
     if(!getPropertyByName("TSettings", &prop)) return false;
     tarif_settings_s tsettings;
     if(!readData(propertySeg(&prop),prop.address, sizeof(tarif_settings_s), &tsettings)) return false;

     N_of_Titles = tsettings.TarifTitleTotal;
     N_of_Zones = tsettings.TarifZoneTotal;

     if(!getPropertyByName("pTitle", &prop)) return false;
     if(!readData(propertySeg(&prop),prop.address, sizeof(TarifTitle_s)*N_of_Titles, pTitle)) return false;

     if(!getPropertyByName("pTarif", &prop)) return false;
     if(!readData(propertySeg(&prop),prop.address, sizeof(Tarif_s)*N_of_Zones, pTarif)) return false;

     return true;

 }

  bool amper_dev::setTarifData()
  {
      DTI_Property prop;

      tarif_settings_s tsettings;

      tsettings.TarifTitleTotal = N_of_Titles;
      tsettings.TarifZoneTotal = N_of_Zones;

      if(!getPropertyByName("TSettings", &prop)) return false;
      if(!writeData(propertySeg(&prop),prop.address, sizeof(tarif_settings_s), &tsettings)) return false;

      if(!getPropertyByName("pTitle", &prop)) return false;

      if(!writeData(propertySeg(&prop),prop.address, sizeof(TarifTitle_s)*N_of_Titles, pTitle)) return false;

      if(!getPropertyByName("pTarif", &prop)) return false;
      if(!writeData(propertySeg(&prop),prop.address, sizeof(Tarif_s)*N_of_Zones, pTarif)) return false;

      runEvent(STORE_NEW_TARIF);
      return true;

  }

  bool amper_dev::getAddersData()
  {
      DTI_Property prop;

      if(getPropertyByName("pM_INTEGS", &prop)) {
          if(!readData(propertySeg(&prop),prop.address, sizeof(meter_data.itgr), &meter_data.itgr)) return false;
          is_pBindAdder = false;
          log_1 << "pM_INTEGS load bytes:" << sizeof(meter_data.itgr);
          return true;
      }

      if(getPropertyByName("pBindAdd", &prop)) {
          if(!readData(propertySeg(&prop),prop.address, sizeof(BindAdder_s)*3, pBindAdder)) return false;
          is_pBindAdder = true;
          log_1 << "pBindAdd load";
      }

      return true;

  }

  bool amper_dev::getCurrentData()
  {

      DTI_Property prop;

      if(!getPropertyByName("ADE_Inst", &prop)) return false;

      log_1 << "ADE_Inst addr" << prop.address;

      if(!readData(propertySeg(&prop),prop.address, sizeof(ADE_Inst_Reg), &ADE_Inst)) return false;

      return true;
  }

  bool amper_dev::getADECals(int phase)
  {
      DTI_Property prop;

      log_1 << "phase" << phase;
      switch (phase) {
      case 0: if (!runMethod("ADE_Crd_A", 0)) {return false;} break;
      case 1: if (!runMethod("ADE_Crd_B", 0)) {return false;} break;
      case 2: if (!runMethod("ADE_Crd_C", 0)) {return false;} break;
      case 3: if (!runMethod("ADE_Crd_N", 0)) {return false;} break;
      default: return false;
      }

      log_1 << "read phase ok";
      if(!getPropertyByName("ADE_Cal", &prop)) return false;

      if(!readData(propertySeg(&prop),
                   prop.address + phase * sizeof(ADE_Cal_Reg),
                   sizeof(ADE_Cal_Reg),
                   &ADE_Cal)) return false;

      return true;

  }
  bool amper_dev::setADECals(int phase)
  {
      DTI_Property prop;

      if(!getPropertyByName("ADE_Cal", &prop)) return false;

      if(!writeData(propertySeg(&prop),
                    prop.address + phase * sizeof(ADE_Cal_Reg),
                    sizeof(ADE_Cal_Reg),
                    &ADE_Cal)) return false;

      switch (phase) {
      case 0: return runMethod("ADE_Cwr_A", 0);
      case 1: return runMethod("ADE_Cwr_B", 0);
      case 2: return runMethod("ADE_Cwr_C", 0);
      case 3: return runMethod("ADE_Cwr_N", 0);
      default: return false;
      }
      return false;
  }

  bool amper_dev::runEvent(mQueueType_e event, bool broadcast)
  {
      DTI_Property prop;
      if(!getPropertyByName("Event_run", &prop)) return false;
      return runMethod(prop.id, (quint32)event, broadcast);
  }

  bool amper_dev::getAmperDateTime(QDateTime *time)
  {
      QVariant tag;
      if(!getValueOfProperty_byName("CUR_time", &tag)) return false;
      *time = tag.toDateTime();
      return true;
  }

  bool amper_dev::setAmperDateTime(QDateTime time)
  {
      DTI_Property p;
      if(!getPropertyByName("setTime", &p)) return false;
      time.setTimeSpec(Qt::UTC);
      if(!runMethod(p.id, time.toTime_t())) return false;
      return true;

  }

  bool amper_dev::saveAllSettings(const char *filename)
  {
      quint8 data[4096];

      if(!readData(EE_RW, 0, 4096, data)) return false;

      QFile file(filename);

      if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) return false;

      file.write((const char*)data, 4096);

      file.close();

      return true;

  }

  bool amper_dev::restoreAllSettings(const char *filename)
  {
      quint8 data[4096];
      QFile file(filename);
      if(!file.open(QIODevice::ReadOnly)) return false;
      if(file.read((char*)data, 4096) != 4096) return false;
      file.close();
      erase4kblock(0);
      if(!writeData(EE_RW, 0, 4096, data)) return false;
      return true;
  }



  /*static quint32 CRC32(quint32 *buffer, quint32 size)
  {
      quint32 i, crc = 0, data;

      while (size--) {
          data = *buffer++;
          for (i = 32; i; i--) {
              if ((crc ^ data) & 1) {
                  crc = (crc >> 1) ^ 0x8408;
              } else {
                  crc = (crc >> 1);
              }
              data >>= 1;
          }
      }
      return(crc);
  }*/

  quint16 amper_dev::CRC_calc(char *data, quint32 size)
  {
    quint16 crc = 0x0;
 //   quint16  *data;
    quint8 byte;
    while(size--)//for (data = start; data < end; data++)
    {
      byte = *data;
      crc  = (crc >> 8) | (crc << 8);
      crc ^= byte;
      crc ^= (crc & 0xff) >> 4;
      crc ^= crc << 12;
      crc ^= (crc & 0xff) << 5;
      data ++;
    }
    return crc;
  }

 /* bool amper_dev::writeFirmware(soft_update* update_struct, const char *filename)
  {

      erase64kblock(UPDATE_ADD);

      QFile binFile(filename);
      binFile.open(QIODevice::ReadOnly);
      update_struct->end_add = update_struct->start_add + binFile.size();
      QByteArray ba = binFile.readAll();
      update_struct->crc = CRC_calc(ba.data(), binFile.size());
      if(!writeData(EE_RW, UPDATE_ADD, sizeof(soft_update), update_struct)) return false;



//      soft_update aa;

//      if(!readData(EE_RW, UPDATE_ADD, sizeof(soft_update), &aa)) return false;

//      update_struct->ver = aa.ver;
      //char tmpmas[32];
      for(int i = 0; i < ba.size(); i += 32)
      {
          if(!writeData(EE_RW, UPDATE_ADD + i, 32, &ba.data()[i])) return false;
          //if(i >= update_struct->end_add) break;
          //binFile.read((char*)(&tmpmas[0]), 32);
      }


  }*/

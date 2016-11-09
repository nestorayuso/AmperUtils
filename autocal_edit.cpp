#include "autocal_edit.h"
#include "ui_autocal_edit.h"
#include <QTime>
#include<QMessageBox>
#include <QThread>

//#define AMPER_SUPPLY_CURRENT    (0.00217)
//#define AMPER_SUPPLY_APOWER    (1.447)      // 1200Wt
//#define AMPER_SUPPLY_RPOWER    (1.748)

autocal_edit::autocal_edit(amper_dev *ad, QProgressBar *pb, int phase, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::autocal_edit)
    , dev(ad)
    , bar(pb)
    , phase(phase)
{
    ui->setupUi(this);
}

autocal_edit::~autocal_edit()
{
    delete ui;
}

void autocal_edit::sendText2Progress(QString s)
{
    ui->textBrowser->append(QTime::currentTime().toString("h:mm:ss")+" "+s);
    qApp->processEvents();
}

bool autocal_edit::delay(quint32 seconds)
{
    quint32 timer = 0;
    do
    {
      QThread::msleep(10);
      qApp->processEvents();
      if(stop) return false;

    }while(++timer < seconds*100);
    return true;
}

bool autocal_edit::stage_1()    //поиск устройств
{
    quint8 number = 0;
    dev_num = 0;
    amper_dev *new_dev;
    devices[0] = dev;
    sendText2Progress("Этап №1. Поиск устройств:");
    do
    {
        qApp->processEvents();
        if(stop) return false;
        if(devices[number]->interface()->checkAdd(0))
        {
            new_dev = new amper_dev(devices[number]->interface());
            if(!new_dev->connect(0))
            {
                delete new_dev;
                continue;
            }
            if(devices[number]->interface()->checkAdd(number + 1))
            {
                QMessageBox::warning(this,"Ошибка автокалибровки","Адрес занят!");
                delete new_dev;
                return false;
            }
            if(!new_dev->setDTI_address(number + 1))
            {
                if((new_dev->error() != ERR_ACCESS_DENIED) || !new_dev->logIn("strij"))
                {
                    QMessageBox::warning(this,"Ошибка автокалибровки","Связь с узлом потеряна!");
                    delete new_dev;
                    return false;
                }
            }
            devices[++number] = new_dev;
            if(!devices[number]->getSettings())
            {
                QMessageBox::warning(this,"Ошибка автокалибровки","Связь с узлом потеряна!");
                return false;
            }
            sendText2Progress(QString::number(number)+ " = " + QString::number(devices[number]->Settings.ProtC_ID, 16));
            if(!devices[number]->runEvent(RELAY_ON))
            {
                QMessageBox::warning(this,"Ошибка автокалибровки","Связь с узлом потеряна!");
                return false;
            }
            continue;
        }

        if((devices[number]->interface()->checkAdd(number+1)))
        {
          new_dev = new amper_dev(devices[number]->interface());
          if(!new_dev->connect(number + 1))
          {
              delete new_dev;
              continue;
          }
          devices[++number] = new_dev;
          if(!devices[number]->getSettings())
          {
              QMessageBox::warning(this,"Ошибка автокалибровки","Связь с узлом потеряна!");
              return false;
          }
          sendText2Progress(QString::number(number)+ " = " + QString::number(devices[number]->Settings.ProtC_ID, 16));
          if(!devices[number]->runEvent(RELAY_ON))
          {
              if((devices[number]->error() != ERR_ACCESS_DENIED) || !devices[number]->logIn("strij"))
              {
                QMessageBox::warning(this,"Ошибка автокалибровки","Связь с узлом потеряна!");
                return false;
              }
          }
          continue;
        }
        if(dev->interface()->checkAdd(ETALON_ADD))
        {
            new_dev = new amper_dev(dev->interface());
            if(!new_dev->connect(ETALON_ADD))
            {
                delete new_dev;
                continue;
            }
            devices[ETALON_ADD] = new_dev;
            if(!devices[ETALON_ADD]->logIn("strij"))
            {
                QMessageBox::warning(this,"Ошибка автокалибровки", "Пароль не верный!");
                return false;
            }
            if(!devices[ETALON_ADD]->runEvent(RELAY_OFF))
            {
              QMessageBox::warning(this,"Ошибка автокалибровки","Связь с узлом потеряна!");
              return false;
            }
            break;
        }
    }
    while(1);

    sendText2Progress("Поиск устройств закончен, эталон найден");
    dev_num = number;
    return true;

}

bool autocal_edit::stage_2()    //калибровка напряжения
{
    //quint32 timer = 0;
    sendText2Progress("Этап №2. Калибровка напряжения:");
    QVariant alfa = 0.0;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    if(!delay(2)) return false;
    if(!devices[ETALON_ADD]->runEvent(RELAY_OFF))
    {
      QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
      return false;
    }
    if(!delay(1)) return false;
    alfa = 0.98;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление

    if(!delay(U_CAL_TIME)) return false;
   /* do
    {
      QThread::usleep(1);
      qApp->processEvents();
      if(stop) return false;

    }while(++timer < U_CAL_TIME);*/

    alfa = 10;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    if(!delay(2)) return false;

    if(!devices[ETALON_ADD]->getCurrentData())
    {
        QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
        return false;
    }
    sendText2Progress("Uetal = " + QString::number(devices[ETALON_ADD]->ADE_Inst.vrms,'f', 1));
    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->getCurrentData())
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
        float delta = (devices[i]->ADE_Inst.vrms - devices[ETALON_ADD]->ADE_Inst.vrms)/devices[ETALON_ADD]->ADE_Inst.vrms*100;
        sendText2Progress("U"+ QString::number(i) + " = " + QString::number(devices[i]->ADE_Inst.vrms,'f', 1) + "   поправка:" + QString::number(delta, 'f', 4) + "%");
        if(abs(delta) > 20)
        {
            sendText2Progress("Внимание!!! Слишком большая разница!");
        }

        qApp->processEvents();
        if(stop) return false;
    }

    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->getADECals(phase))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
        qApp->processEvents();
        if(stop) return false;
        devices[i]->ADE_Cal.avgain = devices[i]->ADE_Cal.avgain * devices[ETALON_ADD]->ADE_Inst.vrms / devices[i]->ADE_Inst.vrms ;

        if(!devices[i]->setADECals(phase)||!devices[i]->runEvent(SAVE_SETTINGS))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
    }
    if(!delay(1)) return false;
    alfa = 0.0;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    sendText2Progress("Калибровка напряжения завершена успешно");
    return true;
}

bool autocal_edit::stage_3()    //калибровка тока
{
    //quint32 timer = 0;
    sendText2Progress("Этап №3. Калибровка тока:");
    QVariant alfa = 0.0;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    if(!delay(4)) return false;
    if(!devices[ETALON_ADD]->runEvent(RELAY_ON))
    {
      QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
      return false;
    }
    if(!delay(1)) return false;
    alfa = 0.98;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    /*do
    {
      QThread::usleep(1);
      qApp->processEvents();
      if(stop) return false;

    }while(++timer < I_CAL_TIME);*/
    if(!delay(I_CAL_TIME)) return false;

    alfa = 10;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    if(!delay(4)) return false;

    if(!devices[ETALON_ADD]->runEvent(RELAY_OFF))
    {
      QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
      return false;
    }

    if(!devices[ETALON_ADD]->getCurrentData())
    {
        QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
        return false;
    }
    sendText2Progress("Ietal = " + QString::number(devices[ETALON_ADD]->ADE_Inst.irms,'f', 4));
    float utoch[255];
    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->getCurrentData())
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
        utoch[i] = devices[ETALON_ADD]->ADE_Inst.irms + AMPER_SUPPLY_CURRENT*(dev_num - i + 1);
        float delta = (devices[i]->ADE_Inst.irms - utoch[i])/utoch[i]*100;
        sendText2Progress("I"+ QString::number(i) + " = " + QString::number(devices[i]->ADE_Inst.irms,'f', 4) + "   поправка:" + QString::number(delta, 'f', 4) + "%");
        if(abs(delta) > 20)
        {
            sendText2Progress("Внимание!!! Слишком большая разница!");
        }

        qApp->processEvents();
        if(stop) return false;
    }

    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->getADECals(phase))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
        qApp->processEvents();
        if(stop) return false;
        devices[i]->ADE_Cal.aigain = devices[i]->ADE_Cal.aigain * utoch[i] / devices[i]->ADE_Inst.irms ;

        if(!devices[i]->setADECals(phase)||!devices[i]->runEvent(SAVE_SETTINGS))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
    }
    if(!delay(1)) return false;
    alfa = 0.0;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    sendText2Progress("Калибровка тока завершена успешно");
    return true;
}


bool autocal_edit::stage_4()    //калибровка энергии
{
    //quint32 timer = 0;
    sendText2Progress("Этап №4. Калибровка энергии:");
    QVariant alfa = 0.0;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true); //сброс накопления
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true); //сброс накопления
    if(!delay(4)) return false;
    alfa = 0.98;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    if(!delay(5)) return false;
    QTime time;
    time.start();
    if(!devices[ETALON_ADD]->runEvent(RELAY_ON))
    {
      QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
      return false;
    }
    do
    {
      QThread::msleep(100);
      qApp->processEvents();
      if(stop) return false;
      if(!devices[ETALON_ADD]->getCurrentData())
      {
          QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
          return false;
      }

    }while(devices[ETALON_ADD]->ADE_Inst.aenergy/2000.0 < E_CAL_WH);

    if(!devices[ETALON_ADD]->runEvent(RELAY_OFF))
    {
      QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
      return false;
    }
    float el_hours = ((float)time.elapsed())/1000/3600;

    if(!delay(3)) return false;

    alfa = 10;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    if(!delay(4)) return false;

    if(!devices[ETALON_ADD]->getCurrentData())
    {
        QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
        return false;
    }
    sendText2Progress("AEetal = " + QString::number(devices[ETALON_ADD]->ADE_Inst.aenergy/2000.0,'f', 2));
    sendText2Progress("REetal = " + QString::number(devices[ETALON_ADD]->ADE_Inst.renergy/2000.0,'f', 2));
    float utoch_a[255];
    float utoch_r[255];
    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->getCurrentData())
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
        utoch_a[i] = (float)devices[ETALON_ADD]->ADE_Inst.aenergy/2000.0 + AMPER_SUPPLY_APOWER*(dev_num - i + 1)*el_hours;
        utoch_r[i] = (float)devices[ETALON_ADD]->ADE_Inst.renergy/2000.0 - AMPER_SUPPLY_RPOWER*(dev_num - i + 1)*el_hours;
        float delta_a = (devices[i]->ADE_Inst.aenergy/2000.0 - utoch_a[i])/utoch_a[i]*100;
        float delta_r = (devices[i]->ADE_Inst.renergy/2000.0 - utoch_r[i])/utoch_r[i]*100;
        sendText2Progress("AE"+ QString::number(i) + " = " + QString::number(devices[i]->ADE_Inst.aenergy/2000.0,'f', 2) + "   поправка:" + QString::number(delta_a, 'f', 4) + "%");
        if(abs(delta_a) > 20)
        {
            sendText2Progress("Внимание!!! Слишком большая разница!");
        }
        sendText2Progress("RE"+ QString::number(i) + " = " + QString::number(devices[i]->ADE_Inst.renergy/2000.0,'f', 2) + "   поправка:" + QString::number(delta_r, 'f', 4) + "%");
        if(abs(delta_r) > 20)
        {
            sendText2Progress("Внимание!!! Слишком большая разница!");
        }

        qApp->processEvents();
        if(stop) return false;
    }

    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->getADECals(phase))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
        qApp->processEvents();
        if(stop) return false;
        devices[i]->ADE_Cal.awgain = (float)devices[i]->ADE_Cal.awgain * utoch_a[i] / (devices[i]->ADE_Inst.aenergy/2000.0) ;
        devices[i]->ADE_Cal.avargain = (float)devices[i]->ADE_Cal.avargain * utoch_r[i] / (devices[i]->ADE_Inst.renergy/2000.0) ;
        if(!devices[i]->setADECals(phase)||!devices[i]->runEvent(SAVE_SETTINGS))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
    }
    if(!delay(1)) return false;
    alfa = 0.0;
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    devices[ETALON_ADD]->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);  //начать накопление
    sendText2Progress("Калибровка энергии завершена успешно");
    return true;
}

bool autocal_edit::stage_5()    //установка времени
{
    sendText2Progress("Этап №5. Установка времени и даты:");

    for(quint8 i = 1; i<= dev_num; i++ )
    {
        if(!devices[i]->setAmperDateTime(QDateTime::currentDateTime()))
        {
            QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
            return false;
        }
    }

    sendText2Progress("Время и дата успешно установлены");
    return true;

}

bool autocal_edit::stage_6()    //завершение
{
    sendText2Progress("Этап №6. Завершение калибровки");
    quint8 tmp = devices[ETALON_ADD]->interface()->TryNum;
    devices[ETALON_ADD]->interface()->TryNum = 2;
    for(quint8 i = 1; i<= dev_num; i++ )
    {

        if(!devices[i]->getSettings())
        {
   //         QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
   //         return false;
        }
        devices[i]->Settings.DTI_address = 0;
        if(!devices[i]->setSettings())
        {
   //         QMessageBox::warning(this,"Ошибка автокалибровки", "Связь с узлом потеряна!");
   //         return false;
        }
        disconnect();
    }

    devices[ETALON_ADD]->runEvent(SAVE_SETTINGS,true);
    devices[ETALON_ADD]->runEvent(SAVE_SETTINGS,true);
    devices[ETALON_ADD]->runEvent(RESET_FAST_DL,true);
    devices[ETALON_ADD]->interface()->TryNum = tmp;
    sendText2Progress("Калибровка успешно завершена");
    return true;
}


void autocal_edit::on_pushButton_2_clicked()
{
    stop = 0;
    bool result = 1;
    sendText2Progress("Старт калибровки");

    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(true);

    if(ui->checkBox->isChecked()) result = stage_1();
    if(!result) goto cal_end;

    if(ui->checkBox_2->isChecked()) result = stage_2();
    if(!result) goto cal_end;

    if(ui->checkBox_3->isChecked()) result = stage_3();
    if(!result) goto cal_end;

    if(ui->checkBox_4->isChecked()) result = stage_4();
    if(!result) goto cal_end;

    if(ui->checkBox_5->isChecked()) result = stage_5();
    if(!result) goto cal_end;

    if(ui->checkBox_6->isChecked()) result = stage_6();
    if(!result) goto cal_end;

cal_end:
    if(stop) sendText2Progress("Отменено пользователем");
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_3->setEnabled(false);
}

void autocal_edit::on_pushButton_3_clicked()
{
    stop = 1;
}

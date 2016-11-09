#include "tarif_edit.h"
#include "ui_tarif_edit.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

#include "tariff.h"

tarif_edit::tarif_edit(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::tarif_edit)
    , dev(ad)
    , bar(pb)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);
    ptariff = new TARIFF_TABLE;

    wte = new widget_tariff_edit(parent);
    ui->horizontalLayout_2->insertWidget(0, wte);

    ui->horizontalGroupBox_per_1->hide();
    ui->horizontalGroupBox_per_2->hide();
    allPButtons_1 = ui->horizontalGroupBox_per_1->findChildren<QPushButton *>();
    allPButtons_2 = ui->horizontalGroupBox_per_2->findChildren<QPushButton *>();

    QObject::connect(ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(readClicked()));
    QObject::connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(writeClicked()));

    for(int i = 0; i != 72; i ++)
    {
        QObject::connect(allPButtons_1[i], SIGNAL(toggled(bool)), this, SLOT(butToggled(bool)));
        QObject::connect(allPButtons_2[i], SIGNAL(toggled(bool)), this, SLOT(butToggled(bool)));

    }

    for(int i = 0; i != 24; i ++)
    {
        allPButtons_1[i]->setChecked(true);
        allPButtons_2[i]->setChecked(true);
    }
    all_checkboxes = ui->verticalGroupBox->findChildren<QCheckBox *>();

    QList<QCheckBox *>::iterator i;
    for(i = all_checkboxes.begin(); i != all_checkboxes.end(); i++)
        QObject::connect((*i), SIGNAL(clicked()), this, SLOT(checkBoxClicked()));

    readAllData();
}

tarif_edit::~tarif_edit()
{
    delete (TARIFF_TABLE*) ptariff;
    delete ui;
}

void tarif_edit::readAllData()
{
    readClicked();
}

void tarif_edit::butToggled(bool e)
{
    Q_UNUSED(e);
    int i;
    for(i = 0; i != 72; i ++)
    {
        if(allPButtons_1[i]->hasFocus()) break;
    }
    if(i != 72)
    {
        if(allPButtons_1[i]->isChecked()&&(allPButtons_1[(i+24)%72]->isChecked()||allPButtons_1[(i+48)%72]->isChecked()))
        {
            allPButtons_1[(i+24)%72]->setChecked(false);
            allPButtons_1[(i+48)%72]->setChecked(false);
        }
        else if(!allPButtons_1[i]->isChecked()&&(!allPButtons_1[(i+24)%72]->isChecked()&&!allPButtons_1[(i+48)%72]->isChecked()))
        {
            allPButtons_1[(i+24)%72]->setChecked(true);
        }
        allPButtons_1[i]->clearFocus();
        return;
    }

    for(i = 0; i != 72; i ++)
    {
        if(allPButtons_2[i]->hasFocus()) break;
    }
    if(i != 72)
    {
        if(allPButtons_2[i]->isChecked()&&(allPButtons_2[(i+24)%72]->isChecked()||allPButtons_2[(i+48)%72]->isChecked()))
        {
            allPButtons_2[(i+24)%72]->setChecked(false);
            allPButtons_2[(i+48)%72]->setChecked(false);
        }
        else if(!allPButtons_2[i]->isChecked()&&(!allPButtons_2[(i+24)%72]->isChecked()&&!allPButtons_2[(i+48)%72]->isChecked()))
        {
            allPButtons_2[(i+24)%72]->setChecked(true);
        }
        allPButtons_2[i]->clearFocus();
    }

}


void tarif_edit::readClicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(readClicked()));
        return;
    }
    bar->setValue(50);

    DTI_Property prop;
    if(!dev->getPropertyByName("pTariff", &prop)) {
        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
        bar->setValue(0);
        return;
    }
    if(!dev->readData(dev->propertySeg(&prop),prop.address, sizeof(TARIFF_TABLE), ptariff)) {
        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
        bar->setValue(0);
        return;
    }

    wte->setData(ptariff);

//    if(!dev->getTarifData())
//    {
//        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
//        bar->setValue(0);
//        return;
//    }

//    TARIFF_TABLE * tt = (TARIFF_TABLE *)ptariff;
//    for (int i = 0; i < TARIFF_TABLE_ITEMS; i++) {
//        setZoneButton((uint8_t)(i*4)+0, tt->h2[i].hh0);
//        setZoneButton((uint8_t)(i*4)+1, tt->h2[i].hh1);
//        setZoneButton((uint8_t)(i*4)+2, tt->h2[i].hh2);
//        setZoneButton((uint8_t)(i*4)+3, tt->h2[i].hh3);
//    }
//    for(int i = 0; i != dev->N_of_Zones; i++)
//    {
//        for(int t = dev->pTarif[i].time_zone.start; t <= dev->pTarif[i].time_zone.end; t += 10*60)
//        {
//            setZoneButton(t, dev->pTarif[i].title);
//        }
//    }
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);
}

void tarif_edit::setZoneButton(quint32 time, RateZoneType_e name)   //seconds from midnight
{
    quint8 hour = time / 3600;
    quint8 minutes = (time%3600)/60;

    quint8 index = hour*2 + ((minutes>=30)?1:0);

    if(index >= 24) index += 48;

    switch(name)
    {
    case NIGHT_ZONE_RATE:
        break;
    case PEAK_ZONE_RATE:
        index += 24;
        break;
    case HALF_PEAK_ZONE_RATE:
        index += 48;
        break;
    }
    if(index < 72)
    {
        allPButtons_1[index]->setChecked(true);
        allPButtons_1[(index + 24)%72]->setChecked(false);
        allPButtons_1[(index + 48)%72]->setChecked(false);
    }
    else
    {
        index -= 72;
        allPButtons_2[index]->setChecked(true);
        allPButtons_2[(index + 24)%72]->setChecked(false);
        allPButtons_2[(index + 48)%72]->setChecked(false);
    }

}

void tarif_edit::setZoneButton(uint8_t zone, uint8_t tariff)
{
    quint8 index = zone;

    if(index >= 24) index += 48;

    index += tariff * 24;
//    switch(name)
//    {
//    case NIGHT_ZONE_RATE:
//        break;
//    case PEAK_ZONE_RATE:
//        index += 24;
//        break;
//    case HALF_PEAK_ZONE_RATE:
//        index += 48;
//        break;
//    }
    if(index < 72)
    {
        allPButtons_1[index]->setChecked(true);
        allPButtons_1[(index + 24)%72]->setChecked(false);
        allPButtons_1[(index + 48)%72]->setChecked(false);
    }
    else
    {
        index -= 72;
        allPButtons_2[index]->setChecked(true);
        allPButtons_2[(index + 24)%72]->setChecked(false);
        allPButtons_2[(index + 48)%72]->setChecked(false);
    }
}

static int Tarif_StartCmp (const void* p1, const void* p2)
{
  if ( ((Tarif_s*)p1)->time_zone.start <  ((Tarif_s*)p2)->time_zone.start ) return -1;
  if ( ((Tarif_s*)p1)->time_zone.start == ((Tarif_s*)p2)->time_zone.start ) return 0;
  if ( ((Tarif_s*)p1)->time_zone.start >  ((Tarif_s*)p2)->time_zone.start ) return 1;
  return 0;
}

void tarif_edit::writeClicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(writeClicked()));
        return;
    }

    QByteArray arr = wte->getData();

    DTI_Property prop;

    bool err = false;
    for (;;) {
        if (arr.size() != sizeof(TARIFF_TABLE)) {err = true; break;}
        if (!dev->getPropertyByName("pTariff", &prop)) {err = true; break;}
        if (!dev->writeData(dev->propertySeg(&prop), prop.address, arr.size(), arr.data())) {err = true; break;}
        if (!dev->runEvent(STORE_NEW_TARIF)) {err = true; break;}
        break;
    }
    if (err) {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        bar->setValue(0);
        return;
    }
    dev->runEvent(SEND_TARIFS_EVENT);
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");
}




quint64 tarif_edit::getZoneBitPack(RateZoneType_e name)
{
    quint8 tarif;

    switch(name)
    {
        case NIGHT_ZONE_RATE:
            tarif = 0;
        break;
        case PEAK_ZONE_RATE:
            tarif = 1;
        break;
        case HALF_PEAK_ZONE_RATE:
            tarif = 2;
        break;
        default:
            tarif = 0;
        break;
    }

    quint64 bits = 0;

    for(int i = 0; i != 24; i++)
    {
        if(allPButtons_1[i+tarif*24]->isChecked()) bits += (1<<i);
        if(allPButtons_2[i+tarif*24]->isChecked()) bits += ((quint64)1<<(i+24));
    }

    return bits;
}

void tarif_edit::checkBoxClicked()
{

    QList<QCheckBox *>::iterator i;
    ui->pushButton_3->setEnabled(true);
    for(i = all_checkboxes.begin(); i != all_checkboxes.end(); i++)
    {
        if((*i)->isChecked())
        {
            QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);
            pSettings->beginGroup("Tarifs");

            for(quint8 tarif = 0; tarif <= 2; tarif++)
            {
                quint64 bits  =  pSettings->value((*i)->objectName() + "_T" + QString::number(tarif+1)).toULongLong();

                for(int i = 0; i != 24; i++)
                {
                    allPButtons_1[i+tarif*24]->setChecked(bits&(1<<i));
                    allPButtons_2[i+tarif*24]->setChecked(bits&((quint64)1<<(i+24)));
                }
            }

            pSettings->endGroup();

            delete pSettings;

            break;
        }
    }


}

void tarif_edit::on_pushButton_3_clicked()
{

    QList<QCheckBox *>::iterator i;
    for(i = all_checkboxes.begin(); i != all_checkboxes.end(); i++)
    {
        if((*i)->isChecked())
        {

            QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);
            pSettings->beginGroup("Tarifs");

            quint64 bits;

            for(quint32 z = (quint32)NIGHT_ZONE_RATE; z <= (quint32)HALF_PEAK_ZONE_RATE; z++)
            {
                bits = getZoneBitPack((RateZoneType_e)z);
                pSettings->setValue((*i)->objectName() + "_T" + QString::number((quint8)(z&0xff)), bits);

            }

            pSettings->endGroup();

            delete pSettings;

            break;
        }
    }
}

#include "par_edit.h"
#include "ui_par_edit.h"
#include "advanced_edit.h"
#include <QMessageBox>
#include <QThread>
#include <math.h>
#include <QFileDialog>

#include "widget_meter_arch_data_setttings.h"
#include "logger.h"

//#define  UNITS_PER_WH   (2000.0)

typedef struct {
    uint32_t adc_cover;
    uint32_t adc_body;
    uint32_t hall;
} SENSORS_EXT_Current;

par_edit::par_edit(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Par_edit)
    , dev(ad)
    , bar(pb)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    wdg_profile_power_settings = new widget_METER_ARCH_DATA_SETTTINGS(dev, this);
    ui->verticalLayout_settings_profile_power->addWidget(wdg_profile_power_settings);

    tm3 = new QTimer(this);
    tm3->setInterval(110);

    QObject::connect(ui->pushButton_4, SIGNAL(clicked(bool)), this, SLOT(timeSetClicked(bool)));
    QObject::connect(tm3, SIGNAL(timeout()), this, SLOT(timeReadTimer()));

    QObject::connect(this, SIGNAL(finished(int)), this, SLOT(stopall(int)));

    QValidator* validator1 = new QIntValidator( 0, 20000, this );
    ui->lineEdit->setValidator(validator1);

    readAllData();
}

par_edit::~par_edit()
{
    delete ui;
}

void par_edit::readAllData()
{
    //addersReadTimer();
    timeReadTimer();
    //currentReadTimer();
    on_pushButton_9_clicked();

    DTI_Property prop;
    if (dev->getPropertyByName("pM_PPOWST", &prop)) {
        dev->readData(dev->propertySeg(&prop), prop.address, sizeof(METER_ARCH_DATA_SETTINGS), &wdg_profile_power_settings->mads);
        wdg_profile_power_settings->slot_update_ui_from_data();
    }
}

void par_edit::timeSetClicked(bool e)
{
    Q_UNUSED(e);
    if(ui->checkBox_3->isChecked())
    {
        if(ui->pushButton_4->text() == "Стоп")
        {
            ui->pushButton_4->setText("Старт");
            tm3->stop();
        }
        else
        {
            ui->pushButton_4->setText("Стоп");
            tm3->start();
        }
    }
    else
    {
        tm3->start();
    }

}

void par_edit::timeReadTimer()
{
    if(!dev->isReady()) return;
    QDateTime dt;
    bar->setValue(100);
    if(!dev->getAmperDateTime(&dt))
    {

        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
        ui->checkBox_3->setChecked(false);
        ui->pushButton_4->setText("Cчитать");
        tm3->stop();
        bar->setValue(0);
        return;
    }
    ui->dateTimeEdit->setDateTime(dt);
    if(!ui->checkBox_3->isChecked()) tm3->stop();
    bar->setValue(0);
}

void par_edit::slot_SENSORS_upd()
{
    SENSORS_EXT_Current sensors_ext_curr;
    DTI_Property prop;
    if (!dev->getPropertyByName("pSENS_cur", &prop)) return;
    if (!dev->readData(dev->propertySeg(&prop), prop.address, sizeof(sensors_ext_curr), &sensors_ext_curr)) return;
    ui->lineEdit_sensors->setText(QString("%1 %2 %3")
                                  .arg(sensors_ext_curr.hall)
                                  .arg(sensors_ext_curr.adc_cover)
                                  .arg(sensors_ext_curr.adc_body));
    if (ui->pushButton_upd_sensors->isChecked()) {
        QTimer::singleShot(1000, this, SLOT(slot_SENSORS_upd()));
    }
}

void par_edit::on_checkBox_3_clicked(bool checked)
{
    Q_UNUSED(checked);
    if(ui->checkBox_3->isChecked())
    {
         ui->pushButton_4->setText("Старт");
    }
    else ui->pushButton_4->setText("Cчитать");
}

void par_edit::on_pushButton_3_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_3_clicked()));
        return;
    }
    bar->setValue(50);
    if(!dev->setAmperDateTime(QDateTime::currentDateTime()))
    {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        bar->setValue(0);
        return;
    }
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");
}

void par_edit::on_pushButton_6_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_6_clicked()));
        return;
    }
    bar->setValue(50);
    dev->runEvent(RELAY_ON);
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);
}

void par_edit::on_pushButton_5_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_5_clicked()));
        return;
    }
    bar->setValue(50);
    dev->runEvent(RELAY_OFF);
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);
}

void par_edit::on_pushButton_7_clicked()
{
 /*   bar->setValue(50);
    if(!dev->setProtC_address(0x6f22f8))
    {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        bar->setValue(0);
        return;
    }
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");*/
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_7_clicked()));
        return;
    }
    bar->setValue(50);
    if(!dev->getSettings())
    {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        bar->setValue(0);
        return;
    }

    dev->Settings.Power_limit = ui->lineEdit->text().toFloat();

    if(dev->Settings.Power_limit == 0) dev->Settings.Power_limit = 65535;
    bar->setValue(100);
    if(!dev->setSettings() || !dev->runEvent(SAVE_SETTINGS) || !dev->runEvent(SET_PWR_LIMIT_EVENT))
    {
        bar->setValue(0);
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        return;
    }
    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");

}

void par_edit::stopall(int)
{
    tm3->stop();
    ui->checkBox_3->setChecked(false);
    QThread::usleep(100);
}

void par_edit::on_pushButton_9_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_9_clicked()));
        return;
    }
    bar->setValue(100);
    if(!dev->getSettings())
    {
        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
    }
    else
    {

     //   ui->lineEdit->setText(QString::number(dev->Settings.Power_limit, 'f', 0));
        switch(dev->Settings.Pulse0_energy_type)
        {
            case ACTIVE_ENERGY:
                ui->comboBox->setCurrentText("Активная");
            break;
            case REACTIVE_ENERGY:
                ui->comboBox->setCurrentText("Реактивная");
            break;
        }

        switch(dev->Settings.Pulse1_energy_type)
        {
            case ACTIVE_ENERGY:
                ui->comboBox_3->setCurrentText("Активная");
            break;
            case REACTIVE_ENERGY:
                ui->comboBox_3->setCurrentText("Реактивная");
            break;
        }

        ui->comboBox_2->setCurrentText(QString::number(dev->Settings.Pulse0_freq));
        ui->comboBox_4->setCurrentText(QString::number(dev->Settings.Pulse1_freq));
        ui->lineEdit->setText(QString::number((dev->Settings.Power_limit == 65535)?0:dev->Settings.Power_limit, 'f', 0));
    }

    bar->setValue(0);
}

void par_edit::on_pushButton_8_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_8_clicked()));
        return;
    }
    bar->setValue(50);

    if(!dev->getSettings())
    {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        bar->setValue(0);
        return;
    }

    if(ui->comboBox->currentText() == "Активная") dev->Settings.Pulse0_energy_type = ACTIVE_ENERGY;
    if(ui->comboBox->currentText() == "Реактивная") dev->Settings.Pulse0_energy_type = REACTIVE_ENERGY;
    if(ui->comboBox_3->currentText() == "Активная") dev->Settings.Pulse1_energy_type = ACTIVE_ENERGY;
    if(ui->comboBox_3->currentText() == "Реактивная") dev->Settings.Pulse1_energy_type = REACTIVE_ENERGY;

    dev->Settings.Pulse0_freq = ui->comboBox_2->currentText().toUInt();
    dev->Settings.Pulse1_freq = ui->comboBox_4->currentText().toUInt();

    bar->setValue(100);
    if(!dev->setSettings() || !dev->runEvent(SAVE_SETTINGS))
    {
        bar->setValue(0);
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        return;
    }
    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");
}

//void par_edit::slot_read_end(bool ok)
//{
////    if (wdg_itgr == qobject_cast<widget_itgr *>(sender())) {
////        //wdg_mcurr->slot_start_after_ms(10);
////        log_1 << ok;
//////        if (ui->checkBox_curr_autoupdate->isChecked()) {
//////            wdg_mcurr->slot_start_after_ms(200);
//////        }
////    }
////    if (wdg_mcurr == qobject_cast<widget_meter_curr *>(sender())) {
////        if (ui->checkBox_curr_autoupdate->isChecked()) {
////            wdg_mcurr->slot_start_after_ms(200);
////        } else {
////            //wdg_itgr->slot_start_after_ms(10);

////        }
////    }
//}

void par_edit::on_pushButton_calibr_A_clicked()
{
    advanced_edit *ae = new advanced_edit(dev, bar, 0, 1.0/500.0, this);
    ae->setWindowTitle("Device #" + QString::number(dev->deviceAddress()));
    ae->show();
}

void par_edit::on_pushButton_calibr_B_clicked()
{
    advanced_edit *ae = new advanced_edit(dev, bar, 1, 1.0/500.0, this);
    ae->setWindowTitle("Device #" + QString::number(dev->deviceAddress()));
    ae->show();
}

void par_edit::on_pushButton_calibr_C_clicked()
{
    advanced_edit *ae = new advanced_edit(dev, bar, 2, 1.0/500.0, this);
    ae->setWindowTitle("Device #" + QString::number(dev->deviceAddress()));
    ae->show();
}

void par_edit::on_pushButton_calibr_N_clicked()
{
    advanced_edit *ae = new advanced_edit(dev, bar, 3, 1.0/500.0, this);
    ae->setWindowTitle("Device #" + QString::number(dev->deviceAddress()));
    ae->show();
}

void par_edit::on_pushButton_upd_sensors_clicked(bool checked)
{
    Q_UNUSED(checked);
    slot_SENSORS_upd();
}

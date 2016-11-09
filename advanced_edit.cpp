#include "advanced_edit.h"
#include "ui_advanced_edit.h"
#include <QMessageBox>
#include <QThread>
#include <QtMath>

#include "logger.h"

advanced_edit::advanced_edit(amper_dev *ad, QProgressBar *pb, int phase, float K, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::advanced_edit)
    , dev(ad)
    , bar(pb)
    , phase(phase)
    , K(K)
{
    ui->setupUi(this);

    tm1 = new QTimer(this);
    tm1->setInterval(200);

    tm2 = new QTimer(this);
    tm2->setInterval(100);
    QObject::connect(tm1, SIGNAL(timeout()), this, SLOT(currentReadTimer()));
    QObject::connect(tm2, SIGNAL(timeout()), this, SLOT(secundomer()));

    QString phase_str;
    switch (phase) {
    case 0: phase_str = tr("Фаза A"); break;
    case 1: phase_str = tr("Фаза В"); break;
    case 2: phase_str = tr("Фаза С"); break;
    case 3: phase_str = tr("Нейтраль"); break;
    default: break;
    }
    setWindowTitle(QString("Device #%1").arg(dev->deviceAddress())+phase_str);

    if (!dev->connect(dev->deviceAddress())) {
        log_1 << "error connect";
    }
    log_1 << "connect ok" << (int)dev->deviceAddress();
    if(!dev->getSettings()) {
        log_1 << "error getSettings";
    }
    log_1 << "getSettings ok" << (int)dev->deviceAddress();
    if (!dev->logIn("strij")) {
        log_1 << "error logIn";
    }
    log_1 << "logIn ok" << (int)dev->deviceAddress();
}

advanced_edit::~advanced_edit()
{
    delete ui;
}

uint32_t advanced_edit::new_ADE_Cal_record(uint32_t old, double mul)
{
    uint32_t mid_val = 0x800000;
    old &= 0xffffff;
    uint32_t val;
//    if (qFabs(mul) < 3.0) {
//        mul /= 2;
//    }
//    val = (uint32_t)((double)(mid_val + old) * mul);
    val = (uint32_t)((double)mid_val * mul);
    val += old;
    val -= mid_val;
    if (old >= mid_val) { // отклонение в минус от нуля
        if (val > mid_val && val < mid_val * 2) {
            val |= 0xF000000;
        } else
        if (val >= mid_val * 2) {
            val &= mid_val - 1;
        }
    } else { // отклонение в плюс от нуля
//        val = (uint32_t)((double)mid_val * mul);
//        val += old;
//        val -= mid_val;
        if (val > -mid_val) {
            val &= 0xFFFFFFF;
        } else
        if (val >= mid_val) val = mid_val - 1;
        //val = (uint32_t)((double)old * mul);
        //log_1 << "+" << old << mul << val;
    }
    return val;

//    log_1 << old << mul << (double)old * mul << (uint32_t)((double)old * mul);
//    return (uint32_t)((double)old * mul);
}

bool advanced_edit::ADE_Cal_to_ui()
{
    ui->lineEdit_IGAIN->setText(QString::number(dev->ADE_Cal.aigain, 16).toUpper());
    ui->lineEdit_VGAIN->setText(QString::number(dev->ADE_Cal.avgain, 16).toUpper());
    ui->lineEdit_WGAIN->setText(QString::number(dev->ADE_Cal.awgain, 16).toUpper());
    ui->lineEdit_VARGAIN->setText(QString::number(dev->ADE_Cal.avargain, 16).toUpper());
    ui->lineEdit_VAGAIN->setText(QString::number(dev->ADE_Cal.avagain, 16).toUpper());
    ui->lineEdit_IRMSOS->setText(QString::number(dev->ADE_Cal.airmsos, 16).toUpper());
    ui->lineEdit_VRMSOS->setText(QString::number(dev->ADE_Cal.vrmsos, 16).toUpper());
    ui->lineEdit_WATTOS->setText(QString::number(dev->ADE_Cal.awattos, 16).toUpper());
    ui->lineEdit_VAROS->setText(QString::number(dev->ADE_Cal.avaros, 16).toUpper());
    ui->lineEdit_VAOS->setText(QString::number(dev->ADE_Cal.avaos, 16).toUpper());
    return true;
}
bool advanced_edit::ui_to_ADE_Cal()
{
    bool Ok;
    quint32 value = ui->lineEdit_IGAIN->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.aigain = value;
    value = ui->lineEdit_VGAIN->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.avgain = value;
    value = ui->lineEdit_WGAIN->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.awgain = value;
    value = ui->lineEdit_VARGAIN->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.avargain = value;
    value = ui->lineEdit_VAGAIN->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.avagain = value;
    value = ui->lineEdit_IRMSOS->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.airmsos = value;
    value = ui->lineEdit_VRMSOS->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.vrmsos = value;
    value = ui->lineEdit_WATTOS->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.awattos = value;
    value = ui->lineEdit_VAROS->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.avaros = value;
    value = ui->lineEdit_VAOS->text().toUInt(&Ok, 16);
    if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return Ok;}
    dev->ADE_Cal.avaos = value;
    return Ok;
}

void advanced_edit::on_pushButton_read_ADE_Cal_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_read_ADE_Cal_clicked()));
        return;
    }
    bar->setValue(100);
    if(!dev->getADECals(phase))
    {

        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
        bar->setValue(0);
        return;
    }
    ADE_Cal_to_ui();
    bar->setValue(0);
}

void advanced_edit::on_pushButton_write_ADE_Cal_clicked()
{
    if(!dev->isReady()) {
        QTimer::singleShot(1, this, SLOT(on_pushButton_write_ADE_Cal_clicked()));
        return;
    }
    if (!ui_to_ADE_Cal()) return;

    bar->setValue(100);
    if(!dev->setADECals(phase) || !dev->runEvent(SAVE_SETTINGS)) {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        bar->setValue(0);
        return;
    }

    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");
}


//void advanced_edit::on_pushButton_4_clicked()
//{
//    bar->setValue(50);
//    if(!dev->runMethod("ADE_CAIOS", 0))
//    {
//        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
//        bar->setValue(0);
//        return;
//    }
//    on_pushButton_2_clicked();
//}

void advanced_edit::currentReadTimer()
{
    if(!dev->isReady()) return;
    bar->setValue(100);

    QVariant alfa;
    if (!dev->getCurrentData() || !dev->getValueOfProperty_byName("ADE_alfa", &alfa, 0)) {
        log_1 << "dev" << dev->deviceAddress() << "err" << dev->error_message();
        QMessageBox::warning(this, "Ошибка чтения", dev->error_message());
        ui->checkBox_read_curr_loop->clicked(false);
        bar->setValue(0);
        return;
    }
    if (!ui->checkBox_read_curr_loop->isChecked()) tm1->stop();

    ui->lineEdit_curr_I->setText(QString::number(dev->ADE_Inst.irms, 'f', 5));
    ui->lineEdit_curr_U->setText(QString::number(dev->ADE_Inst.vrms, 'f', 3));
    ui->lineEdit_curr_P->setText(QString::number(dev->ADE_Inst.aenergy * K, 'f', 3));
    ui->lineEdit_curr_Q->setText(QString::number(dev->ADE_Inst.renergy * K, 'f', 3));
    ui->lineEdit_curr_S->setText(QString::number(dev->ADE_Inst.apenergy * K, 'f', 3));

    ui->lineEdit_alfa->setText(QString::number(alfa.toFloat(), 'f', 2));

    bar->setValue(0);
}

void advanced_edit::on_pushButton_read_curr_clicked()
{
    bool tm_run = tm1->isActive();
    ui->checkBox_read_curr_loop->clicked(false);
    if (!tm_run) {
        tm1->start();
    }
}

void advanced_edit::on_checkBox_read_curr_loop_clicked(bool checked)
{
    ui->checkBox_read_curr_loop->setChecked(checked);
    if (checked) {
        ui->pushButton_read_curr->setText("Стоп");
        tm1->start();
    } else {
        ui->pushButton_read_curr->setText("Cчитать");
        tm1->stop();
    }
}

void advanced_edit::on_pushButton_7_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_7_clicked()));
        return;
    }
    bar->setValue(50);
    QVariant alfa = 0;
    dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);
    dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);

    ui->pushButton_start_stop->setChecked(false);
    ui->pushButton_start_stop->setText("Старт");
    tm2->stop();

   // QTime time(0,0,0,0);
    //ui->timeEdit->setTime(time);
    ui->lineEdit_time->setText(QString::number(0, 'f', 4));
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);

}

//void advanced_edit::on_pushButton_9_clicked()
//{

//    if(!ui->pushButton_9->isChecked())
//    {
//        bar->setValue(50);
//        QVariant alfa = 10;
//        if(!dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true))
//        {
//            /*QMessageBox::warning(this,"Ошибка записи",dev->error_message());
//            bar->setValue(0);
//            ui->pushButton_9->setChecked(false);
//            bar->setValue(0);
//            return;*/
//        }
//        bar->setValue(100);
//        QThread::msleep(100);
//        bar->setValue(0);
//    }

//}

//void advanced_edit::on_pushButton_9_toggled(bool checked)
//{

//    if(checked)
//    {
//         if(!dev->isReady())
//         {
//           QTimer::singleShot(1, ui->pushButton_9, SLOT(toggle()));
//           return;
//         }
//         bar->setValue(50);
//         QVariant alfa = 0.98;
//         if(!dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true))
//         {
//           /*  QMessageBox::warning(this,"Ошибка записи",dev->error_message());
//             bar->setValue(0);
//             ui->pushButton_9->setChecked(false);
//             bar->setValue(0);
//             return;*/
//         }
//         ui->pushButton_9->setText("Стоп");
//         bar->setValue(100);
//         QThread::msleep(100);
//         bar->setValue(0);
//         tm2->start();
//         starttime = QDateTime::currentDateTime().toMSecsSinceEpoch();
////         starttime = QTime::currentTime();


//    }
//    else
//    {

//        ui->pushButton_9->setText("Старт");
//        tm2->stop();
//        //ui->pushButton_6->setText("Старт");
//        //tm1->stop();

//    }
//}

void advanced_edit::secundomer()
{
    endtime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    double dt = endtime - starttime; //QDateTime::currentDateTime().toTime_t() - starttime;
    dt /= 3600 * 1000;
    ui->lineEdit_time->setText(QString::number(dt, 'f', 4));
    //ui->lineEdit->setText( QString::number(-QTime::currentTime().msecsTo(starttime)/1000.0/3600, 'f', 4));
}

void advanced_edit::on_pushButton_8_clicked()
{
    bool Ok;
    float value;
    float old;
    double dt = endtime - starttime;
    dt /= 3600 * 1000;
    if (!ui->lineEdit_new_I->text().isEmpty() && !ui->lineEdit_new_delta_I->text().isEmpty()) {
        value = ui->lineEdit_new_I->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        value += ui->lineEdit_new_delta_I->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        old = ui->lineEdit_curr_I->text().toFloat(&Ok);
        log_1 << "new I" << (100 - value / old * 100);
        dev->ADE_Cal.aigain = new_ADE_Cal_record(dev->ADE_Cal.aigain, value / old);
    }
    if (!ui->lineEdit_new_U->text().isEmpty() && !ui->lineEdit_new_delta_U->text().isEmpty()) {
        value = ui->lineEdit_new_U->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        value += ui->lineEdit_new_delta_U->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        old = ui->lineEdit_curr_U->text().toFloat(&Ok);
        log_1 << "new U" << (100 - value / old * 100);
        dev->ADE_Cal.avgain = new_ADE_Cal_record(dev->ADE_Cal.avgain, value / old);
    }
    if (!ui->lineEdit_new_P->text().isEmpty() && !ui->lineEdit_new_delta_P->text().isEmpty()) {
        value = ui->lineEdit_new_P->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        value += ui->lineEdit_new_delta_P->text().toFloat(&Ok) * dt;
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        old = ui->lineEdit_curr_P->text().toFloat(&Ok);
        log_1 << "new P" << (100 - value / old * 100);
        dev->ADE_Cal.awgain = new_ADE_Cal_record(dev->ADE_Cal.awgain, value / old);
    }
    if (!ui->lineEdit_new_Q->text().isEmpty() && !ui->lineEdit_new_delta_Q->text().isEmpty()) {
        value = ui->lineEdit_new_Q->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        value += ui->lineEdit_new_delta_Q->text().toFloat(&Ok) * dt;
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        old = ui->lineEdit_curr_Q->text().toFloat(&Ok);
        log_1 << "new Q" << (100 - value / old * 100);
        dev->ADE_Cal.avargain = new_ADE_Cal_record(dev->ADE_Cal.avargain, value / old);
    }
    if (!ui->lineEdit_new_S->text().isEmpty() && !ui->lineEdit_new_delta_S->text().isEmpty()) {
        value = ui->lineEdit_new_S->text().toFloat(&Ok);
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        value += ui->lineEdit_new_delta_S->text().toFloat(&Ok) * dt;
        if(!Ok) { QMessageBox::warning(this,"Ошибка ввода", "Не корректное значение"); return;}
        old = ui->lineEdit_curr_S->text().toFloat(&Ok);
        log_1 << "new S" << (100 - value / old * 100);
        dev->ADE_Cal.avagain = new_ADE_Cal_record(dev->ADE_Cal.avagain, value / old);
    }
    ADE_Cal_to_ui();
}

void advanced_edit::on_toolButton_clicked()
{
    amper_dev *device = new amper_dev(dev->interface());
    quint8 address = ui->lineEdit_19->text().toUInt();
    if (!device->interface()->checkAdd(address)) {
        log_1 << "error checkAdd" << (int)address;
        delete device;
        return;
    }
    if (!device->connect(address)) {
        log_1 << "error connect" << (int)address;
        delete device;
        return;
    }
    advanced_edit *ae = new advanced_edit(device, bar, phase, 1.0 / 2000.0, this);
    //ae->setWindowTitle("Эталон");
    ae->setWindowTitle("Device #" + QString::number(device->deviceAddress()));
    ae->show();
}

void advanced_edit::on_pushButton_4_clicked()
{
       bar->setValue(50);
       if(!dev->isReady())
       {
         QTimer::singleShot(1, ui->pushButton_4, SLOT(click()));
         return;
       }

       if(!dev->setDTI_address(ui->lineEdit_21->text().toUInt()))
       {
           QMessageBox::warning(this,"Ошибка записи",dev->error_message());
           bar->setValue(0);
           return;
       }
       setWindowTitle("Device #" + QString::number(dev->deviceAddress()));
       bar->setValue(100);
       QThread::msleep(100);
       bar->setValue(0);
       QMessageBox::information(this,"Запись данных","Успешно!");
}

void advanced_edit::on_toolButton_2_clicked()
{
    autocal_edit *ac = new autocal_edit(dev, bar, phase, this);
    ac->show();
}

void advanced_edit::on_pushButton_start_stop_clicked(bool checked)
{
    if (checked) {
        if(!dev->isReady()) {
//          QTimer::singleShot(1, ui->pushButton_start_stop, SLOT(toggle()));
          return;
        }
        bar->setValue(50);
        QVariant alfa = 0.98;
        dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);
        dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);
        ui->pushButton_start_stop->setText("Стоп");
        bar->setValue(100);
        QThread::msleep(100);
        bar->setValue(0);
        tm2->start();
        starttime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        return;
    }

    bar->setValue(50);
    QVariant alfa = 10;
    dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);
    dev->setValueOfProperty_byName("ADE_alfa", &alfa, 0, true);
    bar->setValue(100);
    QThread::msleep(100);
    bar->setValue(0);

    ui->pushButton_start_stop->setText("Старт");
    tm2->stop();
}

void advanced_edit::on_pushButton_releOn_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_releOn_clicked()));
        return;
    }
//    bar->setValue(50);
    dev->runEvent(RELAY_ON);
//    bar->setValue(100);
//    QThread::msleep(100);
//    bar->setValue(0);
}

void advanced_edit::on_pushButton_releOff_clicked()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_pushButton_releOff_clicked()));
        return;
    }
//    bar->setValue(50);
    dev->runEvent(RELAY_OFF);
//    bar->setValue(100);
//    QThread::msleep(100);
//    bar->setValue(0);
}

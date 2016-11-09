#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QProgressBar>
#include <QSerialPort>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>

#include "amper_dev.h"
#include "advanced_edit.h"
#include "settings.h"
#include "tarif_edit.h"
#include "par_edit.h"
#include "amper_dev.h"
#include "archive_view.h"
#include "settings.h"
#include "dialog_cur_value.h"
//#include "maindialog.h"
#include "DTI_url.h"

#include "logger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dtii(0)
    , dev(0)
{
    ui->setupUi(this);

    DTI_url * durl = new DTI_url(this);
    dtii = new DTI_interface(durl);
    dev = new amper_dev;
    dev->setInterface(dtii);

    bar = new QProgressBar(this);
    bar->setMaximumHeight(10);
    ui->statusbar->addWidget(bar, 10);
    slot_set_win_title(QString());
    connect(ui->action_exit, SIGNAL(triggered(bool)), this, SLOT(slot_exit()));
    connect(ui->action_connect, SIGNAL(triggered(bool)), this, SLOT(slot_connect(bool)));
    connect(this, SIGNAL(destroyed(QObject*)), this, SLOT(slot_exit()));

    ui->menu_tools->setEnabled(false);

    setCentralWidget(new settings(dev, bar, this));

//    durl->slot_get_API_key();
//    durl->slot_UL_read("limit=10&modem=6F2367");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_set_win_title(QString titl)
{
    QString t = tr("Конфигуратор электросчетчиков СТРИЖ");
    if (titl.isEmpty()) {
        setWindowTitle(t);
    } else {
        setWindowTitle(t + tr(" — ") + titl);
    }
}

void MainWindow::slot_connect(bool isConnecting)
{
    if (isConnecting) {
        slot_connect();
    } else {
        slot_disconnect();
    }
}

void MainWindow::slot_connect()
{
    log_1 << "connect start";
    settings * set = qobject_cast<settings *>(centralWidget());
    if (!set) return;

    bar->setEnabled(true);
    bar->setValue(10);

    QSettings dti_settings("dti.ini", QSettings::IniFormat, 0);

    dti_settings.beginGroup("Comunications");
    if (dti_settings.value("way_to_connect","").toString() == "URL") {
        log_1 << "open url";
        dev->interface()->url_api_key = dti_settings.value("URL_API_KEY","").toString();
        dev->interface()->dev_id = dti_settings.value("dev_id","").toString().toInt(0, 16);
        if (dev->interface()->url_api_key.isEmpty()) {
            bar->setEnabled(false);
            bar->setValue(0);
            QMessageBox::warning(this,"Ошибка соединения","Не задан API key");
            return;
        }
//        dev->interface()->url_set_last_timestamp();
//        log_1 << "last timestamp" << dev->interface()->server_last_timestamp << QDateTime::fromTime_t(dev->interface()->server_last_timestamp).toString("yyyy_MM_dd___hh_mm_ss");
//        return;
    } else {
        log_1 << "open port";// << dev->interface()->serial->portName();
        QSerialPort::SerialPortError error = dev->interface()->openport();
        if(error) {
            bar->setEnabled(false);
            bar->setValue(0);
            QMessageBox::warning(this,"Ошибка соединения",dev->error_message());
            return;
        }
    }
    dti_settings.endGroup();

    bar->setValue(30);

    //QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);
    dti_settings.beginGroup("Comunications");
    quint8 address = dti_settings.value("Address","0").toUInt();
    dti_settings.endGroup();
    //delete pSettings;

    log_1 << "dev connect" << address;
    if(!dev->connect(address)) {
        dev->interface()->closeport();
        bar->setEnabled(false);
        bar->setValue(0);
        //QTimer::singleShot(1000, this, SLOT(slot_connect()));
        ui->action_connect->setChecked(false);
        QMessageBox::warning(this,"Ошибка соединения",dev->error_message());
        return;
    }
    bar->setValue(50);
    log_1 << "dev getSettings";
    if(dev->getSettings()) {
        slot_set_win_title(QString::number(dev->Settings.ProtC_ID, 16).toUpper());
    }
    if(!dev->logIn(set->password)) {
        QMessageBox::warning(this,"Ошибка аутентификациии", "Только чтение!");
    }
    set->logged(dev->isLogged());
    bar->setValue(100);

    ui->menu_tools->setEnabled(true);
//    QWidget * w = centralWidget();
//    if (w) {
//        w->deleteLater();
//    }

//    setCentralWidget(new tarif_edit(dev, bar, this));
//    Dialog * mainD = new Dialog(dev, bar, this);
//    setCentralWidget(mainD);
//    connect(mainD, SIGNAL(signal_set_win_title(QString)), this, SLOT(slot_set_win_title(QString)));
//    mainD->hide_all();
//    mainD->show();

    log_1 << "connect end";
}

void MainWindow::slot_disconnect()
{
    bar->setValue(0);

    ui->menu_tools->setEnabled(false);

    QWidget * w = centralWidget();
    if (w) {
        w->deleteLater();
    }
    setCentralWidget(new settings(dev, bar, this));

    bar->setValue(10);
    if(dev->isLogged()) dev->runEvent(RESET_FAST_DL);
    bar->setValue(30);
    dev->disconnect();
    bar->setValue(60);
    dev->interface()->closeport();
    bar->setValue(100);
}

void MainWindow::calibration_phase(int phase)
{
    close_central_widget();
//    setCentralWidget(new advanced_edit(dev, bar, phase, this));
    advanced_edit *ae = new advanced_edit(dev, bar, phase, 1.0/500.0, this);
    ae->setWindowFlags(Qt::Widget);
    setCentralWidget(ae);
//    ae->show();
    QString phase_str;
    switch (phase) {
    case 0: phase_str = tr("Фаза A"); break;
    case 1: phase_str = tr("Фаза В"); break;
    case 2: phase_str = tr("Фаза С"); break;
    case 3: phase_str = tr("Нейтраль"); break;
    default: break;
    }
    ui->statusbar->showMessage(phase_str);
}

void MainWindow::close_central_widget()
{
    QWidget * w = centralWidget();
    if (w) {
        w->close();
        QTimer::singleShot(1, w, SLOT(deleteLater()));
    }
}

void MainWindow::on_action_calibration_A_triggered()
{
    calibration_phase(0);
}

void MainWindow::on_action_calibration_B_triggered()
{
    calibration_phase(1);
}

void MainWindow::on_action_calibration_C_triggered()
{
    calibration_phase(2);
}

void MainWindow::on_action_calibration_N_triggered()
{
    calibration_phase(3);
}

void MainWindow::on_action_curr_triggered()
{
    close_central_widget();
    setCentralWidget(new dialog_CUR_VALUE(dev, bar, this));
}

void MainWindow::on_action_tariff_triggered()
{
    close_central_widget();
    setCentralWidget(new tarif_edit(dev, bar, this));
}

void MainWindow::on_action_arch_triggered()
{
    close_central_widget();
    setCentralWidget(new archive_view(dev, bar, this));
}

void MainWindow::on_action_dev_settings_triggered()
{
    close_central_widget();
    setCentralWidget(new par_edit(dev, bar, this));
}

void MainWindow::on_action_update_sw_triggered()
{
    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(on_action_update_sw_triggered()));
        return;
    }
    bar->setValue(0);
    QString s = QFileDialog::getOpenFileName(this, tr("Open File"),"","*.bin");
    if(s == "") return;
    soft_update s_up;
    s_up.start_add = 0x800;
    s_up.ver = 9;
    s_up.reserved[0] = s_up.reserved[1] = 0;
    //dev->writeFirmware(&s_up,s.toLocal8Bit().data());
    QFile binFile(s);
    binFile.open(QIODevice::ReadOnly);
    s_up.end_add = s_up.start_add + binFile.size();
    QByteArray ba = binFile.readAll();
    binFile.close();
    s_up.crc = dev->CRC_calc(ba.data(), binFile.size());
    s_up.crc_of_this_struct = dev->CRC_calc((char *)&s_up, sizeof(s_up) - 2);

    if(!dev->runEvent(CLEAR_UPDATE_AREA)) {
       bar->setValue(0);
       QMessageBox::warning(this,"Ошибка записи",dev->error_message());
       return;
    }

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();

    for(int i = 0; i < ba.size(); i += 1024)
    {
        if(!dev->writeData(EE_RW, UPDATE_ADD + s_up.start_add + i, 1024, &ba.data()[i]))
        {
            bar->setValue(0);
            QMessageBox::warning(this,"Ошибка записи",dev->error_message());
            return;
        }
        bar->setValue(i*100/ba.size());
    }

    if(!dev->writeData(EE_RW, UPDATE_ADD, sizeof(soft_update), &s_up))
    {
        bar->setValue(0);
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        return;
    }

    if(!dev->runEvent(UPDATE_CHECK))
    {
        bar->setValue(0);
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        return;
    }
    bar->setValue(0);

    QMessageBox::information(this,"Запись данных","Успешно!");
}

void MainWindow::on_action_connect_settings_triggered()
{
    close_central_widget();
    settings * set = new settings(dev, bar, this);
    setCentralWidget(set);
    set->logged(dev->isLogged());
}

#include <Windows.h>
void MainWindow::slot_exit()
{
    log_1 << "exit";
    qApp->quit();
    TerminateProcess(GetCurrentProcess(), 0);
}

//6f2367 calibr
// 53fc62
// 30fe8e
// 118a53
// 1630e2
// 121563

//6f2364 calibr C
// 10325
// FF563B1
// 0
// 0
// 0
// FFA6120

//6f2364 A
// FFE5DCE      FFD6A06
// FF5A7C3      FF5A7C3
// 7789D        7A210
// 2B137C       2C9202
// C2BEC        C2BEC
//6f2364 B
// FFE5DCC      FFDA4E3
// FF6389C      FF6389C
// 75A36        6936D   0.15%   1.28%
// 28F862       2AB447  0.48%   0.74%
// BE015        BE015
//6f2364 C
// FFE402A
// FF5D7C9
// 883DA
// 2BFD6F
// D1A5C
//6f2364 N
// FF6EF1F


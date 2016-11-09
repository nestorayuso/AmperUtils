#include "maindialog.h"
#include "ui_maindialog.h"
#include <QThread>
#include <QFont>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include "logger.h"

Dialog::Dialog(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
//    , interface(0)
    , dev(ad)
    , pb(pb)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    connected = 0;
    currentW = 0;
//    interface = new DTI_interface;
//    dev = new amper_dev;
//    dev->setInterface(interface);

    te = new tarif_edit(dev, pb, this);
   // ui->gridLayout->addWidget(te);
   // te->setWindowFlags(Qt::Widget);
//    te->setAmperDev(dev);
//    te->setProgressBar(pb);
//    wte = new widget_tariff_edit(dev, pb, this);

    pe = new par_edit(dev, pb, this);
//    pe->setWindowFlags(Qt::Widget);
//    pe->setAmperDev(dev);
//    pe->setProgressBar(pb);

    set = new settings(dev, pb, this);
    //set->setWindowFlags(Qt::Widget);
//    set->setAmperDev(dev);
//    set->setProgressBar(pb);

    av = new archive_view(dev, pb, this);
    //av->setWindowFlags(Qt::Widget);
    //av->setAmperDev(dev);
    //av->setProgressBar(pb);

    QObject::connect(ui->pushButton_5, SIGNAL(clicked(bool)), this, SLOT(conButClicked(bool)));
    QObject::connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(submenu1Clicked(bool)));
    QObject::connect(ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(submenu2Clicked(bool)));
    QObject::connect(ui->pushButton_4, SIGNAL(clicked(bool)), this, SLOT(submenu3Clicked(bool)));
    QObject::connect(ui->pushButton_3, SIGNAL(clicked(bool)), this, SLOT(submenu4Clicked(bool)));
    QObject::connect(ui->pushButton_6, SIGNAL(clicked(bool)), this, SLOT(submenu5Clicked(bool)));

//    titl = windowTitle();
//log_1 << titl;
    //ui->pushButton_5->click();

    connected = 1;
    ui->pushButton_5->setText("Разорв.соед.");
    QThread::msleep(100);
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_4->setEnabled(true);
    ui->pushButton_6->setEnabled(true);

}

void Dialog::hide_all()
{
    //wte->hide();
    te->hide();
    pe->hide();
    set->hide();
    av->hide();
}

Dialog::~Dialog()
{
    //log_1 << "~Dialog()";
//    delete te;
//    delete av;
//    delete set;
//    delete pe;

//    delete dev;
//    delete interface;
    delete ui;
    //log_1 << "~Dialog()";
}


void Dialog::conButClicked(bool checked)
{
    if(!connected)
    {
        pb->setEnabled(true);
        pb->setValue(10);
        log_1 << "open port";
        QSerialPort::SerialPortError error = dev->interface()->openport();
        if(error)
        {
            pb->setEnabled(false);
            pb->setValue(0);
            QMessageBox::warning(this,"Ошибка соединения",dev->error_message());
            return;
        }
        pb->setValue(30);

        QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);
        pSettings->beginGroup("Comunications");
        quint8 address = pSettings->value("Address","0").toUInt();
        pSettings->endGroup();
        delete pSettings;

        log_1 << "dev connect" << address;
        if(!dev->connect(address))
        {
            dev->interface()->closeport();
            pb->setEnabled(false);
            pb->setValue(0);
            QMessageBox::warning(this,"Ошибка соединения",dev->error_message());
            return;
        }
        pb->setValue(50);
        log_1 << "dev getSettings";
        if(dev->getSettings())
        {
            log_1 << QString::number(dev->Settings.ProtC_ID, 16).toUpper();
            emit signal_set_win_title(QString::number(dev->Settings.ProtC_ID, 16).toUpper());
        }
        if(!dev->logIn(set->password))
        {
            QMessageBox::warning(this,"Ошибка аутентификациии", "Только чтение!");
        }
        set->logged(dev->isLogged());
        pb->setValue(100);
        connected = 1;
        ui->pushButton_5->setText("Разорв.соед.");
        QThread::msleep(100);
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_4->setEnabled(true);
        ui->pushButton_6->setEnabled(true);
        pb->setValue(0);
    }
    else
    {

        if(currentW)
        {
            currentW->close();
            ui->gridLayout->removeWidget(currentW);
            currentW->hide();
            currentW = 0;

        }
        if(dev->isLogged()) dev->runEvent(RESET_FAST_DL);
        dev->disconnect();
        dev->interface()->closeport();
        set->logged(false);
        QFont font;
        ui->pushButton_2->font();
        font.setPointSize(10);
        font.setBold(false);
        ui->pushButton->setFont(font);
        ui->pushButton_3->setFont(font);
        ui->pushButton_4->setFont(font);
        ui->pushButton_2->setFont(font);
        ui->pushButton_6->setFont(font);
        pb->setEnabled(false);
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
        ui->pushButton_6->setEnabled(false);
        ui->pushButton_5->setText("Установить соед.");
        connected = 0;
    }

}

void Dialog::submenu1Clicked(bool)
{
    if(currentW)
    {
        ui->gridLayout->removeWidget(currentW);
        currentW->hide();
    }
    else
    {
        te->hide();
        //wte->hide();
    }
    QFont font;
    ui->pushButton_2->font();
    font.setPointSize(10);
    font.setBold(false);
    ui->pushButton->setFont(font);
    ui->pushButton_3->setFont(font);
    ui->pushButton_4->setFont(font);
    ui->pushButton_2->setFont(font);
    ui->pushButton_6->setFont(font);

    ui->gridLayout->addWidget(pe);
    currentW = pe;
    font.setPointSize(11);
    font.setBold(true);
    ui->pushButton->setFont(font);
    currentW->show();
    pe->readAllData();

}

void Dialog::submenu2Clicked(bool)
{

  //  te->show();

    if(currentW)
    {
        ui->gridLayout->removeWidget(currentW);
        currentW->hide();
    }
    else
    {
        pe->hide();

    }
    QFont font;
    ui->pushButton_2->font();
    font.setPointSize(10);
    font.setBold(false);
    ui->pushButton->setFont(font);
    ui->pushButton_3->setFont(font);
    ui->pushButton_4->setFont(font);
    ui->pushButton_2->setFont(font);
    ui->pushButton_6->setFont(font);

    currentW = te;
//    currentW = wte;
    ui->gridLayout->addWidget(te);
    font.setPointSize(11);
    font.setBold(true);
    ui->pushButton_2->setFont(font);
    currentW->show();
    te->readAllData();

}

void Dialog::submenu3Clicked(bool)
{

    if(currentW)
    {
        ui->gridLayout->removeWidget(currentW);
        currentW->hide();
    }
    QFont font;
    ui->pushButton_2->font();
    font.setPointSize(10);
    font.setBold(false);
    ui->pushButton->setFont(font);
    ui->pushButton_3->setFont(font);
    ui->pushButton_4->setFont(font);
    ui->pushButton_2->setFont(font);
    ui->pushButton_6->setFont(font);

    ui->gridLayout->addWidget(av);
    currentW = av;
    font.setPointSize(11);
    font.setBold(true);
    ui->pushButton_4->setFont(font);
    currentW->show();

}

void Dialog::submenu4Clicked(bool)
{

    if(currentW)
    {
        ui->gridLayout->removeWidget(currentW);
        currentW->hide();
    }
    QFont font;
    ui->pushButton_2->font();
    font.setPointSize(10);
    font.setBold(false);
    ui->pushButton->setFont(font);
    ui->pushButton_3->setFont(font);
    ui->pushButton_4->setFont(font);
    ui->pushButton_2->setFont(font);
    ui->pushButton_6->setFont(font);
    ui->gridLayout->addWidget(set);
    currentW = set;
    font.setPointSize(11);
    font.setBold(true);
    ui->pushButton_3->setFont(font);
    currentW->show();

}

void Dialog::submenu5Clicked(bool)
{
    if(currentW)
    {
        ui->gridLayout->removeWidget(currentW);
        currentW->hide();
    }
    QFont font;
    ui->pushButton_2->font();
    font.setPointSize(10);
    font.setBold(false);
    ui->pushButton->setFont(font);
    ui->pushButton_3->setFont(font);
    ui->pushButton_4->setFont(font);
    ui->pushButton_2->setFont(font);

    //ui->gridLayout->addWidget(te);
    currentW = 0;
    font.setPointSize(11);
    font.setBold(true);
    ui->pushButton_6->setFont(font);


    if(!dev->isReady())
    {
        QTimer::singleShot(1, this, SLOT(submenu5Clicked()));
        return;
    }
    pb->setValue(0);
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

    if(!dev->runEvent(CLEAR_UPDATE_AREA))
    {
       pb->setValue(0);
       QMessageBox::warning(this,"Ошибка записи",dev->error_message());
       return;
    };

    QThread::msleep(2000);

    for(int i = 0; i < ba.size(); i += 1024)
    {
        if(!dev->writeData(EE_RW, UPDATE_ADD + s_up.start_add + i, 1024, &ba.data()[i]))
        {
            pb->setValue(0);
            QMessageBox::warning(this,"Ошибка записи",dev->error_message());
            return;
        }
        pb->setValue(i*100/ba.size());
    }

    if(!dev->writeData(EE_RW, UPDATE_ADD, sizeof(soft_update), &s_up))
    {
        pb->setValue(0);
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        return;
    }

    if(!dev->runEvent(UPDATE_CHECK))
    {
        pb->setValue(0);
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
        return;
    }
    pb->setValue(0);

    QMessageBox::information(this,"Запись данных","Успешно!");

}

//#include <Windows.h>

//void Dialog::done(int r)
//{
//    QDialog::done(r);
//    qApp->quit();
//    TerminateProcess(GetCurrentProcess(), 0);
//}

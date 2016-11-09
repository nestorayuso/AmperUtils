#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QShortcut>
#include <QKeySequence>
#include "dti.h"
#include "autocal_edit.h"

settings::settings(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::settings)
    , dev(ad)
    , bar(pb)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    QShortcut * sh = new QShortcut(QKeySequence("F5"), this);
    connect(sh, SIGNAL(activated()), this, SLOT(update_ports()));

    update_ports();

    QValidator* validator1 = new QIntValidator( 10, 9999, this );
    QValidator* validator2 = new QIntValidator( 0,254, this );
    QValidator* validator3 = new QIntValidator( 0,9999, this );
    QValidator* validator4 = new QIntValidator( 1,20, this );
    ui->TimeOut->setValidator(validator1);
    ui->Address->setValidator(validator2);
    ui->Delay->setValidator(validator3);
    ui->Tries->setValidator(validator4);

    QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);

    pSettings->beginGroup("Comunications");
    ui->SelComm->setCurrentText(pSettings->value("PortNum","COM1").toString());
    ui->SelSpeed->setCurrentText(pSettings->value("BaudeRate","9600").toString());
    ui->Address->setText(pSettings->value("Address","0").toString());
    ui->TimeOut->setText(pSettings->value("Timeout","200").toString());
    ui->Delay->setText(pSettings->value("StrWriteDelay","2").toString());
    ui->Tries->setText(pSettings->value("TryNum","5").toString());
    ui->lineEdit_url_api_key->setText(pSettings->value("URL_API_KEY","").toString());
    if (pSettings->value("way_to_connect","").toString() == "COM") ui->radioButton_COM->setChecked(true);
    if (pSettings->value("way_to_connect","").toString() == "URL") ui->radioButton_URL->setChecked(true);
    ui->lineEdit_dev_id->setText(pSettings->value("dev_id","").toString());

    //ui->Password_edit->setMaxLength(PASSWORD_MAX_LEN);
    QByteArray ba = pSettings->value("PASSWORD","").toString().toLocal8Bit();
    if(ba != "")
    {
        ba = QByteArray::fromHex(ba);
        for(uint i = 0; i != ba.size(); i++ ) ba[i] = (~ba[i])^0xaa;
        ui->Password_edit->setEchoMode(QLineEdit::Password);
        ui->Password_edit->setCursorPosition(0);
        ui->Password_edit->setAlignment(Qt::AlignLeft);
        QPalette pal = ui->Password_edit->palette();
        pal.setColor(QPalette::Text, QColor::fromRgb(0,0,0));
        ui->Password_edit->setPalette(pal);
        QFont font = ui->Password_edit->font();
        font.setItalic(false);
        ui->Password_edit->setFont(font);
        ui->Password_edit->setText(ba.data());
        password = ba;
    }

    pSettings->endGroup();

    delete pSettings;

    QObject::connect(ui->SelComm, SIGNAL(currentIndexChanged(int)), this, SLOT(ValueChanged()));
    QObject::connect(ui->SelSpeed, SIGNAL(currentIndexChanged(int)), this, SLOT(ValueChanged()));
    QObject::connect(ui->TimeOut, SIGNAL(textChanged(QString)), this, SLOT(ValueChanged()));
    QObject::connect(ui->Tries, SIGNAL(textChanged(QString)), this, SLOT(ValueChanged()));
    QObject::connect(ui->Delay, SIGNAL(textChanged(QString)), this, SLOT(ValueChanged()));
    QObject::connect(ui->Address, SIGNAL(textChanged(QString)), this, SLOT(ValueChanged()));
    QObject::connect(ui->lineEdit_url_api_key, SIGNAL(textChanged(QString)), this, SLOT(ValueChanged()));
    QObject::connect(ui->radioButton_COM, SIGNAL(clicked(bool)), this, SLOT(ValueChanged()));
    QObject::connect(ui->radioButton_URL, SIGNAL(clicked(bool)), this, SLOT(ValueChanged()));

    ui->pushButton_autoA->setEnabled(false);
    ui->pushButton_autoB->setEnabled(false);
    ui->pushButton_autoC->setEnabled(false);
    ui->pushButton_autoN->setEnabled(false);
}

void settings::ValueChanged()
{
    QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);
    pSettings->beginGroup("Comunications");
    pSettings->setValue("PortNum", ui->SelComm->currentData().toString());
    pSettings->setValue("BaudeRate", ui->SelSpeed->currentText());
    pSettings->setValue("Timeout", ui->TimeOut->text());
    pSettings->setValue("TryNum", ui->Tries->text());
    pSettings->setValue("Address", ui->Address->text());
    pSettings->setValue("StrWriteDelay", ui->Delay->text());

    pSettings->setValue("URL_API_KEY", ui->lineEdit_url_api_key->text());
    if (ui->radioButton_COM->isChecked()) pSettings->setValue("way_to_connect", "COM");
    if (ui->radioButton_URL->isChecked()) pSettings->setValue("way_to_connect", "URL");
    pSettings->setValue("dev_id", ui->lineEdit_dev_id->text().toUpper());
    pSettings->endGroup();
    delete pSettings;
}

settings::~settings()
{
    delete ui;
}

void settings::update_ports()
{
    QSettings pSettings("dti.ini", QSettings::IniFormat, 0);
    pSettings.beginGroup("Comunications");
    QString port = pSettings.value("PortNum","").toString();

    ui->SelComm->clear();
    QList<QSerialPortInfo> spa = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo spi, spa) {
        QString str;
        str.append(spi.portName()); str.append(" ");
        str.append(spi.manufacturer()); str.append(" ");
        str.append(spi.serialNumber());
        if (spi.hasProductIdentifier()) {
            str.append(" ");
            str.append(QString::number(spi.productIdentifier(), 16).toUpper());
        }
        if (spi.hasVendorIdentifier()) {
            str.append(" ");
            str.append(QString::number(spi.vendorIdentifier(), 16).toUpper());
        }
        ui->SelComm->addItem(str, spi.portName());
        if (port == spi.portName()) {
            ui->SelComm->setCurrentIndex(ui->SelComm->count() - 1);
        }
    }

}

bool locked = 0;

void settings::on_Password_edit_cursorPositionChanged(int arg1, int arg2)
{
    Q_UNUSED(arg1);
    Q_UNUSED(arg2);
    if(locked) return;
    locked = 1;
    if(ui->Password_edit->echoMode() != QLineEdit::Password || ui->Password_edit->text().left(14) == "Введите пароль")
    {
        if((ui->Password_edit->text() != "Введите пароль")&&(ui->Password_edit->text().left(14) == "Введите пароль"))
        {
            ui->Password_edit->setText(ui->Password_edit->text().right(1));
            ui->Password_edit->setCursorPosition(1);
        }
        else
        {
            ui->Password_edit->setText("");
            ui->Password_edit->setCursorPosition(0);
        }

        ui->Password_edit->setEchoMode(QLineEdit::Password);
        ui->Password_edit->setAlignment(Qt::AlignLeft);
        QPalette pal = ui->Password_edit->palette();
        pal.setColor(QPalette::Text, QColor::fromRgb(0,0,0));
        ui->Password_edit->setPalette(pal);
        QFont font = ui->Password_edit->font();
        font.setItalic(false);
        ui->Password_edit->setFont(font);
    }
    locked = 0;
}

void settings::on_Password_edit_editingFinished()
{
    if(locked) return;
    locked = 1;
    if((ui->Password_edit->text() == "")||(ui->Password_edit->text() == "Введите пароль")) {locked = 1; return;}
    if(ui->Password_edit->text().size() > PASSWORD_MAX_LEN)
    {
        QMessageBox::warning(this,"Ошибка ввода", "Максимум 9 символов!");
        ui->Password_edit->setText("");
        locked = 0;
        return;
    }
    password = ui->Password_edit->text().toLocal8Bit();
    QSettings *pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);
    pSettings->beginGroup("Comunications");
    QByteArray ba = password;
    for(uint i = 0; i != ba.size(); i++ ) ba[i] = (~ba[i])^0xaa;
    ba = ba.toHex();
    pSettings->setValue("PASSWORD", ba.data());
    pSettings->endGroup();
    delete pSettings;
    locked = 0;
}


void settings::on_Password_edit_textChanged(const QString &arg1)
{
    if(locked) return;
    locked = 1;

    if(arg1 == "")
    {
        ui->Password_edit->setText("Введите пароль");
        ui->Password_edit->setEchoMode(QLineEdit::Normal);
        ui->Password_edit->setAlignment(Qt::AlignHCenter);
        QPalette pal = ui->Password_edit->palette();
        pal.setColor(QPalette::Text, QColor::fromRgb(156,156,156));
        ui->Password_edit->setPalette(pal);
        QFont font = ui->Password_edit->font();
        font.setItalic(true);
        ui->Password_edit->setFont(font);
    }
    locked = 0;
}

void settings::logged(bool log)
{
    ui->Edit_Pass_label->setEnabled(log);
    ui->Edit_Pass_label_2->setEnabled(log);
    ui->New_Password_edit->setEnabled(log);
    ui->New_Password_edit_2->setEnabled(log);
    ui->pushButton->setEnabled(log);

    ui->pushButton_autoA->setEnabled(log);
    ui->pushButton_autoB->setEnabled(log);
    ui->pushButton_autoC->setEnabled(log);
    ui->pushButton_autoN->setEnabled(log);
}

void settings::on_pushButton_clicked()
{
    if((ui->New_Password_edit->text().size() < 4)||(ui->New_Password_edit->text().size() > PASSWORD_MAX_LEN )) {
        QMessageBox::warning(this,"Ошибка ввода", "Пароль должен быть длиной 4-9 симв.!");
        return;
    }
    if(ui->New_Password_edit->text() != ui->New_Password_edit_2->text()) {
        QMessageBox::warning(this,"Ошибка ввода", "Повторный ввод не соответствует первому!");
        return;
    }
    bar->setValue(100);
    if(!dev->changePassword(ui->New_Password_edit->text().toLocal8Bit())) {
        QMessageBox::warning(this,"Ошибка записи",dev->error_message());
    }
    bar->setValue(0);
    QMessageBox::information(this,"Запись данных","Успешно!");
}

void settings::on_pushButton_autoA_clicked()
{
    autocal_edit *ac = new autocal_edit(dev, bar, 0, this);
    ac->show();
}

void settings::on_pushButton_autoB_clicked()
{
    autocal_edit *ac = new autocal_edit(dev, bar, 1, this);
    ac->show();
}

void settings::on_pushButton_autoC_clicked()
{
    autocal_edit *ac = new autocal_edit(dev, bar, 2, this);
    ac->show();
}

void settings::on_pushButton_autoN_clicked()
{
    autocal_edit *ac = new autocal_edit(dev, bar, 3, this);
    ac->show();
}

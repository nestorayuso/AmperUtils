#include "dti.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QApplication>
#include <QJsonDocument>

#include "logger.h"

static quint8 CRC8(quint8 * data, quint8 len);

static uint8_t slip_CRC8byte(uint8_t data)
{
   uint8_t crc = 0;
   if(data & 1)     crc ^= 0x5e;
   if(data & 2)     crc ^= 0xbc;
   if(data & 4)     crc ^= 0x61;
   if(data & 8)     crc ^= 0xc2;
   if(data & 0x10)  crc ^= 0x9d;
   if(data & 0x20)  crc ^= 0x23;
   if(data & 0x40)  crc ^= 0x46;
   if(data & 0x80)  crc ^= 0x8c;
   return crc;
}

uint8_t slip_CRC8(uint8_t * data, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc = slip_CRC8byte(data[i] ^ crc);
    }
    return crc;
}

DTI_interface::DTI_interface(DTI_url *dti_url)
    : dti_url(dti_url)
    , server_last_timestamp(0)
{
    locker_state = false;

    serial = new QSerialPort;

    pSettings = new QSettings("dti.ini", QSettings::IniFormat, 0);

    QFile file("dti.ini");
    if(!file.exists())
    {
        qDebug() << "dti.ini not found!";
    }

    pSettings->sync();
    if(pSettings->status() != QSettings::NoError)
    {
        qDebug() << "dti.ini format wrong!";

    }

    setSLIP(1);
    ready = 1;
    no_wait = 0;
    last_error = ERR_SUCCESS;

//    QByteArray arr_rx = QByteArray::fromHex("3b0105e400000032");
//    if (arr_rx.size() <= 8) {
//        log_1 << "error CRC len" << arr_rx.size() << arr_rx.toHex();
//        while (arr_rx.size() >= 1) {
//            arr_rx.remove(arr_rx.size() - 1, 10);
//            if ((quint8)arr_rx[arr_rx.size() - 1] == CRC8((quint8*)arr_rx.data(),(quint8)(arr_rx.size() - 1))) {
//                log_1 << "ok CRC len" << arr_rx.size() << arr_rx.toHex() << (int)arr_rx[arr_rx.size() - 1];
//                break;
//            }
//            log_1 << "error CRC len" << arr_rx.size() << arr_rx.toHex();
//            log_1 << QString::number((quint8)arr_rx[arr_rx.size() - 1], 16)
//                  << QString::number(CRC8((quint8*)arr_rx.data(),(quint8)(arr_rx.size() - 1)), 16);
//        }
//    }
}

DTI_interface::~DTI_interface()
{
    closeport();
    delete serial;

}

QSerialPort::SerialPortError DTI_interface::openport()
{

    pSettings->beginGroup("Comunications");
    serial->setPortName(pSettings->value("PortNum").toString());
    if (!serial->open(QIODevice::ReadWrite)) {
        log_1 << "cant open" << serial->portName();
        pSettings->endGroup();
        return serial->error();
    }
    log_1 << "open" << serial->portName();
    serial->setBaudRate(pSettings->value("BaudeRate").toInt());
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    ReadTimeout = pSettings->value("Timeout").toInt();
    TryNum = pSettings->value("TryNum").toInt();
    StrWriteDelay = pSettings->value("StrWriteDelay").toInt();
    pSettings->endGroup();
    //QSerialPort::SerialPortError e = serial->error();
    if(serial->error()) last_error = ERR_QSERIAL;
    else last_error = ERR_SUCCESS;
    return serial->error();
}

void DTI_interface::closeport()
{
    if (serial->isOpen())
        serial->close();
}



quint8 DTI_interface::HexAsciiToChar(quint8 a, quint8 b)
{
    quint8 result=0xFF;
    if (a>=0x30 && a<=0x39) result=(a-0x30)*16;
    if (a>=0x41 && a<=0x46) result=(a-0x37)*16;
    if (result==0xFF) return 0xFF;
    if (b>=0x30 && b<=0x39) result+=(b-0x30);
    if (b>=0x41 && b<=0x46) result+=(b-0x37);
    return result;
}

void DTI_interface::CharToHexAscii(quint8 c, quint8 *h1,  quint8 *h2)
{

    if((c>>4)>9) *h1 = (c>>4)+0x37;
    else *h1 = (c>>4)+0x30;

    if((c&0x0F)>9) *h2 = (c&0x0F)+0x37;
    else *h2=(c&0x0F)+0x30;

}

quint8 DTI_interface::Tele_CalcLRC(quint8* Mas, qint32 Num)
{
    qint32 i;
    quint8 Sum=0;
    for (i=0;i<Num;i++) Sum+=Mas[i];
    return (0xFF-Sum)+1;
}



bool DTI_interface::SLIP_Send(quint8 cmd, quint8 *payload, quint8 len)
{
    // Encode to SLIP

    QByteArray ba;\
    ba.append(SLIP_START);
    serial->write(ba);
    serial->flush();
    serial->waitForBytesWritten(10);
    /*//QEventLoop loop;
    //QTimer::singleShot(10, &loop, SLOT(quit()));
    //loop.exec();*/
    QThread::msleep(10);
    ba.append(cmd);
    for(int i=0; i<len; i++)
    {
        if((payload[i]==SLIP_START) || (payload[i]==SLIP_END) || (payload[i]==SLIP_ESC))
        {
            ba.append(SLIP_ESC);
            ba.append(payload[i] ^ 0xFF);
        }
        else ba.append(payload[i]);
    }
    ba.append(SLIP_END);

    if(serial->write(ba) == -1)
    {
        last_error = ERR_QSERIAL;
        return false;
    }

    return true;
}


bool DTI_interface::SLIP_SetRxMode(quint8 DL_type, quint8 mode)
{
    if(!isSLIP) return true;
    quint8 data[10];
    data[0] = 0x05;
    data[1] = mode;
    data[2] = 0;
    data[3] = 0;
//    if(!m_mutex.tryLock(200)) {last_error = ERR_PORT_LOCKED; return false;}
    if(!locker_lock()) {last_error = ERR_PORT_LOCKED; return false;}
    serial->clear();
//    QThread::msleep(StrWriteDelay);
    QEventLoop loop;
    QTimer::singleShot(StrWriteDelay, &loop, SLOT(quit()));
    loop.exec();
    if(!SLIP_Send(DL_type, data, 4 )) {
        locker_unlock();
        //m_mutex.unlock();
        return false;
    }
    serial->flush();
    serial->waitForBytesWritten(200);
    locker_unlock();
    //m_mutex.unlock();
    //QThread::msleep(5000);
    return true;
}

static quint8 CRC8byte(quint8 data)
{
   quint8 crc = 0;
   if(data & 1)     crc ^= 0x5e;
   if(data & 2)     crc ^= 0xbc;
   if(data & 4)     crc ^= 0x61;
   if(data & 8)     crc ^= 0xc2;
   if(data & 0x10)  crc ^= 0x9d;
   if(data & 0x20)  crc ^= 0x23;
   if(data & 0x40)  crc ^= 0x46;
   if(data & 0x80)  crc ^= 0x8c;
   return crc;
}

static quint8 CRC8(quint8 * data, quint8 len)
{
    quint8 crc = 0;
    for(quint8 i = 0; i < len; i++)
    {
        crc = CRC8byte(data[i] ^ crc);
    }
    return crc;
}


QByteArray CMD_to_SLIP(quint8 cmd, quint8 *payload, quint8 len)
{
    QByteArray ba;
    ba.append(SLIP_START);
    ba.append(cmd);
    for(int i=0; i<=len; i++) {
        quint8 c;
        if (i == len) {
            if (1) c = CRC8(payload, len);
            else break;
        } else c = payload[i];

        if ((c==SLIP_START) || (c==SLIP_END) || (c==SLIP_ESC)) {
            ba.append(SLIP_ESC);
            ba.append(c ^ 0xFF);
        } else ba.append(c);
    }
    ba.append(SLIP_END);

    return ba;
}

enum{SLIP_MODE_START, SLIP_MODE_RECEIVING_PAYLOAD, SLIP_MODE_RECEIVING_CMD, SLIP_MODE_ESC};

typedef struct
{
    quint8 cmd;
    quint8 payload[256];
    quint8 len;
}slip_buf_t;

slip_buf_t slip_rxbuf;

quint8 DTI_interface::SLIP_Receive(QByteArray read)
{
    quint8 c;
    static quint8 mode = SLIP_MODE_START;

//    log_1 << "SLIP RX" << read.toHex();

    for(quint16 j = 0; j < read.size(); j++)
    {
        c = read[j];

        switch (mode)
        {
        case  SLIP_MODE_START:
            if(c == SLIP_START)
            {
                slip_rxbuf.len = 0;
                memset(slip_rxbuf.payload, 0, 256);
                mode = SLIP_MODE_RECEIVING_CMD;
            }
            break;
        case SLIP_MODE_RECEIVING_PAYLOAD:
        case SLIP_MODE_RECEIVING_CMD:
            if(c == SLIP_ESC)
            {
                mode = SLIP_MODE_ESC;
                break;
            }
            if(c == SLIP_START)
            {
                slip_rxbuf.len = 0;
                memset(slip_rxbuf.payload, 0, 256);
                mode = SLIP_MODE_RECEIVING_CMD;
                break;
            }
            if(c == SLIP_END)
            {
                mode = SLIP_MODE_START;
                if(1) {
                    slip_rxbuf.len--;
                    if(CRC8(&slip_rxbuf.payload[0], slip_rxbuf.len) != slip_rxbuf.payload[slip_rxbuf.len]) return 2;
                }
                return 1;
            }
            if(mode == SLIP_MODE_RECEIVING_CMD)
            {
                slip_rxbuf.cmd = c;
                mode = SLIP_MODE_RECEIVING_PAYLOAD;
                break;
            }
            if(mode == SLIP_MODE_RECEIVING_PAYLOAD)
            {
                slip_rxbuf.payload[slip_rxbuf.len++] = c;
                break;
            }
            break;
        case SLIP_MODE_ESC:
            slip_rxbuf.payload[slip_rxbuf.len++] = c ^ 0xFF;
            mode = SLIP_MODE_RECEIVING_PAYLOAD;
            break;
        default:
            mode = SLIP_MODE_START;
            break;
        }

        if(slip_rxbuf.len == 255)
        {
            mode = SLIP_MODE_START;
        }
    }
    return 0;
}

bool DTI_interface::MakeTry(quint8* SendString, quint16 SendNum, quint8* ReceiveString, quint16 *ReceiveNum)
{
    return MakeTry_new(SendString, SendNum, ReceiveString, ReceiveNum);
}

bool DTI_interface::MakeTry_old(quint8* SendString, quint16 SendNum, quint8* ReceiveString, quint16 *ReceiveNum)
{

    quint32 i,j;

    bool start=0;

    quint8 *tmp_ptr;

    quint8 Send[1000];

    quint8 Receive[1000];

    memset(Receive, 0, sizeof(Receive));

    tmp_ptr = &Send[1];

    *(tmp_ptr++)=':';

    quint8 h1,h2;

//    log_1 << "TX" << QByteArray((char*)SendString, SendNum).toHex() << isSLIP;

    for(i=0;i!=SendNum;i++)
    {
        CharToHexAscii(SendString[i],&h1, &h2);
        *(tmp_ptr++) = h1;
        *(tmp_ptr++) = h2;
    }
    CharToHexAscii(Tele_CalcLRC(SendString,SendNum),&h1, &h2);
    *(tmp_ptr++) = h1;
    *(tmp_ptr++) = h2;
    *(tmp_ptr++) = '\n';
    *(tmp_ptr++) = '\r';
    if(!locker_lock()) {last_error = ERR_PORT_LOCKED; return false;}
//    if(!m_mutex.tryLock(200)) {last_error = ERR_PORT_LOCKED; return false;}

//    QThread::msleep(StrWriteDelay);
    QEventLoop loop;
    QTimer::singleShot(StrWriteDelay, &loop, SLOT(quit()));
    loop.exec();

    serial->clear(QSerialPort::Input);
    int res;
    if(isSLIP)
    {
        Send[0] = 0x02;
        if(!SLIP_Send(0x43, (quint8 *)Send, (quint8)((uint)tmp_ptr - (uint)(&Send[0])) )) {
            locker_unlock();
            //m_mutex.unlock();
            return false;
        }
    }
    else
        if((res = serial->write((const char*)Send, (uint)tmp_ptr - (uint)(&Send[1]) )) != (int)((uint)tmp_ptr - (uint)(&Send[1]))) {
            //m_mutex.unlock();
            locker_unlock();
            return false;
        }

    serial->flush();
    if(no_wait)
    {
        locker_unlock();
        //m_mutex.unlock();
        return true;
    }

    serial->waitForBytesWritten(200);

    //qApp->processEvents();
    //QThread::msleep(10);
    //QApplication::processEvents();
//    /*QTimer::singleShot(10, &loop, SLOT(quit()));
//    loop.exec();*/
    i=0;

    QByteArray read;
    //while(serial->waitForReadyRead(ReadTimeout))
    while(1)
    {
        QEventLoop loop;
        QObject::connect(serial, SIGNAL(readyRead()), &loop, SLOT(quit()));
        QTimer::singleShot(2000, &loop, SLOT(quit()));
        loop.exec();

        read = serial->readAll();
        if (read.isEmpty()) {
            log_1 << "no response";
            last_error = ERR_BAD_PAKET;
            locker_unlock();
            return false;
        }
        log_1 << "read" << read.toHex();

        for(quint16 j = 0; j < read.size(); j++)
        {
            if(read[j] == ':')
            {
                start = 1;
                continue;
            }
            else if(!start) continue;

            Receive[i] = read[j];

            if((read[j] == '\n') || (read[j] == '\r'))
            {
                break;
            }

            if(!(((read[j]>='0')&&(read[j]<='9'))||
                ((read[j]>='A')&&(read[j]<='F'))))
            {
                last_error = ERR_BAD_PAKET;
                locker_unlock();
                //m_mutex.unlock();
                return false;
            }

            if(++i == 1000)
            {
                last_error = ERR_BAD_PAKET;
                locker_unlock();
                //m_mutex.unlock();
                return false;
            }


        }
        if((Receive[i] == '\n') || (Receive[i] == '\r'))  break;
    }
    //serial->clear();
    locker_unlock();
    //m_mutex.unlock();
    if(!i)
    {
//        QByteArray ba;
//        ba.append(SLIP_START);
//        serial->write(ba);
//        serial->write(ba);
//        serial->flush();
//        serial->waitForBytesWritten(10);
//        QTimer::singleShot(10, &loop, SLOT(quit()));
//        loop.exec();
        last_error = ERR_READ_TIMOUT;
        return false;
    }

    if(i%2||((i/=2)<2))
    {
        last_error = ERR_BAD_PAKET;
        return false;
    }

    for(j=0;j!=i;j++) Receive[j]=HexAsciiToChar(Receive[j*2],Receive[j*2+1]);

    if(Tele_CalcLRC((quint8 *)Receive, i-1) != (quint8)Receive[i-1])
    {
        last_error = ERR_BAD_LRC;
        return false;
    }

    for(*ReceiveNum = 0; *ReceiveNum != i - 1; (*ReceiveNum)++)
    {
        ReceiveString[*ReceiveNum] = Receive[*ReceiveNum];
    }

    last_error = ERR_SUCCESS;
    return true;
}

bool DTI_interface::MakeTry_new(quint8* SendString, quint16 SendNum, quint8* ReceiveString, quint16 *ReceiveNum)
{
    QByteArray arr_send((char*)SendString, (int)SendNum);

    arr_send.prepend(0x3B);

//    log_1 << "TX" << QByteArray::fromHex("10").toHex() << arr_send.toHex();

    arr_send.append((char)CRC8((quint8*)arr_send.data(), arr_send.size()));

    if (url_api_key.isEmpty()) {
        arr_send = CMD_to_SLIP(0x10, (quint8 *)arr_send.data(), arr_send.size());

        if (!serial->isOpen()) return false;

        QEventLoop loop;
        QTimer::singleShot(StrWriteDelay, &loop, SLOT(quit()));
        loop.exec();

        serial->clear(QSerialPort::Input);
        serial->write(arr_send);
        serial->flush();
        log_2 << "TX slip" << arr_send.size() << arr_send.toHex();
        if(no_wait)
        {
            locker_unlock();
            return true;
        }

        serial->waitForBytesWritten(200);

        QByteArray read;
        while(1)
        {
            QEventLoop loop;
            QObject::connect(serial, SIGNAL(readyRead()), &loop, SLOT(quit()));
            QTimer::singleShot(2000, &loop, SLOT(quit()));
            loop.exec();

            read = serial->readAll();
            if (read.isEmpty()) {
                log_2 << "no response";
                last_error = ERR_READ_TIMOUT;
                locker_unlock();
                return false;
            }
            //log_1 << "read" << read.toHex();
            log_2 << "RX slip" << read.size() << read.toHex();

            int res = SLIP_Receive(read);
            if (!res) continue;
            if (res == 1) {
                QByteArray arr_rx((char*)slip_rxbuf.payload, slip_rxbuf.len);
                if ((quint8)arr_rx[arr_rx.size() - 1] != CRC8((quint8*)arr_rx.data(),(quint8)(arr_rx.size() - 1))) {
                    bool isOk = false;
                    if (arr_rx.size() <= 8) {
//                        log_1 << "error CRC len" << arr_rx.size() << arr_rx.toHex();
                        while (arr_rx.size() >= 1) {
                            arr_rx.remove(arr_rx.size() - 1, 10);
                            if ((quint8)arr_rx[arr_rx.size() - 1] == CRC8((quint8*)arr_rx.data(),(quint8)(arr_rx.size() - 1))) {
//                                log_1 << "ok CRC len" << arr_rx.size() << arr_rx.toHex();
                                isOk = true;
                                break;
                            }
//                            log_1 << "error CRC len" << arr_rx.size() << arr_rx.toHex();
                        }
                    }
                    if (!isOk) {
                        log_1 << "error CRC" << arr_rx.toHex();
                        last_error = ERR_BAD_PAKET;
                        break;
                    }
                }
//                log_1 << "RX" << QByteArray(arr_rx.data(), arr_rx.size() - 1).toHex();
                for (*ReceiveNum = 0; *ReceiveNum != arr_rx.size() - 2; (*ReceiveNum)++) {
                    ReceiveString[*ReceiveNum] = arr_rx.at(*ReceiveNum + 1);
                }
//                if (slip_rxbuf.payload[slip_rxbuf.len - 1] != CRC8((quint8*)slip_rxbuf.payload, slip_rxbuf.len - 1)) {
//                    log_1 << "RX error crc" << QByteArray((const char*)slip_rxbuf.payload, slip_rxbuf.len).toHex();
//                    break;
//                }
//                log_1 << "RX" << QByteArray((const char*)&slip_rxbuf.cmd, 1).toHex() << QByteArray((const char*)slip_rxbuf.payload, slip_rxbuf.len - 1).toHex();
//                for (*ReceiveNum = 0; *ReceiveNum != slip_rxbuf.len - 2; (*ReceiveNum)++) {
//                    ReceiveString[*ReceiveNum] = slip_rxbuf.payload[*ReceiveNum + 1];
//                }
                break;
            }
            if (res == 2) {
                locker_unlock();
                last_error = ERR_BAD_PAKET;
                return false;
            }
        }
        locker_unlock();
    } else {
        url_set_last_timestamp();
        if (!url_send_DL(arr_send)) {
            last_error = ERR_SUCCESS;
            return false;
        }
//            dti_url->arr_reply.clear();
//            QEventLoop loop;
//            QObject::connect(dti_url, SIGNAL(signal_replyFinished(QByteArray)), &loop, SLOT(quit()));
//            dti_url->slot_DL_send("modem="+QString::number(dev_id, 16).toUpper()+"&message="+arr_send.toHex().toUpper()+"&key="+url_api_key);
//            QTimer::singleShot(10000, &loop, SLOT(quit()));
//            if (dti_url->arr_reply.isEmpty()) {
//                loop.exec();
//            }
//            log_1 << dti_url->arr_reply;
        QTime tim;
        tim.start();
        while(1) {
            QVariantList vlist = url_get_last_UL();
            if (vlist.isEmpty()) {
                if (tim.elapsed() < 5 * 60 * 1000) {
                    continue;
                }
                log_1 << "no response";
                last_error = ERR_READ_TIMOUT;
                locker_unlock();
                return false;
            }
            QByteArray arr_rx = QByteArray::fromHex(vlist.first().toMap().value("payload","").toString().toLocal8Bit());
            if ((quint8)arr_rx[arr_rx.size() - 1] != CRC8((quint8*)arr_rx.data(),(quint8)(arr_rx.size() - 1))) {
                bool isOk = false;
                if (arr_rx.size() <= 8) {
                    log_1 << "error CRC len" << arr_rx.size() << arr_rx.toHex();
                    while (arr_rx.size() >= 1) {
                        arr_rx.remove(arr_rx.size() - 1, 10);
                        if ((quint8)arr_rx[arr_rx.size() - 1] == CRC8((quint8*)arr_rx.data(),(quint8)(arr_rx.size() - 1))) {
                            log_1 << "ok CRC len" << arr_rx.size() << arr_rx.toHex();
                            isOk = true;
                            break;
                        }
                        log_1 << "error CRC len" << arr_rx.size() << arr_rx.toHex();
                    }
                }
                if (!isOk) {
                    log_1 << "error CRC" << arr_rx.toHex();
                    last_error = ERR_BAD_PAKET;
                    break;
                }
            }
//                log_1 << "RX" << arr_rx.toHex();
            log_1 << "RX" << QByteArray(arr_rx.data(), arr_rx.size() - 1).toHex();
            for (*ReceiveNum = 0; *ReceiveNum != arr_rx.size() - 2; (*ReceiveNum)++) {
                ReceiveString[*ReceiveNum] = arr_rx.at(*ReceiveNum + 1);
            }
            break;
        }
        locker_unlock();
//        } else {
//            QEventLoop loop;
//            QTimer::singleShot(10000, &loop, SLOT(quit()));
//            loop.exec();
//        }
        //modem=6f2367&message=01&key=u87ijuqu3luecedw3co5iwpqk57b1cc76cubs9rwjo1c526vivosthmh8rdx7sp6

    }
    return true;
}


bool DTI_interface::checkAdd(quint8 dti_add)
{
    quint16 tn = TryNum;
    quint16 to = ReadTimeout;
    ReadTimeout = 200;
    TryNum = 1;
    quint32 data;
    bool result = readData(dti_add, 0, 1, &data);
    TryNum = tn;
    ReadTimeout = to;
    return result;
}

bool DTI_interface::readData(quint8 dti_add, quint32 mem_add, quint16 len, void* Data)
{

    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    quint8 SendString[20];
    quint8 ReceiveString[1000];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 1;  //read command
    SendString[2] = mem_add&0xff;
    SendString[3] = (mem_add>>8)&0xff;
    SendString[4] = (mem_add>>16)&0xff;
    SendString[5] = len&0xff;
    SendString[6] = (len>>8)&0xff;

    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 7, ReceiveString, &ReceiveNum);
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_REMOTE_UNKN_COMM;ready = 1;return false;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
        if(ReceiveNum < 4) {last_error = ERR_BAD_PAKET; continue;}
        if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }
    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    quint32 datalen = ReceiveString[2] + ReceiveString[3]*256;

    if(datalen > len) {ready = 1;return false;}

    for( i = 0; i < datalen; i++)
    {
        ((quint8*)Data)[i] = ReceiveString[i+4];
    }
    last_error = ERR_SUCCESS;
    ready = 1;
    return true;

}




bool DTI_interface::readInfo(quint8 dti_add, quint32 mem_add, quint16 len, void* Data)
{
    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    quint8 SendString[20];
    quint8 ReceiveString[1000];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 0;  //read info command
    SendString[2] = mem_add&0xff;
    SendString[3] = (mem_add>>8)&0xff;
    SendString[4] = (mem_add>>16)&0xff;
    SendString[5] = len&0xff;
    SendString[6] = (len>>8)&0xff;

    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 7, ReceiveString, &ReceiveNum);
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_REMOTE_UNKN_COMM;ready = 1;return false;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
        if(ReceiveNum < 4) {last_error = ERR_BAD_PAKET; continue;}
        if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }

    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    quint32 datalen = ReceiveString[2] + ReceiveString[3]*256;

    if(datalen > len) {ready = 1;return false;}

    for( i = 0; i < datalen; i++)
    {
        ((quint8*)Data)[i] = ReceiveString[i+4];
    }
    last_error = ERR_SUCCESS;
    ready = 1;
    return true;

}


bool DTI_interface::getPropertyByName(quint8 dti_add, const char *name, DTI_Property *prop)
{
    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    quint8 SendString[20];
    quint8 ReceiveString[1000];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 3;  //get property by name command
    for(i= 0; i != 10; i++) SendString[2+i] = name[i];

    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 12, ReceiveString, &ReceiveNum);
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_REMOTE_UNKN_COMM;ready = 1;return false;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
       if(ReceiveString[1] == 0) {last_error = ERR_REMOTE_NO_PROPERTY; ready = 1; return false;}
        if(ReceiveNum < 3) {last_error = ERR_BAD_PAKET; continue;}
        if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }
    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    quint8 len = sizeof(DTI_Property);//ReceiveString[2];

    for( i = 0; i < len; i++)
    {
        ((quint8*)prop)[i] = ReceiveString[i+3];
    }
    last_error = ERR_SUCCESS;
    ready = 1;
    return true;

}


bool DTI_interface::logIn(quint8 dti_add, const char* password)
{
    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    QByteArray ba(password);
    if(ba.size() > PASSWORD_MAX_LEN)
    {
      last_error = ERR_INVALID_PASSWORD;
    }

    quint8 SendString[50];
    quint8 ReceiveString[50];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 5;  //login command
    for(i= 0; i != 10; i++) SendString[2+i] = ba[i];
    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 12, ReceiveString, &ReceiveNum);
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_SUCCESS; ready = 1;return true;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
       if(ReceiveNum < 2) {last_error = ERR_BAD_PAKET; continue;}
        if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }

    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    last_error = ERR_SUCCESS;
    ready = 1;
    return true;
}

bool DTI_interface::logOut(quint8 dti_add)
{
    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    quint8 SendString[50];
    quint8 ReceiveString[50];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 5;  //login command
    SendString[2] = 0;  //logout
    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 3, ReceiveString, &ReceiveNum);
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_SUCCESS; ready = 1;return true;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
       if(ReceiveNum < 2) {last_error = ERR_BAD_PAKET; continue;}
        if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }

    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    last_error = ERR_SUCCESS;
    ready = 1;
    return true;


}

bool DTI_interface::url_set_last_timestamp()
{
    dti_url->arr_reply.clear();
    QEventLoop loop;
    QObject::connect(dti_url, SIGNAL(signal_replyFinished(QByteArray)), &loop, SLOT(quit()));
    dti_url->slot_UL_read("");
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    if (dti_url->arr_reply.isEmpty()) {
        loop.exec();
    }
    if (dti_url->arr_reply.isEmpty()) {
        return false;
    }
    QVariantList vlist = dti_url->UL_to_vlist(dti_url->arr_reply);
    if (vlist.isEmpty()) return false;
    //log_1 << vlist.first().toMap();
//    log_1 << (vlist.first().toMap().value("modem_id", "").toString().toInt(0, 16) == dev_id);
    server_last_timestamp = vlist.first().toMap().value("time_saved", "0").toUInt();
    if (!server_last_timestamp) return false;
    return true;
}

QVariantList DTI_interface::url_get_last_UL()
{
    dti_url->arr_reply.clear();
    QEventLoop loop;
    QObject::connect(dti_url, SIGNAL(signal_replyFinished(QByteArray)), &loop, SLOT(quit()));
    dti_url->slot_UL_read("");
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    if (dti_url->arr_reply.isEmpty()) {
        loop.exec();
    }
    if (dti_url->arr_reply.isEmpty()) {
        return QVariantList();
    }
    QVariantList vlist = dti_url->UL_to_vlist(dti_url->arr_reply);
    QVariantList vlist_out;
    for (int i = 0; i < vlist.size(); ++i) {
        if (vlist.at(i).toMap().value("time_saved", "0").toUInt() > server_last_timestamp &&
            vlist.at(i).toMap().value("modem_id", "").toString().toUInt(0, 16) == dev_id) {
            vlist_out.append(vlist.at(i));
        }
    }
//    log_1 << vlist_out.count();
    return vlist_out;
}

bool DTI_interface::url_send_DL(QByteArray arr_send)
{
    dti_url->arr_reply.clear();
    QEventLoop loop;
    QObject::connect(dti_url, SIGNAL(signal_replyFinished(QByteArray)), &loop, SLOT(quit()));
    dti_url->slot_DL_send("modem="+QString::number(dev_id, 16).toUpper()+"&message="+arr_send.toHex().toUpper()+"&key="+url_api_key);
    QTimer::singleShot(10000, &loop, SLOT(quit()));
    if (dti_url->arr_reply.isEmpty()) {
        loop.exec();
    }
    if (dti_url->arr_reply.isEmpty()) {
        return false;
    }
    QVariantMap vmap = QJsonDocument::fromJson(dti_url->arr_reply).toVariant().toMap();
    if (vmap.value("result","").toString() == "ok") return true;
    return false;
}

bool DTI_interface::runMethod(quint8 dti_add, quint16 method_id, quint32 param)
{

    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    quint8 SendString[50];
    quint8 ReceiveString[50];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 4;  //get property by name command
    for(i= 0; i != 2; i++) SendString[2+i] = ((quint8*)(&method_id))[i];
    for(i= 0; i != 4; i++) SendString[4+i] = ((quint8*)(&param))[i];
    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 8, ReceiveString, &ReceiveNum);
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_REMOTE_UNKN_COMM;ready = 1;return false;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
        if(ReceiveNum < 2) {last_error = ERR_BAD_PAKET; continue;}
        if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }

    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    last_error = ERR_SUCCESS;
    ready = 1;
    return true;

}

bool DTI_interface::writeData(quint8 dti_add, quint32 mem_add, quint16 len, void* Data)
{
    log_1 << "dev addr" << dti_add << mem_add << QByteArray((char*)Data, len).toHex();

    ready = 0;
    bool result;

    quint32 i;
    quint16 ReceiveNum;

    quint8 SendString[1000];
    quint8 ReceiveString[1000];

    SendString[0] = dti_add;  //dev address
    SendString[1] = 2;  //write command
    SendString[2] = mem_add&0xff;
    SendString[3] = (mem_add>>8)&0xff;
    SendString[4] = (mem_add>>16)&0xff;
    SendString[5] = len&0xff;
    SendString[6] = (len>>8)&0xff;
    for(i = 0; i < len; i++)
         SendString[7+i] = ((quint8*)Data)[i];

    for(i = 0; i!= TryNum; i++)
    {
        result = MakeTry(SendString, 7 + len, ReceiveString, &ReceiveNum);
        if(no_wait) break;
        if(!result) continue;
        if(ReceiveString[1] == 0x81) {last_error = ERR_REMOTE_UNKN_COMM;ready = 1;return false;}
        if(ReceiveString[1] == 0x82) {last_error = ERR_REMOTE_WRITE_PROTECT;ready = 1;return false;}
        if(ReceiveString[1] == 0x83) {last_error = ERR_REMOTE_OUT_OF_MEMORY;ready = 1;return false;}
        if(ReceiveString[1] == 0x84) {last_error = ERR_REMOTE_NOT_SUPPORTED;ready = 1;return false;}
        if(ReceiveString[1] == 0x85) {last_error = ERR_INVALID_PASSWORD;ready = 1;return false;}
        if(ReceiveString[1] == 0x86) {last_error = ERR_ACCESS_DENIED;ready = 1;return false;}
       if(ReceiveString[1] != SendString[1]) {last_error =  ERR_BAD_PAKET;continue;}
        break;
    }
    if(i == TryNum) {ready = 1;return false;}
    if(!result) {ready = 1;return false;}

    last_error = ERR_SUCCESS;
    ready = 1;
    return true;

}

QString DTI_interface::error_message()
{

    switch(last_error)
    {

    case ERR_SUCCESS:
        return "ERR_SUCCESS";
    case ERR_QSERIAL:
        return "ERR_QSERIAL";
    case  ERR_BAD_PAKET:
        return "ERR_BAD_PAKET";
    case ERR_READ_TIMOUT:
        return "ERR_READ_TIMOUT";
    case ERR_BAD_LRC:
        return "ERR_BAD_LRC";
    case ERR_REMOTE_UNKN_COMM:
        return "ERR_REMOTE_UNKN_COMM";
    case ERR_REMOTE_WRITE_PROTECT:
        return "ERR_REMOTE_WRITE_PROTECT";
    case ERR_REMOTE_NO_PROPERTY:
        return "ERR_REMOTE_NO_PROPERTY";
    case ERR_PORT_LOCKED:
        return "ERR_PORT_LOCKED";
    case ERR_REMOTE_OUT_OF_MEMORY:
        return "ERR_REMOTE_OUT_OF_MEMORY";
    case ERR_REMOTE_NOT_SUPPORTED:
        return "ERR_REMOTE_NOT_SUPPORTED";
    case ERR_INVALID_PASSWORD:
        return "ERR_INVALID_PASSWORD";
    case ERR_ACCESS_DENIED:
        return "ERR_ACCESS_DENIED";
    default:
        return "ERR_UNKNOWN";
    }

}

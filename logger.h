#ifndef LOGGER_H
#define LOGGER_H

#include <QtDebug>
#include <QDateTime>

#define log_1 qDebug() << __FILE__ << QString("{%1}").arg(__LINE__,4,10,QChar(' ')).toStdString().c_str()
#define log_2 log_1 << QDateTime::currentDateTime().toString("hh.mm.ss_zzz")
#define arrToHex(a, b) QByteArray((char*)a, b).toHex().toUpper()
#define bitsToHex(a) QByteArray(1,(char)a).toHex().toUpper()

#endif // LOGGER_H

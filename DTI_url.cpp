#include "DTI_url.h"
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QVariant>
#include <QUrlQuery>

#include "logger.h"

DTI_url::DTI_url(QObject *parent)
    : QObject(parent)
    , nam(0)
{
    nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_nam_replyFinished(QNetworkReply*)), Qt::QueuedConnection);
    slot_UL_read("");
}

QVariantList DTI_url::UL_to_vlist(QByteArray reply_data)
{
    reply_data.remove(0, 3);
    reply_data.remove(reply_data.length()-4, 10);
//    reply_data.remove("<p>");
//    reply_data.remove("</p>");
    QJsonParseError jpe;
    QVariantList vlist = QJsonDocument::fromJson(reply_data, &jpe).toVariant().toList();
    if (jpe.error != QJsonParseError::NoError) {
        log_1 << jpe.errorString() << reply_data;
        return QVariantList();
    }
//    log_1 << QJsonDocument::fromVariant(vlist).toJson(QJsonDocument::Indented);
    return vlist;
}

void DTI_url::slot_UL_read(QString param)
{
    QNetworkRequest request;
//    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
//    config.setProtocol(QSsl::TlsV1SslV3);
//    request.setSslConfiguration(config);
    QUrl url("http://ruddy.tech/log");
//    QUrl url("https://b.strij.net/api/last/");
//    QList<QPair<QString, QString> > uqlist;
//    uqlist.append(qMakePair(QString("modem"), QString("6F2367")));
//    uqlist.append(qMakePair(QString("limit"), QString("10")));
//    QUrlQuery uq;
//    uq.setQueryItems(uqlist);
//    url.setQuery(uq);
    url.setQuery(param);
    request.setUrl(url);
//    log_1 << "UL_read" << url;
//    request.setUrl(QUrl("https://b.strij.net/api/last/?limit=10&modem=6F2367"));
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");
    nam->get(request);
}

void DTI_url::slot_DL_send(QString param)
{
    QNetworkRequest request;
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(config);
    QUrl url("https://b.strij.net/api/sendmessage/");
    url.setQuery(param);
    request.setUrl(url);
    log_1 << "DL_send" << url;
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");
    nam->get(request);
}

void DTI_url::slot_get_API_key()
{
    QNetworkRequest request;
//    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
//    config.setProtocol(QSsl::TlsV1SslV3);
//    request.setSslConfiguration(config);
    QUrl url("http://b.strij.net/api/docs/");
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");
    nam->get(request);
}

void DTI_url::slot_nam_replyFinished(QNetworkReply *nr)
{
    if (nr->error() != QNetworkReply::NoError) {
        log_1 << nr << nr->errorString() << nr->readAll();
        nr->deleteLater();
        return;
    }
    arr_reply = nr->readAll();
    nr->deleteLater();
//    log_1 << arr_reply;
    emit signal_replyFinished(arr_reply);
//    log_1 << arr.size();
//    QVariant v = QJsonDocument::fromJson(arr).toVariant();
//    QVariantList vlist = v.toList();
//    log_1 << v.toList();
//    QJsonParseError jpe;
//    QVariantList vlist = QJsonDocument::fromJson(arr, &jpe).toVariant().toList();
//    if (jpe.error != QJsonParseError::NoError) {
//        log_1 << jpe.errorString() << arr;
//        return;
//    }
//    log_1 << vlist.count();
//    log_1 << QJsonDocument::fromVariant(vlist.first()).toJson(QJsonDocument::Indented);
}


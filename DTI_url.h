#ifndef DTI_URL_H
#define DTI_URL_H

#include <QObject>
#include <QNetworkReply>

class QNetworkAccessManager;
class DTI_url : public QObject
{
    Q_OBJECT
public:
    explicit DTI_url(QObject *parent = 0);
    QByteArray arr_reply;
    QVariantList UL_to_vlist(QByteArray reply_data);
protected:
    QNetworkAccessManager * nam;

signals:
    void signal_replyFinished(QByteArray reply_data);
public slots:
    void slot_UL_read(QString param);
    void slot_DL_send(QString param);
    void slot_get_API_key();

protected slots:
    void slot_nam_replyFinished(QNetworkReply* nr);
};

#endif // DTI_URL_H

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include "amper_dev.h"
#include <QProgressBar>

namespace Ui {
class settings;
}

class settings : public QWidget
{
    Q_OBJECT

public:
    explicit settings(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~settings();
    void setAmperDev(amper_dev *ad) {dev = ad;}
    void setProgressBar(QProgressBar *pb) {bar = pb;}

public slots:
    void ValueChanged();
    void logged(bool log);
    void update_ports();
private slots:
    void on_Password_edit_cursorPositionChanged(int arg1, int arg2);
    void on_Password_edit_editingFinished();
    void on_Password_edit_textChanged(const QString &arg1);
    void on_pushButton_clicked();

    void on_pushButton_autoA_clicked();

    void on_pushButton_autoB_clicked();

    void on_pushButton_autoC_clicked();

    void on_pushButton_autoN_clicked();

public:
    QByteArray password;
private:
    Ui::settings *ui;
    amper_dev * dev;
    QProgressBar *bar;
};

#endif // SETTINGS_H

#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include "tarif_edit.h"
#include "par_edit.h"
#include "amper_dev.h"
#include "archive_view.h"
#include "settings.h"
#include "widget_tariff_edit.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~Dialog();
    amper_dev* getAmperDev() {return dev;}
public slots:
    void hide_all();
    void conButClicked(bool checked);
    void submenu1Clicked(bool);
    void submenu2Clicked(bool);
    void submenu3Clicked(bool);
    void submenu4Clicked(bool);
    void submenu5Clicked(bool);

//    void done(int r);

private:
    bool connected;
    Ui::Dialog *ui;
    tarif_edit *te;
    //widget_tariff_edit *wte;
    par_edit *pe;
    settings *set;
    archive_view *av;
//    DTI_interface *interface;
    amper_dev *dev;
    QWidget *currentW;
    QProgressBar *pb;
signals:
    void signal_set_win_title(QString);
};

#endif // MAINDIALOG_H

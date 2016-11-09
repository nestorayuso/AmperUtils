#ifndef DIALOG_CUR_VALUE_H
#define DIALOG_CUR_VALUE_H

#include <QDialog>
#include "amper_dev.h"
#include <QProgressBar>

namespace Ui {
class dialog_CUR_VALUE;
}

class widget_itgr;
class widget_meter_curr;
class dialog_CUR_VALUE : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_CUR_VALUE(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~dialog_CUR_VALUE();

private:
    Ui::dialog_CUR_VALUE *ui;
    amper_dev * dev;
    QProgressBar *bar;
    widget_itgr *wdg_itgr;
    widget_meter_curr * wdg_mcurr;
protected slots:
    void slot_read_end(bool ok);
private slots:
    void on_checkBox_upd_curr_toggled(bool checked);
};

#endif // DIALOG_CUR_VALUE_H

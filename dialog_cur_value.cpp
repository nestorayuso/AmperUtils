#include "dialog_cur_value.h"
#include "ui_dialog_cur_value.h"

#include "logger.h"
#include "widget_itgr.h"
#include "widget_meter_curr.h"

dialog_CUR_VALUE::dialog_CUR_VALUE(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::dialog_CUR_VALUE)
    , dev(ad)
    , bar(pb)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    wdg_itgr = new widget_itgr(ad, pb, this);
    ui->verticalLayout_itgr->insertWidget(0, wdg_itgr);
//    ui->gridLayout_2->addWidget(wdg_itgr, ui->gridLayout_2->rowCount(), 0);
    connect(wdg_itgr, SIGNAL(signal_read_end(bool)), this, SLOT(slot_read_end(bool)));
    wdg_mcurr = new widget_meter_curr(ad, pb, this);
    ui->verticalLayout_curr->insertWidget(0, wdg_mcurr);
//    ui->gridLayout_2->addWidget(wdg_mcurr, ui->gridLayout_2->rowCount()-1, 1);
    connect(wdg_mcurr, SIGNAL(signal_read_end(bool)), this, SLOT(slot_read_end(bool)));

    wdg_mcurr->slot_start_after_ms(10);
}

dialog_CUR_VALUE::~dialog_CUR_VALUE()
{
    delete ui;
}

void dialog_CUR_VALUE::slot_read_end(bool ok)
{
    if (wdg_itgr == qobject_cast<widget_itgr *>(sender())) {
        //wdg_mcurr->slot_start_after_ms(10);
//        log_1 << ok;
//        if (ui->checkBox_curr_autoupdate->isChecked()) {
//            wdg_mcurr->slot_start_after_ms(200);
//        }
    }
    if (wdg_mcurr == qobject_cast<widget_meter_curr *>(sender())) {
        if (ui->checkBox_upd_curr->isChecked()) {
            wdg_mcurr->slot_start_after_ms(200);
        } else {
            wdg_itgr->slot_start_after_ms(10);
        }
    }
}

void dialog_CUR_VALUE::on_checkBox_upd_curr_toggled(bool checked)
{
    if (checked) {
        wdg_mcurr->slot_start_after_ms(10);
    }
}

#include "widget_meter_arch_data_setttings.h"
#include "ui_widget_meter_arch_data_setttings.h"

#include <QMessageBox>

#include "logger.h"

widget_METER_ARCH_DATA_SETTTINGS::widget_METER_ARCH_DATA_SETTTINGS(amper_dev *ad, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::widget_METER_ARCH_DATA_SETTTINGS)
    , dev(ad)
{
    ui->setupUi(this);
    memset(&mads, 0, sizeof(mads));

    ui->comboBox_param_id_MDT_1->addItem(tr("A+"),  MDT_P_p);
    ui->comboBox_param_id_MDT_1->addItem(tr("A+d"), MDT_P_p_d);
    ui->comboBox_param_id_MDT_1->addItem(tr("A-"),  MDT_P_n);
    ui->comboBox_param_id_MDT_1->addItem(tr("A-d"), MDT_P_n_d);
    ui->comboBox_param_id_MDT_1->addItem(tr("R+"),  MDT_Q_p);
    ui->comboBox_param_id_MDT_1->addItem(tr("R+d"), MDT_Q_p_d);
    ui->comboBox_param_id_MDT_1->addItem(tr("R-"),  MDT_Q_n);
    ui->comboBox_param_id_MDT_1->addItem(tr("R-d"), MDT_Q_n_d);
    ui->comboBox_param_id_MDT_1->addItem(tr("ПОЛНАЯ"), MDT_S);
    ui->comboBox_param_id_MDT_1->addItem(tr("ПОЛНАЯd"), MDT_S_d);
    ui->comboBox_param_id_MDT_1->addItem(tr("QI"), MDT_Q1);
    ui->comboBox_param_id_MDT_1->addItem(tr("QII"), MDT_Q2);
    ui->comboBox_param_id_MDT_1->addItem(tr("QIII"), MDT_Q3);
    ui->comboBox_param_id_MDT_1->addItem(tr("QIV"), MDT_Q4);
    ui->comboBox_param_id_MDT_1->addItem(tr("U"), MDT_U);
    ui->comboBox_param_id_MDT_1->addItem(tr("I"), MDT_I);
    ui->comboBox_param_id_MDT_1->addItem(tr("F"), MDT_F);
    ui->comboBox_param_id_MDT_1->addItem(tr("cosFi"), MDT_cosFi);

    ui->comboBox_param_id_phase_1->addItem(tr("Σ фаз"), 0);
    ui->comboBox_param_id_phase_1->addItem(tr("фаз 1"), 1);
    ui->comboBox_param_id_phase_1->addItem(tr("фаз 2"), 2);
    ui->comboBox_param_id_phase_1->addItem(tr("фаз 3"), 3);

    ui->comboBox_param_id_tariff_1->addItem(tr("Σ тарифов"), 0);
    ui->comboBox_param_id_tariff_1->addItem(tr("тариф 1"), 1);
    ui->comboBox_param_id_tariff_1->addItem(tr("тариф 2"), 2);
    ui->comboBox_param_id_tariff_1->addItem(tr("тариф 3"), 3);
    ui->comboBox_param_id_tariff_1->addItem(tr("тариф 4"), 4);

    ui->comboBox_param_id_MDT_2->addItem(tr("A+"),  MDT_P_p);
    ui->comboBox_param_id_MDT_2->addItem(tr("A+d"), MDT_P_p_d);
    ui->comboBox_param_id_MDT_2->addItem(tr("A-"),  MDT_P_n);
    ui->comboBox_param_id_MDT_2->addItem(tr("A-d"), MDT_P_n_d);
    ui->comboBox_param_id_MDT_2->addItem(tr("R+"),  MDT_Q_p);
    ui->comboBox_param_id_MDT_2->addItem(tr("R+d"), MDT_Q_p_d);
    ui->comboBox_param_id_MDT_2->addItem(tr("R-"),  MDT_Q_n);
    ui->comboBox_param_id_MDT_2->addItem(tr("R-d"), MDT_Q_n_d);
    ui->comboBox_param_id_MDT_2->addItem(tr("ПОЛНАЯ"), MDT_S);
    ui->comboBox_param_id_MDT_2->addItem(tr("ПОЛНАЯd"), MDT_S_d);
    ui->comboBox_param_id_MDT_2->addItem(tr("QI"), MDT_Q1);
    ui->comboBox_param_id_MDT_2->addItem(tr("QII"), MDT_Q2);
    ui->comboBox_param_id_MDT_2->addItem(tr("QIII"), MDT_Q3);
    ui->comboBox_param_id_MDT_2->addItem(tr("QIV"), MDT_Q4);
    ui->comboBox_param_id_MDT_2->addItem(tr("U"), MDT_U);
    ui->comboBox_param_id_MDT_2->addItem(tr("I"), MDT_I);
    ui->comboBox_param_id_MDT_2->addItem(tr("F"), MDT_F);
    ui->comboBox_param_id_MDT_2->addItem(tr("cosFi"), MDT_cosFi);

    ui->comboBox_param_id_phase_2->addItem(tr("Σ фаз"), 0);
    ui->comboBox_param_id_phase_2->addItem(tr("фаз 1"), 1);
    ui->comboBox_param_id_phase_2->addItem(tr("фаз 2"), 2);
    ui->comboBox_param_id_phase_2->addItem(tr("фаз 3"), 3);

    ui->comboBox_param_id_tariff_2->addItem(tr("Σ тарифов"), 0);
    ui->comboBox_param_id_tariff_2->addItem(tr("тариф 1"), 1);
    ui->comboBox_param_id_tariff_2->addItem(tr("тариф 2"), 2);
    ui->comboBox_param_id_tariff_2->addItem(tr("тариф 3"), 3);
    ui->comboBox_param_id_tariff_2->addItem(tr("тариф 4"), 4);

    ui->comboBox_param_id_MDT_3->addItem(tr("A+"),  MDT_P_p);
    ui->comboBox_param_id_MDT_3->addItem(tr("A+d"), MDT_P_p_d);
    ui->comboBox_param_id_MDT_3->addItem(tr("A-"),  MDT_P_n);
    ui->comboBox_param_id_MDT_3->addItem(tr("A-d"), MDT_P_n_d);
    ui->comboBox_param_id_MDT_3->addItem(tr("R+"),  MDT_Q_p);
    ui->comboBox_param_id_MDT_3->addItem(tr("R+d"), MDT_Q_p_d);
    ui->comboBox_param_id_MDT_3->addItem(tr("R-"),  MDT_Q_n);
    ui->comboBox_param_id_MDT_3->addItem(tr("R-d"), MDT_Q_n_d);
    ui->comboBox_param_id_MDT_3->addItem(tr("ПОЛНАЯ"), MDT_S);
    ui->comboBox_param_id_MDT_3->addItem(tr("ПОЛНАЯd"), MDT_S_d);
    ui->comboBox_param_id_MDT_3->addItem(tr("QI"), MDT_Q1);
    ui->comboBox_param_id_MDT_3->addItem(tr("QII"), MDT_Q2);
    ui->comboBox_param_id_MDT_3->addItem(tr("QIII"), MDT_Q3);
    ui->comboBox_param_id_MDT_3->addItem(tr("QIV"), MDT_Q4);
    ui->comboBox_param_id_MDT_3->addItem(tr("U"), MDT_U);
    ui->comboBox_param_id_MDT_3->addItem(tr("I"), MDT_I);
    ui->comboBox_param_id_MDT_3->addItem(tr("F"), MDT_F);
    ui->comboBox_param_id_MDT_3->addItem(tr("cosFi"), MDT_cosFi);

    ui->comboBox_param_id_phase_3->addItem(tr("Σ фаз"), 0);
    ui->comboBox_param_id_phase_3->addItem(tr("фаз 1"), 1);
    ui->comboBox_param_id_phase_3->addItem(tr("фаз 2"), 2);
    ui->comboBox_param_id_phase_3->addItem(tr("фаз 3"), 3);

    ui->comboBox_param_id_tariff_3->addItem(tr("Σ тарифов"), 0);
    ui->comboBox_param_id_tariff_3->addItem(tr("тариф 1"), 1);
    ui->comboBox_param_id_tariff_3->addItem(tr("тариф 2"), 2);
    ui->comboBox_param_id_tariff_3->addItem(tr("тариф 3"), 3);
    ui->comboBox_param_id_tariff_3->addItem(tr("тариф 4"), 4);

    ui->comboBox_param_id_MDT_4->addItem(tr("A+"),  MDT_P_p);
    ui->comboBox_param_id_MDT_4->addItem(tr("A+d"), MDT_P_p_d);
    ui->comboBox_param_id_MDT_4->addItem(tr("A-"),  MDT_P_n);
    ui->comboBox_param_id_MDT_4->addItem(tr("A-d"), MDT_P_n_d);
    ui->comboBox_param_id_MDT_4->addItem(tr("R+"),  MDT_Q_p);
    ui->comboBox_param_id_MDT_4->addItem(tr("R+d"), MDT_Q_p_d);
    ui->comboBox_param_id_MDT_4->addItem(tr("R-"),  MDT_Q_n);
    ui->comboBox_param_id_MDT_4->addItem(tr("R-d"), MDT_Q_n_d);
    ui->comboBox_param_id_MDT_4->addItem(tr("ПОЛНАЯ"), MDT_S);
    ui->comboBox_param_id_MDT_4->addItem(tr("ПОЛНАЯd"), MDT_S_d);
    ui->comboBox_param_id_MDT_4->addItem(tr("QI"), MDT_Q1);
    ui->comboBox_param_id_MDT_4->addItem(tr("QII"), MDT_Q2);
    ui->comboBox_param_id_MDT_4->addItem(tr("QIII"), MDT_Q3);
    ui->comboBox_param_id_MDT_4->addItem(tr("QIV"), MDT_Q4);
    ui->comboBox_param_id_MDT_4->addItem(tr("U"), MDT_U);
    ui->comboBox_param_id_MDT_4->addItem(tr("I"), MDT_I);
    ui->comboBox_param_id_MDT_4->addItem(tr("F"), MDT_F);
    ui->comboBox_param_id_MDT_4->addItem(tr("cosFi"), MDT_cosFi);

    ui->comboBox_param_id_phase_4->addItem(tr("Σ фаз"), 0);
    ui->comboBox_param_id_phase_4->addItem(tr("фаз 1"), 1);
    ui->comboBox_param_id_phase_4->addItem(tr("фаз 2"), 2);
    ui->comboBox_param_id_phase_4->addItem(tr("фаз 3"), 3);

    ui->comboBox_param_id_tariff_4->addItem(tr("Σ тарифов"), 0);
    ui->comboBox_param_id_tariff_4->addItem(tr("тариф 1"), 1);
    ui->comboBox_param_id_tariff_4->addItem(tr("тариф 2"), 2);
    ui->comboBox_param_id_tariff_4->addItem(tr("тариф 3"), 3);
    ui->comboBox_param_id_tariff_4->addItem(tr("тариф 4"), 4);
}

widget_METER_ARCH_DATA_SETTTINGS::~widget_METER_ARCH_DATA_SETTTINGS()
{
    delete ui;
}

void widget_METER_ARCH_DATA_SETTTINGS::slot_update_ui_from_data()
{
    ui->spinBox->setValue(mads.timeout / 60);
    ui->comboBox_agiment_time->setCurrentIndex(mads.start_at_time_alignment);
    log_1 << "кол-во параметров" << mads.count_param;

    int index;
    index = ui->comboBox_param_id_MDT_1->findData(mads.param_id[0].MDT); ui->comboBox_param_id_MDT_1->setCurrentIndex(index);
    index = ui->comboBox_param_id_MDT_2->findData(mads.param_id[1].MDT); ui->comboBox_param_id_MDT_2->setCurrentIndex(index);
    index = ui->comboBox_param_id_MDT_3->findData(mads.param_id[2].MDT); ui->comboBox_param_id_MDT_3->setCurrentIndex(index);
    index = ui->comboBox_param_id_MDT_4->findData(mads.param_id[3].MDT); ui->comboBox_param_id_MDT_4->setCurrentIndex(index);

    index = ui->comboBox_param_id_phase_1->findData(mads.param_id[0].phase); ui->comboBox_param_id_phase_1->setCurrentIndex(index);
    index = ui->comboBox_param_id_phase_2->findData(mads.param_id[1].phase); ui->comboBox_param_id_phase_2->setCurrentIndex(index);
    index = ui->comboBox_param_id_phase_3->findData(mads.param_id[2].phase); ui->comboBox_param_id_phase_3->setCurrentIndex(index);
    index = ui->comboBox_param_id_phase_4->findData(mads.param_id[3].phase); ui->comboBox_param_id_phase_4->setCurrentIndex(index);

    index = ui->comboBox_param_id_tariff_1->findData(mads.param_id[0].tariff); ui->comboBox_param_id_tariff_1->setCurrentIndex(index);
    index = ui->comboBox_param_id_tariff_2->findData(mads.param_id[1].tariff); ui->comboBox_param_id_tariff_2->setCurrentIndex(index);
    index = ui->comboBox_param_id_tariff_3->findData(mads.param_id[2].tariff); ui->comboBox_param_id_tariff_3->setCurrentIndex(index);
    index = ui->comboBox_param_id_tariff_4->findData(mads.param_id[3].tariff); ui->comboBox_param_id_tariff_4->setCurrentIndex(index);
}

void widget_METER_ARCH_DATA_SETTTINGS::slot_update_data_from_ui()
{
    mads.timeout = ui->spinBox->value() * 60;
    mads.start_at_time_alignment = ui->comboBox_agiment_time->currentIndex();

    mads.param_id[0].MDT = ui->comboBox_param_id_MDT_1->currentData().toInt();
    mads.param_id[1].MDT = ui->comboBox_param_id_MDT_2->currentData().toInt();
    mads.param_id[2].MDT = ui->comboBox_param_id_MDT_3->currentData().toInt();
    mads.param_id[3].MDT = ui->comboBox_param_id_MDT_4->currentData().toInt();

    mads.param_id[0].phase = ui->comboBox_param_id_phase_1->currentData().toInt();
    mads.param_id[1].phase = ui->comboBox_param_id_phase_2->currentData().toInt();
    mads.param_id[2].phase = ui->comboBox_param_id_phase_3->currentData().toInt();
    mads.param_id[3].phase = ui->comboBox_param_id_phase_4->currentData().toInt();

    mads.param_id[0].tariff = ui->comboBox_param_id_tariff_1->currentData().toInt();
    mads.param_id[1].tariff = ui->comboBox_param_id_tariff_2->currentData().toInt();
    mads.param_id[2].tariff = ui->comboBox_param_id_tariff_3->currentData().toInt();
    mads.param_id[3].tariff = ui->comboBox_param_id_tariff_4->currentData().toInt();
}

void widget_METER_ARCH_DATA_SETTTINGS::on_pushButton_Save_clicked()
{
    slot_update_data_from_ui();
    DTI_Property prop;
    if (!dev->getPropertyByName("pM_PPOWST", &prop)) {
        log_1 << "error getPropertyByName";
        return;
    }
    if (!dev->writeData(dev->propertySeg(&prop), prop.address, sizeof(METER_ARCH_DATA_SETTINGS), &mads)) {
        log_1 << "error writeData";
        return;
    }
    if (!dev->runEvent(SAVE_SETTINGS)) {
        log_1 << "error runEvent";
        QMessageBox::warning(this,"Ошибка записи", dev->error_message());
        return;
    }
}

#include "archive_view.h"
#include "ui_archive_view.h"

#include <QFileDialog>
#include <QTimer>

#include "meter.h"
#include "logger.h"

archive_view::archive_view(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::archive_view)
    , dev(ad)
    , bar(pb)
    , proxy_model(0)
    , time_revisible(0)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    connect(ui->radioButton_arch_data1, SIGNAL(clicked(bool)), this, SLOT(slot_radioButton_arch_clicked()));
    connect(ui->radioButton_arch_data2, SIGNAL(clicked(bool)), this, SLOT(slot_radioButton_arch_clicked()));
    connect(ui->radioButton_arch_event, SIGNAL(clicked(bool)), this, SLOT(slot_radioButton_arch_clicked()));
    proxy_model = new QSortFilterProxyModel(this);

    timer_select_arch = new QTimer(this);
    connect(timer_select_arch, SIGNAL(timeout()), this, SLOT(slot_radioButton_arch_clicked()));
//    connect(ui->radioButton_arch_data1, SIGNAL(toggled(bool)), this, SLOT(slot_radioButton_arch_toggled(bool)));
//    connect(ui->radioButton_arch_data2, SIGNAL(toggled(bool)), this, SLOT(slot_radioButton_arch_toggled(bool)));
//    cthread = new communication_thread(this);
//    connect(cthread, SIGNAL(signal_readFlash(int,QByteArray)), this,    SLOT(slot_readFlash(int,QByteArray)), Qt::DirectConnection);
//    connect(this,    SIGNAL(signal_readFlash(int,int)),        cthread, SLOT(slot_readFlash(int,int)),        Qt::QueuedConnection);
    uint8_t p = 0;
    uint8_t k = 0;
    model_arch_data * m = new model_arch_data(dev, this);
    mad.append(m);
    connect(m, SIGNAL(signal_persent_complated(float)), this, SLOT(slot_persent_complated(float)));
    connect(this, SIGNAL(signal_readFlash(int,int)), m, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
    //ui->tableView->setModel(m);
    METER_ARCH_DATA_SETTINGS _ads;
    METER_ARCH_DATA_SETTINGS * ads = &_ads;
    //ads->timeout = 10; //60 * 60 * 24; // сутки
    //ads->start_at_time_alignment = METER_TIME_ALIGMENT_min;
    p = 0;
    for (k = 1; k <= 4; ++k) {
        ads->param_id[p].MDT = MDT_P_p;
        ads->param_id[p].phase = 0;
        ads->param_id[p].tariff = k;
        p++;
        ads->param_id[p].MDT = MDT_P_n;
        ads->param_id[p].phase = 0;
        ads->param_id[p].tariff = k;
        p++;
        ads->param_id[p].MDT = MDT_Q_p;
        ads->param_id[p].phase = 0;
        ads->param_id[p].tariff = k;
        p++;
        ads->param_id[p].MDT = MDT_Q_n;
        ads->param_id[p].phase = 0;
        ads->param_id[p].tariff = k;
        p++;
    }
    m->set_header(QByteArray((char*)ads, sizeof(METER_ARCH_DATA_SETTINGS)));
    m->flash_start_addr = 0x01000;
    m->flash_curr_addr = m->flash_start_addr;
    m->flash_len = 0x15000 - m->flash_start_addr;

    m = new model_arch_data_profile(dev, this);
    mad.append(m);
    connect(m, SIGNAL(signal_persent_complated(float)), this, SLOT(slot_persent_complated(float)));
    connect(this, SIGNAL(signal_readFlash(int,int)), m, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
    //ui->tableView->setModel(m);
    //METER_ARCH_DATA_SETTINGS _ads;
    //METER_ARCH_DATA_SETTINGS * ads = &_ads;
    //ads->timeout = 10; //60 * 60 * 24; // сутки
    //ads->start_at_time_alignment = METER_TIME_ALIGMENT_min;
    p = 0;
    for (k = 0; k < 1; ++k) {
        ads->param_id[p].MDT = MDT_P_p_d;
        ads->param_id[p].phase = k;
        ads->param_id[p].tariff = 0;
        p++;
        ads->param_id[p].MDT = MDT_P_n_d;
        ads->param_id[p].phase = k;
        ads->param_id[p].tariff = 0;
        p++;
        ads->param_id[p].MDT = MDT_Q_p_d;
        ads->param_id[p].phase = k;
        ads->param_id[p].tariff = 0;
        p++;
        ads->param_id[p].MDT = MDT_Q_n_d;
        ads->param_id[p].phase = k;
        ads->param_id[p].tariff = 0;
        p++;
    }

    DTI_Property prop;
    if (dev->getPropertyByName("pM_PPOWST", &prop)) {
        dev->readData(dev->propertySeg(&prop), prop.address, sizeof(METER_ARCH_DATA_SETTINGS), ads);
    }

    m->set_header(QByteArray((char*)ads, sizeof(METER_ARCH_DATA_SETTINGS)));
    m->flash_start_addr = 0x15000;
    m->flash_curr_addr = m->flash_start_addr;
    m->flash_len = 0x29000 - m->flash_start_addr;

    m = new model_arch_event(dev, this);
    mad.append(m);
    connect(m, SIGNAL(signal_persent_complated(float)), this, SLOT(slot_persent_complated(float)));
    connect(this, SIGNAL(signal_readFlash(int,int)), m, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
    m->flash_start_addr = 0x29000;
    m->flash_curr_addr = m->flash_start_addr;
    m->flash_len = 0x2B000 - m->flash_start_addr;

//    int id = 0x6f233E;
//    QStringList slist;
//    for (int i = 0; i < 20; ++i) {
//        slist << QString::number(id + i, 16).toUpper();
//    }
//    log_1 << slist.join(",");
}

archive_view::~archive_view()
{
    delete ui;
}

void archive_view::slot_persent_complated(float persent)
{
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    bar->setValue(persent * 100);
}

//void archive_view::slot_readFlash(int addr, QByteArray data)
//{
//    //log_1 << QString::number(addr,16) << data.size();
//    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
//    bar->setValue(mad1->progress_load() * 100);
//    //if (data.size() < communication_thread_packet_size) mad1->read_buff.clear();
//}

void archive_view::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    log_1 << visible;
    model_arch_data * m;
    if (visible) {
        static model_arch_search * mas = 0;
        if (mas) {
            mas = new model_arch_search(dev, this);
            mas->flash_start_addr = 0x01000;
            mas->flash_len = 0x15000 - mas->flash_start_addr;
            mas->flash_record_len = 72;
            mas->f_h = mas->flash_len / mas->flash_record_len - 0;
            mas->f_l = 0;
            mas->flash_curr_addr = mas->f_h / 2;

            connect(this, SIGNAL(signal_readFlash(int,int)), mas, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
            connect(mas, SIGNAL(signal_persent_complated(float)), this, SLOT(slot_persent_complated(float)));
            mas->connect_dev(true);
            proxy_model->setSourceModel(mas);
            ui->tableView->setModel(proxy_model);
            emit signal_readFlash(mas->flash_curr_addr, mas->flash_len);
        } else {
            slot_radioButton_arch_clicked();
        }
//        m->connect_dev(true);
//        emit signal_readFlash(m->flash_curr_addr, m->flash_len - (m->flash_curr_addr - m->flash_start_addr));
    } else {
        for (int i = 0; i < ui->horizontalLayout_arch_index->count(); ++i) {
            QRadioButton *rb = qobject_cast<QRadioButton*>(ui->horizontalLayout_arch_index->itemAt(i)->widget());
            if (!rb) continue;
            m = mad.at(i);
            m->connect_dev(false);
        }
//        m->connect_dev(false);
    }
}

void archive_view::slot_radioButton_arch_toggled(bool checked)
{
    int lindex = -1;
//    QList<QRadioButton *> rbs = ui->horizontalLayout_arch_index->findChildren<QRadioButton *>();
//    log_1 << rbs;
//    for (int i = 0; i < rbs.count(); ++i) {
//        bool ch = rbs.at(i)->isChecked();
//        mad.at(i)->connect_dev(ch);
//        if (ch) lindex = i;
//        log_1 << i;
//    }
//    log_1 << lindex;
//    if (lindex >= 0) {
//        model_arch_data * m = mad.at(lindex);
//        emit signal_readFlash(m->flash_curr_addr, m->flash_len - (m->flash_curr_addr - m->flash_start_addr));
//    }
//    for (int i = 0; i < ui->horizontalLayout_arch_index->count(); ++i) {
//        QRadioButton *rb = qobject_cast<QRadioButton*>(ui->horizontalLayout_arch_index->itemAt(i));
//        if (!rb) continue;
//        log_1 << rb;
//    }
    log_1 << sender() << checked;
    QRadioButton *rb = qobject_cast<QRadioButton*>(sender());
    if (!rb) return;
    if (rb->objectName() == "radioButton_arch_data1") lindex = 0;
    if (rb->objectName() == "radioButton_arch_data2") lindex = 1;
    if (lindex < 0) return;
    if (lindex >= mad.size()) return;
    model_arch_data * m = mad.at(lindex);
    if (checked) {
        m->connect_dev(true);
        emit signal_readFlash(m->flash_curr_addr, m->flash_len - (m->flash_curr_addr - m->flash_start_addr));
    } else {
        m->connect_dev(false);
    }
}

void archive_view::slot_radioButton_arch_clicked()
{
    log_1 << sender();
    if (qobject_cast<QRadioButton *>(sender())) {
        timer_select_arch->start(500);
    } else {
        timer_select_arch->stop();
    }
    int lindex = -1;
    model_arch_data * m;
    for (int i = 0; i < ui->horizontalLayout_arch_index->count(); ++i) {
        QRadioButton *rb = qobject_cast<QRadioButton*>(ui->horizontalLayout_arch_index->itemAt(i)->widget());
        if (!rb) continue;
        if (rb->isChecked()) lindex = i;
        m = mad.at(i);
        m->connect_dev(false);
    }
    if (lindex < 0) return;
    m = mad.at(lindex);
    m->connect_dev(true);
    emit signal_readFlash(m->flash_curr_addr, m->flash_len - (m->flash_curr_addr - m->flash_start_addr));
    proxy_model->setSourceModel(m);
    ui->tableView->setModel(proxy_model);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void archive_view::on_pushButton_clear_clicked()
{
    int lindex = -1;
    model_arch_data * m;
    for (int i = 0; i < ui->horizontalLayout_arch_index->count(); ++i) {
        QRadioButton *rb = qobject_cast<QRadioButton*>(ui->horizontalLayout_arch_index->itemAt(i)->widget());
        if (!rb) continue;
        if (rb->isChecked()) lindex = i;
    }
    if (lindex < 0) return;
    m = mad.at(lindex);
    //m->connect_dev(false);
    m->clear();
}

void archive_view::on_pushButton_save_as_clicked()
{
    QString fname = QFileDialog::getSaveFileName(this, "Сохранить как",
                                                 qApp->applicationDirPath()+"/"+QDateTime::currentDateTime().toString("yyyy_MM_dd___hh_mm_ss__zzz"),
                                                 tr("Data (*.csv)"));
    if (fname.isEmpty()) return;
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly)) return;
    {
        QStringList slist;
        for (int col = 0, col_count = proxy_model->columnCount(); col < col_count; ++col) {
            slist.append("\""+proxy_model->headerData(col, Qt::Horizontal).toString()+"\"");
        }
        QString str = slist.join(";");
        str.replace("\n","_");
        str += "\r\n";
        file.write(str.toLocal8Bit());
    }
    for (int row = 0, row_count = proxy_model->rowCount(); row < row_count; ++row) {
        QStringList slist;
        for (int col = 0, col_count = proxy_model->columnCount(); col < col_count; ++col) {
            slist.append("\""+proxy_model->data(proxy_model->index(row, col)).toString()+"\"");
        }
        QString str = slist.join(";");
        str += "\r\n";
        file.write(str.toLocal8Bit());
    }
    log_1 << fname;
    log_1 << proxy_model->rowCount() << proxy_model->data(proxy_model->index(0,0));
}

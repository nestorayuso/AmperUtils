#ifndef ARCHIVE_VIEW_H
#define ARCHIVE_VIEW_H
#include "amper_dev.h"
#include <QDialog>
#include <QProgressBar>
#include <model_arch_data.h>
#include <model_arch_data_profile.h>
#include <model_arch_search.h>
#include <model_arch_event.h>
#include <QSortFilterProxyModel>
//#include "communication_thread.h"

namespace Ui {
class archive_view;
}

class archive_view : public QDialog
{
    Q_OBJECT

public:
    explicit archive_view(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~archive_view();
    void setAmperDev(amper_dev *ad) {dev = ad;}
    void setProgressBar(QProgressBar *pb) {bar = pb;}

private:
    Ui::archive_view *ui;
    amper_dev * dev;
    QProgressBar *bar;
    QList<model_arch_data *> mad;
    QSortFilterProxyModel * proxy_model;
    //model_arch_data *mad1;
    //communication_thread * cthread;

    QTimer * timer_select_arch;
    //QByteArray thread_buff;
    uint time_revisible;
signals:
    void signal_readFlash(int addr_start, int len);

public slots:
    void slot_persent_complated(float persent);
    //void slot_readFlash(int addr, QByteArray data);
    void setVisible(bool visible);
protected slots:
    void slot_radioButton_arch_clicked();
    void slot_radioButton_arch_toggled(bool checked);
private slots:
    void on_pushButton_clear_clicked();
    void on_pushButton_save_as_clicked();
};

#endif // ARCHIVE_VIEW_H

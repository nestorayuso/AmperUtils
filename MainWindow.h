#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class DTI_interface;
class amper_dev;
class QProgressBar;
class QNetworkAccessManager;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Ui::MainWindow *ui;

    DTI_interface *dtii;
    amper_dev *dev;
protected:
//    Dialog *mainD;
    QProgressBar * bar;
    void calibration_phase(int phase);
    void close_central_widget();
public slots:
    void slot_set_win_title(QString titl);
    void slot_exit();
    void slot_connect(bool isConnecting);
    void slot_connect();
    void slot_disconnect();
private slots:
    void on_action_calibration_A_triggered();
    void on_action_calibration_B_triggered();
    void on_action_calibration_C_triggered();
    void on_action_calibration_N_triggered();
    void on_action_curr_triggered();
    void on_action_tariff_triggered();
    void on_action_arch_triggered();
    void on_action_update_sw_triggered();
    void on_action_connect_settings_triggered();
    void on_action_dev_settings_triggered();
};

#endif // MAINWINDOW_H

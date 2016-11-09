#ifndef SUPDATE_H
#define SUPDATE_H

#include <QWidget>

namespace Ui {
class supdate;
}

class supdate : public QWidget
{
    Q_OBJECT

public:
    explicit supdate(QWidget *parent = 0);
    ~supdate();

private:
    Ui::supdate *ui;
};

#endif // SUPDATE_H

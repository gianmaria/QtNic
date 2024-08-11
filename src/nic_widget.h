#ifndef NIC_WIDGET_H
#define NIC_WIDGET_H

#include <QWidget>

namespace Ui {
class Nic_Widget;
}

class Nic_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Nic_Widget(QWidget *parent = nullptr);
    ~Nic_Widget();

private:
    Ui::Nic_Widget *ui;
};

#endif // NIC_WIDGET_H

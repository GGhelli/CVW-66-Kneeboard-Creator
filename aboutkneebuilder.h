#ifndef ABOUTKNEEBUILDER_H
#define ABOUTKNEEBUILDER_H

#include <QWidget>

namespace Ui {
class AboutKneebuilder;
}

class AboutKneebuilder : public QWidget
{
    Q_OBJECT

public:
    explicit AboutKneebuilder(QWidget *parent = nullptr);
    ~AboutKneebuilder();

private:
    Ui::AboutKneebuilder *ui;
};

#endif // ABOUTKNEEBUILDER_H

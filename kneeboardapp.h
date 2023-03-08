#ifndef KNEEBOARDAPP_H
#define KNEEBOARDAPP_H

#include <QApplication>

class KneeboardApp :
    public QApplication
{
    Q_OBJECT
public:
    KneeboardApp(int &argc, char *argv[]);

    static KneeboardApp *instance();
    static QString dataRoot();
    static QString database();
};

#endif // KNEEBOARDAPP_H

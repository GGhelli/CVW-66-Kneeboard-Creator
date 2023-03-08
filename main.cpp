#include <QFile>
#include <QDebug>
#include <QDir>

#include "kneeboardmainwnd.h"
#include "kneeboardapp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    KneeboardApp a(argc, argv);

    QFile css(qApp->applicationDirPath()+"/styles.css");
    if( css.open(QIODevice::ReadOnly))
        a.setStyleSheet(css.readAll());

    qDebug() << KneeboardApp::dataRoot();

    KneeboardMainWnd w;
    w.setWindowTitle(APP_NAME);
    w.show();
    return a.exec();
}

#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

#include "kneeboardapp.h"

KneeboardApp::KneeboardApp(int &argc, char *argv[]) :
    QApplication(argc, argv)
{
    setApplicationName(APP_NAME);
    setOrganizationName(ORGANIZATION_NAME);
    setOrganizationDomain(ORGANIZATION_DOMAIN);
    setApplicationVersion(APP_VERSION);
    setApplicationDisplayName(QString(APP_DISPLAY_NAME).replace("_", " "));

    auto dbPath = dataRoot() + "/database";
    if( !QDir().mkpath(dbPath) )
        QMessageBox::critical(nullptr, tr("Kneeboard"), tr("Cannot create data path [%1]").arg(dbPath));
}

KneeboardApp *KneeboardApp::instance()
{
    return dynamic_cast<KneeboardApp*>(qApp);
}

QString KneeboardApp::dataRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QString KneeboardApp::database()
{
    return dataRoot() + "/database";
}

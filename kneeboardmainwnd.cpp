#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QImage>

#include "kneeboardapp.h"
#include "kneeboardmainwnd.h"
#include "editdocumentwdg.h"

#include "ui_kneeboardmainwnd.h"

enum class KneeboardEntryType {
    Group,
    File
};

class KneeboardMainWndData {
public:
    KneeboardMainWndData(KneeboardMainWnd *parent) : p(parent) {}

    QString m_currentDatabase;

    QString getBuildFolder() const {
        auto saveDir = p->ui->edtKneeboardDir->text();
        if( !QFileInfo(saveDir).exists() || !QFileInfo(saveDir).isDir() )
        {
            auto savedGames =  QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Saved Games";
            saveDir = QFileDialog::getExistingDirectory(p, KneeboardMainWnd::tr("Save to"),savedGames);
            if( saveDir.isNull() )
                return QString();
            p->ui->edtKneeboardDir->setText(saveDir);
            p->ui->edtKneeboardDir->setToolTip(saveDir);
        }
        return saveDir;
    }
    bool editGroup(QString groupFolder, QString title) {
        QDialog dlg(p);
        dlg.setLayout(new QHBoxLayout);
        auto edit = new EditDocumentWdg(&dlg);
        dlg.layout()->addWidget(edit);
        dlg.setWindowTitle(title);
        edit->loadGroup(groupFolder);
        edit->connect(edit, SIGNAL(saved()), &dlg, SLOT(accept()));
        edit->connect(edit, SIGNAL(cancelled()), &dlg, SLOT(reject()));
        dlg.setGeometry(p->geometry());
        return dlg.exec()==QDialog::Accepted;
    }
private:
    KneeboardMainWnd *p;
};

KneeboardMainWnd::KneeboardMainWnd(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KneeboardMainWnd),
    d(std::make_unique<KneeboardMainWndData>(this))
{
    QSettings settings;
    QSettings ini(qApp->applicationDirPath() + "/kneeboard.ini", QSettings::IniFormat);

    ui->setupUi(this);
    auto dbPath = KneeboardApp::database();
    QDir dbDir(dbPath);
    ui->cmbDatabase->clear();

    // Read aricrafts from ini
    ini.beginGroup("Aircrafts");
    for(const auto &aircraft: ini.allKeys() )
    {
        qDebug() << "Adding predifined group" << aircraft;
        ui->cmbDatabase->addItem(aircraft);
    }
    ini.endGroup();

    for(const auto &entry: dbDir.entryInfoList(QDir::Filter::NoDotAndDotDot|QDir::Filter::Dirs) )
    {
        qDebug() << "Adding entry" << entry.absoluteFilePath();
        if( ui->cmbDatabase->findText(entry.fileName(), Qt::MatchFlag::MatchExactly)<0 )
            ui->cmbDatabase->addItem(entry.fileName());
    }

    auto lastAircraft = settings.value("lastAircraft", ui->cmbDatabase->currentText()).toString();
    qDebug() << "Loading last aircraft" << lastAircraft;
    ui->cmbDatabase->setCurrentText(lastAircraft);
    changeDatabase(lastAircraft);

    connect(ui->btnAddDatabase, SIGNAL(clicked(bool)), this, SLOT(addDatabase()));
    connect(ui->btnRemoveDatabase, &QToolButton::clicked, [this](bool) {
        removeDatabase(ui->cmbDatabase->currentText());
    });
    connect(ui->cmbDatabase, SIGNAL(currentTextChanged(QString)), this, SLOT(changeDatabase(QString)));
    connect(ui->lstGroups, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
        editGroup(item->text());
    });
    connect(ui->btnUp, SIGNAL(clicked(bool)), this, SLOT(moveUp()));
    connect(ui->btnDown, SIGNAL(clicked(bool)), this, SLOT(moveDown()));
    connect(ui->btnAdd, SIGNAL(clicked(bool)), this, SLOT(addGroup()));
    connect(ui->btnClearKneeboard, SIGNAL(clicked(bool)), this, SLOT(clearKneeboard()));
    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(ui->btnChooseOutputFolder, SIGNAL(clicked(bool)), this, SLOT(changeOutputDir()));
    connect(ui->btnEditKneeboard, SIGNAL(clicked(bool)), this, SLOT(editKneeboard()));
    connect(ui->btnRemove, &QAbstractButton::clicked, [this](bool) {
        auto item = ui->lstGroups->currentItem();
        if( !item || !item->isSelected() )
            return;
        this->removeGroup(item->text());
    });
    connect(ui->btnModifyGroup, &QAbstractButton::clicked, [this](bool) {
        auto item = ui->lstGroups->currentItem();
        if( !item || !item->isSelected() )
            return;
        editGroup(item->text());
    });
    connect(ui->btnAddToKneeboard, &QAbstractButton::clicked, [this](bool) {
        auto item = ui->lstGroups->currentItem();
        if( !item || !item->isSelected() )
            return;
        addGroupToKneeboard(item->text());
    });
    connect(ui->btnRemoveFromKneeboard, &QAbstractButton::clicked, [this](bool) {
        auto item = ui->lstKneeboard->currentItem();
        if( !item || !item->isSelected() )
            return;
        removeGroupFromKneeboard(item->text());
    });
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionInfo, &QAction::triggered, [this](bool) {
        QMessageBox::about(this, qApp->applicationName(), tr("%1 ver.%2 by Exup for CVW-66").arg(qApp->applicationName(), qApp->applicationVersion()));
    });

    auto db = settings.value("currentDatabase").toString();
    qDebug() << db;
    ui->cmbDatabase->setCurrentText(db);

    ui->lstKneeboard->installEventFilter(this);
//    changeDatabase(ui->cmbDatabase->currentText());

//    setWindowIcon(QIcon(":/kneeboard.svg"));
}

KneeboardMainWnd::~KneeboardMainWnd()
{
    delete ui;
}

void KneeboardMainWnd::addDatabase()
{
    auto dbName = QInputDialog::getText(this, "Add Database", "Name of the new database");
    if( dbName.isEmpty() )
        return;
    else if( QFileInfo(KneeboardApp::database() + "/" + dbName).exists() )
    {
        QMessageBox::critical(this, tr("Kneeboard"), tr("Group %1 alread exists"));
        return;
    }
    if( !QDir().mkpath(KneeboardApp::database() + "/" + dbName) )
    {
        QMessageBox::critical(this, tr("Kneeboard"), tr("Error creating group %1: cannot create folder"));
        return;
    }
    ui->cmbDatabase->addItem(dbName);

    // Create the default ini file
    QFile iniFile(KneeboardApp::database() + "/" + dbName + "/defaults.ini");
    iniFile.open(QIODevice::WriteOnly);
}

void KneeboardMainWnd::removeDatabase(QString dbname)
{
    if( dbname.isEmpty() )
        return;
    if( QMessageBox::information(this, tr("Kneeboard"), tr("Do you really want to remove the [%1] database? All contents will be lost").arg(dbname))==QMessageBox::No )
        return;
    auto dbIdx = ui->cmbDatabase->currentIndex();
    ui->cmbDatabase->removeItem(dbIdx);
    QDir(KneeboardApp::database() + "/" + dbname).removeRecursively();
}

void KneeboardMainWnd::changeDatabase(QString dbName)
{
    if( dbName.isEmpty() )
        return;
    QSettings settings;
    settings.setValue("lastAircraft", dbName);

    d->m_currentDatabase = dbName;
    ui->lstGroups->clear();
    ui->lstKneeboard->clear();
    QDir dbDir(KneeboardApp::database() + "/" + dbName);
    for(const auto &entry: dbDir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot) )
        ui->lstGroups->addItem(entry.fileName());

    QSettings ini(KneeboardApp::database() + "/" + d->m_currentDatabase + "/defaults.ini", QSettings::IniFormat);
    ini.setValue("Kneeboard/database", dbName);
    auto saveDir = ini.value("Kneeboard/SaveDir").toString();
    ui->edtKneeboardDir->setText(saveDir);
    ui->edtKneeboardDir->setToolTip(saveDir);
    // Load groups from default.ini
    for(auto idx=0; idx <ui->lstGroups->count(); ++idx)
    {
        auto group = ini.value(QString("Kneeboard/groups/group%1").arg(idx)).toString().split("|");
        qDebug() << group;
        if( group.isEmpty() )
            continue;
        bool ok;
        auto type = static_cast<KneeboardEntryType>(group.value(0).toUInt(&ok));
        if( !ok || !ui->lstKneeboard->findItems(group.value(1), Qt::MatchFlag::MatchExactly).isEmpty() )
            continue;
        if( (type==KneeboardEntryType::Group && group.count()==2) )        // Lok if group is defined
        {
            auto item = new QListWidgetItem(ui->lstKneeboard, static_cast<int>(type));
            item->setText(group.value(1));
        }
        else if( type==KneeboardEntryType::File && group.count()==3  )
        {
            auto item = new QListWidgetItem(ui->lstKneeboard, static_cast<int>(type));
            item->setText(group.value(1));
            item->setData(Qt::UserRole, group.value(2));
        }
    }
}

void KneeboardMainWnd::editGroup(QString group)
{
    d->editGroup(KneeboardApp::database() + "/" + d->m_currentDatabase + "/" + group, tr("Edit %1").arg(group));
}

void KneeboardMainWnd::addGroup()
{
    if( d->m_currentDatabase.isEmpty() )
        return;
    auto groupName = QInputDialog::getText(this, tr("New group"), tr("New group name"));
    if( groupName.isEmpty() )
        return;
    QDir groupDir(KneeboardApp::database() + "/" + d->m_currentDatabase + "/" + groupName);
    if( !groupDir.exists() )
        ui->lstGroups->addItem(groupName);
    if( groupDir.exists() && QMessageBox::question(this, tr("Existing group"), tr("The group alread exists, if you choose to proceed it's contents will be lost. Are you sure you want to overwrite?"))==QMessageBox::Yes )
        groupDir.removeRecursively();
    else if( !QDir().mkpath(groupDir.absolutePath()) )
    {
        QMessageBox::critical(this, tr("New group"), tr("Cannot create folder for group[%1]").arg(groupName));
        return;
    }
    editGroup(groupName);
}

void KneeboardMainWnd::removeGroup(QString group)
{
    QDir groupDir(KneeboardApp::database() + "/" + d->m_currentDatabase + "/" + group);
    if( groupDir.exists() && QMessageBox::question(this, tr("Existing group"), tr("Are you sure you want to remove group [%1] and all it's contents?").arg(group))==QMessageBox::Yes)
    {
        groupDir.removeRecursively();
        for(auto item: ui->lstGroups->findItems(group, Qt::MatchFlag::MatchExactly) )
            delete item;

    }
}

void KneeboardMainWnd::addGroupToKneeboard(QString group)
{
    if( ui->lstKneeboard->findItems(group, Qt::MatchFlag::MatchExactly).isEmpty() )
    {
        auto kneeboardGroup = new QListWidgetItem(ui->lstKneeboard, static_cast<int>(KneeboardEntryType::Group));
        kneeboardGroup->setText(group);
    }
}

void KneeboardMainWnd::removeGroupFromKneeboard(QString group)
{
    for(auto item: ui->lstKneeboard->findItems(group, Qt::MatchFlag::MatchExactly) )
        delete item;
}

void KneeboardMainWnd::moveUp()
{
    auto item = ui->lstKneeboard->currentItem();
    if( !item || !item->isSelected())
        return;
    int row = ui->lstKneeboard->currentRow();
    if( row==0 )
        return;
    ui->lstKneeboard->takeItem(row);
    ui->lstKneeboard->insertItem(row-1, item);
    item->setSelected(true);
    ui->lstKneeboard->setCurrentItem(item);
}

void KneeboardMainWnd::moveDown()
{
    auto item = ui->lstKneeboard->currentItem();
    if( !item || !item->isSelected())
        return;
    int row = ui->lstKneeboard->currentRow();
    if( row==ui->lstKneeboard->count()-1 )
        return;
    ui->lstKneeboard->takeItem(row);
    ui->lstKneeboard->insertItem(row+1, item);
    item->setSelected(true);
    ui->lstKneeboard->setCurrentItem(item);

}

void KneeboardMainWnd::clearKneeboard()
{
    if( ui->lstKneeboard->count() && QMessageBox::question(this, tr("Clear Kneeboard"), tr("Are you sure you want to clear the current kneeboard?"))==QMessageBox::Yes )
        ui->lstKneeboard->clear();
}

void KneeboardMainWnd::save()
{
    if( ui->lstKneeboard->count()==0 )
        return;
    QSettings settings;
    settings.setValue("currentDatabase", d->m_currentDatabase);

    auto dbFolder = KneeboardApp::database() + "/" + d->m_currentDatabase;
    auto saveDir = d->getBuildFolder();
    if( saveDir.isEmpty() )
        return;

    QDialog dlg(this);
    dlg.setLayout(new QHBoxLayout);
    auto edit = new EditDocumentWdg(&dlg);
    edit->setSaveFolder(saveDir);
    dlg.layout()->addWidget(edit);
    dlg.setWindowTitle(tr("Create kneeboard"));
    edit->connect(edit, SIGNAL(saved()), &dlg, SLOT(accept()));
    edit->connect(edit, SIGNAL(cancelled()), &dlg, SLOT(reject()));
    dlg.setGeometry(geometry());


    // Save all groups to the kneeboard!
    auto imageIdx{1};
    for(auto groupIdx=0; groupIdx<ui->lstKneeboard->count(); ++groupIdx)
    {
        auto groupItem = ui->lstKneeboard->item(groupIdx);
        auto group = groupItem->text();
        switch( static_cast<KneeboardEntryType>(groupItem->type()) )
        {
        case KneeboardEntryType::Group:
        {
            qDebug() << "Adding group" << group;
            auto groupFolder = dbFolder + "/" + group;
            edit->addFolder(groupFolder);
            break;
        }
        case KneeboardEntryType::File:
        {
            auto file = groupItem->data(Qt::UserRole).toString();
            edit->addFile(file, "");
            break;
        }
        default:
            break;
        }
    }
    if( dlg.exec()==QDialog::Accepted )
    {
        // Save everything
        QSettings ini(dbFolder  + "/defaults.ini", QSettings::IniFormat);
        ini.remove("Kneeboard/groups");
        for(auto idx=0; idx <ui->lstKneeboard->count(); ++idx)
        {
            auto groupItem = ui->lstKneeboard->item(idx);
            auto group = groupItem->text();
            switch(static_cast<KneeboardEntryType>(groupItem->type()))
            {
            case KneeboardEntryType::Group:
                ini.setValue(QString("Kneeboard/groups/group%1").arg(idx), QString("%1|%2").arg(groupItem->type()).arg(group));
                break;
            case KneeboardEntryType::File:
                ini.setValue(QString("Kneeboard/groups/group%1").arg(idx), QString("%1|%2|%3").arg(groupItem->type()).arg(group, groupItem->data(Qt::UserRole).toString()));
                break;
            }
        }
        ini.setValue("Kneeboard/SaveDir", saveDir);
        ini.sync();
    }
//    editKneeboard();
//    QMessageBox::information(this, tr("Build kneeboard"), tr("The kneeboard has been succesfully built"));
}

void KneeboardMainWnd::changeOutputDir()
{
    auto saveDir = ui->edtKneeboardDir->text();
    saveDir = QFileDialog::getExistingDirectory(this, tr("Output dir"), saveDir);
    if( saveDir.isEmpty() )
        return;
    ui->edtKneeboardDir->setText(saveDir);
    ui->edtKneeboardDir->setToolTip(saveDir);
}

void KneeboardMainWnd::editKneeboard()
{
    auto kneeboardFolder = d->getBuildFolder();
    d->editGroup(kneeboardFolder, tr("Edit kneeboard"));
}


bool KneeboardMainWnd::eventFilter(QObject *obj, QEvent *event)
{
    if( obj!=ui->lstKneeboard )
        return QObject::eventFilter(obj, event);
    auto acceptUrl = [](const QUrl &url) {
        if( !(url.scheme() == "file"  && url.isLocalFile()) )
            return false;
        QFileInfo file(url.toLocalFile());
        if( !file.isFile() && file.suffix().toLower()!="pdf" )
            return false;
        return true;

    };

    switch(event->type())
    {
    case QEvent::DragEnter:
    {
        QDragEnterEvent *enter = dynamic_cast<QDragEnterEvent*>(event);
        for(const auto &url: enter->mimeData()->urls()) // Accept only files and PDF/image files
        {
            qDebug() << url;
            if( !acceptUrl(url) )
                return QObject::eventFilter(obj, event);
        }
        enter->acceptProposedAction();
        return true;
    }
    case QEvent::Drop:
    {
        auto *drop = dynamic_cast<QDropEvent*>(event);
        for(const auto &url: drop->mimeData()->urls())
        {
            QFileInfo file(url.toLocalFile());
            auto item = new QListWidgetItem(ui->lstKneeboard, static_cast<int>(KneeboardEntryType::File));
            item->setText(file.fileName());
            item->setToolTip(file.absoluteFilePath());
            item->setData(Qt::UserRole, file.absoluteFilePath());
        }
        return true;
    }
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}


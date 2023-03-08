#include <QProcess>
#include <QKeyEvent>
#include <QSettings>
#include <QDropEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>
#include <QImage>
#include <QIcon>
#include <QTemporaryDir>
#include <QFileDialog>

#include "editdocumentwdg.h"
#include "ui_editdocumentwdg.h"

EditDocumentWdg::EditDocumentWdg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditDocumentWdg)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->lstImages->setViewMode(QListWidget::IconMode);
    ui->lstImages->setResizeMode(QListWidget::Adjust);
    ui->lstImages->setIconSize(QSize(200,200));

    connect(ui->lstImages, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this, SLOT(itemActivated(QListWidgetItem *)));
    connect(ui->btnClear, &QPushButton::clicked, [this](bool){
        if( QMessageBox::question(this, tr("Kneeboard"), tr("Are you sure you want to clear all the kneeoboard?"))==QMessageBox::Yes )
        {
            ui->lstImages->clear();
            ui->wdgPreview->setImage(QImage());
        }
    });
    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(ui->btnDel, SIGNAL(clicked(bool)), this, SLOT(deleteCurrentItem()));
    connect(ui->btnMoveBack, SIGNAL(clicked(bool)), this, SLOT(moveBackward()));
    connect(ui->btnMoveForward, SIGNAL(clicked(bool)), this, SLOT(moveForward()));
    connect(ui->btnAddImage, SIGNAL(clicked(bool)), this, SLOT(addFile()));
    connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SIGNAL(cancelled()));

    connect(ui->sldThumbnailSize, &QSlider::valueChanged, [this](int value) {
        ui->lstImages->setIconSize(QSize(value, value));
    });
    ui->lstImages->installEventFilter(this);

//    qDebug() << ui->wdgPreview->geometry();
//    qDebug() << ui->lstImages->geometry();
//    auto previewWidth = ui->wdgPreview->height() * 3 / 4;
//    auto delta = ui->wdgPreview->width() - previewWidth;
//    QList<int> sizes;
//    sizes << ui->lstImages->width() + delta;
//    sizes << previewWidth;
//    ui->splitter->setSizes(sizes);
}

EditDocumentWdg::~EditDocumentWdg()
{
    delete ui;
}

void EditDocumentWdg::loadGroup(QString folder)
{
    QDir groupDir(folder);
    QStringList filter{"*.pdf", "*.jpg", "*.jpeg", "*.png", "*.bmp"};
    for(const auto &entry: groupDir.entryInfoList(filter, QDir::Filter::Files, QDir::SortFlag::Name) )
    {
        auto filename = entry.completeBaseName().mid(4); // Remove the ###- iniziale
        addFile(entry.absoluteFilePath(), filename);
    }
    m_folder = folder;
}

void EditDocumentWdg::addFile()
{
    QSettings settings;

    auto openDir = settings.value("openDir").toString();
    auto file = QFileDialog::getOpenFileName(this, tr("Kneeboard"), openDir, "PDF File (*.pdf);; Image File (*.png *.jpg *.jpeg *.bmp)" );
    if( file.isEmpty())
        return;
    addFile(file, QFileInfo(file).completeBaseName());
    settings.setValue("openDir", QFileInfo(file).absolutePath());
}

void EditDocumentWdg::addFile(QString file, QString imgName)
{
    QSettings ini(qApp->applicationDirPath() + "/kneeboard.ini", QSettings::IniFormat);
    auto height = ini.value("kneeboard/imageHeight", 1200).toUInt();

    try {
        if( QFileInfo(file).suffix().compare("pdf", Qt::CaseInsensitive)==0 )
        {
            qDebug() << "Adding PDF";
            auto pdfName = QFileInfo(file).completeBaseName();
            QTemporaryDir dir;
            if( !dir.isValid()  )
                throw( tr("Cannot create tmp dir: %1").arg(dir.errorString()) );
            // mutool draw -o dir/filename-%3d.png -h height pdfFile
            QStringList args = {
                "draw",
                "-o",
                QDir::toNativeSeparators(dir.path() + "/%3d-" + pdfName + ".png"),
                "-h",
                QString::number(height),
                QDir::toNativeSeparators(file)
            };
            QProcess mutool;
            mutool.setProgram(qApp->applicationDirPath() + "/mutool.exe");
            mutool.setArguments(args);
            mutool.start();
            if( !mutool.waitForStarted() )
                throw(QString("Error executing %1 %2: %3").arg(mutool.program(), mutool.arguments().join("|"), mutool.errorString()));
            if( !mutool.waitForFinished() )
                throw(QString("Error executing %1 %2: %3").arg(mutool.program(), mutool.arguments().join("|"), mutool.errorString()));

            qDebug() << mutool.readAllStandardOutput();
            qDebug() << mutool.readAllStandardError();
            for(const auto &entry: QDir(dir.path()).entryInfoList(QStringList() << "*.png"))
                addFile(entry.absoluteFilePath(), pdfName);
        }
        else
        {
            QImage img(file);
            if( img.isNull() )
            {
                QMessageBox::critical(this, "Kneeboard", tr("[%1] is not an image or is an unsupported format").arg(file));
                return;
            }

            auto item = new QListWidgetItem(ui->lstImages);
            item->setText(QString("Image %1").arg(ui->lstImages->count()));
            item->setIcon(QIcon(QPixmap::fromImage(img)));
            item->setData(Qt::UserRole, QVariant::fromValue(img));
            item->setData(Qt::UserRole+1, imgName.isEmpty() ? QFileInfo(file).completeBaseName() : imgName);
//            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsDragEnabled|Qt::ItemIsEnabled|Qt::ItemIsDropEnabled);
        }
    } catch( QString err ) {
        QMessageBox::critical(this, "PDF", err);
        return;
    }
}

void EditDocumentWdg::addFolder(QString folder)
{
    QDir dir(folder);

    for(const auto &entry: dir.entryInfoList(QStringList() << "*.pdf" << "*.png" << "*.bmp" << "*.jpg" << "*.jpeg"))
        addFile(entry.absoluteFilePath(), entry.completeBaseName());
}

void EditDocumentWdg::save()
{
//    QSettings settings;

//    auto saveDir = settings.value("savedir").toString();
//    saveDir = QFileDialog::getExistingDirectory(this, tr("Kneeboard"), saveDir);
//    if( saveDir.isEmpty() )
//        return;
//    settings.setValue("savedir", saveDir);
//    if( !QFileInfo(saveDir).exists() )
//        QDir().mkpath(saveDir);

    auto saveDir = m_folder;
    QDir dir(saveDir);
    for(const auto &entry: dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Filter::NoDotAndDotDot) )
    {
        qDebug() << "Removing" << entry;
        if( entry.isDir() )
            QDir(entry.absoluteFilePath()).removeRecursively();
        else
            QFile::remove(entry.absoluteFilePath());
    }
    for(auto itemIdx=0; itemIdx <ui->lstImages->count(); ++itemIdx)
    {
        QString baseName = ui->lstImages->item(itemIdx)->data(Qt::UserRole + 1).toString();
        auto filename = QString("%1-%2.png").arg(itemIdx+1, 3, 10, QChar('0')).arg(baseName);
        auto img = ui->lstImages->item(itemIdx)->data(Qt::UserRole).value<QImage>();
        if( !img.isNull() )
        {
            qDebug() << "Saving" << filename;
            img.save(saveDir + "/" + filename);
        }
    }
    emit saved();
}

void EditDocumentWdg::moveForward()
{
    auto selected = ui->lstImages->selectedItems();
    if( selected.isEmpty() )
        return;
    auto firstIdx =  ui->lstImages->row(selected.first());
    auto lastIdx =  ui->lstImages->row(selected.last());
    if( lastIdx+1==ui->lstImages->count() )
        return;
    QList<QListWidgetItem*> items;
    for(auto idx=firstIdx; idx<=lastIdx; ++idx)
        items << ui->lstImages->item(idx);
    for(auto idx=firstIdx; idx<=lastIdx; ++idx)
        ui->lstImages->takeItem(firstIdx);
    for(int i=0; i<items.size(); ++i)
    {
        ui->lstImages->insertItem(firstIdx+1+i, items[i]);
        items[i]->setSelected(true);
    }
}

void EditDocumentWdg::moveBackward()
{
    auto selected = ui->lstImages->selectedItems();
    if( selected.isEmpty() )
        return;
    auto firstIdx =  ui->lstImages->row(selected.first());
    auto lastIdx =  ui->lstImages->row(selected.last());
    if( firstIdx==0 )
        return;
    QList<QListWidgetItem*> items;
    for(auto idx=firstIdx; idx<=lastIdx; ++idx)
        items << ui->lstImages->item(idx);
    for(auto idx=firstIdx; idx<=lastIdx; ++idx)
        ui->lstImages->takeItem(firstIdx);
    for(int i=0; i<items.size(); ++i)
    {
        ui->lstImages->insertItem(firstIdx-1+i, items[i]);
        items[i]->setSelected(true);
    }
}

void EditDocumentWdg::itemActivated(QListWidgetItem *item)
{
    qDebug() << Q_FUNC_INFO;
    if( item )
    {
        QImage img = item->data(Qt::UserRole).value<QImage>();
        ui->wdgPreview->setImage(img);
    }

}

void EditDocumentWdg::deleteCurrentItem()
{
    for( auto item: ui->lstImages->selectedItems() )
        delete item;
}

bool EditDocumentWdg::eventFilter(QObject *obj, QEvent *event)
{
    if( event->type()==QEvent::KeyPress )
    {
        qDebug() << "KeyPress";
        auto key = dynamic_cast<QKeyEvent*>(event);
        if( key->matches(QKeySequence::Delete) )
            deleteCurrentItem();
    }
    return QObject::eventFilter(obj, event);
}

void EditDocumentWdg::dragEnterEvent(QDragEnterEvent *event)
{
    for(const auto &url: event->mimeData()->urls())
    {
        if( url.scheme() == "file" )
        {
            event->acceptProposedAction();
            return;
        }
    }
}

void EditDocumentWdg::dropEvent(QDropEvent *ev)
{
    for(const auto &url: ev->mimeData()->urls())
    {
        if( url.scheme()=="file")
        {
            auto filename = url.toLocalFile();
            if( QFileInfo(filename).isDir() )
                addFolder(filename);
            else
                addFile(filename, QFileInfo(filename).completeBaseName());
        }
    }
}

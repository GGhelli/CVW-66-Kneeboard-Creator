#ifndef EDITDOCUMENTWDG_H
#define EDITDOCUMENTWDG_H

#include <QWidget>

namespace Ui {
class EditDocumentWdg;
}

class EditDocumentWdg : public QWidget
{
    Q_OBJECT

public:
    explicit EditDocumentWdg(QWidget *parent = nullptr);
    ~EditDocumentWdg();

public slots:
    void loadGroup(QString folder);
    void addFile();
    void addFile(QString file, QString imgName);
    void addFolder(QString folder);
    void save();
    void moveForward();
    void moveBackward();

private slots:
    void itemActivated(class QListWidgetItem *item);
    void deleteCurrentItem();

signals:
    void saved();
    void cancelled();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *ev) override;

private:
    Ui::EditDocumentWdg *ui;
    QString m_folder;
};

#endif // EDITDOCUMENTWDG_H

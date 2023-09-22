#ifndef KNEEBOARDMAINWND_H
#define KNEEBOARDMAINWND_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class KneeboardMainWnd; }
QT_END_NAMESPACE

class KneeboardMainWnd : public QMainWindow
{
    Q_OBJECT

public:
    KneeboardMainWnd(QWidget *parent = nullptr);
    ~KneeboardMainWnd();

public slots:
    void addDatabase();
    void removeDatabase(QString dbname);
    void changeDatabase(QString dbName);
    void editGroup(QString group);
    void addGroup();
    void removeGroup(QString group);
    void addGroupToKneeboard(QString group);
    void removeGroupFromKneeboard(QString group);
    void moveUp();
    void moveDown();
    void clearKneeboard();

    void save();
    void changeOutputDir();
    void editKneeboard();

private slots:

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::KneeboardMainWnd *ui;
    friend class KneeboardMainWndData;
    std::unique_ptr<class KneeboardMainWndData> d;
};

#endif // KNEEBOARDMAINWND_H

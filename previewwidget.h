#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QFrame>
#include <QWidget>

class PreviewWidget : public QFrame
{
    Q_OBJECT

public:
    PreviewWidget(QWidget *parent);

    void setImage(QImage image);

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    QImage m_image;
};

#endif // PREVIEWWIDGET_H

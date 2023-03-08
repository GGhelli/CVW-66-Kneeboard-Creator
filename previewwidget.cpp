#include <QPainter>
#include <QPaintEvent>
#include "previewwidget.h"

PreviewWidget::PreviewWidget(QWidget *parent) :
    QFrame(parent)
{
}

void PreviewWidget::setImage(QImage image)
{
    m_image = image;
    update();
}

void PreviewWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    auto pal = palette();

    auto rc = rect();
    painter.fillRect(rc, pal.window());

    if( m_image.isNull())
        return;

    auto srcRc = m_image.rect();
    auto dstSize = srcRc.size().scaled(rc.size(), Qt::KeepAspectRatio);
    QRect dstRect(QPoint((rc.width() - dstSize.width()) / 2, (rc.height() - dstSize.height()) / 2), dstSize);
    painter.setRenderHints( QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform);
    painter.drawImage(dstRect, m_image, srcRc);
}

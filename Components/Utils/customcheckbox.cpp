#include "customcheckbox.h"
#include <QPainter>
#include <QMouseEvent>

CustomCheckBox::CustomCheckBox(QString s, bool val, bool en, QWidget *parent)
    : QWidget(parent)
{
    m_checked = val;
    m_text = s;
    setEnabled(en);
    setCursor(Qt::PointingHandCursor);
    updateSize();
}

void CustomCheckBox::updateSize() {
    setMinimumSize(sizeHint());
}

QSize CustomCheckBox::sizeHint() const {
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(m_text);
    int height = qMax(25, fm.height() + 10);
    return QSize(checkboxSize + spacing + textWidth + padding * 2, height);
}

void CustomCheckBox::setVal(bool val) {
    m_checked = val;
}

bool CustomCheckBox::isChecked() const {
    return m_checked;
}

void CustomCheckBox::setText(const QString& text) {
    m_text = text;
    update();
}

QString CustomCheckBox::text() const {
    return m_text;
}

void CustomCheckBox::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect boxRect(3, (height() - 18) / 2, 18, 18);

    // Draw checkbox background
    QColor boxColor = this->isEnabled() ? QColor("#00b894") : QColor(120,120,120);
    painter.setBrush(boxColor);
    painter.setPen(QPen(boxColor, 1));
    painter.drawRoundedRect(boxRect, 3, 3);

    // Draw tick if checked
    if (m_checked) {
        painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        QPoint p1(boxRect.left() + 4, boxRect.top() + 9);
        QPoint p2(boxRect.left() + 8,  boxRect.bottom() - 4);
        QPoint p3(boxRect.right() - 4, boxRect.top() + 5);

        painter.drawLine(p1, p2);
        painter.drawLine(p2, p3);
    }

    // Draw text
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    QFontMetrics fm(font);
    int textY = (height() + fm.ascent() - fm.descent()) / 2;
    painter.drawText(QPoint(boxRect.right() + spacing, textY), m_text);
}

void CustomCheckBox::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        m_checked = !m_checked;
        update();
        emit toggled(m_checked);
    }
}

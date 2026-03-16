#ifndef SIGNALSMASK_H
#define SIGNALSMASK_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QGuiApplication>
#include <QMouseEvent>
#include "../../Utils/customcheckbox.h"

class SignalsMask : public QDialog
{
    Q_OBJECT
public:
    SignalsMask(bool s1, bool s2, bool s3, bool s4, bool s5, bool s6, std::vector<bool> maskval);
    void setVal(bool s1, bool s2, bool s3, bool s4, bool s5, bool s6);

protected:
    void showEvent(QShowEvent *event) {
        QDialog::showEvent(event);

        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        move(screenGeometry.center() - rect().center());
    }

    void mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            QWidget* child = childAt(event->pos());
            if (child && (child == topBar || topBar->isAncestorOf(child))) {
                dragging = true;
                dragStartPos = event->globalPosition().toPoint() - this->pos();
            } else {
                dragging = false;
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *event) {
        if (dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - dragStartPos);
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        dragging = false;
    }

private slots:
    void Close();

signals:

public:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn, *saveBtn, *set, *unset;

    std::map<QString, CustomCheckBox*> mp;

    QStringList list = {
        "g1c", "g1p", "g1w", "g1y", "g1m", "g1n", "g1s", "g1l", "g1x",
        "g2c", "g2d", "g2s", "g2l", "g2x", "g2p", "g2w", "g2y", "g2m", "g2n",
        "g5i", "g5q", "g5x",
        "r1c", "r1p", "r2c", "r2p", "r3i", "r3q", "r3x", "r4a", "r4b", "r4x",
        "r6a", "r6b", "r6x",
        "e1c", "e1a", "e1b", "e1x", "e1z",
        "e5i", "e5q", "e5x", "e6a", "e6b", "e6c", "e6x", "e6z",
        "e7i", "e7q", "e7x", "e8i", "e8q", "e8x",
        "q1c", "q1s", "q1l", "q1x", "q1z", "q1e", "q1b", "q2s", "q2l", "q2x",
        "q5i", "q5q", "q5x", "q5d", "q5p", "q5z", "q6s", "q6l", "q6x", "q6e", "q6z",
        "b2i", "b2q", "b2x", "b7i", "b7q", "b7x", "b6i", "b6q", "b6x",
        "b1d", "b1p", "b1s", "b1x", "b1l", "b1z", "b5d", "b5p", "b5x",
        "b7d", "b7p", "b7z", "b8d", "b8p", "b8x", "b6d", "b6p", "b6z",
        "i5a", "i5b", "i5c", "i5x", "i9a", "i9b", "i9c", "i1x", "i1d",
        "s1c", "s5i", "s5q", "s5x"
    };
};

#endif // SIGNALSMASK_H

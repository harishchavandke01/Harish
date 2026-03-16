#ifndef ADJUSTOPTIONS_H
#define ADJUSTOPTIONS_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QGuiApplication>
#include <QMouseEvent>
#include "../../../Utils/customcheckbox.h"

struct AdjustmentOptions
{
    bool constrained = true;
    bool useCovariance = true;
};

class AdjustOptions : public QDialog
{
    Q_OBJECT
public:
    explicit AdjustOptions(AdjustmentOptions &opts, QWidget *parent = nullptr);

public:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *heading;
    QLabel *lb1;
    QRadioButton *constrained;
    QRadioButton *free;

    CustomCheckBox *useConv;
    QPushButton *save;

private slots:
    void onClose();
    void onSave();

private:
    AdjustmentOptions &options;

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

signals:
};

#endif // ADJUSTOPTIONS_H

#ifndef ADJUSTOPTIONS_H
#define ADJUSTOPTIONS_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include "../../../Utils/customcheckbox.h"

struct AdjustmentOptions
{
    bool constrained    = true;   // not used for mode selection (mode is auto)
    bool useCovariance  = true;
};

class AdjustOptions : public QDialog
{
    Q_OBJECT
public:
    explicit AdjustOptions(AdjustmentOptions &opts, QWidget *parent = nullptr);

public:
    QWidget     *topBar;
    QPoint       dragStartPos;
    bool         dragging = false;
    QLabel      *icon;
    QLabel      *title;
    QPushButton *closeBtn;

    QLabel         *heading;
    CustomCheckBox *useConv;
    QPushButton    *save;

private slots:
    void onClose();
    void onSave();

private:
    AdjustmentOptions &options;

protected:
    void showEvent(QShowEvent *event) override {
        QDialog::showEvent(event);
        move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
    }
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            QWidget *child = childAt(event->pos());
            dragging = child && (child == topBar || topBar->isAncestorOf(child));
            if (dragging) dragStartPos = event->globalPosition().toPoint() - pos();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (dragging && (event->buttons() & Qt::LeftButton))
            move(event->globalPosition().toPoint() - dragStartPos);
    }
    void mouseReleaseEvent(QMouseEvent *) override { dragging = false; }
};

#endif // ADJUSTOPTIONS_H

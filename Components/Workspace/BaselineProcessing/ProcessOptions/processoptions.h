#ifndef PROCESSOPTIONS_H
#define PROCESSOPTIONS_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QComboBox>
#include "../../../Utils/customcheckbox.h"

class ProcessOptions : public QDialog
{
    Q_OBJECT
public:
    ProcessOptions();
    void setOp(QString dir);
    void setOption(bool val = true);
    void setbasedata(QString file);
    void setRinexheaderdata(QString lat, QString lon, QString ht);


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
    void pmodechanged(QString pmode);


public:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *lab1, *lab2, *lab3, *lab4, *lab5, *lab6, *lab7, *lab8, *lab9, *lab10, *lab11, *lab8_1;
    QLineEdit *outputDir;

    QLabel *heading;
    QWidget *contentWidget;

    CustomCheckBox *gps, *glonass, *galileo, *qzss, *beidou, *sbs;

    QComboBox *pmode, *freq, *filter, *ele, *gpsamb, *gloamb, *beiamb, *elemask;
    QPushButton *savebtn, *resetbtn;

};

#endif // PROCESSOPTIONS_H

#ifndef EDITOPTIONS_H
#define EDITOPTIONS_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QGroupBox>
#include "../../../Utils/utils.h"

class EditOptions : public QDialog
{
    Q_OBJECT
public:
    explicit EditOptions(const QString &_obsPath, QWidget *parent = nullptr);

signals:

private:
    Utils *utils;
    const QString obsPath;
    void setbasedata(QString file);

public:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *heading;
    QLabel *lb1, *lb2, *lb3, *lb4, *lb5, *lb6;
    QLineEdit *markerName, *markerNumber, *markerType, *observer, *antennaName;
    QLineEdit *posz, *posx, *posy, *poleh, *polee, *polen;
    QLineEdit *receiver;

    QLineEdit *lat, *lon, * height;
    QPushButton *baseJson;

    QPushButton *save, *reset;

signals:
    void pointIdEdited(const QString &obsPath, const QString &pointId);

private slots:
    void Close();
    void onSaveClicked();
    void onResetClicked();

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
};

#endif // EDITOPTIONS_H

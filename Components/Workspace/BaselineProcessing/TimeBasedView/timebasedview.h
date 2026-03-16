#ifndef TIMEBASEDVIEW_H
#define TIMEBASEDVIEW_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "../../../Utils/utils.h"

class TimeBasedView : public QDialog
{
    Q_OBJECT
public:
    explicit TimeBasedView(QWidget *parent = nullptr);
    void viewTimeMap(const QVector<StaticJob> &jobs, const QMap<QString, FileEntry> &files);

private:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *heading;

    QGraphicsView *leftView;
    QGraphicsScene *leftScene;

    QGraphicsView *view;
    QGraphicsScene *scene;

signals:

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

#endif // TIMEBASEDVIEW_H

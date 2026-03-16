#ifndef FIXSTATIONS_H
#define FIXSTATIONS_H

#include <QWidget>
#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QGuiApplication>
#include "../../../Context/projectcontext.h"

class FixStations : public QDialog
{
    Q_OBJECT
public:
    explicit FixStations(ProjectContext *_projectContext,QWidget *parent = nullptr);

private:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *heading;
    ProjectContext *projectContext;
    QTableWidget *table;
    QPushButton *saveBtn;
    QPushButton *resetBtn;
    void populateTable();

private slots:
    void onSave();
    void onReset();
    void Close();

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

#endif // FIXSTATIONS_H

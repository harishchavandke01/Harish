#ifndef FIXSTATIONS_H
#define FIXSTATIONS_H

#include <QDialog>
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>
#include "../../../Context/projectcontext.h"

class FixStations : public QDialog
{
    Q_OBJECT
public:
    explicit FixStations(ProjectContext *_projectContext, QWidget *parent = nullptr);

private:
    // Title bar
    QWidget     *topBar;
    QLabel      *icon;
    QLabel      *title;
    QPushButton *closeBtn;
    QPoint       dragStartPos;
    bool         dragging = false;

    // Content
    QLabel       *heading;
    QLabel       *infoLabel;
    QTableWidget *table;
    QPushButton  *saveBtn;
    QPushButton  *resetBtn;

    ProjectContext *projectContext;

    // Column indices
    enum Col {
        COL_STATION = 0,
        COL_FIXED   = 1,
        COL_EAST    = 2,
        COL_NORTH   = 3,
        COL_HEIGHT  = 4,
        COL_COUNT   = 5
    };

    void populateTable();

private slots:
    void onSave();
    void onReset();
    void onClose();

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

#endif // FIXSTATIONS_H

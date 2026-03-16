#ifndef RAWDATACHECKIN_H
#define RAWDATACHECKIN_H

#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QGuiApplication>
#include <QMouseEvent>
#include "../../utils.h"

class RawDataCheckIn : public QDialog
{
    Q_OBJECT
public:
    explicit RawDataCheckIn(const QMap<QString, FileEntry> &_files,QWidget *parent = nullptr);
    QMap<QString, FileEntry> getSelectedFiles() const;

private:
    QMap<QString, FileEntry> files;
    QMap<QString, QString> pointIdToReferenceObs;
    Utils *utils;
    struct Coords { double x; double y; double z; };
    QMap<QString, Coords> mergedCoordinates;

    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *heading;


    QTableWidget * table;
    QPushButton *saveBtn;
    QPushButton *resetBtn;
    QPushButton *cancelBtn;

private:
    enum Columns {
        COL_IMPORT = 0,
        COL_POINT_ID,
        COL_FILE,
        COL_START,
        COL_END,
        COL_DURATION,
        COL_ANTENNA,
        COL_RECEIVER,
        COL_COUNT
    };


    void setUpTable();
    void populateTable();
    void clearTable();

signals:

private slots:
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

#endif // RAWDATACHECKIN_H

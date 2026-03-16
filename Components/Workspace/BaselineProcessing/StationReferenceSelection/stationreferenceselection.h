#ifndef STATIONREFERENCESELECTION_H
#define STATIONREFERENCESELECTION_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QTableWidget>
#include "../../utils.h"
class StationReferenceSelection : public QDialog
{
    Q_OBJECT
public:
    explicit StationReferenceSelection(const QString &pointId, const QVector<StationRealization> &grp, QWidget *parent = nullptr);
    QString selectedObsPath() const;

private:
    QWidget *topBar;
    QPoint dragStartPos;
    bool dragging;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;

    QLabel *heading;

    QString m_pointId;
    QVector<StationRealization> m_grp;

    QTableWidget *table;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;

    QButtonGroup *radioGroup;
    QString m_selectedObsPath;

    void setUpTable();
    void populateTable();
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

#endif // STATIONREFERENCESELECTION_H

#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>

class CustomMessageBox : public QDialog
{
    Q_OBJECT
public:
    explicit CustomMessageBox(QString type, QString message, QString returntype, QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) {
        QDialog::showEvent(event);

        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        move(screenGeometry.center() - rect().center());
    }

    void mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            dragging = true;
            dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) {
        if (dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - dragStartPos);
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        Q_UNUSED(event);
        dragging = false;
    }

public slots:
    void Close();

signals:

private:
    QPoint dragStartPos;
    QLabel *title;
    QLabel *icon;
    QLabel *message;
    QPushButton *closeBtn;
    QPushButton *yesBtn;
    QPushButton *noBtn;
    QPushButton *okayBtn;
    bool dragging;
public:
    bool response;
};

#endif // CUSTOMMESSAGEBOX_H

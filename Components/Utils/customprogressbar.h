#ifndef CUSTOMPROGRESSBAR_H
#define CUSTOMPROGRESSBAR_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

class CustomProgressBar : public QDialog
{
    Q_OBJECT
public:
    explicit CustomProgressBar(int processcount, QWidget *parent = nullptr);

public slots:
    Q_INVOKABLE void updateCurrent(int value)
    {
        if (isclosed) return;

        if (overallProgress) {
            overallProgress->setValue(value);
        }
    }


    Q_INVOKABLE void setCurrentProcess(int p) {
        currp = p;
    }

    Q_INVOKABLE void setStatus(const QString& text) {
        if(currentprocess) currentprocess->setText(text);
    }

    int getCurrentProgress() {
        return currentProgress->value();
    }

    void Close() {
        this->close();
        isclosed = true;
    }

signals:

private:
    QLabel *heading;
    QPushButton *closeBtn;
    QLabel *currentprocess;
    QLabel *info;
    QProgressBar *currentProgress;
    QProgressBar *overallProgress;
    int count;
    int currp;

public:
    bool isclosed;
};

#endif // CUSTOMPROGRESSBAR_H

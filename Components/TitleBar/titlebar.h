#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPoint>

class MainWindow;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(MainWindow *window, QWidget *parent = nullptr);
    void setWindowTitle(const QString &title);

public slots:
    void Minimize();
    void Maximize();
    void Close();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void setupTitleBar();

    QLabel *title;
    QLabel *logoLabel;
    QPushButton *minimizeButton;
    QPushButton *maximizeButton;
    QPushButton *closeButton;

    QPoint dragPosition;
    MainWindow *m_window;
};

#endif // TITLEBAR_H

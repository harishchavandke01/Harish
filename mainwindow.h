#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Components/CreateProject/createproject.h"
#include "Components/TitleBar/titlebar.h"
#include <QPoint>
#include <QMenu>
#include <QStackedWidget>
#include "Components/Workspace/workspace.h"
class QMouseEvent;
class QShowEvent;
class QResizeEvent;
class QScreen;
class QPainter;
class QPaintEvent;

class TitleBar;

class BlurOverlay : public QWidget {
public:
    explicit BlurOverlay(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        hide();
    }

    void showOverlay() {
        raise();
        show();
    }

    void hideOverlay() {
        hide();
    }

protected:
    void paintEvent(QPaintEvent *event) override;
};

class CustomWidget : public QWidget {
    Q_OBJECT
public:
    explicit CustomWidget(QWidget *parent = nullptr);

    bool blur = false;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    BlurOverlay *overlay = nullptr;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void Close();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    enum ResizeRegion {
        None, Left, Right, Top, Bottom,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    void updateCursor(const QPoint &pos);

    ResizeRegion resizeRegion = None;
    QPoint dragStartPos;
    int borderWidth = 8;
    bool firstShow = true;

private:
    QStackedWidget *mainStackedWidget;


    TitleBar *titleBar = nullptr;
    CreateProject *createCard = nullptr;
    QMenu *projectMenu = nullptr;

    QWidget *pageCreateProject = nullptr;
    Workspace *workspace;

};

#endif // MAINWINDOW_H

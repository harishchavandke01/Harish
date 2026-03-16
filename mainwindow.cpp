#include "mainwindow.h"
#include "Components/TitleBar/titlebar.h"
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QStackedWidget>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/images/images/surveypod.png"));

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);

    CustomWidget *central = new CustomWidget(this);
    central->setMouseTracking(true);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    titleBar = new TitleBar(this, central);

    createCard = new CreateProject();
    createCard->setFixedWidth(420);
    pageCreateProject = new QWidget(central);
    QVBoxLayout * vlayout = new QVBoxLayout(pageCreateProject);
    vlayout->setContentsMargins(20,20,20,20);
    vlayout->addWidget(createCard,0,Qt::AlignTop | Qt::AlignLeft);

    workspace = new Workspace(this);

    mainStackedWidget = new QStackedWidget();
    mainStackedWidget->addWidget(pageCreateProject);
    mainStackedWidget->addWidget(workspace);
    mainStackedWidget->setCurrentWidget(pageCreateProject);

    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(mainStackedWidget);

    connect(createCard, &CreateProject::projectFolderSelected,this, [this](const QString &folder){
        workspace->setProjectFolder(folder);
        mainStackedWidget->setCurrentWidget(workspace);
    });
}

MainWindow::~MainWindow()
{

}


void MainWindow::Close()
{
    QString jsonFile = QCoreApplication::applicationDirPath() + "/Data/rinex_headers.json";

    if (QFile::exists(jsonFile)) {
        QFile::remove(jsonFile);
    }
    close();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (isMaximized())
        return;

    if (event->button() == Qt::LeftButton) {
        QPoint localPos = event->pos();
        updateCursor(localPos);

        if (resizeRegion != None) {
            dragStartPos = event->globalPosition().toPoint();
        } else {
            resizeRegion = None;
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (isMaximized())
        return;

    if (event->button() == Qt::LeftButton) {
        resizeRegion = None;
        unsetCursor();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isMaximized())
        return;

    QPointF globalPos = event->globalPosition();
    QPointF localPos = mapFromGlobal(globalPos.toPoint());

    if ((event->buttons() & Qt::LeftButton) && resizeRegion != None) {
        QRect geom = geometry();
        int dx = static_cast<int>(globalPos.x()) - dragStartPos.x();
        int dy = static_cast<int>(globalPos.y()) - dragStartPos.y();

        switch (resizeRegion) {
        case Left:        geom.setLeft(geom.left() + dx); break;
        case Right:       geom.setRight(geom.right() + dx); break;
        case Top:         geom.setTop(geom.top() + dy); break;
        case Bottom:      geom.setBottom(geom.bottom() + dy); break;
        case TopLeft:     geom.setTopLeft(geom.topLeft() + QPoint(dx, dy)); break;
        case TopRight:    geom.setTopRight(geom.topRight() + QPoint(dx, dy)); break;
        case BottomLeft:  geom.setBottomLeft(geom.bottomLeft() + QPoint(dx, dy)); break;
        case BottomRight: geom.setBottomRight(geom.bottomRight() + QPoint(dx, dy)); break;
        default: break;
        }

        QScreen *scr = screen();
        if (scr) {
            QRect screenRect = scr->availableGeometry();

            if (geom.width() > screenRect.width())
                geom.setWidth(screenRect.width());

            if (geom.height() > screenRect.height())
                geom.setHeight(screenRect.height());

            if (geom.left() < screenRect.left())
                geom.moveLeft(screenRect.left());

            if (geom.top() < screenRect.top())
                geom.moveTop(screenRect.top());

            if (geom.right() > screenRect.right())
                geom.moveRight(screenRect.right());

            if (geom.bottom() > screenRect.bottom())
                geom.moveBottom(screenRect.bottom());
        }

        setGeometry(geom);
        dragStartPos = globalPos.toPoint();
    } else {
        updateCursor(localPos.toPoint());
    }
}

void MainWindow::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (isMaximized())
        return;

    unsetCursor();
    resizeRegion = None;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (!firstShow)
        return;

    firstShow = false;

    QScreen *scr = QGuiApplication::primaryScreen();
    if (!scr)
        return;

    QRect screenGeometry = scr->availableGeometry();
    int x = screenGeometry.x() + (screenGeometry.width() - width()) / 2;
    int y = screenGeometry.y() + (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::updateCursor(const QPoint &pos)
{
    if (isMaximized())
        return;

    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    ResizeRegion region = None;
    if (x <= borderWidth && y <= borderWidth) region = TopLeft;
    else if (x >= w - borderWidth && y <= borderWidth) region = TopRight;
    else if (x <= borderWidth && y >= h - borderWidth) region = BottomLeft;
    else if (x >= w - borderWidth && y >= h - borderWidth) region = BottomRight;
    else if (x <= borderWidth) region = Left;
    else if (x >= w - borderWidth) region = Right;
    else if (y <= borderWidth) region = Top;
    else if (y >= h - borderWidth) region = Bottom;

    resizeRegion = region;

    switch (region) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    case Top:
    case Bottom:
        setCursor(Qt::SizeVerCursor); break;
    case Left:
    case Right:
        setCursor(Qt::SizeHorCursor); break;
    default:
        unsetCursor(); break;
    }
}

void BlurOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor overlayColor(50, 50, 50, 80);
    painter.setBrush(overlayColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

CustomWidget::CustomWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    overlay = new BlurOverlay(this);
    overlay->resize(size());
    overlay->hide();
}

void CustomWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (!overlay)
        return;

    if (blur)
        overlay->showOverlay();
    else
        overlay->hideOverlay();
}

void CustomWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (overlay)
        overlay->resize(size());
}


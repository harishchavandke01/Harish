#include "timebasedview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsTextItem>
#include <QScrollBar>
#include <QPainter>
#include <algorithm>

class HoverRectItem : public QGraphicsRectItem
{
public:
    HoverRectItem(const QRectF &rect, const QString &tooltip, const QBrush &brush): QGraphicsRectItem(rect)
    {
        setBrush(brush);
        setPen(Qt::NoPen);
        setAcceptHoverEvents(true);
        setToolTip(tooltip);
    }
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override{
        setCursor(Qt::PointingHandCursor);
        QGraphicsRectItem::hoverEnterEvent(event);
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override{
        unsetCursor();
        QGraphicsRectItem::hoverLeaveEvent(event);
    }
};

TimeBasedView::TimeBasedView(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(1000, 550);
    setObjectName("timeBasedView");

    icon = new QLabel();
    icon->setPixmap(QPixmap(":/images/images/surveypod.png"));
    icon->setFixedSize(20,20);
    icon->setScaledContents(true);

    title = new QLabel("Baseline Processing");
    title->setObjectName("guidetitle");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/cross.svg"));
    closeBtn->setIconSize(QSize(16,16));
    closeBtn->setFixedSize(22,22);
    closeBtn->setFlat(true);
    closeBtn->setAutoDefault(false);
    closeBtn->setObjectName("guideclosebtn");
    closeBtn->setStyleSheet("QPushButton { background-color:white; } QPushButton:hover { background-color: #dddddd; }");

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(5,0,5,0);
    hlayout->addWidget(icon);
    hlayout->addWidget(title);
    hlayout->addStretch();
    hlayout->addWidget(closeBtn);

    topBar = new QWidget();
    topBar->setLayout(hlayout);
    topBar->setObjectName("guideTitleBar");
    topBar->setStyleSheet("background-color: #404040; border: none;");

    heading = new QLabel("Time Based View");
    heading->setStyleSheet("font-size:16px; font-weight:bold;");

    leftScene = new QGraphicsScene(this);
    leftView  = new QGraphicsView(leftScene, this);
    leftView->setContentsMargins(0,0,0,5);
    leftView->setFixedWidth(180);
    leftView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftView->setFrameShape(QFrame::NoFrame);

    QString scrollStyle =
        "QScrollBar:vertical { width: 8px; background: transparent; }"
        "QScrollBar::handle:vertical { background: rgba(0,0,0,0.5); border-radius: 4px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(0,0,0,0.8); }"
        "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical { height: 0px; }"

        "QScrollBar:horizontal { height: 8px; background: transparent; }"
        "QScrollBar::handle:horizontal { background: rgba(0,0,0,0.5); border-radius: 4px; min-width: 20px; }"
        "QScrollBar::handle:horizontal:hover { background: rgba(0,0,0,0.8); }"
        "QScrollBar::sub-line:horizontal, QScrollBar::add-line:horizontal { width: 0px; }";

    scene = new QGraphicsScene(this);
    view  = new QGraphicsView(scene, this);
    view->setStyleSheet(scrollStyle);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setFrameShape(QFrame::NoFrame);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setCursor(Qt::OpenHandCursor);

    connect(view->verticalScrollBar(), &QScrollBar::valueChanged, leftView->verticalScrollBar(), &QScrollBar::setValue);
    connect(leftView->verticalScrollBar(), &QScrollBar::valueChanged,view->verticalScrollBar(), &QScrollBar::setValue);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(5,5,5,5);
    contentLayout->addWidget(leftView);
    contentLayout->addWidget(view);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(topBar,0,Qt::AlignTop);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(heading,0,Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(contentLayout);

    setLayout(mainLayout);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
}

void TimeBasedView::viewTimeMap(const QVector<StaticJob> &jobs, const QMap<QString, FileEntry> &files)
{
    if (jobs.isEmpty())
        return;
    scene->clear();
    leftScene->clear();
    QDateTime globalStart = jobs.first().baseStart;
    QDateTime globalEnd   = jobs.first().baseEnd;

    for (const StaticJob &job : jobs)
    {
        globalStart = std::min({globalStart, job.baseStart, job.roverStart});
        globalEnd   = std::max({globalEnd, job.baseEnd, job.roverEnd});
    }

    qint64 totalSeconds = globalStart.secsTo(globalEnd);
    if (totalSeconds <= 0)
        return;
    int availableWidth = view->viewport()->width() - 40;
    if (availableWidth < 600)
        availableWidth = 1000;

    double scale = (double)availableWidth / totalSeconds;
    auto timeToX = [&](const QDateTime &t) {
        return globalStart.secsTo(t) * scale;
    };
    const int IST_OFFSET = 19800;

    double barHeight = 14;
    int rowHeight = 2 * barHeight + 12;

    int targetLines = 8;
    qint64 rawStep = totalSeconds / targetLines;

    int interval;
    if (rawStep <= 300) interval = 300;
    else if (rawStep <= 900) interval = 900;
    else if (rawStep <= 1800) interval = 1800;
    else if (rawStep <= 3600) interval = 3600;
    else interval = 7200;

    int minLabelSpacing = 100;
    double lastLabelX = -1000;

    for (qint64 t = 0; t <= totalSeconds; t += interval)
    {
        double x = timeToX(globalStart.addSecs(t));
        scene->addLine(x, 0, x, jobs.size() * rowHeight, QPen(QColor(220,220,220)));

        if (x - lastLabelX >= minLabelSpacing)
        {
            QDateTime tick = globalStart.addSecs(t).addSecs(IST_OFFSET);
            QString textStr = tick.toString("dd-MM HH:mm");
            QGraphicsTextItem *text = scene->addText(textStr);
            QRectF rect = text->boundingRect();
            text->setPos(x - rect.width()/2, -25);
            lastLabelX = x;
        }
    }
    int row = 0;
    for (const StaticJob &job : jobs)
    {
        double y = row * rowHeight;

        QString baseName = files.contains(job.baseObs)
                ? files[job.baseObs].pointId
                : job.baseObs;

        QString roverName = files.contains(job.roverObs)
                ? files[job.roverObs].pointId
                : job.roverObs;

        QGraphicsTextItem *label =leftScene->addText(baseName + " → " + roverName);
        label->setPos(5, y + 4);

        QString baseTip =
            baseName + " ( " +
            job.baseStart.addSecs(IST_OFFSET).toString("HH:mm:ss") +
            " - " +
            job.baseEnd.addSecs(IST_OFFSET).toString("HH:mm:ss") +
            " )";

        HoverRectItem *baseRect =
            new HoverRectItem(
                QRectF(timeToX(job.baseStart),
                       y,
                       job.baseStart.secsTo(job.baseEnd) * scale,
                       barHeight),
                baseTip,
                QBrush(Qt::blue));
        scene->addItem(baseRect);

        QString roverTip =
            roverName + " ( " +
            job.roverStart.addSecs(IST_OFFSET).toString("HH:mm:ss") +
            " - " +
            job.roverEnd.addSecs(IST_OFFSET).toString("HH:mm:ss") +
            " )";

        HoverRectItem *roverRect =
            new HoverRectItem(
                QRectF(timeToX(job.roverStart),
                       y + barHeight,
                       job.roverStart.secsTo(job.roverEnd) * scale,
                       barHeight),
                roverTip,
                QBrush(Qt::green));
        scene->addItem(roverRect);

        row++;
    }

    double totalHeight = jobs.size() * rowHeight;
    double topMargin   = -40;
    double bottomMargin = 40;
    scene->setSceneRect(-20, topMargin, totalSeconds * scale + 40, totalHeight + bottomMargin);
    leftScene->setSceneRect(-10, topMargin, 180, totalHeight + bottomMargin);
}

#include "customprogressbar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "customloadingbar.h"

CustomProgressBar::CustomProgressBar(int processcount, QWidget *parent)
    : QDialog{parent}
{
    count = processcount;
    isclosed = false;
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background: #fafafa; border: 2px solid black;");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setModal(true);

    if(count == 0) {
        heading = new QLabel("Loading...");
        heading->setObjectName("pbheading");

        closeBtn = new QPushButton();
        closeBtn->setIcon(QIcon(":/images/images/close2.svg"));
        closeBtn->setIconSize(QSize(30, 30));
        closeBtn->setObjectName("pbclosebtn");

        currentprocess = new QLabel("Initializing...");
        currentprocess->setStyleSheet("border: none;");
        currentprocess->setObjectName("pbcurrentprocess");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 10, 20, 20);

        QHBoxLayout *hlayout = new QHBoxLayout();
        hlayout->setContentsMargins(0, 10, 0, 10);
        hlayout->addWidget(heading);
        hlayout->addStretch(1);
        hlayout->addWidget(closeBtn);
        QWidget *topBar = new QWidget();
        topBar->setMinimumWidth(300);
        topBar->setStyleSheet("border: none;");
        topBar->setLayout(hlayout);

        layout->addWidget(topBar);
        layout->setSpacing(0);
        layout->addSpacing(10);
        layout->addWidget(currentprocess, 0, Qt::AlignHCenter);
        layout->addSpacing(20);

        CustomLoadingBar *lb = new CustomLoadingBar();
        lb->setGeometry(96, 16, 64, 32);
        lb->setType(CustomLoadingBar::ball_rotate);
        lb->setMinimumSize(50, 50);
        lb->setColor(QColor("#0EA3A9"));
        lb->start();
        layout->addWidget(lb, 0, Qt::AlignHCenter);

        connect(closeBtn, &QPushButton::clicked, this, &CustomProgressBar::Close);
        return;
    }

    heading = new QLabel("Processing in Progress...");
    heading->setObjectName("pbheading");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/close2.svg"));
    closeBtn->setIconSize(QSize(30, 30));
    closeBtn->setObjectName("pbclosebtn");

    currentprocess = new QLabel("Initializing...");
    currentprocess->setStyleSheet("border: none;");
    currentprocess->setObjectName("pbcurrentprocess");

    info = new QLabel("Overall Progress : ");
    info->setStyleSheet("border: none;");
    info->setObjectName("pbinfo");
    currentProgress = new QProgressBar();
    currentProgress->setRange(0, 0);
    currentProgress->setTextVisible(false);
    currentProgress->setStyleSheet(R"(
        QProgressBar {
            min-width: 350px;
            max-height: 14px;
            border: 1px solid gray;
            text-align: center;
            background-color: #eeeeee;
        }

        QProgressBar::chunk {
            background-color: #0da5a3;
            width: 4px;
            height: 12px;
        }
    )");

    overallProgress = new QProgressBar();
    overallProgress->setRange(0, count);
    overallProgress->setValue(0);
    overallProgress->setStyleSheet(R"(
        QProgressBar {
            min-width: 350px;
            max-height: 14px;
            border: 1px solid gray;
            text-align: center;
            background-color: #eeeeee;
        }

        QProgressBar::chunk {
            background-color: #0da5a3;
            width: 4px;
            height: 12px;
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 10, 20, 10);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 10, 0, 10);
    hlayout->addWidget(heading);
    hlayout->addStretch(1);
    hlayout->addWidget(closeBtn);
    QWidget *topBar = new QWidget();
    topBar->setStyleSheet("border: none;");
    topBar->setLayout(hlayout);

    layout->addWidget(topBar);
    layout->setSpacing(0);
    layout->addSpacing(10);
    layout->addWidget(currentprocess);
    layout->addSpacing(10);
    layout->addWidget(currentProgress);
    layout->addSpacing(20);
    layout->addWidget(info);
    layout->addSpacing(10);
    layout->addWidget(overallProgress);
    layout->addSpacing(10);

    connect(closeBtn, &QPushButton::clicked, this, &CustomProgressBar::Close);
}

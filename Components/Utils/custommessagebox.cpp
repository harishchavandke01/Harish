#include "custommessagebox.h"
#include <QHBoxLayout>
#include <QScreen>
#include <QGuiApplication>

CustomMessageBox::CustomMessageBox(QString type, QString _message, QString returntype, QWidget *parent)
    : QDialog{parent}
{
    setAttribute(Qt::WA_StyledBackground, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setModal(true);
    setMinimumWidth(300);

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/close.svg"));
    closeBtn->setObjectName("closeBtn");
    closeBtn->setFixedSize(30, 30);

    title = new QLabel();
    title->setObjectName("mbTitle");

    message = new QLabel();
    message->setText(_message);
    message->setObjectName("mbMessage");

    icon = new QLabel();
    icon->setObjectName("mbicon");
    icon->setFixedSize(50, 50);
    icon->setScaledContents(true);

    if(type == "INFO") {
        title->setText("Info");
        QPixmap pixmap = QPixmap(":/images/images/info.svg");
        icon->setPixmap(pixmap.scaled(60, 60));
    } else if(type == "WARNING") {
        title->setText("Warning");
        QPixmap pixmap = QPixmap(":/images/images/warning.svg");
        icon->setPixmap(pixmap.scaled(60, 60));
    } else if(type == "ERROR") {
        title->setText("Error");
        QPixmap pixmap = QPixmap(":/images/images/error.svg");
        icon->setPixmap(pixmap.scaled(60, 60));
    } else if(type == "QUESTION") {
        title->setText("Error");
        QPixmap pixmap = QPixmap(":/images/images/question.svg");
        icon->setPixmap(pixmap.scaled(60, 60));
    }

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(5, 0, 0, 0);
    hlayout->addWidget(title);
    hlayout->addStretch(1);
    hlayout->addWidget(closeBtn);
    QWidget *topBar = new QWidget();
    topBar->setLayout(hlayout);
    topBar->setStyleSheet("background: #404040;");

    hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(20, 20, 20, 20);
    hlayout->addWidget(icon);
    hlayout->addSpacing(10);
    hlayout->addWidget(message);

    QWidget *middleBar = new QWidget();
    middleBar->setLayout(hlayout);
    middleBar->setStyleSheet("background: #E9E9E9;");

    QWidget *bottomBar = new QWidget();

    if(returntype == "YES_NO") {
        yesBtn = new QPushButton("Yes");
        yesBtn->setObjectName("yesbtn");
        yesBtn->setStyleSheet("background-color: black;");
        noBtn = new QPushButton("No");
        noBtn->setObjectName("nobtn");
        noBtn->setStyleSheet("background-color: black;");

        hlayout = new QHBoxLayout();
        hlayout->setContentsMargins(5, 10, 5, 10);
        hlayout->addStretch(1);
        hlayout->addWidget(yesBtn);
        hlayout->addSpacing(10);
        hlayout->addWidget(noBtn);
        hlayout->addSpacing(10);

        bottomBar->setLayout(hlayout);
        bottomBar->setStyleSheet("background: #E9E9E9;");

        connect(yesBtn, &QPushButton::clicked, this, &CustomMessageBox::Close);
        connect(noBtn, &QPushButton::clicked, this, &CustomMessageBox::Close);
    } else if(returntype == "OK") {
        okayBtn = new QPushButton("OK");
        okayBtn->setObjectName("okbtn");
        okayBtn->setStyleSheet("background-color: black;");

        hlayout = new QHBoxLayout();
        hlayout->setContentsMargins(5, 10, 5, 10);
        hlayout->addStretch(1);
        hlayout->addWidget(okayBtn);
        hlayout->addStretch(1);

        bottomBar->setLayout(hlayout);
        bottomBar->setStyleSheet("background: #E9E9E9;");

        connect(okayBtn, &QPushButton::clicked, this, &CustomMessageBox::Close);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(topBar);
    layout->setSpacing(0);
    layout->addWidget(middleBar);
    layout->setSpacing(0);
    layout->addWidget(bottomBar);

    connect(closeBtn, &QPushButton::clicked, this, &CustomMessageBox::Close);
}

void CustomMessageBox::Close() {
    QObject* senderObj = sender();
    QPushButton* button = qobject_cast<QPushButton*>(senderObj);
    if (button) {
        if (button == yesBtn) {
            response = true;
        } else if (button == noBtn) {
            response = false;
        }
    }
    this->close();
}



#include "titlebar.h"
#include "../../mainwindow.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPixmap>
#include <QIcon>


TitleBar::TitleBar(MainWindow *window, QWidget *parent) : QWidget(parent), m_window(window)
{
    setObjectName("titleBar");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(30);
    setupTitleBar();
}

void TitleBar::setupTitleBar()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(4, 0, 0, 0);
    layout->setSpacing(8);

    logoLabel = new QLabel(this);
    QPixmap pix(":/images/images/logo.png");

    int logoHeight = 35;
    pix = pix.scaledToHeight(logoHeight, Qt::SmoothTransformation);

    logoLabel->setPixmap(pix);
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    minimizeButton = new QPushButton(this);
    minimizeButton->setObjectName("minimizeButton");
    minimizeButton->setFixedSize(40, 32);
    minimizeButton->setIcon(QIcon(":/images/images/minimize.svg"));
    minimizeButton->setFlat(true);

    maximizeButton = new QPushButton(this);
    maximizeButton->setObjectName("maximizeButton");
    maximizeButton->setFixedSize(40, 32);
    maximizeButton->setIcon(QIcon(":/images/images/maximize.svg"));
    maximizeButton->setFlat(true);

    closeButton = new QPushButton(this);
    closeButton->setObjectName("closeButton");
    closeButton->setFixedSize(40, 32);
    closeButton->setIcon(QIcon(":/images/images/close.svg"));
    closeButton->setFlat(true);

    layout->addWidget(logoLabel);
    layout->addSpacing(10);
    layout->addStretch();
    layout->addWidget(minimizeButton);
    layout->addWidget(maximizeButton);
    layout->addWidget(closeButton);

    setLayout(layout);

    connect(minimizeButton, &QPushButton::clicked, this, &TitleBar::Minimize);
    connect(maximizeButton, &QPushButton::clicked, this, &TitleBar::Maximize);
    connect(closeButton,   &QPushButton::clicked, this, &TitleBar::Close);
}

void TitleBar::Minimize() {
    if (m_window)
        m_window->showMinimized();
}

void TitleBar::Maximize() {
    if (!m_window) return;

    if (m_window->isMaximized()) {
        maximizeButton->setIcon(QIcon(":/images/images/maximize.svg"));
        m_window->showNormal();
    } else {
        maximizeButton->setIcon(QIcon(":/images/images/maximize_2.svg"));
        m_window->showMaximized();
    }
}

void TitleBar::Close() {
    if (m_window) m_window->Close();
}

void TitleBar::mousePressEvent(QMouseEvent *event) {
    if (!m_window || m_window->isMaximized()) return;
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPosition().toPoint() - m_window->frameGeometry().topLeft();
        event->accept();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (!m_window || m_window->isMaximized()) return;
    if (event->buttons() & Qt::LeftButton) {
        m_window->move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

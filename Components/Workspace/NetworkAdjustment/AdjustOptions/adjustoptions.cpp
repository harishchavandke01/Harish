#include "adjustoptions.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

AdjustOptions::AdjustOptions(AdjustmentOptions &opts, QWidget *parent)
    : QDialog(parent)
    , options(opts)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(360, 200);
    setObjectName("editOptions");

    // ── Title bar ────────────────────────────────────────────────
    icon = new QLabel();
    icon->setPixmap(QPixmap(":/images/images/surveypod.png"));
    icon->setFixedSize(20, 20);
    icon->setScaledContents(true);

    title = new QLabel("Network Adjustment");
    title->setObjectName("guidetitle");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/cross.svg"));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setFixedSize(22, 22);
    closeBtn->setFlat(true);
    closeBtn->setAutoDefault(false);
    closeBtn->setObjectName("guideclosebtn");
    closeBtn->setStyleSheet(
        "QPushButton { background-color:white; }"
        "QPushButton:hover { background-color:#dddddd; }");
    connect(closeBtn, &QPushButton::clicked, this, &AdjustOptions::onClose);

    QHBoxLayout *tbl = new QHBoxLayout();
    tbl->setContentsMargins(5, 0, 5, 0);
    tbl->addWidget(icon);
    tbl->addWidget(title);
    tbl->addStretch();
    tbl->addWidget(closeBtn);

    topBar = new QWidget();
    topBar->setLayout(tbl);
    topBar->setObjectName("guideTitleBar");
    topBar->setStyleSheet("background-color:#404040; border:none;");

    // ── Content ──────────────────────────────────────────────────
    heading = new QLabel("Options");
    heading->setStyleSheet("font-size:16px; font-weight:bold; border:none;");

    QLabel *modeNote = new QLabel(
        "Adjustment mode (constrained / free) is determined automatically "
        "by whether any control points are selected.");
    modeNote->setWordWrap(true);
    modeNote->setStyleSheet("font-size:11px; color:#666; border:none;");

    useConv = new CustomCheckBox("Use baseline covariance matrix", options.useCovariance);

    save = new QPushButton("Save");
    save->setObjectName("editOpBtns");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet(
        "QPushButton { background-color:#00b894; }"
        "QPushButton:hover { background-color:#00d2a8; }");
    connect(save, &QPushButton::clicked, this, &AdjustOptions::onSave);

    QVBoxLayout *contentLay = new QVBoxLayout();
    contentLay->setContentsMargins(16, 12, 16, 12);
    contentLay->setSpacing(10);
    contentLay->addWidget(heading, 0, Qt::AlignHCenter);
    contentLay->addWidget(modeNote);
    contentLay->addWidget(useConv);
    contentLay->addStretch();
    contentLay->addWidget(save, 0, Qt::AlignRight);

    QWidget *contentWidget = new QWidget();
    contentWidget->setLayout(contentLay);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->addWidget(topBar, 0, Qt::AlignTop);
    mainLay->addWidget(contentWidget, 1);
    setLayout(mainLay);
}

void AdjustOptions::onClose()  { close(); }
void AdjustOptions::onSave()
{
    options.useCovariance = useConv->isChecked();
    accept();
}

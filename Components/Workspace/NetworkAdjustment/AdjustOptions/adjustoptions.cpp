#include "adjustoptions.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
AdjustOptions::AdjustOptions(AdjustmentOptions &opts,QWidget *parent) : QDialog(parent), options(opts)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(350, 250);
    setObjectName("editOptions");

    icon = new QLabel();
    icon->setPixmap(QPixmap(":/images/images/surveypod.png"));
    icon->setFixedSize(20,20);
    icon->setScaledContents(true);

    title = new QLabel("Network adjustment");
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

    connect(closeBtn, &QPushButton::clicked, this, &AdjustOptions::onClose);

    heading = new QLabel("Adjustment Options");
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");

    QGroupBox *typeBox = new QGroupBox("Adjustment type");
    QVBoxLayout *typeLayout = new QVBoxLayout(typeBox);

    constrained = new QRadioButton("Constrained adjustment");
    free = new QRadioButton("Free network (advanced)");
    free->setEnabled(false);

    constrained->setChecked(options.constrained);
    free->setChecked(!options.constrained);

    typeLayout->addWidget(constrained);
    typeLayout->addWidget(free);
    free->setChecked(!options.constrained);

    useConv = new CustomCheckBox("Use baseline covariance",options.useCovariance);

    save = new QPushButton("Save");
    save->setObjectName("editOpBtns");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");

    connect(save, &QPushButton::clicked, this, &AdjustOptions::onSave);

    QWidget * cwid = new QWidget();
    QVBoxLayout *clay = new QVBoxLayout(cwid);
    clay->addWidget(typeBox);
    clay->addWidget(useConv);
    clay->addWidget(save,0,Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(topBar,0, Qt::AlignTop);
    mainLayout->addWidget(heading,0,Qt::AlignHCenter);
    mainLayout->addWidget(cwid);
    setLayout(mainLayout);
}

void AdjustOptions::onClose()
{
    this->close();
}

void AdjustOptions::onSave()
{
    options.constrained = constrained->isChecked();
    options.useCovariance = useConv->isChecked();
    accept();
}

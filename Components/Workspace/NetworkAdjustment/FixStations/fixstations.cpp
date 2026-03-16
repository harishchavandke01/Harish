#include "fixstations.h"
#include <QVBoxLayout>
#include <QHeaderView>
FixStations::FixStations(ProjectContext *_projectContext, QWidget *parent)
    : QDialog{parent}, projectContext(_projectContext)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(600, 450);
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

    connect(closeBtn, &QPushButton::clicked, this, &FixStations::Close);

    heading = new QLabel("Fix stations");
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");


    table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({
        "Station", "Fixed", "X", "Y", "Z"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    saveBtn = new QPushButton("Save");
    saveBtn->setObjectName("editOpBtns");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");

    resetBtn = new QPushButton("Reset");
    resetBtn->setObjectName("editOpBtns");
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");

    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(resetBtn);

    QWidget * contentsw = new QWidget();
    QVBoxLayout *vlay = new QVBoxLayout(contentsw);
    vlay->addWidget(heading, 0, Qt::AlignHCenter);
    vlay->addWidget(table);
    vlay->addLayout(btnLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(topBar,0,Qt::AlignTop);
    mainLayout->addWidget(contentsw);
    setLayout(mainLayout);

    populateTable();

    connect(saveBtn, &QPushButton::clicked, this, &FixStations::onSave);
    connect(resetBtn, &QPushButton::clicked, this, &FixStations::onReset);
}

void FixStations::populateTable()
{
    if (!projectContext)
        return;

    table->clearContents();
    table->setRowCount(projectContext->stations.size());

    int row = 0;
    for (auto it = projectContext->stations.begin();
         it != projectContext->stations.end(); ++it, ++row)
    {
        const QString &key = it.key();
        qDebug()<<"key = "<<key;
        const ProjectStation &st = it.value();

        // Station name
        table->setItem(row, 0, new QTableWidgetItem(key));

        // Fixed checkbox
        QTableWidgetItem *fixItem = new QTableWidgetItem();
        fixItem->setCheckState(st.isFixed ? Qt::Checked : Qt::Unchecked);
        fixItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        table->setItem(row, 1, fixItem);

        // X Y Z (read-only)
        table->setItem(row, 2, new QTableWidgetItem(QString::number(st.ecef.X, 'f', 5)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(st.ecef.Y, 'f', 5)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(st.ecef.Z, 'f', 5)));
    }
}


void FixStations::onSave()
{
    for (int row = 0; row < table->rowCount(); ++row) {
        QString stationKey = table->item(row, 0)->text();
        bool fixed = (table->item(row, 1)->checkState() == Qt::Checked);

        if (projectContext->stations.contains(stationKey)) {
            projectContext->stations[stationKey].isFixed = fixed;
        }
    }
    accept();
}

void FixStations::onReset()
{
    for (int row = 0; row < table->rowCount(); ++row) {
        table->item(row, 1)->setCheckState(Qt::Unchecked);

        QString key = table->item(row, 0)->text();
        if (projectContext->stations.contains(key))
            projectContext->stations[key].isFixed = false;
    }
}


void FixStations::Close()
{
    this->close();
}

#include "fixstations.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <cmath>

FixStations::FixStations(ProjectContext *_projectContext, QWidget *parent)
    : QDialog(parent)
    , projectContext(_projectContext)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(640, 480);
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

    connect(closeBtn, &QPushButton::clicked, this, &FixStations::onClose);

    // ── Content ──────────────────────────────────────────────────
    heading = new QLabel("Control Points");
    heading->setStyleSheet("font-size:16px; font-weight:bold; border:none;");

    infoLabel = new QLabel(
        "Check a station to fix it as a control point.\n"
        "If no stations are checked, a Free Network adjustment will run.");
    infoLabel->setStyleSheet("font-size:12px; color:#666; border:none;");
    infoLabel->setWordWrap(true);

    // ── Table ────────────────────────────────────────────────────
    table = new QTableWidget();
    table->setColumnCount(COL_COUNT);
    table->setHorizontalHeaderLabels({
        "Station ID", "Fixed", "Easting (m)", "Northing (m)", "Height (m)"
    });
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    // ── Buttons ──────────────────────────────────────────────────
    saveBtn = new QPushButton("Save");
    saveBtn->setObjectName("editOpBtns");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton { background-color:#00b894; }"
        "QPushButton:hover { background-color:#00d2a8; }");

    resetBtn = new QPushButton("Clear All");
    resetBtn->setObjectName("editOpBtns");
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet(
        "QPushButton { background-color:#2d3436; }"
        "QPushButton:hover { background-color:#636e72; }");

    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->addStretch();
    btnLay->addWidget(resetBtn);
    btnLay->addWidget(saveBtn);

    QVBoxLayout *contentLay = new QVBoxLayout();
    contentLay->setContentsMargins(12, 12, 12, 12);
    contentLay->setSpacing(8);
    contentLay->addWidget(heading, 0, Qt::AlignHCenter);
    contentLay->addWidget(infoLabel);
    contentLay->addWidget(table, 1);
    contentLay->addLayout(btnLay);

    QWidget *contentWidget = new QWidget();
    contentWidget->setLayout(contentLay);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->addWidget(topBar, 0, Qt::AlignTop);
    mainLay->addWidget(contentWidget, 1);
    setLayout(mainLay);

    connect(saveBtn,  &QPushButton::clicked, this, &FixStations::onSave);
    connect(resetBtn, &QPushButton::clicked, this, &FixStations::onReset);

    populateTable();
}

void FixStations::populateTable()
{
    if (!projectContext) return;

    table->clearContents();
    table->setRowCount(projectContext->stations.size());

    int row = 0;
    for (auto it = projectContext->stations.constBegin();
         it != projectContext->stations.constEnd(); ++it, ++row)
    {
        const QString       &key = it.key();
        const ProjectStation &st  = it.value();

        // Col 0: Station ID (human-readable), key stored as UserRole
        QString displayName = st.stationId.isEmpty() ? key : st.stationId;
        auto *nameItem = new QTableWidgetItem(displayName);
        nameItem->setData(Qt::UserRole, key);    // store internal key for save
        nameItem->setToolTip(key);               // show full path on hover
        table->setItem(row, COL_STATION, nameItem);

        // Col 1: Fixed checkbox
        auto *fixItem = new QTableWidgetItem();
        fixItem->setCheckState(st.isFixed ? Qt::Checked : Qt::Unchecked);
        fixItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        fixItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, COL_FIXED, fixItem);

        // Col 2-4: Easting / Northing / Orthometric height (more readable than ECEF)
        auto fmt = [](double v) -> QString {
            return std::isfinite(v) ? QString::number(v, 'f', 3) : "—";
        };
        table->setItem(row, COL_EAST,   new QTableWidgetItem(fmt(st.easting)));
        table->setItem(row, COL_NORTH,  new QTableWidgetItem(fmt(st.northing)));
        table->setItem(row, COL_HEIGHT, new QTableWidgetItem(fmt(st.orthometric)));

        // Center-align numeric columns
        for (int c : {COL_EAST, COL_NORTH, COL_HEIGHT})
            table->item(row, c)->setTextAlignment(Qt::AlignCenter);
    }
}

void FixStations::onSave()
{
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem *nameItem  = table->item(row, COL_STATION);
        QTableWidgetItem *fixItem   = table->item(row, COL_FIXED);
        if (!nameItem || !fixItem) continue;

        QString key   = nameItem->data(Qt::UserRole).toString();
        bool    fixed = (fixItem->checkState() == Qt::Checked);

        if (projectContext->stations.contains(key))
            projectContext->stations[key].isFixed = fixed;
    }
    accept();
}

void FixStations::onReset()
{
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem *fixItem = table->item(row, COL_FIXED);
        if (fixItem) fixItem->setCheckState(Qt::Unchecked);
    }
}

void FixStations::onClose()
{
    close();
}

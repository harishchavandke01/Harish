#include "fixstations.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <cmath>
#include "../../../Utils/customcheckbox.h"

FixStations::FixStations(ProjectContext *_projectContext, QWidget *parent)
    : QDialog(parent)
    , projectContext(_projectContext)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(640, 480);
    setObjectName("editOptions");

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

    heading = new QLabel("Control Points");
    heading->setStyleSheet("font-size:16px; font-weight:bold; border:none;");

    infoLabel = new QLabel(
        "Check a station to fix it as a control point.\n"
        "If no stations are checked, a Free Network adjustment will run.");
    infoLabel->setStyleSheet("font-size:10px; color:#666; border:none;");
    infoLabel->setWordWrap(true);

    table = new QTableWidget();
    table->setColumnCount(COL_COUNT);
    table->setHorizontalHeaderLabels({
        "Station ID", "Fixed", "Easting (m)", "Northing (m)", "Height (m)"
    });
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);

    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(R"(
        QTableWidget::item:selected {
            background-color: #00d2a8;;
            color: white;
        }
        QTableWidget::item:selected:!active {
            background-color: #00b894;;
        }
        QTableWidget QScrollBar {
            background: transparent;
            border: none;
        }
        QTableWidget QScrollBar:vertical {
            width: 6px;
            margin: 2px 0 2px 0;
        }

        QTableWidget QScrollBar::handle:vertical {
            background: #2c2c2c;
            min-height: 25px;
            border-radius: 3px;
        }

        QTableWidget QScrollBar::handle:vertical:hover {
            background: #000000;
        }

        QTableWidget QScrollBar::add-line:vertical,
        QTableWidget QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QTableWidget QScrollBar::add-page:vertical,
        QTableWidget QScrollBar::sub-page:vertical {
            background: transparent;
        }

        QTableWidget QScrollBar:horizontal {
            height: 6px;
            margin: 0 2px 0 2px;
        }

        QTableWidget QScrollBar::handle:horizontal {
            background: #2c2c2c;
            min-width: 25px;
            border-radius: 3px;
        }

        QTableWidget QScrollBar::handle:horizontal:hover {
            background: #000000;
        }

        QTableWidget QScrollBar::add-line:horizontal,
        QTableWidget QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        QTableWidget QScrollBar::add-page:horizontal,
        QTableWidget QScrollBar::sub-page:horizontal {
            background: transparent;
        }
    )");

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
    QMap<QString, QString> uniqueStations;
    for (auto it = projectContext->stations.constBegin(); it != projectContext->stations.constEnd(); ++it) {
        const QString &sid = it.value().stationId;
        if (!uniqueStations.contains(sid)) uniqueStations[sid] = it.key();
    }

    table->setRowCount(uniqueStations.size());
    int row = 0;
    for (auto it = uniqueStations.constBegin(); it != uniqueStations.constEnd(); ++it, ++row)
    {
        QString stationId = it.key();
        QString uid = it.value();
        const ProjectStation &st = projectContext->stations[uid];

        auto *nameItem = new QTableWidgetItem(stationId);
        nameItem->setData(Qt::UserRole, stationId);
        table->setItem(row, COL_STATION, nameItem);

        CustomCheckBox *cb = new CustomCheckBox("", true);
        bool isFixed = false;
        for (auto &s : projectContext->stations) {
            if (s.stationId == stationId && s.isFixed) {
                isFixed = true;
                break;
            }
        }
        cb->setVal(isFixed);

        QWidget *cellWidget = new QWidget(table);
        QHBoxLayout *lay = new QHBoxLayout(cellWidget);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setAlignment(Qt::AlignCenter);
        lay->addWidget(cb);
        table->setCellWidget(row, COL_FIXED, cellWidget);

        auto fmt = [](double v) -> QString {
            return std::isfinite(v) ? QString::number(v, 'f', 3) : "—";
        };

        table->setItem(row, COL_EAST,   new QTableWidgetItem(fmt(st.easting)));
        table->setItem(row, COL_NORTH,  new QTableWidgetItem(fmt(st.northing)));
        table->setItem(row, COL_HEIGHT, new QTableWidgetItem(fmt(st.orthometric)));

        for (int c : {COL_EAST, COL_NORTH, COL_HEIGHT})
            table->item(row, c)->setTextAlignment(Qt::AlignCenter);
    }
}

void FixStations::onSave()
{
    for (int row = 0; row < table->rowCount(); ++row) {

        QTableWidgetItem *nameItem = table->item(row, COL_STATION);
        if (!nameItem) continue;

        QString stationId = nameItem->data(Qt::UserRole).toString();

        QWidget *cell = table->cellWidget(row, COL_FIXED);
        CustomCheckBox *cb = cell->findChild<CustomCheckBox *>();
        bool fixed = cb && cb->isChecked();
        for (auto it = projectContext->stations.begin(); it != projectContext->stations.end(); ++it) {
            if (it.value().stationId == stationId) {
                it.value().isFixed = fixed;
            }
        }
    }
    accept();
}

void FixStations::onReset()
{
    for (int row = 0; row < table->rowCount(); ++row) {
        QWidget *cell = table->cellWidget(row, COL_FIXED);
        if (!cell) continue;

        CustomCheckBox *cb = cell->findChild<CustomCheckBox *>();
        if (cb)
        {
            cb->setVal(false);
            cb->update();
        }
    }
}

void FixStations::onClose()
{
    close();
}

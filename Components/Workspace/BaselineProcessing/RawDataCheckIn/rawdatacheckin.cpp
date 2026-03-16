#include "rawdatacheckin.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>
#include "../StationReferenceSelection/stationreferenceselection.h"
#include "../../../Utils/custommessagebox.h"
#include "../../../Utils/customcheckbox.h"

RawDataCheckIn::RawDataCheckIn(const QMap<QString, FileEntry> &_files,QWidget *parent) : files(_files), QDialog{parent}
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(850, 450);
    setObjectName("rawDataCheckIn");

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

    heading = new QLabel("Raw Data Check In");
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");

    table = new QTableWidget();

    saveBtn = new QPushButton("Save");
    saveBtn->setObjectName("rawDataCheckInBtns");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");

    resetBtn = new QPushButton("Reset");
    resetBtn->setObjectName("rawDataCheckInBtns");
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");

    cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("rawDataCheckInBtns");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");

    setUpTable();
    populateTable();

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(resetBtn);
    btnLayout->addWidget(cancelBtn);

    QWidget * contentWidget = new QWidget();
    QVBoxLayout * lay = new QVBoxLayout(contentWidget);
    lay->setContentsMargins(15,10,15,10);
    lay->addWidget(table);
    lay->addLayout(btnLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(topBar,0, Qt::AlignTop);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(heading,0, Qt::AlignHCenter);
    mainLayout->addWidget(contentWidget);
    mainLayout->addStretch();
    setLayout(mainLayout);

    connect(closeBtn, &QPushButton::clicked, this, [this](){
        reject();
    });

    connect(cancelBtn, &QPushButton::clicked, this, [this](){
        reject();
    });

    connect(resetBtn, &QPushButton::clicked, this, &RawDataCheckIn::onResetClicked);
    connect(saveBtn, &QPushButton::clicked, this, &RawDataCheckIn::onSaveClicked);
}

void RawDataCheckIn::setUpTable()
{
    table->setColumnCount(COL_COUNT);
    table->setHorizontalHeaderLabels({
        "Import",
        "Point ID",
        "File Name",
        "Start Time",
        "End Time",
        "Duration",
        "Antenna Type",
        "Receiver Type"
    });

    table->setMinimumHeight(300);
    table->setMinimumWidth(820);
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

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSortingEnabled(true);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QHeaderView *h = table->horizontalHeader();
    h->setStretchLastSection(true);
}
static QString computeDuration(const QString &start, const QString &end)
{
    QString s1 = start.left(26);
    QString s2 = end.left(26);

    static const QRegularExpression rx(R"((\d{2}:\d{2}):(\d)(\.))");

    s1.replace(rx, R"(\1:0\2\3)");
    s2.replace(rx, R"(\1:0\2\3)");

    int dot = s1.indexOf('.');
    if (dot != -1) s1 = s1.left(dot + 4);

    dot = s2.indexOf('.');
    if (dot != -1) s2 = s2.left(dot + 4);

    QDateTime t1 = QDateTime::fromString(s1, "yyyy-MM-dd HH:mm:ss.zzz");
    QDateTime t2 = QDateTime::fromString(s2, "yyyy-MM-dd HH:mm:ss.zzz");

    if (!t1.isValid() || !t2.isValid())
        return "Invalid";

    qint64 secs = t1.secsTo(t2);
    if (secs < 0)
        return "Invalid";

    int h = secs / 3600;
    int m = (secs % 3600) / 60;
    int s = secs % 60;

    return QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void RawDataCheckIn::populateTable()
{
    table->setRowCount(files.size());
    int row = 0;
    for(auto it  = files.begin(); it!= files.end(); it++){
        const QString &obsPath = it.key();
        QJsonObject header = utils->readRinexHeader(obsPath);
        QString pointId = files[obsPath].pointId;
        QString fileName = QFileInfo(obsPath).fileName();
        QString startTime = header["time_first_obs"].toString();
        QString endTime = header["time_last_obs"].toString();
        QString duration = computeDuration(startTime, endTime);
        QString antType = header["antenna_type"].toString();
        QString recType = header["receiver_type"].toString();
        QJsonObject posxyz = header["posxyz"].toObject();
        double X = posxyz["X"].toDouble();
        double Y = posxyz["Y"].toDouble();
        double Z = posxyz["Z"].toDouble();
        CustomCheckBox *cb = new CustomCheckBox("", true);

        QWidget *cellWidget = new QWidget(table);
        QHBoxLayout *lay = new QHBoxLayout(cellWidget);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setAlignment(Qt::AlignCenter);
        lay->addWidget(cb);

        table->setCellWidget(row, COL_IMPORT, cellWidget);
        QTableWidgetItem *pidItem = new QTableWidgetItem(pointId);
        pidItem->setData(Qt::UserRole, obsPath);
        pidItem->setData(Qt::UserRole + 1, X);
        pidItem->setData(Qt::UserRole + 2, Y);
        pidItem->setData(Qt::UserRole + 3, Z);

        table->setItem(row, COL_POINT_ID, pidItem);

        table->setItem(row, COL_FILE, new QTableWidgetItem(fileName));
        table->setItem(row, COL_START, new QTableWidgetItem(startTime));
        table->setItem(row, COL_END, new QTableWidgetItem(endTime));
        table->setItem(row, COL_DURATION, new QTableWidgetItem(duration));
        table->setItem(row, COL_ANTENNA, new QTableWidgetItem(antType));
        table->setItem(row, COL_RECEIVER, new QTableWidgetItem(recType));
        row++;
    }
}

void RawDataCheckIn::clearTable()
{
    table->clearContents();
    table->setRowCount(0);
}

void RawDataCheckIn::onSaveClicked()
{
    int noPointId = 0;
    for (int row = 0; row < table->rowCount(); ++row) {
        CustomCheckBox *cb = nullptr;
        QWidget *cell = table->cellWidget(row, COL_IMPORT);
        if (!cell)
            continue;

        if (!cb)
            cb = cell->findChild<CustomCheckBox *>();

        if (!cb || !cb->isChecked())
            continue;

        if(table->item(row, COL_POINT_ID)->text().isEmpty()){
            noPointId++;
        }
    }
    if(noPointId>0){
        CustomMessageBox *mb = new CustomMessageBox("ERROR",QString("Point ID cannot be empty\n%1 files found no Point ID").arg(noPointId),"OK", this);
        mb->exec();
        return;
    }
    QVector<StationRealization> selected;
    for (int row = 0; row < table->rowCount(); ++row) {

        QWidget *cell = table->cellWidget(row, COL_IMPORT);
        if (!cell)
            continue;

        CustomCheckBox *cb = cell->findChild<CustomCheckBox *>();
        if (!cb || !cb->isChecked())
            continue;

        QTableWidgetItem *pidItem = table->item(row, COL_POINT_ID);
        if (!pidItem)
            continue;

        StationRealization s;
        s.row = row;
        s.pointId = pidItem->text().trimmed();
        s.obsPath = pidItem->data(Qt::UserRole).toString();
        s.X = pidItem->data(Qt::UserRole + 1).toDouble();
        s.Y = pidItem->data(Qt::UserRole + 2).toDouble();
        s.Z = pidItem->data(Qt::UserRole + 3).toDouble();
        selected.push_back(s);
    }
    QMap<QString, QVector<StationRealization>> groups;
    for (const StationRealization &s : selected) {
        groups[s.pointId].push_back(s);
    }
    const double H_TOL = 30.0;
    const double V_TOL = 45.0;

    for (auto it = groups.begin(); it != groups.end(); ++it) {

        const QString &pointId = it.key();
        const QVector<StationRealization> &grp = it.value();
        if (grp.size() <= 1)
            continue;

        double Hmax = 0.0;
        double Vmax = 0.0;

        for (int i = 0; i < grp.size(); ++i) {
            for (int j = i + 1; j < grp.size(); ++j) {
                double dX = grp[i].X - grp[j].X;
                double dY = grp[i].Y - grp[j].Y;
                double dZ = std::abs(grp[i].Z - grp[j].Z);
                double H = std::sqrt(dX * dX + dY * dY);
                Hmax = std::max(Hmax, H);
                Vmax = std::max(Vmax, dZ);
            }
        }
        if (Hmax > H_TOL || Vmax > V_TOL) {
            CustomMessageBox mb("WARNING",QString("Point ID '%1' appears multiple times but coordinates are farther apart than allowed tolerance.\nPlease rename the Point IDs before continuing.").arg(pointId),"OK",this);
            mb.exec();
            return;
        }

        if (!pointIdToReferenceObs.contains(pointId)) {
            CustomMessageBox *mb = new CustomMessageBox("INFO",QString( "Multiple files exist for Point Id '%1'.\nYou must select a reference coordinate or rename the pointId,\nDo you want to select reference ?").arg(pointId),"YES_NO",this);
            mb->exec();
            if (!mb->response)
                return;

            StationReferenceSelection srs(pointId, grp, this);
            if (srs.exec() != QDialog::Accepted)
                return;
            QString refObs = srs.selectedObsPath();
            if (refObs.isEmpty())
                return;
            pointIdToReferenceObs[pointId] = refObs;
        }
    }
    mergedCoordinates.clear();
    for (int row = 0; row < table->rowCount(); ++row) {
        QWidget *cell = table->cellWidget(row, COL_IMPORT);
        if (!cell) continue;
        CustomCheckBox *cb = cell->findChild<CustomCheckBox *>();
        if (!cb || !cb->isChecked()) continue;
        QTableWidgetItem *pidItem = table->item(row, COL_POINT_ID);
        if (!pidItem) continue;
        QString key = pidItem->data(Qt::UserRole).toString();
        QString pointId = pidItem->text().trimmed();

        QJsonObject updates;
        updates["marker_name"] = pointId;
        updates["antenna_type"] = table->item(row, COL_ANTENNA)->text().trimmed();
        updates["receiver_type"] = table->item(row, COL_RECEIVER)->text().trimmed();

        if (pointIdToReferenceObs.contains(pointId)) {
            QString refObsPath = pointIdToReferenceObs[pointId];
            double finalX, finalY, finalZ;
            if (mergedCoordinates.contains(pointId)) {
                Coords c = mergedCoordinates[pointId];
                finalX = c.x; finalY = c.y; finalZ = c.z;
            } else {
                QJsonObject refHeader = utils->readRinexHeader(refObsPath);
                QJsonObject refPos = refHeader["posxyz"].toObject();
                finalX = refPos["X"].toDouble();
                finalY = refPos["Y"].toDouble();
                finalZ = refPos["Z"].toDouble();
                mergedCoordinates[pointId] = {finalX, finalY, finalZ};
            }
            QJsonObject finalPosObj;
            finalPosObj["X"] = finalX;
            finalPosObj["Y"] = finalY;
            finalPosObj["Z"] = finalZ;
            updates["finalposxyz"] = finalPosObj;
        }
        utils->updateRinexHeader(key, updates);
    }
    accept();
}

void RawDataCheckIn::onResetClicked()
{
    table->setSortingEnabled(false);
    clearTable();
    table->setRowCount(0);
    populateTable();
    table->setSortingEnabled(true);
}

QMap<QString, FileEntry> RawDataCheckIn::getSelectedFiles() const
{
    QMap<QString, FileEntry> result;
    for (int row = 0; row < table->rowCount(); ++row) {

        QWidget *cell = table->cellWidget(row, COL_IMPORT);
        if (!cell) continue;

        CustomCheckBox *cb = cell->findChild<CustomCheckBox *>();
        if (!cb || !cb->isChecked()) continue;

        QTableWidgetItem *pidItem = table->item(row, COL_POINT_ID);
        if (!pidItem) continue;

        QString key = pidItem->data(Qt::UserRole).toString();
        QString newId = pidItem->text().trimmed();

        if (!files.contains(key)) continue;
        FileEntry f = files.value(key);
        f.pointId = newId;

        if (mergedCoordinates.contains(newId)) {
            Coords c = mergedCoordinates[newId];
            f.refX = c.x;
            f.refY = c.y;
            f.refZ = c.z;
            f.hasReference = true;
        } else {
            f.hasReference = false;
            f.refX = 0; f.refY = 0; f.refZ = 0;
        }
        result.insert(key, f);
    }
    return result;
}

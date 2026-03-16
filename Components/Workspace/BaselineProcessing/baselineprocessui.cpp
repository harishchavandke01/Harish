#include "baselineprocessui.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include "../../../Utils/custommessagebox.h"
#include "../../../Utils/ProcessUtils/processutils.h"
#include "GenerateReport/generatereport.h"
#include "TimeBasedView/timebasedview.h"
#include "EditOptions/editoptions.h"
#include "GenerateReport/generateloopclosurereport.h"
#include "../../Utils/LoopClosureUtils/loopclosureutils.h"

BaselineProcessUI::BaselineProcessUI(ProjectContext * _projectContext, QMainWindow *parent): QMainWindow{parent}, projectContext(_projectContext)
{
    leftWidget = new QWidget();
    leftWidget->setObjectName("BaselineLeftWidget");
    QVBoxLayout * llay = new QVBoxLayout(leftWidget);
    llay->setSpacing(8);
    // llay->setAlignment(Qt::AlignCenter);

    heading = new QLabel("Baseline Process");
    heading->setObjectName("baselineHeading");

    obsFilesPickBtn = new QPushButton("Select files");
    obsFilesPickBtn->setObjectName("obsFilesPickBtn");

    settingBtn = new QPushButton();
    settingBtn->setIcon(QIcon(":/images/images/optionw.svg"));
    settingBtn->setObjectName("settingBtn");

    processBtn = new QPushButton("Process");
    processBtn->setObjectName("processBtn");

    QWidget *hprocessw = new QWidget();
    QHBoxLayout *hprocessl = new QHBoxLayout(hprocessw);
    hprocessl->setContentsMargins(0,0,0,0);
    hprocessl->addWidget(settingBtn);
    hprocessl->addWidget(processBtn);

    reportBtn = new QPushButton("Report");
    reportBtn->setObjectName("reportBtn");

    googleEarth = new QPushButton("Google Earth");
    googleEarth->setObjectName("reportBtn");

    timeView = new QPushButton("Time Based View");
    timeView->setObjectName("reportBtn");

    loopClosureReport = new QPushButton("Loop Closure");
    loopClosureReport->setObjectName("reportBtn");

    llay->addWidget(heading);
    llay->addSpacing(15);
    llay->addWidget(obsFilesPickBtn);
    llay->addWidget(hprocessw);
    llay->addWidget(reportBtn);
    llay->addWidget(googleEarth);
    llay->addWidget(timeView);
    llay->addSpacing(20);
    llay->addWidget(loopClosureReport);
    llay->addStretch();

    chart = new QChart();
    chartView = new ChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(700, 500);
    chart->createDefaultAxes();
    chart->zoomReset();
    chart->zoom(0.9);

    const auto hAxes = chart->axes(Qt::Horizontal);
    if (!hAxes.isEmpty()) {
        hAxes.first()->setTitleText("Easting (x)");
    }
    const auto vAxes = chart->axes(Qt::Vertical);
    if (!vAxes.isEmpty()) {
        vAxes.first()->setTitleText("Northing (y)");
    }

    centralWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(leftWidget, 0);
    mainLayout->addWidget(chartView, 1);
    setCentralWidget(centralWidget);

    rightDock = new QDockWidget();
    rightDock->setStyleSheet(R"(QDockWidget {background: white;} QDockWidget::title { background: white; border:1px solid black;} )");
    rightDock->setMinimumWidth(120);
    rightDock->setFeatures(QDockWidget::DockWidgetClosable);

    dockList = new QListWidget(rightDock);
    rightDock->setWidget(dockList);
    dockList->setContextMenuPolicy(Qt::CustomContextMenu);
    dockList->setObjectName("dockList");
    dockList->setUniformItemSizes(false);
    dockList->setSelectionMode(QAbstractItemView::SingleSelection);

    dockList->setStyleSheet(R"qss(
        QListWidget {
            border: none;
            outline: 0;
            font-size: 11px;
            color: #1f2d2a;
        }

        QListWidget::item {
            padding: 5px 7px;
            border: 1px solid transparent;
            min-height: 15px;
        }

        QListWidget::item:hover {
            background: rgba(0, 184, 148, 60);
            border-color: rgba(0, 184, 148, 120);
        }

        QListWidget::item:selected {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:0,
                stop:0 #00b894,
                stop:1 #00d2a8
            );
            color: #ffffff;
            border-color: #00a888;
            font-weight: bold;
        }

        QListWidget QScrollBar:vertical {
            width: 8px;
            background: transparent;
            margin: 2px;
        }

        QListWidget QScrollBar::handle:vertical {
            background: #2b2b2b;
            border-radius: 4px;
            min-height: 30px;
        }

        QListWidget QScrollBar::handle:vertical:hover {
            background: #000000;
        }

        QListWidget QScrollBar::add-line:vertical,
        QListWidget QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QListWidget QScrollBar::add-page:vertical,
        QListWidget QScrollBar::sub-page:vertical {
            background: none;
        }
    )qss");

    addDockWidget(Qt::RightDockWidgetArea, rightDock);
    processOptionDialog = new ProcessOptions();
    utils = new Utils();

    connect(chartView, &ChartView::doubleClicked, this,[this](){
        rightDock->show();
    });

    connect(settingBtn, &QPushButton::clicked, this, [&]() {
        processOptionDialog->setOp(projectFolder+"/staticProcessedFiles");
        processOptionDialog->exec();
    });
    connect(dockList, &QListWidget::customContextMenuRequested, this, &BaselineProcessUI::onFileRightClicked);
    connect(dockList, &QListWidget::itemClicked, this, &BaselineProcessUI::onDockItemClicked);
    connect(obsFilesPickBtn, &QPushButton::clicked, this, &BaselineProcessUI::onSelectObsFilesClicked);
    connect(processBtn, &QPushButton::clicked, this, &BaselineProcessUI::onProcessClicked);
    connect(reportBtn, &QPushButton::clicked, this, &BaselineProcessUI::onReportClicked);
    connect(chartView, &ChartView::pointClicked, this, &BaselineProcessUI::onPointClickedOnChart);
    connect(googleEarth, &QPushButton::clicked, this, &BaselineProcessUI::onGoogleEarthClicked);
    connect(loopClosureReport, &QPushButton::clicked, this, &BaselineProcessUI::onLoopClosureReportClicked);
    connect(timeView, &QPushButton::clicked, this, [=](){
        TimeBasedView *timeBasedView = new TimeBasedView();
        timeBasedView->viewTimeMap(staticJobs,files);
        timeBasedView->exec();
    });
}

void BaselineProcessUI::onSelectObsFilesClicked()
{
    QStringList obsFilePaths = QFileDialog::getOpenFileNames(this, "Observation files", QString(), "RINEX Files (*.obs , *.*o)");
    if (obsFilePaths.isEmpty()) return;

    QStringList duplicates;
    QStringList missingNavs;
    int newlyAdded = 0;

    QMap<QString, FileEntry> oldFiles = files;

    for (QString &path : obsFilePaths) {
        QFileInfo info(path);
        QString fileName = info.fileName();
        QString key = utils->getNormalizedPath(path);

        if (isFileAlreadySelected(path, fileName)) {
            duplicates.append(path);
            continue;
        }
        QString navPath = utils->findNavForObs(path);
        if (navPath.isEmpty()) {
            missingNavs.append(path);
            continue;
        }

        if (!files.contains(key)) {
            utils->parseRinexHeaderToJson(path);
        }
        QJsonObject jobj = utils->readRinexHeader(path);
        FileEntry f;

        QString pointId;
        QString markerName = jobj["marker_name"].toString().trimmed();
        pointId = markerName;

        f.pointId = pointId;
        f.obs = path;
        f.nav = navPath;
        f.role = FileEntry::Rover;
        files[key] = f;
        newlyAdded++;
    }

    QString summary;
    if (!duplicates.isEmpty())
        summary += QString("Duplicate files skipped: %1\n").arg(duplicates.size());

    if (!missingNavs.isEmpty())
        summary += QString("Files missing NAV skipped: %1\n").arg(missingNavs.size());

    if (!summary.isEmpty()) {
        CustomMessageBox mb("INFO","Some files were skipped:\n\n" + summary.trimmed(),"OK");
        mb.exec();
    }

    if(newlyAdded==0) return;

    rawDataCheckIn = new RawDataCheckIn(files);
    int result = rawDataCheckIn->exec();
    if(result != QDialog::Accepted){
        files = oldFiles;
        return;
    }

    files = rawDataCheckIn->getSelectedFiles();
    showFilesInDockList();
    startBatchSingleProcessing();
}

// void BaselineProcessUI::onProcessClicked()
// {
//     if (files.isEmpty()) {
//         CustomMessageBox("ERROR", "No files added\nPlease select observation files.", "OK").exec();
//         return;
//     }
//     QSet<QString> uniqueBaseIds;
//     QSet<QString> uniqueRoverIds;
//     int roverMissingNavsCount = 0;

//     for (auto it = files.begin(); it != files.end(); ++it) {
//         const FileEntry &entry = it.value();

//         if (entry.role == FileEntry::Base) {
//             uniqueBaseIds.insert(entry.pointId);
//         }
//         else if (entry.role == FileEntry::Rover) {
//             uniqueRoverIds.insert(entry.pointId);
//             if (entry.nav.isEmpty())
//                 roverMissingNavsCount++;
//         }
//     }

//     int baseCount = uniqueBaseIds.size();
//     int roverCount = uniqueRoverIds.size();

//     if (roverCount == 0) {
//         CustomMessageBox("ERROR", "No rover files found\nPlease add at least one rover file.", "OK").exec();
//         return;
//     }

//     if(baseCount == 0) {
//         CustomMessageBox("ERROR", "No base files found\nPlease mark at least one base file.", "OK").exec();
//         return;
//     }

//     if (roverMissingNavsCount > 0) {
//         QString msg = QString("Nav file missing for %1 rover file(s).\n" "Those rovers will be skipped.\n\n" "Do you want to continue?") .arg(roverMissingNavsCount);
//         CustomMessageBox *mb = new CustomMessageBox("INFO", msg, "YES_NO", this);
//         mb->exec();
//         if (!mb->response)
//             return;
//     }

//     QString summary = QString("Processing Details:\n Rovers available: %1\n Bases available: %2\n\nDo you want to proceed?") .arg(roverCount) .arg(baseCount);
//     CustomMessageBox *mb = new CustomMessageBox("INFO",summary, "YES_NO", this);
//     mb->exec();
//     if (!mb->response)
//         return;

//     staticJobs.clear();
//     posData.clear();
//     resolvedCoordinates.clear();

//     QString outDir = projectFolder + "/staticProcessedFiles";
//     QDir().mkpath(outDir);

//     QVector<FileEntry> allStations;
//     for (auto it = files.begin(); it != files.end(); ++it)
//         allStations.append(it.value());

//     QVector<FileEntry> bases;
//     QVector<FileEntry> rovers;

//     for (const auto &st : allStations) {
//         if (st.role == FileEntry::Base)
//             bases.append(st);
//         else if (st.role == FileEntry::Rover && !st.nav.isEmpty())
//             rovers.append(st);
//     }

//     QSet<QString> processedCrossTies;

//     auto getPairId = [](QString p1, QString p2) {
//         if (p1 > p2) std::swap(p1, p2);
//         return p1 + "_" + p2;
//     };

//     for (const FileEntry &base : bases) {
//         QDateTime bST, bET;
//         if (!utils->parseObsTimeRange(base.obs, bST, bET))
//             continue;

//         for (const FileEntry &rover : rovers) {
//             if (base.obs == rover.obs)
//                 continue;
//             QDateTime rST, rET;
//             if (!utils->parseObsTimeRange(rover.obs, rST, rET))
//                 continue;

//             QDateTime overlapStart = qMax(bST, rST);
//             QDateTime overlapEnd   = qMin(bET, rET);

//             if (overlapStart.isValid() && overlapStart.secsTo(overlapEnd) >= 1800) {
//                 StaticJob radialJob;
//                 QFileInfo bInfo(base.obs);
//                 QFileInfo rInfo(rover.obs);

//                 radialJob.baseObs  = base.obs;
//                 radialJob.roverObs = rover.obs;
//                 radialJob.nav      = !base.nav.isEmpty() ? base.nav : rover.nav;
//                 radialJob.outPos   = outDir + "/" + bInfo.completeBaseName() + "_" + rInfo.completeBaseName() + ".pos";
//                 radialJob.sortTime = overlapStart;
//                 staticJobs.append(radialJob);

//                 for (const FileEntry &otherRover : rovers) {
//                     if (rover.obs == otherRover.obs)
//                         continue;

//                     QString pairId = getPairId(rover.obs, otherRover.obs);
//                     if (processedCrossTies.contains(pairId))
//                         continue;

//                     QDateTime oST, oET;
//                     if (!utils->parseObsTimeRange(otherRover.obs, oST, oET))
//                         continue;

//                     QDateTime crossStart = qMax(rST, oST);
//                     QDateTime crossEnd   = qMin(rET, oET);

//                     if (crossStart.isValid() && crossStart.secsTo(crossEnd) >= 0) {

//                         StaticJob crossJob;
//                         QFileInfo rInfo1(rover.obs);
//                         QFileInfo rInfo2(otherRover.obs);

//                         qint64 dur1 = rST.secsTo(rET);
//                         qint64 dur2 = oST.secsTo(oET);

//                         if (dur1 >= dur2) {
//                             crossJob.baseObs  = rover.obs;
//                             crossJob.roverObs = otherRover.obs;
//                             crossJob.nav = !rover.nav.isEmpty() ? rover.nav : otherRover.nav;
//                             crossJob.outPos   = outDir + "/" + rInfo1.completeBaseName() + "_" + rInfo2.completeBaseName() + ".pos";
//                         } else {
//                             crossJob.baseObs  = otherRover.obs;
//                             crossJob.roverObs = rover.obs;
//                             crossJob.nav      = !otherRover.nav.isEmpty() ? otherRover.nav : rover.nav;
//                             crossJob.outPos   = outDir + "/" + rInfo2.completeBaseName() + "_" + rInfo1.completeBaseName() + ".pos";
//                         }
//                         crossJob.sortTime = crossStart;
//                         staticJobs.append(crossJob);
//                         processedCrossTies.insert(pairId);
//                     }
//                 }
//             }
//         }
//     }

//     QSet<QString> controlPoints;
//     for (const FileEntry &b : bases) {
//         controlPoints.insert(b.obs);
//     }
//     std::sort(staticJobs.begin(), staticJobs.end(), [&](const StaticJob &a, const StaticJob &b) {
//         qint64 timeDiff = a.sortTime.secsTo(b.sortTime);
//         if (qAbs(timeDiff) > 30) {
//             return a.sortTime < b.sortTime;
//         }
//         bool aIsFromControl = controlPoints.contains(a.baseObs);
//         bool bIsFromControl = controlPoints.contains(b.baseObs);
//         if (aIsFromControl && !bIsFromControl) return true;
//         if (!aIsFromControl && bIsFromControl) return false;
//         return QFileInfo(a.roverObs).completeBaseName() < QFileInfo(b.roverObs).completeBaseName();
//     });
//     if (staticJobs.isEmpty()) {
//         CustomMessageBox("ERROR", "No valid base–rover baselines found.", "OK").exec();
//         return;
//     }
//     jsonObsData = utils->readRinexHeader();
//     startBatchStaticProcessing();
// }

// void BaselineProcessUI::onProcessClicked()
// {
//     if (files.isEmpty()) {
//         CustomMessageBox("ERROR", "No files added\nPlease select observation files.", "OK").exec();
//         return;
//     }
//     QSet<QString> uniqueBaseIds;
//     QSet<QString> uniqueRoverIds;
//     int roverMissingNavsCount = 0;

//     for (auto it = files.begin(); it != files.end(); ++it) {
//         const FileEntry &entry = it.value();

//         if (entry.role == FileEntry::Base) {
//             uniqueBaseIds.insert(entry.pointId);
//         }
//         else if (entry.role == FileEntry::Rover) {
//             uniqueRoverIds.insert(entry.pointId);
//             if (entry.nav.isEmpty())
//                 roverMissingNavsCount++;
//         }
//     }

//     int baseCount = uniqueBaseIds.size();
//     int roverCount = uniqueRoverIds.size();

//     if (roverCount == 0) {
//         CustomMessageBox("ERROR", "No rover files found\nPlease add at least one rover file.", "OK").exec();
//         return;
//     }

//     if(baseCount == 0) {
//         CustomMessageBox("ERROR", "No base files found\nPlease mark at least one base file.", "OK").exec();
//         return;
//     }

//     if (roverMissingNavsCount > 0) {
//         QString msg = QString("Nav file missing for %1 rover file(s).\n" "Those rovers will be skipped.\n\n" "Do you want to continue?") .arg(roverMissingNavsCount);
//         CustomMessageBox *mb = new CustomMessageBox("INFO", msg, "YES_NO", this);
//         mb->exec();
//         if (!mb->response)
//             return;
//     }

//     QString summary = QString("Processing Details:\n Rovers available: %1\n Bases available: %2\n\nDo you want to proceed?") .arg(roverCount) .arg(baseCount);
//     CustomMessageBox *mb = new CustomMessageBox("INFO",summary, "YES_NO", this);
//     mb->exec();
//     if (!mb->response)
//         return;

//     staticJobs.clear();
//     posData.clear();
//     resolvedCoordinates.clear();

//     QString outDir = projectFolder + "/staticProcessedFiles";
//     QDir().mkpath(outDir);

//     QVector<FileEntry> allStations;
//     for (auto it = files.begin(); it != files.end(); ++it)
//         allStations.append(it.value());

//     QVector<FileEntry> bases;
//     QVector<FileEntry> rovers;

//     for (const auto &st : allStations) {
//         if (st.role == FileEntry::Base)
//             bases.append(st);
//         else if (st.role == FileEntry::Rover && !st.nav.isEmpty())
//             rovers.append(st);
//     }

//     QSet<QString> processedCrossTies;

//     auto getPairId = [](QString p1, QString p2) {
//         if (p1 > p2) std::swap(p1, p2);
//         return p1 + "_" + p2;
//     };

//     for (const FileEntry &base : bases) {
//         QDateTime bST, bET;
//         if (!utils->parseObsTimeRange(base.obs, bST, bET))
//             continue;

//         for (const FileEntry &rover : rovers) {
//             if (base.obs == rover.obs)
//                 continue;
//             QDateTime rST, rET;
//             if (!utils->parseObsTimeRange(rover.obs, rST, rET))
//                 continue;

//             QDateTime overlapStart = qMax(bST, rST);
//             QDateTime overlapEnd   = qMin(bET, rET);

//             if (overlapStart.isValid() && overlapStart.secsTo(overlapEnd) >= 0) {
//                 StaticJob radialJob;
//                 QFileInfo bInfo(base.obs);
//                 QFileInfo rInfo(rover.obs);

//                 radialJob.baseObs  = base.obs;
//                 radialJob.roverObs = rover.obs;
//                 radialJob.nav      = !base.nav.isEmpty() ? base.nav : rover.nav;
//                 radialJob.outPos   = outDir + "/" + bInfo.completeBaseName() + "_" + rInfo.completeBaseName() + ".pos";
//                 //-------
//                 radialJob.baseStart = bST;
//                 radialJob.baseEnd = bET;
//                 radialJob.roverStart = rST;
//                 radialJob.roverEnd = rET;
//                 radialJob.overlapStart = overlapStart;
//                 radialJob.overlapEnd = overlapEnd;
//                 //------
//                 radialJob.sortTime = overlapStart;
//                 staticJobs.append(radialJob);

//                 for (const FileEntry &otherRover : rovers) {
//                     if (rover.obs == otherRover.obs)
//                         continue;

//                     QString pairId = getPairId(rover.obs, otherRover.obs);
//                     if (processedCrossTies.contains(pairId))
//                         continue;

//                     QDateTime oST, oET;
//                     if (!utils->parseObsTimeRange(otherRover.obs, oST, oET))
//                         continue;

//                     QDateTime crossStart = qMax(rST, oST);
//                     QDateTime crossEnd   = qMin(rET, oET);

//                     if (crossStart.isValid() && crossStart.secsTo(crossEnd) >= 0) {

//                         StaticJob crossJob;
//                         QFileInfo rInfo1(rover.obs);
//                         QFileInfo rInfo2(otherRover.obs);

//                         qint64 dur1 = rST.secsTo(rET);
//                         qint64 dur2 = oST.secsTo(oET);

//                         if (dur1 >= dur2) {
//                             crossJob.baseObs  = rover.obs;
//                             crossJob.roverObs = otherRover.obs;
//                             crossJob.nav = !rover.nav.isEmpty() ? rover.nav : otherRover.nav;
//                             crossJob.outPos   = outDir + "/" + rInfo1.completeBaseName() + "_" + rInfo2.completeBaseName() + ".pos";
//                             //----
//                             crossJob.baseStart = rST;
//                             crossJob.baseEnd = rET;
//                             crossJob.roverStart = oST;
//                             crossJob.roverEnd = oET;
//                             crossJob.overlapStart = crossStart;
//                             crossJob.overlapEnd = crossEnd;
//                             //----
//                         } else {
//                             crossJob.baseObs  = otherRover.obs;
//                             crossJob.roverObs = rover.obs;
//                             crossJob.nav      = !otherRover.nav.isEmpty() ? otherRover.nav : rover.nav;
//                             crossJob.outPos   = outDir + "/" + rInfo2.completeBaseName() + "_" + rInfo1.completeBaseName() + ".pos";
//                             //----
//                             crossJob.baseStart = oST;
//                             crossJob.baseEnd = oET;
//                             crossJob.roverStart = rST;
//                             crossJob.roverEnd = rET;
//                             crossJob.overlapStart = crossStart;
//                             crossJob.overlapEnd = crossEnd;
//                             //----
//                         }
//                         crossJob.sortTime = crossStart;

//                         staticJobs.append(crossJob);
//                         processedCrossTies.insert(pairId);
//                     }
//                 }
//             }
//         }
//     }

//     QSet<QString> controlPoints;
//     for (const FileEntry &b : bases) {
//         controlPoints.insert(b.obs);
//     }
//     std::sort(staticJobs.begin(), staticJobs.end(), [&](const StaticJob &a, const StaticJob &b) {
//         qint64 timeDiff = a.sortTime.secsTo(b.sortTime);
//         if (qAbs(timeDiff) > 30) {
//             return a.sortTime < b.sortTime;
//         }
//         bool aIsFromControl = controlPoints.contains(a.baseObs);
//         bool bIsFromControl = controlPoints.contains(b.baseObs);
//         if (aIsFromControl && !bIsFromControl) return true;
//         if (!aIsFromControl && bIsFromControl) return false;
//         return QFileInfo(a.roverObs).completeBaseName() < QFileInfo(b.roverObs).completeBaseName();
//     });
//     if (staticJobs.isEmpty()) {
//         CustomMessageBox("ERROR", "No valid base–rover baselines found.", "OK").exec();
//         return;
//     }
//     jsonObsData = utils->readRinexHeader();
//     startBatchStaticProcessing();
// }



void BaselineProcessUI::onProcessClicked()
{
    if (files.isEmpty()) {
        CustomMessageBox("ERROR", "No files added\nPlease select observation files.", "OK").exec();
        return;
    }
    QSet<QString> uniqueBaseIds;
    QSet<QString> uniqueRoverIds;
    int roverMissingNavsCount = 0;

    for (auto it = files.begin(); it != files.end(); ++it) {
        const FileEntry &entry = it.value();

        if (entry.role == FileEntry::Base) {
            uniqueBaseIds.insert(entry.pointId);
        }
        else if (entry.role == FileEntry::Rover) {
            uniqueRoverIds.insert(entry.pointId);
            if (entry.nav.isEmpty())
                roverMissingNavsCount++;
        }
    }

    int baseCount = uniqueBaseIds.size();
    int roverCount = uniqueRoverIds.size();

    if (roverCount == 0) {
        CustomMessageBox("ERROR", "No rover files found\nPlease add at least one rover file.", "OK").exec();
        return;
    }

    if(baseCount == 0) {
        CustomMessageBox("ERROR", "No base files found\nPlease mark at least one base file.", "OK").exec();
        return;
    }

    if (roverMissingNavsCount > 0) {
        QString msg = QString("Nav file missing for %1 rover file(s).\n" "Those rovers will be skipped.\n\n" "Do you want to continue?") .arg(roverMissingNavsCount);
        CustomMessageBox *mb = new CustomMessageBox("INFO", msg, "YES_NO", this);
        mb->exec();
        if (!mb->response)
            return;
    }

    QString summary = QString("Processing Details:\n Rovers available: %1\n Bases available: %2\n\nDo you want to proceed?") .arg(roverCount) .arg(baseCount);
    CustomMessageBox *mb = new CustomMessageBox("INFO",summary, "YES_NO", this);
    mb->exec();
    if (!mb->response)
        return;

    staticJobs.clear();
    posData.clear();
    resolvedCoordinates.clear();

    QString outDir = projectFolder + "/staticProcessedFiles";
    QDir().mkpath(outDir);

    QVector<FileEntry> allStations;
    for (auto it = files.begin(); it != files.end(); ++it)
        allStations.append(it.value());

    QVector<FileEntry> bases;
    QVector<FileEntry> rovers;

    for (const auto &st : allStations) {
        if (st.role == FileEntry::Base)
            bases.append(st);
        else if (st.role == FileEntry::Rover && !st.nav.isEmpty())
            rovers.append(st);
    }

    QSet<QString> processedCrossTies;

    auto getPairId = [](QString p1, QString p2) {
        if (p1 > p2) std::swap(p1, p2);
        return p1 + "_" + p2;
    };

    for (const FileEntry &base : bases) {
        QDateTime bST, bET;
        if (!utils->parseObsTimeRange(base.obs, bST, bET))
            continue;

        for (const FileEntry &rover : rovers) {
            if (base.obs == rover.obs)
                continue;
            QDateTime rST, rET;
            if (!utils->parseObsTimeRange(rover.obs, rST, rET))
                continue;

            QDateTime overlapStart = qMax(bST, rST);
            QDateTime overlapEnd   = qMin(bET, rET);

            if (overlapStart.isValid() && overlapStart.secsTo(overlapEnd) >= 0) {
                StaticJob radialJob;
                QFileInfo bInfo(base.obs);
                QFileInfo rInfo(rover.obs);

                radialJob.baseObs  = base.obs;
                radialJob.roverObs = rover.obs;
                radialJob.nav      = !base.nav.isEmpty() ? base.nav : rover.nav;
                radialJob.outPos   = outDir + "/" + bInfo.completeBaseName() + "_" + rInfo.completeBaseName() + ".pos";
                //-------
                radialJob.baseStart = bST;
                radialJob.baseEnd = bET;
                radialJob.roverStart = rST;
                radialJob.roverEnd = rET;
                radialJob.overlapStart = overlapStart;
                radialJob.overlapEnd = overlapEnd;
                //------
                radialJob.sortTime = overlapStart;
                staticJobs.append(radialJob);

                for (const FileEntry &otherRover : rovers) {
                    if (rover.obs == otherRover.obs)
                        continue;

                    QString pairId = getPairId(rover.obs, otherRover.obs);
                    if (processedCrossTies.contains(pairId))
                        continue;

                    QDateTime oST, oET;
                    if (!utils->parseObsTimeRange(otherRover.obs, oST, oET))
                        continue;

                    QDateTime crossStart = qMax(rST, oST);
                    QDateTime crossEnd   = qMin(rET, oET);

                    if (crossStart.isValid() && crossStart.secsTo(crossEnd) >= 0) {

                        StaticJob crossJob;
                        QFileInfo rInfo1(rover.obs);
                        QFileInfo rInfo2(otherRover.obs);

                        qint64 dur1 = rST.secsTo(rET);
                        qint64 dur2 = oST.secsTo(oET);

                        if (dur1 >= dur2) {
                            crossJob.baseObs  = rover.obs;
                            crossJob.roverObs = otherRover.obs;
                            crossJob.nav = !rover.nav.isEmpty() ? rover.nav : otherRover.nav;
                            crossJob.outPos   = outDir + "/" + rInfo1.completeBaseName() + "_" + rInfo2.completeBaseName() + ".pos";
                            //----
                            crossJob.baseStart = rST;
                            crossJob.baseEnd = rET;
                            crossJob.roverStart = oST;
                            crossJob.roverEnd = oET;
                            crossJob.overlapStart = crossStart;
                            crossJob.overlapEnd = crossEnd;
                            //----
                        } else {
                            crossJob.baseObs  = otherRover.obs;
                            crossJob.roverObs = rover.obs;
                            crossJob.nav      = !otherRover.nav.isEmpty() ? otherRover.nav : rover.nav;
                            crossJob.outPos   = outDir + "/" + rInfo2.completeBaseName() + "_" + rInfo1.completeBaseName() + ".pos";
                            //----
                            crossJob.baseStart = oST;
                            crossJob.baseEnd = oET;
                            crossJob.roverStart = rST;
                            crossJob.roverEnd = rET;
                            crossJob.overlapStart = crossStart;
                            crossJob.overlapEnd = crossEnd;
                            //----
                        }

                        crossJob.sortTime = crossStart;
                        staticJobs.append(crossJob);
                        processedCrossTies.insert(pairId);
                    }
                }
            }
        }
    }

    QSet<QString> controlPoints;
    for (const FileEntry &b : bases) {
        controlPoints.insert(b.obs);
    }
    std::sort(staticJobs.begin(), staticJobs.end(), [&](const StaticJob &a, const StaticJob &b) {
        qint64 timeDiff = a.sortTime.secsTo(b.sortTime);
        if (qAbs(timeDiff) > 30) {
            return a.sortTime < b.sortTime;
        }
        bool aIsFromControl = controlPoints.contains(a.baseObs);
        bool bIsFromControl = controlPoints.contains(b.baseObs);
        if (aIsFromControl && !bIsFromControl) return true;
        if (!aIsFromControl && bIsFromControl) return false;
        return QFileInfo(a.roverObs).completeBaseName() < QFileInfo(b.roverObs).completeBaseName();
    });
    if (staticJobs.isEmpty()) {
        CustomMessageBox("ERROR", "No valid base–rover baselines found.", "OK").exec();
        return;
    }
    jsonObsData = utils->readRinexHeader();
    startBatchStaticProcessing();
}



void BaselineProcessUI::onReportClicked()
{
    cancelRequested = false;
    batchProgressBar = new CustomProgressBar(posData.size());
    batchProgressBar->setAttribute(Qt::WA_DeleteOnClose);
    batchProgressBar->setModal(true);
    batchProgressBar->setStatus("Generating report...");
    batchProgressBar->updateCurrent(0);
    batchProgressBar->show();

    connect(batchProgressBar, &CustomProgressBar::rejected, this, [=]() {
        cancelRequested = true;
        batchProgressBar->setStatus("Cancelling...");
    });

    GenerateReport *generateReport = new GenerateReport();
    QThread *thread = new QThread(this);

    generateReport->moveToThread(thread);
    connect(thread, &QThread::started, this, [=]() {
        generateReport->getReport(posData, projectFolder);
    });

    reportGeneratedOK = false;

    connect(generateReport, &GenerateReport::reportGenerationSuccessfull, this, [=]()
    {
        reportGeneratedOK = true;
        if (batchProgressBar && !batchProgressBar->isclosed) {
            batchProgressBar->setStatus("Report generated successfully");
            batchProgressBar->updateCurrent(posData.size());
            QTimer::singleShot(400, this,[=](){
                batchProgressBar->close();
                CustomMessageBox *mb = new CustomMessageBox("INFO","Baseline report generated successfully.","OK");
                mb->exec();
            });
        }
        else{
            CustomMessageBox *mb = new CustomMessageBox("INFO","Baseline report generated successfully.","OK");
            mb->exec();
        }
        if (thread->isRunning())
            thread->quit();
    });

    connect(generateReport, &GenerateReport::reportGenerationFailed, this, [=](const QString &err)
    {
        if (batchProgressBar && !batchProgressBar->isclosed) {
            batchProgressBar->setStatus("Report generation failed");
            QTimer::singleShot(200, batchProgressBar, &QWidget::close);
        }

        CustomMessageBox *mb = new CustomMessageBox("ERROR", err, "OK");
        mb->exec();

        if (thread->isRunning())
            thread->quit();
    });

    connect(batchProgressBar, &QWidget::destroyed, this, [=]() {
        if(posData.isEmpty() || !reportGeneratedOK)
            return;
        QString reportPath = projectFolder + "/baseline_report.pdf";
        if (QFile::exists(reportPath))
            QDesktopServices::openUrl(QUrl::fromLocalFile(reportPath));
    });

    connect(thread, &QThread::finished, generateReport, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void BaselineProcessUI::startBatchSingleProcessing()
{
    batchFiles.clear();
    generatedSinglePosFiles.clear();
    cancelRequested = false;
    currentJobIndex = 0;

    for(auto it = files.begin(); it!=files.end();it++){
        batchFiles.append(it.value());
    }

    if (batchFiles.isEmpty()) {
        CustomMessageBox mb("ERROR", "No valid OBS files selected", "OK");
        mb.exec();
        return;
    }

    batchProgressBar = new CustomProgressBar(batchFiles.size());
    batchProgressBar->setAttribute(Qt::WA_DeleteOnClose);
    batchProgressBar->setModal(true);
    batchProgressBar->show();

    connect(batchProgressBar, &CustomProgressBar::rejected, this, [=]() {
        cancelRequested = true;
        batchProgressBar->setStatus("Cancelling ...");
    });

    singleProcessor = new BaselineProcessing(this);
    processNextSingleJob();
}

void BaselineProcessUI::startBatchStaticProcessing()
{
    currentJobIndex = 0;
    cancelRequested = false;
    generatedStaticPosFiles.clear();

    batchProgressBar = new CustomProgressBar(staticJobs.size());
    batchProgressBar->setModal(true);
    batchProgressBar->show();

    connect(batchProgressBar, &CustomProgressBar::rejected, this, [&]{
        cancelRequested = true;
        batchProgressBar->setStatus("Cancelling...");
    });

    staticProcessor = new BaselineProcessing(this);
    processNextStaticJob();
}

void BaselineProcessUI::processNextSingleJob()
{
    if(cancelRequested){
        batchProgressBar->setStatus("Cancelled...");
        QTimer::singleShot(800, batchProgressBar, &QWidget::close);
        return;
    }

    if (currentJobIndex >= batchFiles.size()) {
        batchProgressBar->setStatus("All files processed.");
        batchProgressBar->updateCurrent(batchFiles.size());
        QTimer::singleShot(500, this, [=]() {
            batchProgressBar->close();
        });
        processAllSinglePosFiles();
        return;
    }
    const FileEntry &f = batchFiles[currentJobIndex];
    QFileInfo info(f.obs);
    QDir dir(projectFolder + "/singleProcessing");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString posFile = projectFolder+"/singleProcessing/"+info.completeBaseName() + ".pos";

    QString obsPath = f.obs;
    QString navPath = f.nav;
    QString posPath = posFile;
    singleProcessor->Process(
        obsPath.replace("/", "\\").toStdString(),
        "",
        navPath.replace("/", "\\").toStdString(),
        posPath.replace("/", "\\").toStdString(),
        QDateTime(), QDateTime(),
        0.0,
        true,true,true,true,true,true,
        10,
        PosMode::Single,
        Frequency::L1_L2,
        FilterType::Combined_no_phase_reset,
        GPSAmbiguity::Off_gps,
        GlonassAmbiguity::Off_glo,
        BDSAmbiguity::Off_bds,
        ElevationType::Ellipsoidal,
        BaseLocationType::Rinex_header,
        false,
        0,0,0
        );
    disconnect(singleProcessor, nullptr, this, nullptr);
    connect(singleProcessor, &BaselineProcessing::Done, this, [=]() {
        batchProgressBar->setStatus(QString("Working on %1 / %2  files").arg(currentJobIndex + 1).arg(batchFiles.size()));
        generatedSinglePosFiles.append(posFile);
        batchProgressBar->updateCurrent(currentJobIndex + 1);
        currentJobIndex++;
        processNextSingleJob();
    });
    connect(singleProcessor, &BaselineProcessing::Failed, this, [=]() {
        batchProgressBar->updateCurrent(currentJobIndex + 1);
        currentJobIndex++;
        processNextSingleJob();
    });
}

// void BaselineProcessUI::processNextStaticJob()
// {
//     if (cancelRequested) {
//         batchProgressBar->setStatus("Cancelled");
//         batchProgressBar->close();
//         return;
//     }

//     if (currentJobIndex >= staticJobs.size()) {
//         batchProgressBar->setStatus("All files processed");
//         batchProgressBar->updateCurrent(staticJobs.size());
//         QTimer::singleShot(500, this, [=]() {
//             batchProgressBar->close();
//         });
//         return;
//     }
//     const StaticJob &job = staticJobs[currentJobIndex];

//     QDateTime st, et;
//     double ti = NULL;

//     PosMode p;
//     if(processOptionDialog->pmode->currentText() == "Static") p = PosMode::Static;
//     else if(processOptionDialog->pmode->currentText() == "Single") p = PosMode::Single;
//     else p = PosMode::Kinematic;

//     Frequency f;
//     if(processOptionDialog->freq->currentText() == "L1") f = Frequency::L1;
//     else if(processOptionDialog->freq->currentText() == "L1 + L2") f = Frequency::L1_L2;
//     else if(processOptionDialog->freq->currentText() == "L1 + L2 + L3") f = Frequency::L1_L2_L3;
//     else f = Frequency::L1_L2_L3_L4;

//     FilterType ft;
//     if(processOptionDialog->filter->currentText() == "Forward") ft = FilterType::Forward;
//     else if(processOptionDialog->filter->currentText() == "Backward") ft = FilterType::Backward;
//     else if(processOptionDialog->filter->currentText() == "Combined") ft = FilterType::Combined;
//     else ft = FilterType::Combined_no_phase_reset;

//     ElevationType e;
//     if(processOptionDialog->ele->currentText() == "Ellipsoidal") e = ElevationType::Ellipsoidal;
//     else if(processOptionDialog->ele->currentText() == "MSL EGM 96") e = ElevationType::MSL_EGM96;
//     else e = ElevationType::MSL_ESM08;

//     GPSAmbiguity g;
//     if(processOptionDialog->gpsamb->currentText() == "Off") g = GPSAmbiguity::Off_gps;
//     else if(processOptionDialog->gpsamb->currentText() == "Continuous") g = GPSAmbiguity::Continuous_gps;
//     else if(processOptionDialog->gpsamb->currentText() == "Instantaneous") g = GPSAmbiguity::Instantaneous_gps;
//     else g = GPSAmbiguity::Fix_and_hold_gps;

//     GlonassAmbiguity gl;
//     if(processOptionDialog->gloamb->currentText() == "Off") gl = GlonassAmbiguity::Off_glo;
//     else if(processOptionDialog->gloamb->currentText() == "On") gl = GlonassAmbiguity::On_glo;
//     else if(processOptionDialog->gloamb->currentText() == "Autocal") gl = GlonassAmbiguity::Autocal_glo;
//     else gl = GlonassAmbiguity::Fix_and_hold_glo;

//     BDSAmbiguity ba;
//     if(processOptionDialog->beiamb->currentText() == "Off") ba = BDSAmbiguity::Off_bds;
//     else ba = BDSAmbiguity::On_bds;

//     BaseLocationType b;

//     double lat_base = 0.0, lon_base = 0.0, height_base = 0.0;
//     QString baseKey = job.baseObs;

//     QString baseFileKey = QFileInfo(job.baseObs).fileName();
//     double currentJobBasePoleHeight = 0.0;

//     if (resolvedCoordinates.contains(baseKey)) {
//         PropagatedCoord pc = resolvedCoordinates[baseKey];
//         lat_base = pc.lat;
//         lon_base = pc.lon;
//         height_base = pc.hgt + currentJobBasePoleHeight;
//         b = BaseLocationType::Lat_Lon_Height;
//     }
//     else if (files.contains(baseKey) && files[baseKey].hasReference) {
//         const FileEntry &fe = files[baseKey];
//         utils->ecefToGeodetic(fe.refX, fe.refY, fe.refZ, lat_base, lon_base, height_base);
//         b = BaseLocationType::Lat_Lon_Height;
//     }
//     else {
//         if(jsonObsData.contains(baseFileKey) && jsonObsData.value(baseFileKey).isObject()){
//             QJsonObject baseFileObj = jsonObsData.value(baseFileKey).toObject();
//             QJsonObject llh = baseFileObj["basellh"].toObject();
//             lat_base = llh["latitude"].toDouble();
//             lon_base = llh["longitude"].toDouble();
//             height_base = llh["height"].toDouble();
//         }
//         if (qFuzzyIsNull(lat_base) && qFuzzyIsNull(lon_base) && qFuzzyIsNull(height_base)) {
//             b = BaseLocationType::Rinex_header;
//         } else {
//             b = BaseLocationType::Lat_Lon_Height;
//         }
//     }

//     QString roverObsPath = job.roverObs;
//     QString baseObsPath = job.baseObs;
//     QString navPath = job.nav;
//     QString outPosPath = job.outPos;

//     if(b == BaseLocationType::Rinex_header){
//         staticProcessor->Process(
//             roverObsPath.replace("/","\\").toStdString(),
//             baseObsPath.replace("/","\\").toStdString(),
//             navPath.replace("/","\\").toStdString(),
//             outPosPath.replace("/","\\").toStdString(),
//             st,et,ti,
//             processOptionDialog->gps->isChecked(), processOptionDialog->glonass->isChecked(),
//             processOptionDialog->galileo->isChecked(), processOptionDialog->qzss->isChecked(),
//             processOptionDialog->beidou->isChecked(), processOptionDialog->sbs->isChecked(),
//             processOptionDialog->elemask->currentText().toInt(),p,f,ft,g,gl,ba,e,b,false);
//     }
//     else{
//         staticProcessor->Process(
//             roverObsPath.replace("/","\\").toStdString(),
//             baseObsPath.replace("/","\\").toStdString(),
//             navPath.replace("/","\\").toStdString(),
//             outPosPath.replace("/","\\").toStdString(),
//             st,et,ti,
//             processOptionDialog->gps->isChecked(), processOptionDialog->glonass->isChecked(),
//             processOptionDialog->galileo->isChecked(), processOptionDialog->qzss->isChecked(),
//             processOptionDialog->beidou->isChecked(), processOptionDialog->sbs->isChecked(),
//             processOptionDialog->elemask->currentText().toInt(),p,f,ft,g,gl,ba,e, b ,false,
//             lat_base,lon_base,height_base);
//     }

//     disconnect(staticProcessor, nullptr, this, nullptr);
//     connect(staticProcessor, &BaselineProcessing::Done, this, [=]() {
//         batchProgressBar->setStatus(QString("Static Processing : %1 / %2 files").arg(currentJobIndex + 1).arg(staticJobs.size()));
//         batchProgressBar->updateCurrent(++currentJobIndex);
//         generatedStaticPosFiles.append(job.outPos);
//         processStaticPosFile(job.outPos, job.roverObs);
//         processNextStaticJob();
//     });

//     connect(staticProcessor, &BaselineProcessing::Failed, this, [=]() {
//         ++currentJobIndex;
//         processNextStaticJob();
//     });
// }


void BaselineProcessUI::processNextStaticJob()
{
    if (cancelRequested) {
        batchProgressBar->setStatus("Cancelled");
        batchProgressBar->close();
        return;
    }

    if (currentJobIndex >= staticJobs.size()) {
        batchProgressBar->setStatus("All files processed");
        batchProgressBar->updateCurrent(staticJobs.size());
        QTimer::singleShot(500, this, [=]() {
            batchProgressBar->close();
        });
        return;
    }
    const StaticJob &job = staticJobs[currentJobIndex];

    QDateTime st, et;
    double ti = 0.0;

    PosMode p;
    if(processOptionDialog->pmode->currentText() == "Static") p = PosMode::Static;
    else if(processOptionDialog->pmode->currentText() == "Single") p = PosMode::Single;
    else p = PosMode::Kinematic;

    Frequency f;
    if(processOptionDialog->freq->currentText() == "L1") f = Frequency::L1;
    else if(processOptionDialog->freq->currentText() == "L1 + L2") f = Frequency::L1_L2;
    else if(processOptionDialog->freq->currentText() == "L1 + L2 + L3") f = Frequency::L1_L2_L3;
    else f = Frequency::L1_L2_L3_L4;

    FilterType ft;
    if(processOptionDialog->filter->currentText() == "Forward") ft = FilterType::Forward;
    else if(processOptionDialog->filter->currentText() == "Backward") ft = FilterType::Backward;
    else if(processOptionDialog->filter->currentText() == "Combined") ft = FilterType::Combined;
    else ft = FilterType::Combined_no_phase_reset;

    ElevationType e;
    if(processOptionDialog->ele->currentText() == "Ellipsoidal") e = ElevationType::Ellipsoidal;
    else if(processOptionDialog->ele->currentText() == "MSL EGM 96") e = ElevationType::MSL_EGM96;
    else e = ElevationType::MSL_ESM08;

    GPSAmbiguity g;
    if(processOptionDialog->gpsamb->currentText() == "Off") g = GPSAmbiguity::Off_gps;
    else if(processOptionDialog->gpsamb->currentText() == "Continuous") g = GPSAmbiguity::Continuous_gps;
    else if(processOptionDialog->gpsamb->currentText() == "Instantaneous") g = GPSAmbiguity::Instantaneous_gps;
    else g = GPSAmbiguity::Fix_and_hold_gps;

    GlonassAmbiguity gl;
    if(processOptionDialog->gloamb->currentText() == "Off") gl = GlonassAmbiguity::Off_glo;
    else if(processOptionDialog->gloamb->currentText() == "On") gl = GlonassAmbiguity::On_glo;
    else if(processOptionDialog->gloamb->currentText() == "Autocal") gl = GlonassAmbiguity::Autocal_glo;
    else gl = GlonassAmbiguity::Fix_and_hold_glo;

    BDSAmbiguity ba;
    if(processOptionDialog->beiamb->currentText() == "Off") ba = BDSAmbiguity::Off_bds;
    else ba = BDSAmbiguity::On_bds;

    BaseLocationType b;
    double base_val1 = 0.0;
    double base_val2 = 0.0;
    double base_val3 = 0.0;
    QString baseKey = job.baseObs;
    QString baseFileKey = QFileInfo(job.baseObs).fileName();
    QString roverFileKey = QFileInfo(job.roverObs).fileName();
    // double currentJobBasePoleHeight = 0.0;

    // if(jsonObsData.contains(baseFileKey) && jsonObsData.value(baseFileKey).isObject()){
    //     QJsonObject baseFileObj = jsonObsData.value(baseFileKey).toObject();
    //     if (baseFileObj.contains("poleHEN") && baseFileObj.value("poleHEN").isObject()) {
    //         currentJobBasePoleHeight = baseFileObj.value("poleHEN").toObject().value("H").toDouble();
    //     }
    // }

    double basepoleheight = 0.0;
    double roverpoleheight = 0.0;
    if(jsonObsData.contains(baseFileKey) && jsonObsData.value(baseFileKey).isObject()){
        QJsonObject baseFileObj = jsonObsData.value(baseFileKey).toObject();
        if (baseFileObj.contains("poleHEN") && baseFileObj.value("poleHEN").isObject()) {
            basepoleheight = baseFileObj.value("poleHEN").toObject().value("H").toDouble();
        }
    }
    if(jsonObsData.contains(roverFileKey) && jsonObsData.value(roverFileKey).isObject()){
        QJsonObject baseFileObj = jsonObsData.value(roverFileKey).toObject();
        if (baseFileObj.contains("poleHEN") && baseFileObj.value("poleHEN").isObject()) {
            roverpoleheight = baseFileObj.value("poleHEN").toObject().value("H").toDouble();
        }
    }


    bool coordFound = false;
    if (resolvedCoordinates.contains(baseKey)) {
        PropagatedCoord pc = resolvedCoordinates[baseKey];
        base_val1 = pc.lat;
        base_val2 = pc.lon;
        // base_val3 = pc.hgt + currentJobBasePoleHeight;
        base_val3 = pc.hgt;
        b = BaseLocationType::Lat_Lon_Height;
        coordFound = true;
    }
    if (!coordFound && jsonObsData.contains(baseFileKey) &&
        jsonObsData.value(baseFileKey).isObject() &&
        jsonObsData.value(baseFileKey).toObject().contains("basellh")) {

        QJsonObject baseFileObj = jsonObsData.value(baseFileKey).toObject();
        QJsonObject llh = baseFileObj["basellh"].toObject();
        double tempLat = llh["latitude"].toDouble();
        double tempLon = llh["longitude"].toDouble();
        double tempHgt = llh["height"].toDouble();

        if (!qFuzzyIsNull(tempLat) || !qFuzzyIsNull(tempLon) || !qFuzzyIsNull(tempHgt)) {
            base_val1 = tempLat;
            base_val2 = tempLon;
            base_val3 = tempHgt;
            b = BaseLocationType::Lat_Lon_Height;
            coordFound = true;
        }
    }
    if (!coordFound && files.contains(baseKey) && files[baseKey].hasReference) {
        const FileEntry &fe = files[baseKey];
        base_val1 = fe.refX;
        base_val2 = fe.refY;
        base_val3 = fe.refZ;
        b = BaseLocationType::ECEF_XYZ;
        coordFound = true;
    }
    if (!coordFound) {
        b = BaseLocationType::Rinex_header;
    }

    QString roverObsPath = job.roverObs;
    QString baseObsPath = job.baseObs;
    QString navPath = job.nav;
    QString outPosPath = job.outPos;

    if(b == BaseLocationType::Rinex_header){
        staticProcessor->Process(
            roverObsPath.replace("/","\\").toStdString(),
            baseObsPath.replace("/","\\").toStdString(),
            navPath.replace("/","\\").toStdString(),
            outPosPath.replace("/","\\").toStdString(),
            st,et,ti,
            processOptionDialog->gps->isChecked(), processOptionDialog->glonass->isChecked(),
            processOptionDialog->galileo->isChecked(), processOptionDialog->qzss->isChecked(),
            processOptionDialog->beidou->isChecked(), processOptionDialog->sbs->isChecked(),
            processOptionDialog->elemask->currentText().toInt(),p,f,ft,g,gl,ba,e,b,roverpoleheight,basepoleheight,false);
    }
    else{
        staticProcessor->Process(
            roverObsPath.replace("/","\\").toStdString(),
            baseObsPath.replace("/","\\").toStdString(),
            navPath.replace("/","\\").toStdString(),
            outPosPath.replace("/","\\").toStdString(),
            st,et,ti,
            processOptionDialog->gps->isChecked(), processOptionDialog->glonass->isChecked(),
            processOptionDialog->galileo->isChecked(), processOptionDialog->qzss->isChecked(),
            processOptionDialog->beidou->isChecked(), processOptionDialog->sbs->isChecked(),
            processOptionDialog->elemask->currentText().toInt(),p,f,ft,g,gl,ba,e, b ,roverpoleheight, basepoleheight,false,
            base_val1,base_val2,base_val3);
    }

    disconnect(staticProcessor, nullptr, this, nullptr);
    connect(staticProcessor, &BaselineProcessing::Done, this, [=]() {
        batchProgressBar->setStatus(QString("Static Processing : %1 / %2 files").arg(currentJobIndex + 1).arg(staticJobs.size()));
        batchProgressBar->updateCurrent(++currentJobIndex);
        generatedStaticPosFiles.append(job.outPos);
        processStaticPosFile(job.outPos, job.roverObs);
        processNextStaticJob();
    });

    connect(staticProcessor, &BaselineProcessing::Failed, this, [=]() {
        ++currentJobIndex;
        processNextStaticJob();
    });
}

void BaselineProcessUI::onFileRightClicked(const QPoint &pos)
{
    QListWidgetItem *item = dockList->itemAt(pos);
    if (!item) return;

    QString targetPointId = item->data(Qt::UserRole).toString();

    QMenu menu(this);
    QAction *deleteAction = menu.addAction(tr("Delete Station"));

    bool currentlyBase = item->text().contains("[Base]");
    QString toggleText = currentlyBase ? tr("Unmark as Base") : tr("Mark as Base");
    QAction *toggleBaseAction = menu.addAction(toggleText);
    QAction *editObsAction = menu.addAction("Edit Station");

    QAction *selected = menu.exec(dockList->viewport()->mapToGlobal(pos));
    if (!selected) return;

    if (selected == deleteAction) {
        CustomMessageBox *mb = new CustomMessageBox("QUESTION", QString("Do you want to remove '%1' and all associated files?").arg(targetPointId), "YES_NO", this);
        mb->exec();
        if (!mb->response) return;
        QList<QString> keysToRemove;
        for (auto it = files.begin(); it != files.end(); ++it) {
            if (it.value().pointId == targetPointId) {
                keysToRemove.append(it.key());
            }
        }
        for (const QString &key : keysToRemove) {
            files.remove(key);
            projectContext->stations.remove(key);
        }
        delete dockList->takeItem(dockList->row(item));
        chartView->drawChart(projectContext->stations, projectContext->baselines);
        return;
    }

    if (selected == toggleBaseAction) {
        FileEntry::Role newRole = currentlyBase ? FileEntry::Rover : FileEntry::Base;
        for (auto it = files.begin(); it != files.end(); ++it) {
            if (it.value().pointId == targetPointId) {
                files[it.key()].role = newRole;
                QString key = it.key();
                if (projectContext->stations.contains(key)) {
                    projectContext->stations[key].isBase = (newRole == FileEntry::Base);
                    projectContext->stations[key].isFixed = (newRole == FileEntry::Base);
                }
            }
        }
        showFilesInDockList();
        chartView->drawChart(projectContext->stations, projectContext->baselines);
        return;
    }

    if (selected == editObsAction) {
        QString firstObsPath;
        for (auto it = files.begin(); it != files.end(); ++it) {
            if (it.value().pointId == targetPointId) {
                firstObsPath = it.value().obs;
                break;
            }
        }
        if (!firstObsPath.isEmpty()) {
            EditOptions *eop = new EditOptions(firstObsPath, this);
            connect(eop, &EditOptions::pointIdEdited, this, [=](const QString &obsPath, const QString &updatedPointId){
                for (auto it = files.begin(); it != files.end(); ++it) {
                    if (it.value().pointId == targetPointId) {
                        files[it.key()].pointId = updatedPointId;

                        QString key = it.key();
                        if (projectContext->stations.contains(key)) {
                            projectContext->stations[key].stationId = updatedPointId;
                        }
                    }
                }
                showFilesInDockList();
                chartView->drawChart(projectContext->stations, projectContext->baselines);
            });
            eop->exec();
        }
    }
}

bool BaselineProcessUI::isFileAlreadySelected(const QString &fullPath, const QString &fileName)
{
    QString key = utils->getNormalizedPath(fullPath);
    if (files.contains(key))
        return true;
    for (auto &entry : files) {
        if (QFileInfo(entry.obs).fileName() == fileName)
            return true;
    }
    return false;
}

void BaselineProcessUI::showFilesInDockList()
{
    dockList->clear();
    QMap<QString, bool> groups;
    QStringList uniquePointIds;

    for (auto it = files.begin(); it != files.end(); ++it) {
        const FileEntry &entry = it.value();
        QString pid = entry.pointId;
        if (pid.isEmpty()) pid = "Unknown";

        if (!groups.contains(pid)) {
            groups[pid] = false;
            uniquePointIds.append(pid);
        }
        if (entry.role == FileEntry::Base) {
            groups[pid] = true;
        }
    }

    uniquePointIds.sort();
    for (const QString &pid : uniquePointIds) {
        bool isBase = groups[pid];

        QString displayName = pid;
        if (isBase) {
            displayName += " [Base]";
        }

        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, pid);

        QFont font = item->font();
        if (isBase) {
            font.setBold(true);
            item->setFont(font);
            item->setBackground(QBrush(QColor(230, 230, 230)));
            item->setForeground(QBrush(Qt::darkRed));
        } else {
            font.setBold(false);
            item->setFont(font);
            item->setBackground(Qt::white);
            item->setForeground(QBrush(Qt::black));
        }

        dockList->addItem(item);
    }
}

void BaselineProcessUI::processAllSinglePosFiles()
{
    for (QString &posFile : generatedSinglePosFiles)
    {
        AvgLLH avg = utils->computeAverageLLHFromPos(posFile);
        if (!avg.valid)
            continue;

        QFileInfo fi(posFile);
        QString baseName = fi.completeBaseName();

        QString uid;
        const FileEntry *fe = nullptr;

        for (auto it = files.begin(); it != files.end(); ++it) {
            if (QFileInfo(it.value().obs).completeBaseName() == baseName) {
                uid = utils->getNormalizedPath(it.value().obs);
                fe = &it.value();
                break;
            }
        }
        if (!fe)
            continue;

        ProjectStation &st = projectContext->stations[uid];
        st.uid = uid;
        st.obsPath  = fe->obs;
        st.stationId = fe->pointId;
        st.singlePosPath = posFile;
        st.isBase = (fe->role == FileEntry::Base);
        st.isFixed = st.isBase;

        if (fe->hasReference) {
            utils->ecefToGeodetic(fe->refX, fe->refY, fe->refZ, st.geo.lat, st.geo.lon,st.geo.h);
            st.ecef.X = fe->refX;
            st.ecef.Y = fe->refY;
            st.ecef.Z = fe->refZ;
        }
        else {
            st.geo.lat = avg.lat;
            st.geo.lon = avg.lon;
            st.geo.h   = avg.h;
        }

        ProcessUtils util;
        UTMResult utm = util.WGS84ToUTM(st.geo.lat, st.geo.lon, st.geo.h);
        st.easting = utm.easting;
        st.northing = utm.northing;
        st.orthometric = utm.altitude;
    }

    if (!projectContext->stations.isEmpty())
        chartView->drawChart(projectContext->stations, projectContext->baselines);
}

void BaselineProcessUI::processStaticPosFile(const QString &posPath, const QString &roverObs)
{
    ProcessUtils *processUtils = new ProcessUtils();
    PosData pd;
    QString err;

    bool ok  = processUtils->parsePosFileIntoPosData(posPath, pd, err);
    if(!ok) {
        delete processUtils;
        return;
    }
    QString baseKey = utils->getNormalizedPath(pd.basePath);
    QString roverKey = utils->getNormalizedPath(pd.roverPath);

    pd.from = files[baseKey].pointId;
    pd.to = files[roverKey].pointId;

    QString statPath = posPath + ".stat";
    if (QFile::exists(statPath)) {
        processUtils->parseSolutionStat(statPath, pd);
    }
    pd.finalize();
    posData.insert(QFileInfo(posPath).fileName(), pd);

    bool isPrimaryVector = false;
    if (files.contains(baseKey)) {
        if (files[baseKey].role == FileEntry::Base) {
            isPrimaryVector = true;
        }
    }

    PropagatedCoord newCoord;
    newCoord.lat = pd.roverPosition.geodetic.lat;
    newCoord.lon = pd.roverPosition.geodetic.lon;
    newCoord.hgt = pd.roverPosition.geodetic.h;
    newCoord.isPrimary = isPrimaryVector;

    bool shouldUpdate = true;
    if (resolvedCoordinates.contains(roverObs)) {
        PropagatedCoord existing = resolvedCoordinates[roverObs];
        if (existing.isPrimary && !newCoord.isPrimary) {
            shouldUpdate = false;
        }
    }
    if (shouldUpdate) {
        resolvedCoordinates.insert(roverObs, newCoord);
    }
    delete processUtils;
    setProjectContextData();
    chartView->drawChart(projectContext->stations, projectContext->baselines);
}

void BaselineProcessUI::setProjectContextData()
{
    projectContext->baselines.clear();
    projectContext->stations.clear();

    auto ensureStation = [&](const QString &uid, const QString &stationId, const SitePosition &sp, const QString &obsPath, bool isBase)
    {
        if (projectContext->stations.contains(uid))
            return;

        ProjectStation st;
        st.uid = uid;
        st.stationId = stationId;
        st.obsPath = obsPath;

        st.geo.lat = sp.geodetic.lat;
        st.geo.lon = sp.geodetic.lon;
        st.geo.h = sp.geodetic.h;

        st.ecef.X = sp.ecef.X;
        st.ecef.Y = sp.ecef.Y;
        st.ecef.Z = sp.ecef.Z;

        st.easting = sp.easting;
        st.northing = sp.northing;
        st.orthometric = sp.orthometric;

        st.isBase  = isBase;
        st.isFixed = isBase;

        projectContext->stations.insert(uid, st);
    };

    for (auto it = posData.begin(); it != posData.end(); ++it)
    {
        const PosData &pd = it.value();

        QString baseKey  = utils->getNormalizedPath(pd.basePath);
        QString roverKey = utils->getNormalizedPath(pd.roverPath);

        FileEntry &baseEntry = files[baseKey];
        FileEntry &roverEntry = files[roverKey];

        ensureStation(baseKey, baseEntry.pointId, pd.basePosition, pd.basePath, true);
        ensureStation(roverKey, roverEntry.pointId, pd.roverPosition, pd.roverPath, false);

        ProjectBaseline bl;
        bl.from = baseKey;
        bl.to = roverKey;
        bl.posPath = it.key();

        bl.length = pd.ellipsoidDistance;
        bl.startTime = pd.processingStart;
        bl.solutionType = pd.solutionTypeSummary;

        bl.fromStationId = baseEntry.pointId;
        bl.toStationId = roverEntry.pointId;

        bl.dX = pd.roverPosition.ecef.X - pd.basePosition.ecef.X;
        bl.dY = pd.roverPosition.ecef.Y - pd.basePosition.ecef.Y;
        bl.dZ = pd.roverPosition.ecef.Z - pd.basePosition.ecef.Z;

        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                bl.cov[i][j] = pd.cov_fixed[i][j];
            }
        }
        bl.rms = pd.RMS;
        bl.sigma0 = pd.sigma0;
        bl.dof = pd.dof;
        projectContext->baselines.append(bl);
    }
    emit projectContext->baselineReady();
}

void BaselineProcessUI::onDockItemClicked(QListWidgetItem *item)
{
    if (item == lastSelectedDockListItem) {
        dockList->clearSelection();
        dockList->setCurrentItem(nullptr);
        lastSelectedDockListItem = nullptr;
        chartView->clearHighlight();
        return;
    }

    lastSelectedDockListItem = item;
    QString targetPointId = item->data(Qt::UserRole).toString();
    for (auto it = files.begin(); it != files.end(); ++it) {
        if (it.value().pointId == targetPointId) {
            QString uid = utils->getNormalizedPath(it.key());
            chartView->highlightStation(uid);
        }
    }
}

void BaselineProcessUI::onPointClickedOnChart(const QStringList &uids)
{
    QString uid = uids.first();
    for (int i = 0; i < dockList->count(); ++i) {
        QListWidgetItem *item = dockList->item(i);
        if (!item)
            continue;
        if (item->data(Qt::UserRole).toString() == uid) {
            dockList->blockSignals(true);
            dockList->setCurrentItem(item);
            dockList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            dockList->blockSignals(false);
            return;
        }
    }
}

void BaselineProcessUI::onGoogleEarthClicked()
{
    QString filePath = projectFolder + "/surveypod_baselines.kml";
    ProcessUtils * putils = new ProcessUtils();
    putils->generateKMLFromPosData(posData, filePath);
    if(utils->hasKMLViewer()){
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }else{
        CustomMessageBox *mb = new CustomMessageBox(
            "INFO",
            "No application is available to open KML files.\n"
            "Would you like to open Google Earth Web instead?\n\n"
            "To import the KML file:\n"
            "1. Click ☰ (Menu)\n"
            "2. Click New → Open local KML file. (ctrl + i)\n"
            "3. Choose the generated KML file from project folder.",
            "YES_NO");
        mb->exec();

        if (mb->response)
            QDesktopServices::openUrl(QUrl("https://earth.google.com/web/"));

    }
}

void BaselineProcessUI::onLoopClosureReportClicked()
{
    if(projectContext==nullptr)
    {
        CustomMessageBox *mb = new CustomMessageBox("ERROR","No baseline processing yet", "OK");
        mb->exec();
        return;
    }
    LoopClosureUtils *lcutil = new LoopClosureUtils();
    lcutil->computeLoopClosures(projectContext);

    GenerateLoopClosureReport *getLCreport = new GenerateLoopClosureReport();
    bool ok = getLCreport->savePDF(projectContext,projectFolder);
    if(ok){
        CustomMessageBox *mb = new CustomMessageBox("INFO", "Loop closure report is generated successfully!", "OK");
        mb->exec();
    }
    delete lcutil;
    delete getLCreport;
}

void BaselineProcessUI::setProjectFolder(const QString &_projectFolder)
{
    projectFolder = _projectFolder;
}

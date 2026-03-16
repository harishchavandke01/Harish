#ifndef BASELINEPROCESSUI_H
#define BASELINEPROCESSUI_H

#include <QWidget>
#include <QLabel>
#include <QMap>
#include <QVector>
#include <QPushButton>
#include <QtCharts>
#include <QDockWidget>
#include "../../Utils/utils.h"
#include "../../Utils/customprogressbar.h"
#include "ChartView/chartview.h"
#include "../../../backend/baselineprocessing.h"
#include "ProcessOptions/processoptions.h"
#include "../../Context/projectcontext.h"
#include "RawDataCheckIn/rawdatacheckin.h"
class BaselineProcessUI : public QMainWindow
{
    Q_OBJECT
public:
    explicit BaselineProcessUI(ProjectContext * _projectContext, QMainWindow *parent = nullptr);
    void setProjectFolder(const QString &_projectFolder);

signals:

private:
    QString projectFolder;
    QLabel * heading;
    QPushButton *obsFilesPickBtn;
    QPushButton *settingBtn;
    QPushButton *processBtn;
    QPushButton *reportBtn;
    QPushButton *googleEarth;
    QPushButton *timeView;
    QPushButton *loopClosureReport;
    QWidget *leftWidget;
    QWidget *centralWidget;

    QChart *chart;
    ChartView *chartView;
    QScatterSeries * basesSeries;
    QScatterSeries * roversSeries;

    QDockWidget *rightDock;
    QDockWidget *infoDock;
    QListWidget *dockList;
    QWidget *infoWidget;


    QMap<QString, FileEntry> files;
    QMap<QString, QString> referenceSelections;
    QMap<QString, PropagatedCoord> resolvedCoordinates;
    QVector<QString> generatedSinglePosFiles;
    QVector<QString> generatedStaticPosFiles;

    QVector<FileEntry> batchFiles;
    QVector<StaticJob> staticJobs;
    QMap<QString, PosData> posData;

    int currentJobIndex = 0;
    bool cancelRequested = false;
    bool reportGeneratedOK = false;
    CustomProgressBar *batchProgressBar = nullptr;
    BaselineProcessing *singleProcessor = nullptr;
    BaselineProcessing *staticProcessor = nullptr;
    ProcessOptions *processOptionDialog = nullptr;
    ProjectContext * projectContext = nullptr;

    QListWidgetItem *lastSelectedDockListItem = nullptr;

    RawDataCheckIn *rawDataCheckIn = nullptr;
    Utils *utils = nullptr;

    QJsonObject jsonObsData;


private:
    void startBatchSingleProcessing();
    void startBatchStaticProcessing();

    void processNextSingleJob();
    void processNextStaticJob();

    void processAllSinglePosFiles();
    void processStaticPosFile(const QString &posPath, const QString &roverObs);

    void onFileRightClicked(const QPoint &pos);
    bool isFileAlreadySelected(const QString &fullPath, const QString &fileName);
    void showFilesInDockList();
    void setProjectContextData();

    void processDerivedBaselines(const QMap<QString, SitePosition> &solvedPoints, const QMap<QString, QStringList> &baseToRovers);

    void setUpInfoDock();

private slots:
    void onSelectObsFilesClicked();
    void onProcessClicked();
    void onReportClicked();
    void onDockItemClicked(QListWidgetItem *item);
    void onPointClickedOnChart(const QStringList &uids);
    void onGoogleEarthClicked();
    void onLoopClosureReportClicked();
};

#endif // BASELINEPROCESSUI_H

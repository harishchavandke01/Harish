#ifndef NETWORKADJUSTMENT_H
#define NETWORKADJUSTMENT_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QChart>
#include <QFrame>
#include <QSet>
#include <QComboBox>
#include "../../Context/projectcontext.h"
#include "../../../Utils/customprogressbar.h"
#include "../BaselineProcessing/ChartView/chartview.h"

class NetworkAdjustment : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkAdjustment(ProjectContext *_projectContext, QWidget*parent = nullptr);

    /** Provide the project's root folder so reports can be saved alongside the project. */
    void setProjectFolder(const QString &folder);

private:
    ProjectContext *projectContext;
    AdjustmentOptions adjOptions;
    QString m_projectFolder;

    QWidget*leftWidget;
    QLabel *heading;
    QLabel *modeBadge;
    QLabel *modeDesc;
    QFrame *divider;

    QPushButton *setControlsBtn;
    QPushButton *adjustNetBtn;
    QPushButton *reportBtn;

    QFrame *statsCard;
    QLabel *statsTitle;
    QComboBox *subnetCombo;
    QLabel *statType;
    QLabel *statSigma0;
    QLabel *statDof;
    QLabel *statRms;
    QLabel *statResult;

    QChart *chart;
    ChartView *chartView;

    //thread
    QVector<int> m_pendingSubnets;
    int m_currentSubnetJob =0;
    bool m_cancelRequested = false;
    CustomProgressBar *m_adjustProgressBar = nullptr;


    void buildLeftPanel();
    void buildStatsCard();
    void connectSignals();

    int fixedCount() const;
    bool isConstrainedMode() const;
    void refreshModeBadge();
    void refreshButtonStates();

    void refreshSubnetCombo();
    void showStatsForSubnet(int subnetIndex);
    void restoreStatsCard();
    void hideStatsCard();

    void startNextSubnetJob();

    /** Update ProjectStation geo/easting/northing from adjusted ECEF after each subnetwork. */
    void propagateAdjustedCoordinates(const SubnetworkResult &result);

private slots:
    void onSetControlsClicked();
    void onAdjustNetClicked();
    void onReportClicked();
    void onBaselineDataReady();
    void onSubnetComboChanged(int comboIndex);

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // NETWORKADJUSTMENT_H

#ifndef NETWORKADJUSTMENT_H
#define NETWORKADJUSTMENT_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QChart>
#include <QFrame>
#include <QSet>
#include "../../Context/projectcontext.h"
#include "AdjustOptions/adjustoptions.h"
#include "../BaselineProcessing/ChartView/chartview.h"

class NetworkAdjustment : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkAdjustment(ProjectContext *_projectContext,
                               QWidget        *parent = nullptr);

private:
    ProjectContext    *projectContext;
    AdjustmentOptions  adjOptions;

    // ── Left panel widgets ────────────────────────────────────────────────
    QWidget     *leftWidget;
    QLabel      *heading;
    QLabel      *modeBadge;
    QLabel      *modeDesc;
    QFrame      *divider;

    QPushButton *setControlsBtn;  // opens FixStations dialog
    QPushButton *adjustNetBtn;    // opens AdjustNetworkDialog (was runBtn)
    QPushButton *reportBtn;       // generates per-subnetwork report

    // ── Stats card ────────────────────────────────────────────────────────
    QFrame *statsCard;
    QLabel *statsTitle;
    QLabel *statSubnet;   // "Subnetwork 1"
    QLabel *statType;     // "Constrained" / "Free Network"
    QLabel *statSigma0;   // σ₀ value
    QLabel *statDof;      // degrees of freedom
    QLabel *statRms;      // RMS 3D
    QLabel *statResult;   // PASSED / FAILED

    // ── Chart ─────────────────────────────────────────────────────────────
    QChart    *chart;
    ChartView *chartView;

    // ── Build helpers ─────────────────────────────────────────────────────
    void buildLeftPanel();
    void buildStatsCard();
    void connectSignals();

    // ── State helpers ─────────────────────────────────────────────────────
    int  fixedCount()      const;
    bool isConstrainedMode() const;
    void refreshModeBadge();
    void refreshButtonStates();

    // ── Stats card display ────────────────────────────────────────────────
    void showStatsCard(const SubnetworkResult &r);
    void restoreStatsCard();   // called on showEvent to restore last result
    void hideStatsCard();

private slots:
    void onSetControlsClicked();
    void onAdjustNetClicked();   // renamed from onRunClicked
    void onReportClicked();
    void onBaselineDataReady();

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // NETWORKADJUSTMENT_H

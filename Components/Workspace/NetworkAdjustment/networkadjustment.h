#ifndef NETWORKADJUSTMENT_H
#define NETWORKADJUSTMENT_H

#include <QWidget>
#include "../../Context/projectcontext.h"
#include "AdjustOptions/adjustoptions.h"
#include "../BaselineProcessing/ChartView/chartview.h"
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QChart>
#include <QWidget>
#include <QFrame>
#include <QSet>

class NetworkAdjustment : public QWidget
{
    Q_OBJECT


public:
    explicit NetworkAdjustment(ProjectContext *_projectContext,QWidget *parent = nullptr);

signals:

private:
    ProjectContext    *projectContext;
    AdjustmentOptions  adjOptions;

    QWidget *leftWidget;
    QLabel *heading;
    QLabel *modeBadge;
    QLabel *modeDesc;

    QFrame *divider;
    QPushButton *setControlsBtn;
    QPushButton *optionsBtn;
    QPushButton *runBtn;
    QPushButton *reportBtn;

    QFrame *statsCard;
    QLabel *statsTitle;
    QLabel *statType;
    QLabel *statSigma0;
    QLabel *statDof;
    QLabel *statRms;
    QLabel *statResult;

    QChart *chart;
    ChartView *chartView;

    void buildLeftPanel();
    void buildStatsCard();
    void connectSignals();

    int  fixedCount() const;
    bool isConstrainedMode() const;

    void refreshModeBadge();
    void refreshButtonStates();
    bool validateNetwork();

    void showStatsCard(bool passed,bool constrained,double sigma0,int dof,double rms3d);
    void hideStatsCard();

private slots:
    void onSetControlsClicked();
    void onOptionsClicked();
    void onRunClicked();
    void onReportClicked();
    void onBaselineDataReady();


protected:
    void showEvent(QShowEvent *event) override;
};

#endif // NETWORKADJUSTMENT_H

#include "networkadjustment.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPainter>
#include <QInputDialog>
#include "../../Utils/custommessagebox.h"
#include "FixStations/fixstations.h"
#include "AdjustNetworkDialog/adjustnetworkdialog.h"
#include "SubnetworkUtils/subnetworkutils.h"

NetworkAdjustment::NetworkAdjustment(ProjectContext *_projectContext, QWidget *parent) : QWidget(parent), projectContext(_projectContext)
{
    buildLeftPanel();
    chart = new QChart();
    chartView = new ChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(700, 500);
    chart->createDefaultAxes();
    chart->zoomReset();
    chart->zoom(0.9);

    const auto hAxes = chart->axes(Qt::Horizontal);
    if (!hAxes.isEmpty())
        hAxes.first()->setTitleText("Easting (m)");
    const auto vAxes = chart->axes(Qt::Vertical);
    if (!vAxes.isEmpty())
        vAxes.first()->setTitleText("Northing (m)");

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(leftWidget,0);
    root->addWidget(chartView, 1);
    setLayout(root);

    connectSignals();
    refreshModeBadge();
    refreshButtonStates();
}


void NetworkAdjustment::connectSignals()
{
    connect(setControlsBtn, &QPushButton::clicked, this, &NetworkAdjustment::onSetControlsClicked);
    connect(adjustNetBtn, &QPushButton::clicked, this, &NetworkAdjustment::onAdjustNetClicked);
    connect(reportBtn, &QPushButton::clicked, this, &NetworkAdjustment::onReportClicked);
    connect(subnetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NetworkAdjustment::onSubnetComboChanged);
    if (projectContext) {
        connect(projectContext, &ProjectContext::baselineReady,this, &NetworkAdjustment::onBaselineDataReady);
        connect(projectContext, &ProjectContext::adjustmentFinished,this, [this]() {
            refreshSubnetCombo();
            restoreStatsCard();
            const auto &ar = projectContext->adjustmentResult;
            if (!ar.mergedAdjustedECEF.isEmpty())
                chartView->drawChartAdjusted( projectContext->stations, projectContext->baselines, ar.mergedAdjustedECEF);
            refreshButtonStates();
        });
    }
}

void NetworkAdjustment::buildLeftPanel()
{
    leftWidget = new QWidget();
    leftWidget->setObjectName("networkLeftWidget");

    QVBoxLayout *lay = new QVBoxLayout(leftWidget);
    lay->setSpacing(8);
    lay->setContentsMargins(12, 16, 12, 12);

    heading = new QLabel("Network\nAdjustment");
    heading->setObjectName("networkHeading");

    modeBadge = new QLabel("FREE NETWORK");
    modeBadge->setObjectName("networkModeBadge");
    modeBadge->setProperty("mode", "free");

    modeDesc = new QLabel("No control points selected");
    modeDesc->setObjectName("networkModeDesc");
    modeDesc->setWordWrap(true);

    divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setStyleSheet("color: #d0d0d0;");

    setControlsBtn = new QPushButton("Control Points");
    setControlsBtn->setObjectName("setBases");
    setControlsBtn->setCursor(Qt::PointingHandCursor);
    setControlsBtn->setToolTip("Mark which stations hold known control coordinates.\nIf none are checked, a Free Network adjustment runs.");

    adjustNetBtn = new QPushButton("Adjust Network");
    adjustNetBtn->setObjectName("runBtn");
    adjustNetBtn->setCursor(Qt::PointingHandCursor);
    adjustNetBtn->setEnabled(false);
    adjustNetBtn->setToolTip("Opens the Adjust Network dialog.\nSelect which subnetworks to adjust and configure weighting.");

    reportBtn = new QPushButton("Get Report");
    reportBtn->setObjectName("getReport");
    reportBtn->setCursor(Qt::PointingHandCursor);
    reportBtn->setEnabled(false);
    reportBtn->setToolTip("Generate the network adjustment report.");
    buildStatsCard();
    lay->addWidget(heading);
    lay->addSpacing(10);
    lay->addWidget(modeBadge);
    lay->addWidget(modeDesc);
    lay->addSpacing(6);
    lay->addWidget(divider);
    lay->addSpacing(6);
    lay->addWidget(setControlsBtn);
    lay->addSpacing(8);
    lay->addWidget(adjustNetBtn);
    lay->addWidget(reportBtn);
    lay->addStretch();
    lay->addWidget(statsCard);
}

void NetworkAdjustment::buildStatsCard()
{
    statsCard = new QFrame();
    statsCard->setObjectName("networkStatsCard");
    statsCard->hide();

    QVBoxLayout *sl = new QVBoxLayout(statsCard);
    sl->setContentsMargins(7, 7, 7, 7);
    sl->setSpacing(5);

    statsTitle = new QLabel("Ajustment Results");
    statsTitle->setObjectName("networkStatsTitle");

    subnetCombo = new QComboBox();
    subnetCombo->setObjectName("networkSubnetCombo");
    subnetCombo->setCursor(Qt::PointingHandCursor);
    subnetCombo->setToolTip("Select subnetwork");

    statType = new QLabel("Type: —");
    statType->setObjectName("networkStatRow");

    statSigma0 = new QLabel("σ₀: —");
    statSigma0->setObjectName("networkStatRow");

    statDof = new QLabel("DOF: —");
    statDof->setObjectName("networkStatRow");

    statRms = new QLabel("RMS: —");
    statRms->setObjectName("networkStatRow");

    statResult = new QLabel("—");
    statResult->setObjectName("networkStatResult");

    for (auto *lbl : {statType, statSigma0, statDof, statRms})
        lbl->setWordWrap(true);

    sl->addWidget(statsTitle);
    sl->addWidget(subnetCombo);
    sl->addWidget(statType);
    sl->addWidget(statSigma0);
    sl->addWidget(statDof);
    sl->addWidget(statRms);
    sl->addWidget(statResult);
}

void NetworkAdjustment::refreshSubnetCombo()
{
    if (!projectContext) return;
    const AdjustmentResult &ar = projectContext->adjustmentResult;
    if (ar.subnetworkResults.isEmpty()) return;

    subnetCombo->blockSignals(true);
    subnetCombo->clear();

    for (auto it = ar.subnetworkResults.constBegin(); it != ar.subnetworkResults.constEnd(); ++it)
    {
        const SubnetworkResult &r = it.value();
        QString label = QString("Subnetwork %1  (%2)").arg(r.subnetworkIndex).arg(r.constrained ? "Constrained" : "Free");
        subnetCombo->addItem(label, r.subnetworkIndex);
    }
    int activeIdx = ar.activeSubnetworkIndex;
    for (int i = 0; i < subnetCombo->count(); ++i) {
        if (subnetCombo->itemData(i).toInt() == activeIdx) {
            subnetCombo->setCurrentIndex(i);
            break;
        }
    }
    subnetCombo->blockSignals(false);
    statsCard->show();
}

void NetworkAdjustment::showStatsForSubnet(int subnetIndex)
{
    if (!projectContext) return;
    const AdjustmentResult &ar = projectContext->adjustmentResult;

    if (!ar.subnetworkResults.contains(subnetIndex)) return;
    const SubnetworkResult &r = ar.subnetworkResults[subnetIndex];

    statType->setText(QString("Type: %1").arg(r.constrained ? "Constrained" : "Free Network"));
    statSigma0->setText(QString("σ₀: %1").arg(qIsNaN(r.sigma0) ? "—" : QString::number(r.sigma0, 'f', 4)));
    statDof->setText(QString("DOF: %1").arg(r.dof > 0 ? QString::number(r.dof) : "—"));
    statRms->setText(QString("RMS: %1").arg(qIsNaN(r.rms3D) ? "—" : QString::number(r.rms3D * 1000.0, 'f', 1) + " mm"));

    if (r.chiSquarePassed) {
        statResult->setText("PASSED");
        statResult->setProperty("resultState", "passed");
    } else {
        statResult->setText("FAILED");
        statResult->setProperty("resultState", "failed");
    }
    statResult->style()->unpolish(statResult);
    statResult->style()->polish(statResult);
}

void NetworkAdjustment::restoreStatsCard()
{
    if (!projectContext) return;
    const AdjustmentResult &ar = projectContext->adjustmentResult;
    if (!ar.anySuccess()) return;

    refreshSubnetCombo();
    int idx = subnetCombo->currentData().toInt();
    if (idx > 0)
        showStatsForSubnet(idx);
}

void NetworkAdjustment::hideStatsCard()
{
    statsCard->hide();
}

int NetworkAdjustment::fixedCount() const
{
    if (!projectContext) return 0;
    QSet<QString> uniqueFixed;
    for (const auto &st : std::as_const(projectContext->stations)) {
        if (st.isFixed)
            uniqueFixed.insert(st.stationId);
    }
    return uniqueFixed.size();
}

bool NetworkAdjustment::isConstrainedMode() const
{
    return fixedCount() > 0;
}

void NetworkAdjustment::refreshModeBadge()
{
    int fixed = fixedCount();
    if (fixed > 0) {
        modeBadge->setText("CONSTRAINED");
        modeBadge->setProperty("mode", "constrained");
        modeDesc->setText(
            QString("%1 control point%2 fixed")
                .arg(fixed).arg(fixed == 1 ? "" : "s"));
    } else {
        modeBadge->setText("FREE NETWORK");
        modeBadge->setProperty("mode", "free");
        modeDesc->setText("No control points — relative solution");
    }
    modeBadge->style()->unpolish(modeBadge);
    modeBadge->style()->polish(modeBadge);
}

void NetworkAdjustment::refreshButtonStates()
{
    bool hasBaselines = projectContext && !projectContext->baselines.isEmpty();
    adjustNetBtn->setEnabled(hasBaselines);
    bool hasResult = projectContext && projectContext->adjustmentResult.anySuccess();
    reportBtn->setEnabled(hasResult);
}

void NetworkAdjustment::onSubnetComboChanged(int comboIndex)
{
    if (comboIndex < 0 || !subnetCombo) return;
    int subnetIndex = subnetCombo->itemData(comboIndex).toInt();
    if (subnetIndex > 0)
        showStatsForSubnet(subnetIndex);
}

void NetworkAdjustment::onSetControlsClicked()
{
    FixStations dlg(projectContext, this);
    dlg.exec();
    refreshModeBadge();
    refreshButtonStates();
    if (projectContext && !projectContext->stations.isEmpty())
        chartView->drawChart(projectContext->stations, projectContext->baselines);
}

void NetworkAdjustment::onAdjustNetClicked()
{
    if (!projectContext || projectContext->baselines.isEmpty()) {
        CustomMessageBox mb("ERROR","No baselines found.\nComplete baseline processing first.","OK", this);
        mb.exec();
        return;
    }

    AdjustNetworkDialog dlg(projectContext, adjOptions, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    QVector<int> toAdjust = dlg.selectedSubnetworks();
    if (toAdjust.isEmpty())
        return;

    // ── BACKEND call ───────────────────────────────────────────────
    //  Replace this block with actual LSSolver calls when the solver
    //       is implemented. For each index in toAdjust:
    //
    //   for (int subnetIdx : toAdjust) {
    //       // Find matching SubnetworkInfo
    //       SubnetworkInfo *info = nullptr;
    //       for (SubnetworkInfo &s : projectContext->subnetworks)
    //           if (s.index == subnetIdx) { info = &s; break; }
    //       if (!info) continue;
    //
    //       // Run solver
    //       LSSolver solver(projectContext, adjOptions, *info);
    //       SubnetworkResult result = solver.solve();
    //
    //       // Store result (merges into mergedAdjustedECEF automatically)
    //       projectContext->adjustmentResult.storeSubnetworkResult(result);
    //       info->hasResult = result.success;
    //   }
    //

    // ── Temporary frontend simulation ─────────────────────────────────────
    for (int subnetIdx : toAdjust) {
        SubnetworkInfo *info = nullptr;
        for (SubnetworkInfo &s : projectContext->subnetworks)
            if (s.index == subnetIdx) { info = &s; break; }
        if (!info) continue;

        SubnetworkResult fakeResult;
        fakeResult.subnetworkIndex = subnetIdx;
        fakeResult.success = true;
        fakeResult.constrained  = info->isConstrained;
        fakeResult.usedCovariance  = adjOptions.useCovariance;
        fakeResult.sigma0  = NAN;
        fakeResult.rms3D  = NAN;
        fakeResult.dof  = 0;
        fakeResult.chiSquarePassed = true;
        fakeResult.adjustedAt = QDateTime::currentDateTime();

        for (const QString &uid : info->stationUIDs) {
            if (projectContext->stations.contains(uid)) {
                const ProjectStation &st = projectContext->stations[uid];
                fakeResult.adjustedECEF[uid] =QVector3D((float)st.ecef.X, (float)st.ecef.Y, (float)st.ecef.Z);
            }
        }
        projectContext->adjustmentResult.storeSubnetworkResult(fakeResult);
        info->hasResult = true;
    }
    // ── End temporary simulation ──────────────────────────────────────────

    refreshSubnetCombo();
    int lastIdx = toAdjust.last();
    showStatsForSubnet(lastIdx);
    chartView->drawChartAdjusted( projectContext->stations, projectContext->baselines, projectContext->adjustmentResult.mergedAdjustedECEF);
    refreshButtonStates();
    emit projectContext->adjustmentFinished();
}

void NetworkAdjustment::onReportClicked()
{
    if (!projectContext) return;
    const AdjustmentResult &ar = projectContext->adjustmentResult;

    if (ar.subnetworkResults.isEmpty()) return;

    // ── BACKEND call ───────────────────────────────────────────────
    //  Replace these message boxes with actual report generation.
    // The report class (NetworkAdjustmentReport) will be wired here.
    // It receives either a single SubnetworkResult or all results.
    //
    // Single subnetwork:
    //   NetworkAdjustmentReport report(projectContext,
    //                                   ar.subnetworkResults[selectedIdx],
    //                                   this);
    //   report.exec();
    //
    // All subnetworks:
    //   NetworkAdjustmentReport report(projectContext, ar, this);
    //   report.exec();
    //

    if (ar.subnetworkResults.size() == 1) {
        int idx = ar.subnetworkResults.constBegin().key();
        const SubnetworkResult &r = ar.subnetworkResults[idx];
        CustomMessageBox mb("INFO", QString("Report for Subnetwork %1\n" "Type: %2\n" "Status: %3\n\n" "[Report generation will be implemented next]").arg(idx).arg(r.constrained ? "Constrained" : "Free Network").arg(r.success ? "Adjusted" : "Not adjusted"),
                            "OK", this);
        mb.exec();
        return;
    }

    QStringList choices;
    for (auto it = ar.subnetworkResults.constBegin();it != ar.subnetworkResults.constEnd(); ++it)
        choices << QString("Subnetwork %1  (%2)").arg(it.key()).arg(it.value().constrained? "Constrained" : "Free Network");
    choices << "All Subnetworks (Combined)";

    bool ok;
    QString chosen = QInputDialog::getItem( this, "Select Report", "Which adjustment report would you like to view?", choices, 0, false, &ok);
    if (!ok) return;
    if (chosen.startsWith("All")) {
        CustomMessageBox mb("INFO","Combined report for all subnetworks.\n\n""[Report generation will be implemented next]","OK", this);
        mb.exec();
    } else {
        int selIdx = chosen.split(" ", Qt::SkipEmptyParts).value(1).toInt();
        if (ar.subnetworkResults.contains(selIdx)) {
            const SubnetworkResult &r = ar.subnetworkResults[selIdx];
            CustomMessageBox mb("INFO", QString("Report for Subnetwork %1\n" "Type: %2\n" "Status: %3\n\n" "[Report generation will be implemented next]")
                                    .arg(selIdx)
                                    .arg(r.constrained ? "Constrained" : "Free Network")
                                    .arg(r.success ? "Adjusted" : "Not adjusted"), "OK", this);
            mb.exec();
        }
    }
}

void NetworkAdjustment::onBaselineDataReady()
{
    refreshModeBadge();
    refreshButtonStates();
    hideStatsCard();
    if (projectContext && !projectContext->stations.isEmpty())
        chartView->drawChart(projectContext->stations, projectContext->baselines);
}

void NetworkAdjustment::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (projectContext && !projectContext->stations.isEmpty()) {
        const AdjustmentResult &ar = projectContext->adjustmentResult;

        if (!ar.mergedAdjustedECEF.isEmpty()) {
            chartView->drawChartAdjusted(projectContext->stations,projectContext->baselines,ar.mergedAdjustedECEF);
        } else {
            chartView->drawChart(projectContext->stations, projectContext->baselines);
        }
        restoreStatsCard();
    }

    refreshModeBadge();
    refreshButtonStates();
}

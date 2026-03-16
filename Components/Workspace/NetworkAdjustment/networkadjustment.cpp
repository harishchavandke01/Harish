#include "networkadjustment.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QChart>
#include <QSpacerItem>
#include <QQueue>
#include <QString>
#include <QPainter>
#include "../../Utils/custommessagebox.h"
#include "FixStations/fixstations.h"

NetworkAdjustment::NetworkAdjustment(ProjectContext *_projectContext, QWidget *parent) : QWidget{parent}, projectContext(_projectContext)
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
    if (!hAxes.isEmpty()) {
        hAxes.first()->setTitleText("Easting (x)");
    }
    const auto vAxes = chart->axes(Qt::Vertical);
    if (!vAxes.isEmpty()) {
        vAxes.first()->setTitleText("Northing (y)");
    }

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setSpacing(0);
    root->addWidget(leftWidget, 0);
    root->addWidget(chartView, 1);
    setLayout(root);

    connectSignals();
    refreshModeBadge();
    refreshButtonStates();
}

void NetworkAdjustment::connectSignals()
{
    connect(setControlsBtn, &QPushButton::clicked,this, &NetworkAdjustment::onSetControlsClicked);
    connect(optionsBtn, &QPushButton::clicked, this, &NetworkAdjustment::onOptionsClicked);
    connect(runBtn, &QPushButton::clicked, this, &NetworkAdjustment::onRunClicked);
    connect(reportBtn, &QPushButton::clicked, this, &NetworkAdjustment::onReportClicked);

    if (projectContext) {
        connect(projectContext, &ProjectContext::baselineReady, this, &NetworkAdjustment::onBaselineDataReady);
        connect(projectContext, &ProjectContext::adjustmentFinished,this, [this]() {
            const auto &r = projectContext->adjustmentResult;
            if (r.success) {
                showStatsCard(true, r.constrained, r.sigma0, r.dof, r.rms3D);
                chartView->drawChartAdjusted(projectContext->stations,projectContext->baselines,projectContext->adjustmentResult.adjustedECEF);
            }
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
    setControlsBtn->setToolTip("Set which stations are fixed control points");

    optionsBtn = new QPushButton("Options");
    optionsBtn->setObjectName("options");
    optionsBtn->setCursor(Qt::PointingHandCursor);

    runBtn = new QPushButton("Run Adjustment");
    runBtn->setObjectName("runBtn");
    runBtn->setCursor(Qt::PointingHandCursor);
    runBtn->setEnabled(false);

    reportBtn = new QPushButton("Get Report");
    reportBtn->setObjectName("getReport");
    reportBtn->setCursor(Qt::PointingHandCursor);
    reportBtn->setEnabled(false);

    buildStatsCard();
    lay->addWidget(heading);
    lay->addSpacing(10);
    lay->addWidget(modeBadge);
    lay->addWidget(modeDesc);
    lay->addSpacing(6);
    lay->addWidget(divider);
    lay->addSpacing(6);
    lay->addWidget(setControlsBtn);
    lay->addWidget(optionsBtn);
    lay->addSpacing(8);
    lay->addWidget(runBtn);
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
    sl->setContentsMargins(10, 10, 10, 10);
    sl->setSpacing(5);

    statsTitle = new QLabel("Last Adjustment");
    statsTitle->setObjectName("networkStatsTitle");

    statType = new QLabel("Type: —");
    statSigma0 = new QLabel("σ₀: —");
    statDof = new QLabel("DOF: —");
    statRms = new QLabel("RMS: — m");
    statResult = new QLabel("—");
    statResult->setObjectName("networkStatResult");

    for (auto *lbl : {statType, statSigma0, statDof, statRms}) {
        lbl->setObjectName("networkStatRow");
        lbl->setWordWrap(true);
    }

    sl->addWidget(statsTitle);
    sl->addWidget(statType);
    sl->addWidget(statSigma0);
    sl->addWidget(statDof);
    sl->addWidget(statRms);
    sl->addWidget(statResult);
}

int NetworkAdjustment::fixedCount() const
{
    if (!projectContext) return 0;
    int n = 0;
    for (const auto &st : std::as_const(projectContext->stations))
        if (st.isFixed) n++;
    return n;
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
        modeDesc->setText( QString("%1 control point%2 fixed").arg(fixed).arg(fixed == 1 ? "" : "s"));
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
    runBtn->setEnabled(hasBaselines);

    bool hasResult = projectContext && projectContext->adjustmentResult.success;
    reportBtn->setEnabled(hasResult);
}


bool NetworkAdjustment::validateNetwork()
{
    if (!projectContext || projectContext->baselines.isEmpty()) {
        CustomMessageBox mb("ERROR", "No baselines found.\nPlease complete baseline processing first.", "OK", this);
        mb.exec();
        return false;
    }

    if (projectContext->stations.isEmpty()) {
        CustomMessageBox mb("ERROR", "No stations found.", "OK", this);
        mb.exec();
        return false;
    }
    if (!isConstrainedMode())
        return true;
    QSet<QString> visited;
    QQueue<QString> queue;

    for (auto it = projectContext->stations.constBegin();
         it != projectContext->stations.constEnd(); ++it) {
        if (it.value().isFixed) {
            queue.enqueue(it.key());
            visited.insert(it.key());
        }
    }

    while (!queue.isEmpty()) {
        QString cur = queue.dequeue();
        for (const ProjectBaseline &bl : std::as_const(projectContext->baselines)) {
            if (bl.from == cur && !visited.contains(bl.to)) {
                visited.insert(bl.to);
                queue.enqueue(bl.to);
            }
            if (bl.to == cur && !visited.contains(bl.from)) {
                visited.insert(bl.from);
                queue.enqueue(bl.from);
            }
        }
    }

    QStringList disconnected;
    for (auto it = projectContext->stations.constBegin();
         it != projectContext->stations.constEnd(); ++it) {
        if (!visited.contains(it.key()))
            disconnected.append(it.value().stationId.isEmpty()
                                    ? it.key()
                                    : it.value().stationId);
    }

    if (!disconnected.isEmpty()) {
        QString msg = QString("These stations are not connected to any control point:\n\n%1\n\n"
                    "Add baselines to connect them, or use a Free Network adjustment "
                    "(deselect all control points).")
                .arg(disconnected.join(", "));
        CustomMessageBox mb("ERROR", msg, "OK", this);
        mb.exec();
        return false;
    }
    return true;
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

void NetworkAdjustment::onOptionsClicked()
{
    AdjustOptions dlg(adjOptions, this);
    dlg.exec();
}

void NetworkAdjustment::onRunClicked()
{
    if (!validateNetwork())
        return;

    bool constrained = isConstrainedMode();

    // ── ALGORITHM PLACEHOLDER ──────────────────────────────────────
    // The actual solver (buildNetworkModel / solveLeastSquares) will be
    // wired in here in the next implementation step.
    // For now: inform the user of what mode will run.
    QString modeStr = constrained ? "Constrained" : "Free Network";
    CustomMessageBox mb("INFO", QString("Ready to run %1 adjustment.\n\n" "Stations : %2\n" "Baselines: %3\n" "Fixed pts: %4\n\n" "Algorithm implementation coming next.")
            .arg(modeStr)
            .arg(projectContext->stations.size())
            .arg(projectContext->baselines.size())
            .arg(fixedCount()),"OK", this);
    mb.exec();

}

void NetworkAdjustment::onReportClicked()
{
    // Stub — report generation implemented in a future step
    CustomMessageBox mb("INFO","Report generation will be implemented in the next step.","OK", this);
    mb.exec();
}

void NetworkAdjustment::onBaselineDataReady()
{
    refreshModeBadge();
    refreshButtonStates();
    if (projectContext && !projectContext->stations.isEmpty())
        chartView->drawChart(projectContext->stations, projectContext->baselines);
}

void NetworkAdjustment::showStatsCard(bool passed, bool constrained, double sigma0, int    dof, double rms3d)
{
    statType->setText(
        QString("Type: %1").arg(constrained ? "Constrained" : "Free Network"));
    statSigma0->setText(
        QString("σ₀: %1").arg(
            qIsNaN(sigma0) ? "—" : QString::number(sigma0, 'f', 4)));
    statDof->setText(
        QString("DOF: %1").arg(dof > 0 ? QString::number(dof) : "—"));
    statRms->setText(
        QString("RMS: %1 m").arg(
            qIsNaN(rms3d) ? "—" : QString::number(rms3d * 1000.0, 'f', 1) + " mm"));

    if (passed) {
        statResult->setText("PASSED");
        statResult->setProperty("resultState", "passed");
    } else {
        statResult->setText("FAILED");
        statResult->setProperty("resultState", "failed");
    }
    statResult->style()->unpolish(statResult);
    statResult->style()->polish(statResult);

    statsCard->show();
}
void NetworkAdjustment::hideStatsCard()
{
    statsCard->hide();
}

void NetworkAdjustment::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (projectContext && !projectContext->stations.isEmpty()) {
        if (projectContext->adjustmentResult.success) {
            chartView->drawChartAdjusted( projectContext->stations, projectContext->baselines, projectContext->adjustmentResult.adjustedECEF);
        } else {
            chartView->drawChart(projectContext->stations,  projectContext->baselines);
        }
    }
    refreshModeBadge();
    refreshButtonStates();
}

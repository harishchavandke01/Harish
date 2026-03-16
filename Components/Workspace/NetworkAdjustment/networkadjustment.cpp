#include "networkadjustment.h"
#include <QVBoxLayout>
#include <QChart>
#include "../../Utils/custommessagebox.h"
#include "FixStations/fixstations.h"
#include <Eigen/Dense>

NetworkAdjustment::NetworkAdjustment(ProjectContext *_projectContext, QWidget *parent) : QWidget{parent}, projectContext(_projectContext)
{
    leftWidget = new QWidget();
    leftWidget->setObjectName("networkLeftWidget");
    QVBoxLayout *llay = new QVBoxLayout(leftWidget);
    llay->setSpacing(8);
    llay->setAlignment(Qt::AlignLeft);

    heading = new QLabel("Network adjust");
    heading->setObjectName("networkHeading");

    runAdjustment = new QPushButton("Run adjustment");
    runAdjustment->setObjectName("runBtn");

    setBases = new QPushButton("Set base");
    setBases->setObjectName("setBases");

    options = new QPushButton("Options");
    options->setObjectName("options");

    report = new QPushButton("Get report");
    report->setObjectName("getReport");

    llay->addWidget(heading);
    llay->addSpacing(20);
    llay->addWidget(runAdjustment);
    llay->addWidget(setBases);
    llay->addWidget(options);
    llay->addWidget(report);
    llay->addStretch();

    chart = new QChart();
    chartView = new ChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(700, 500);
    chart->createDefaultAxes();
    chart->zoomReset();
    chart->zoom(0.9);
    chart->axes(Qt::Horizontal).first()->setTitleText("Easting (x)");
    chart->axes(Qt::Vertical).first()->setTitleText("Northing (y)");

    QWidget *centralWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(leftWidget, 0);
    mainLayout->addWidget(chartView, 1);
    setLayout(mainLayout);

    connect(setBases, &QPushButton::clicked, this, [this]() {
        FixStations dlg(projectContext, this);
        dlg.exec();
        drawNetWorkPreview();
    });
    connect(options, &QPushButton::clicked, this, [this]() {
        AdjustOptions dlg(adjOptions, this);
        dlg.exec();
    });

    connect(runAdjustment, &QPushButton::clicked, this, &NetworkAdjustment::onRunAdjustmentClicked);
}

void NetworkAdjustment::loadDataFromProjectContext()
{
    if(!projectContext)
        return;

    if(projectContext->baselines.isEmpty()){
        return;
    }
    drawNetWorkPreview();
}

void NetworkAdjustment::drawNetWorkPreview()
{
    // chartView->drawNetwork(projectContext->stations, projectContext->baselines, false);
}

bool NetworkAdjustment::validateNetwork()
{
    int fixedCount = 0;
    for(const auto &st: projectContext->stations){
        if(st.isFixed)
            fixedCount++;
    }
    if(fixedCount==0){
        CustomMessageBox mb("ERROR","No fixed stations selected.\nAt least one station must be fixed.","OK");
        mb.exec();
        return false;
    }

    if (projectContext->baselines.isEmpty()) {
        CustomMessageBox mb( "ERROR", "No baselines available for adjustment.", "OK" );
        mb.exec();
        return false;
    }

    return true;
}

void NetworkAdjustment::onRunAdjustmentClicked()
{
    if(!validateNetwork()){
        return;
    }
    buildNetworkModel();
    if (!solveLeastSquares()) {
        CustomMessageBox mb("ERROR", "Network adjustment failed (singular or ill-conditioned system).", "OK");
        mb.exec();
        return;
    }
}

void NetworkAdjustment::buildNetworkModel()
{
    auto &ctx = *projectContext;

    ctx.networkModel = NetworkModel(); // reset

    // 1️⃣ Unknown index mapping
    int idx = 0;
    for (auto it = ctx.stations.begin(); it != ctx.stations.end(); ++it) {
        if (!it.value().isFixed) {
            ctx.networkModel.unknownIndex[it.key()] = idx;
            idx += 3;
        }
    }
    ctx.networkModel.nUnknowns = idx;

    // 2️⃣ Build observations + misclosures
    for (const ProjectBaseline &bl : ctx.baselines)
    {
        if (!ctx.stations.contains(bl.from) ||
            !ctx.stations.contains(bl.to))
            continue;

        const ProjectStation &base  = ctx.stations[bl.from];
        const ProjectStation &rover = ctx.stations[bl.to];

        NetworkModel::Observation obs;
        obs.base = bl.from;
        obs.rover = bl.to;

        obs.dX = bl.dX;
        obs.dY = bl.dY;
        obs.dZ = bl.dZ;

        obs.comp_dX = rover.ecef.X - base.ecef.X;
        obs.comp_dY = rover.ecef.Y - base.ecef.Y;
        obs.comp_dZ = rover.ecef.Z - base.ecef.Z;

        obs.mis_dX = obs.dX - obs.comp_dX;
        obs.mis_dY = obs.dY - obs.comp_dY;
        obs.mis_dZ = obs.dZ - obs.comp_dZ;

        // QMatrix3x3 cov;

        // cov(0,0) = bl.cov[0][0];
        // cov(0,1) = bl.cov[0][1];
        // cov(0,2) = bl.cov[0][2];

        // cov(1,0) = bl.cov[1][0];
        // cov(1,1) = bl.cov[1][1];
        // cov(1,2) = bl.cov[1][2];

        // cov(2,0) = bl.cov[2][0];
        // cov(2,1) = bl.cov[2][1];
        // cov(2,2) = bl.cov[2][2];

        // obs.covariance = cov;

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                obs.covariance[i][j] = bl.cov[i][j];
            }
        }

        ctx.networkModel.observations.append(obs);
    }

    qDebug() << "=== Network Model Built ===";
    qDebug() << "Stations:" << ctx.stations.size();
    qDebug() << "Baselines:" << ctx.baselines.size();
    qDebug() << "Unknowns (XYZ):" << ctx.networkModel.nUnknowns;
    qDebug() << "Observations:" << ctx.networkModel.observations.size();

    for (const auto &obs : ctx.networkModel.observations) {
        qDebug()
        << "Obs:" << obs.base << "->" << obs.rover
        << "misclosure:"
        << obs.mis_dX << obs.mis_dY << obs.mis_dZ;
    }

}

#include <Eigen/Dense>

bool NetworkAdjustment::solveLeastSquares()
{
    auto &ctx = *projectContext;
    auto &model = ctx.networkModel;
    auto &stations = ctx.stations;

    const int nObs = model.observations.size();
    const int nEq  = nObs * 3;
    const int nUnk = model.nUnknowns;

    qDebug() << "=== Solving Network Adjustment ===";
    qDebug() << "Observations:" << nObs;
    qDebug() << "Equations:" << nEq;
    qDebug() << "Unknowns:" << nUnk;


    if (nObs == 0 || nUnk == 0)
        return false;

    // --- Matrices ---
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(nEq, nUnk);
    Eigen::VectorXd l = Eigen::VectorXd::Zero(nEq);
    Eigen::MatrixXd P = Eigen::MatrixXd::Zero(nEq, nEq);

    // --- Build A, l, P ---
    for (int i = 0; i < nObs; ++i)
    {
        const auto &obs = model.observations[i];
        const int row = i * 3;

        // l vector (misclosure)
        l(row + 0) = obs.mis_dX;
        l(row + 1) = obs.mis_dY;
        l(row + 2) = obs.mis_dZ;

        // Design matrix A
        if (!stations[obs.base].isFixed) {
            int col = model.unknownIndex[obs.base];
            A(row + 0, col + 0) = -1.0;
            A(row + 1, col + 1) = -1.0;
            A(row + 2, col + 2) = -1.0;
        }

        if (!stations[obs.rover].isFixed) {
            int col = model.unknownIndex[obs.rover];
            A(row + 0, col + 0) = +1.0;
            A(row + 1, col + 1) = +1.0;
            A(row + 2, col + 2) = +1.0;
        }

        // Weight matrix (inverse covariance)
        // Eigen::Matrix3d C;
        // C << obs.covariance(0,0), obs.covariance(0,1), obs.covariance(0,2),
        //     obs.covariance(1,0), obs.covariance(1,1), obs.covariance(1,2),
        //     obs.covariance(2,0), obs.covariance(2,1), obs.covariance(2,2);


        Eigen::Matrix3d C;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                C(i, j) = obs.covariance[i][j];
            }
        }

        // Safer than inverse()
        Eigen::LDLT<Eigen::Matrix3d> ldlt(C);
        if (ldlt.info() != Eigen::Success) {
            return false;   // covariance invalid / singular
        }

        Eigen::Matrix3d W = ldlt.solve(Eigen::Matrix3d::Identity());

        // Insert into block-diagonal weight matrix
        P.block<3,3>(row, row) = W;
    }

    // --- Solve normal equations ---
    Eigen::MatrixXd N = A.transpose() * P * A;
    Eigen::VectorXd u = A.transpose() * P * l;

    Eigen::LDLT<Eigen::MatrixXd> solver(N);
    if (solver.info() != Eigen::Success)
        return false;

    Eigen::VectorXd x = solver.solve(u);
    if (solver.info() != Eigen::Success)
        return false;

    // --- Store corrections ---
    ctx.adjustmentResult = AdjustmentResult(); // reset
    ctx.adjustmentResult.success = true;

    for (auto it = model.unknownIndex.begin(); it != model.unknownIndex.end(); ++it)
    {
        int idx = it.value();
        ctx.adjustmentResult.stationCorrections[it.key()] =
            QVector3D(x(idx+0), x(idx+1), x(idx+2));
    }

    // --- Residuals ---
    Eigen::VectorXd v = A * x - l;

    double vPv = v.transpose() * P * v;
    ctx.adjustmentResult.dof = nEq - nUnk;
    ctx.adjustmentResult.sigma0 = std::sqrt(vPv / ctx.adjustmentResult.dof);

    double sumV2 = 0.0;

    for (int i = 0; i < nObs; ++i)
    {
        int row = i * 3;
        AdjustmentResult::Residual r;
        r.base  = model.observations[i].base;
        r.rover = model.observations[i].rover;

        r.vX = v(row + 0);
        r.vY = v(row + 1);
        r.vZ = v(row + 2);
        r.vNorm = std::sqrt(r.vX*r.vX + r.vY*r.vY + r.vZ*r.vZ);

        sumV2 += r.vNorm * r.vNorm;
        ctx.adjustmentResult.residuals.append(r);
    }

    ctx.adjustmentResult.rms3D = std::sqrt(sumV2 / nObs);
    qDebug() << "=== Adjustment Statistics ===";
    qDebug() << "DOF:" << ctx.adjustmentResult.dof;
    qDebug() << "Sigma0:" << ctx.adjustmentResult.sigma0;
    qDebug() << "RMS 3D:" << ctx.adjustmentResult.rms3D;


    return true;
}

void NetworkAdjustment::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadDataFromProjectContext();
}

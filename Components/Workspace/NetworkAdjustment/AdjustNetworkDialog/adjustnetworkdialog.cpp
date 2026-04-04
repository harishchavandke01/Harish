#include "adjustnetworkdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFormLayout>
#include <QScrollArea>
#include "../SubnetworkUtils/subnetworkutils.h"
#include "../CovarianceMatrix/covariancematrix.h"

AdjustNetworkDialog::AdjustNetworkDialog(ProjectContext *ctx, AdjustmentOptions &opts, QWidget *parent): QDialog(parent), projectContext(ctx), options(opts)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(640, 480);
    setObjectName("editOptions");
    SubnetworkUtils::detectAndStore(projectContext);
    buildTitleBar();

    tabs = new QTabWidget();
    tabs->setObjectName("adjustNetworkTabs");
    buildSubnetworkTab();
    buildWeightingTab();

    cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("editOpBtns");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet( "QPushButton { background-color:#636e72; color:white; }" "QPushButton:hover { background-color:#74b9ff; }");

    adjustBtn = new QPushButton("Adjust");
    adjustBtn->setObjectName("runBtn");
    adjustBtn->setCursor(Qt::PointingHandCursor);
    adjustBtn->setStyleSheet( "QPushButton { background-color:#00b894; color:white; }" "QPushButton:hover { background-color:#00d2a8; }");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setContentsMargins(12, 6, 12, 12);
    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addSpacing(8);
    btnRow->addWidget(adjustBtn);

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLay = new QVBoxLayout(contentWidget);
    contentLay->setContentsMargins(2, 0, 2, 0);
    contentLay->setSpacing(0);
    contentLay->addWidget(tabs, 1);
    contentLay->addLayout(btnRow);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->addWidget(topBar, 0, Qt::AlignTop);
    mainLay->addWidget(contentWidget, 1);
    setLayout(mainLay);

    connect(cancelBtn, &QPushButton::clicked, this, &AdjustNetworkDialog::onCancelClicked);
    connect(adjustBtn, &QPushButton::clicked, this, &AdjustNetworkDialog::onAdjustClicked);
}

void AdjustNetworkDialog::buildTitleBar()
{
    iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/images/images/surveypod.png"));
    iconLabel->setFixedSize(20, 20);
    iconLabel->setScaledContents(true);

    titleLabel = new QLabel("Adjust Network");
    titleLabel->setObjectName("guidetitle");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/cross.svg"));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setFixedSize(22, 22);
    closeBtn->setFlat(true);
    closeBtn->setAutoDefault(false);
    closeBtn->setObjectName("guideclosebtn");
    closeBtn->setStyleSheet( "QPushButton { background-color:white; }" "QPushButton:hover { background-color:#dddddd; }");
    connect(closeBtn, &QPushButton::clicked, this, &AdjustNetworkDialog::onCancelClicked);

    QHBoxLayout *tbl = new QHBoxLayout();
    tbl->setContentsMargins(5, 0, 5, 0);
    tbl->addWidget(iconLabel);
    tbl->addSpacing(6);
    tbl->addWidget(titleLabel);
    tbl->addStretch();
    tbl->addWidget(closeBtn);

    topBar = new QWidget();
    topBar->setLayout(tbl);
    topBar->setObjectName("guideTitleBar");
    topBar->setStyleSheet("background-color:#404040; border:none;");
    topBar->setFixedHeight(36);
}

void AdjustNetworkDialog::buildSubnetworkTab()
{
    QWidget *page = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(12, 12, 12, 8);
    lay->setSpacing(8);

    subnetSummaryLabel = new QLabel();
    subnetSummaryLabel->setStyleSheet("color:#555; font-size:11px;");

    subnetTable = new QTableWidget();
    subnetTable->setColumnCount(COL_COUNT);
    subnetTable->setHorizontalHeaderLabels({
        "", "Subnetwork", "Stations", "Mode", "Status"
    });
    subnetTable->horizontalHeader()->setSectionResizeMode(COL_CHECK,  QHeaderView::Fixed);
    subnetTable->horizontalHeader()->setSectionResizeMode(COL_NAME,   QHeaderView::ResizeToContents);
    subnetTable->horizontalHeader()->setSectionResizeMode(COL_STNS,   QHeaderView::Stretch);
    subnetTable->horizontalHeader()->setSectionResizeMode(COL_MODE,   QHeaderView::ResizeToContents);
    subnetTable->horizontalHeader()->setSectionResizeMode(COL_STATUS, QHeaderView::ResizeToContents);
    subnetTable->setColumnWidth(COL_CHECK, 40);
    subnetTable->verticalHeader()->setVisible(false);
    subnetTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    subnetTable->setSelectionMode(QAbstractItemView::NoSelection);
    subnetTable->setAlternatingRowColors(true);
    subnetTable->setStyleSheet(R"(
        QTableWidget { border:1px solid #ddd; gridline-color:#eee; }
        QHeaderView::section {
            background-color:#f5f5f5;
            border:none; border-bottom:1px solid #ddd;
            padding:4px; font-weight:bold; font-size:11px;
        }
        QTableWidget::item { padding:4px; font-size:11px; }
    )");

    populateSubnetworkTable();

    lay->addWidget(subnetSummaryLabel);
    lay->addWidget(subnetTable, 1);
    tabs->addTab(page, "Subnetworks");
}

void AdjustNetworkDialog::populateSubnetworkTable()
{
    subnetTable->setRowCount(0);
    const QVector<SubnetworkInfo> &subs = projectContext->subnetworks;

    int total = subs.size();
    int adjusted = 0;
    for (const SubnetworkInfo &s : subs) if (s.hasResult) ++adjusted;

    subnetSummaryLabel->setText(QString("%1 subnetwork%2 detected.%3") .arg(total) .arg(total == 1 ? "" : "s") .arg(adjusted > 0
                     ? QString("  %1 already adjusted.").arg(adjusted)
                     : ""));

    subnetTable->setRowCount(total);
    subnetTable->setIconSize(QSize(16, 16));

    for (int r = 0; r < total; ++r) {
        const SubnetworkInfo &s = subs[r];
        subnetTable->setRowHeight(r, 36);

        CustomCheckBox *cb = new CustomCheckBox("", true);
        QWidget *cbw = new QWidget();
        QHBoxLayout *cbl = new QHBoxLayout(cbw);
        cbl->setContentsMargins(4, 0, 4, 0);
        cbl->setAlignment(Qt::AlignCenter);
        cbl->addWidget(cb);
        cb->setProperty("subnetIndex", s.index);
        subnetTable->setCellWidget(r, COL_CHECK, cbw);

        auto *nameItem = new QTableWidgetItem(QString("Subnetwork %1").arg(s.index));
        nameItem->setData(Qt::UserRole, s.index);
        nameItem->setTextAlignment(Qt::AlignCenter);
        subnetTable->setItem(r, COL_NAME, nameItem);

        QSet<QString> uniqueStationIds;
        for (const QString &uid : s.stationUIDs) {
            const QString &sid = projectContext->stations.value(uid).stationId;
            uniqueStationIds.insert(sid.isEmpty() ? uid : sid);
        }
        QStringList stNames = uniqueStationIds.values();
        stNames.sort();

        QString stText = QString("%1 station%2: %3") .arg(stNames.size()) .arg(stNames.size() == 1 ? "" : "s") .arg(stNames.join(", "));
        auto *stItem = new QTableWidgetItem(stText);
        stItem->setToolTip(stNames.join("\n"));
        subnetTable->setItem(r, COL_STNS, stItem);

        auto *modeItem = new QTableWidgetItem(s.isConstrained ? "Constrained" : "Free Network");
        modeItem->setTextAlignment(Qt::AlignCenter);
        if (s.isConstrained)
            modeItem->setForeground(QBrush(QColor("#0984e3")));
        else
            modeItem->setForeground(QBrush(QColor("#6c5ce7")));
        subnetTable->setItem(r, COL_MODE, modeItem);

        auto *statusItem = new QTableWidgetItem( s.hasResult ? "✓  Adjusted" : " — ");
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(s.hasResult ? QBrush(QColor("#00b894")) : QBrush(QColor("#b2bec3")));
        subnetTable->setItem(r, COL_STATUS, statusItem);
    }
}

void AdjustNetworkDialog::buildWeightingTab()
{
    QWidget *page = new QWidget();
    QVBoxLayout *outerLay = new QVBoxLayout(page);
    outerLay->setContentsMargins(20, 16, 20, 16);
    outerLay->setSpacing(14);

    useCovCheck = new CustomCheckBox("Use baseline covariance matrices", options.useCovariance);


    QLabel *aPrioriLabel = new QLabel("A priori scalar:");
    aPrioriLabel->setStyleSheet("font-size:12px;");
    aPrioriSpin = new QDoubleSpinBox();
    aPrioriSpin->setRange(0.1, 10.0);
    aPrioriSpin->setSingleStep(0.1);
    aPrioriSpin->setDecimals(3);
    aPrioriSpin->setValue(options.aPrioriScalar);
    aPrioriSpin->setFixedWidth(100);
    aPrioriSpin->setToolTip(
        "Multiplies all observation weights.\n"
        "1.000 = use as-is (recommended).\n"
        "Increase to trust observations more than their stated precision.");

    QLabel *sigmaHLabel = new QLabel("Horizontal σ:");
    sigmaHLabel->setStyleSheet("font-size:12px;");
    sigmaHSpin = new QDoubleSpinBox();
    sigmaHSpin->setRange(0.001, 1.0);
    sigmaHSpin->setSingleStep(0.001);
    sigmaHSpin->setDecimals(4);
    sigmaHSpin->setSuffix("  m");
    sigmaHSpin->setValue(options.defaultSigmaH);
    sigmaHSpin->setFixedWidth(100);
    sigmaHSpin->setToolTip("Fallback horizontal precision for ECEF X and Y components.");

    QLabel *sigmaVLabel = new QLabel("Vertical σ:");
    sigmaVLabel->setStyleSheet("font-size:12px;");
    sigmaVSpin = new QDoubleSpinBox();
    sigmaVSpin->setRange(0.001, 1.0);
    sigmaVSpin->setSingleStep(0.001);
    sigmaVSpin->setDecimals(4);
    sigmaVSpin->setSuffix("  m");
    sigmaVSpin->setValue(options.defaultSigmaV);
    sigmaVSpin->setFixedWidth(100);
    sigmaVSpin->setToolTip("Fallback vertical precision for ECEF Z component.");

    QLabel *antennaerrorLabel = new QLabel("Antenna height error : ");
    antennaHeightErrorSpin = new QDoubleSpinBox();
    antennaHeightErrorSpin->setRange(0.0001, 0.01);
    antennaHeightErrorSpin->setSingleStep(0.0001);
    antennaHeightErrorSpin->setDecimals(4);
    antennaHeightErrorSpin->setSuffix(" m");
    antennaHeightErrorSpin->setValue(options.setupErrors.antennaHeightError);
    antennaHeightErrorSpin->setFixedWidth(120);
    antennaHeightErrorSpin->setToolTip("The estimated error for Surveypod instrument height.");

    QLabel *centeringErrorLabel = new QLabel("Centering error : ");
    centeringErrorSpin = new QDoubleSpinBox();
    centeringErrorSpin->setRange(0.0001, 0.01);
    centeringErrorSpin->setSingleStep(0.0001);
    centeringErrorSpin->setDecimals(4);
    centeringErrorSpin->setSuffix(" m");
    centeringErrorSpin->setValue(options.setupErrors.centeringError);
    centeringErrorSpin->setFixedWidth(120);
    centeringErrorSpin->setToolTip("The estimated plumbing and leveling error for \nSurveypod instrument over a survey point.");

    QHBoxLayout * hlay1 = new QHBoxLayout();
    hlay1->setContentsMargins(0,0,0,0);
    hlay1->addWidget(aPrioriLabel);
    hlay1->addWidget(aPrioriSpin);
    hlay1->addWidget(sigmaHLabel);
    hlay1->addWidget(sigmaHSpin);
    hlay1->addWidget(sigmaVLabel);
    hlay1->addWidget(sigmaVSpin);

    QHBoxLayout *hlay2 = new QHBoxLayout();
    hlay2->addWidget(antennaerrorLabel);
    hlay2->addWidget(antennaHeightErrorSpin);
    hlay2->addSpacing(25);
    hlay2->addWidget(centeringErrorLabel);
    hlay2->addWidget(centeringErrorSpin);

    outerLay->addWidget(useCovCheck);
    outerLay->addSpacing(6);
    outerLay->addLayout(hlay1);
    outerLay->addLayout(hlay2);
    outerLay->addStretch();

    tabs->addTab(page, "Weighting");
}

QVector<int> AdjustNetworkDialog::selectedSubnetworks() const
{
    QVector<int> result;
    for (int r = 0; r < subnetTable->rowCount(); ++r) {
        QWidget *cbw = subnetTable->cellWidget(r, COL_CHECK);
        if (!cbw) continue;
        CustomCheckBox *cb = cbw->findChild<CustomCheckBox *>();
        if (cb && cb->isChecked()) {
            QTableWidgetItem *item = subnetTable->item(r, COL_NAME);
            if (item)
                result.append(item->data(Qt::UserRole).toInt());
        }
    }
    return result;
}

void AdjustNetworkDialog::onAdjustClicked()
{
    if (selectedSubnetworks().isEmpty()) {
        subnetSummaryLabel->setText(
            "<span style='color:#e17055;'>Please select at least one "
            "subnetwork to adjust.</span>");
        return;
    }
    options.useCovariance = useCovCheck->isChecked();
    options.aPrioriScalar = aPrioriSpin->value();
    options.defaultSigmaH = sigmaHSpin->value();
    options.defaultSigmaV = sigmaVSpin->value();
    options.setupErrors.antennaHeightError = antennaHeightErrorSpin->value();
    options.setupErrors.centeringError = centeringErrorSpin->value();

    CovarianceMatrix *cvmat = new CovarianceMatrix(projectContext);
    int result = cvmat->exec();
    if (result == QDialog::Accepted) {
        accept();
    }
}

void AdjustNetworkDialog::onCancelClicked()
{
    reject();
}

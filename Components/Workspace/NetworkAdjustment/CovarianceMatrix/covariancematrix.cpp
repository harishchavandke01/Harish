#include "covariancematrix.h"

CovarianceMatrix::CovarianceMatrix(ProjectContext *_projectContext, QWidget *parent)
    : QDialog(parent), ctx(_projectContext)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(450, 320);
    setObjectName("editOptions");

    icon = new QLabel();
    icon->setPixmap(QPixmap(":/images/images/surveypod.png"));
    icon->setFixedSize(20, 20);
    icon->setScaledContents(true);

    title = new QLabel("Network Adjustment");
    title->setObjectName("guidetitle");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/cross.svg"));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setFixedSize(22, 22);
    closeBtn->setFlat(true);
    closeBtn->setAutoDefault(false);
    closeBtn->setObjectName("guideclosebtn");
    closeBtn->setStyleSheet(
        "QPushButton { background-color:white; }"
        "QPushButton:hover { background-color:#dddddd; }");

    QHBoxLayout *tbl = new QHBoxLayout();
    tbl->setContentsMargins(5, 0, 5, 0);
    tbl->addWidget(icon);
    tbl->addWidget(title);
    tbl->addStretch();
    tbl->addWidget(closeBtn);

    topBar = new QWidget();
    topBar->setLayout(tbl);
    topBar->setObjectName("guideTitleBar");
    topBar->setStyleSheet("background-color:#404040; border:none;");

    header = new QLabel("Set covariance matrices");
    header->setStyleSheet("font-size:16px; font-weight:bold; border:none;");

    tempBaselines = ctx->baselines;
    QStringList baselinesList;
    for(auto &baseline : tempBaselines) {
        baselinesList.append(baseline.baselineId);
    }

    cb = new QComboBox();
    cb->addItems(baselinesList);

    xxlabel = new QLabel("XX : ");
    xylabel = new QLabel("XY : ");
    xzlabel = new QLabel("XZ : ");
    yylabel = new QLabel("YY : ");
    yzlabel = new QLabel("YZ : ");
    zzlabel = new QLabel("ZZ : ");

    xxspin = new QDoubleSpinBox();
    xxspin->setRange(0.0, 1e6);
    xxspin->setDecimals(10);
    xxspin->setSingleStep(0.00001);
    xxspin->setFixedWidth(120);

    xyspin = new QDoubleSpinBox();
    xyspin->setRange(-1e6, 1e6);
    xyspin->setSingleStep(0.00001);
    xyspin->setDecimals(10);
    xyspin->setFixedWidth(120);

    xzspin = new QDoubleSpinBox();
    xzspin->setRange(-1e6, 1e6);
    xzspin->setDecimals(10);
    xzspin->setSingleStep(0.00001);
    xzspin->setFixedWidth(120);

    yyspin = new QDoubleSpinBox();
    yyspin->setRange(0.0, 1e6);
    yyspin->setDecimals(10);
    yyspin->setSingleStep(0.00001);
    yyspin->setFixedWidth(120);

    yzspin = new QDoubleSpinBox();
    yzspin->setRange(-1e6, 1e6);
    yzspin->setDecimals(10);
    yzspin->setSingleStep(0.00001);
    yzspin->setFixedWidth(120);

    zzspin = new QDoubleSpinBox();
    zzspin->setRange(0.0, 1e6);
    zzspin->setDecimals(10);
    zzspin->setSingleStep(0.00001);
    zzspin->setFixedWidth(120);

    QHBoxLayout *hlay1 = new QHBoxLayout();
    hlay1->setContentsMargins(0,0,0,0);
    hlay1->setSpacing(5);
    hlay1->addWidget(xxlabel);
    hlay1->addWidget(xxspin);
    hlay1->addSpacing(20);
    hlay1->addWidget(xylabel);
    hlay1->addWidget(xyspin);

    QHBoxLayout *hlay2 = new QHBoxLayout();
    hlay2->setContentsMargins(0,0,0,0);
    hlay2->setSpacing(5);
    hlay2->addWidget(xzlabel);
    hlay2->addWidget(xzspin);
    hlay2->addSpacing(20);
    hlay2->addWidget(yylabel);
    hlay2->addWidget(yyspin);

    QHBoxLayout *hlay3 = new QHBoxLayout();
    hlay3->setContentsMargins(0,0,0,0);
    hlay3->setSpacing(5);
    hlay3->addWidget(yzlabel);
    hlay3->addWidget(yzspin);
    hlay3->addSpacing(20);
    hlay3->addWidget(zzlabel);
    hlay3->addWidget(zzspin);

    xxlabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    xylabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    xzlabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    yylabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    yzlabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    zzlabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    xxspin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    xyspin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    xzspin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    yyspin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    yzspin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    zzspin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    saveBtn = new QPushButton("Save");
    saveBtn->setObjectName("editOpBtns");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton { background-color:#00b894; }"
        "QPushButton:hover { background-color:#00d2a8; }");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *outerLay = new QVBoxLayout(contentWidget);
    outerLay->setContentsMargins(10, 10, 10, 10);

    outerLay->addWidget(cb);
    outerLay->addSpacing(15);

    outerLay->addLayout(hlay1);
    outerLay->addLayout(hlay2);
    outerLay->addLayout(hlay3);

    outerLay->addSpacing(20);
    outerLay->addWidget(saveBtn, 0, Qt::AlignRight);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->addWidget(topBar, 0, Qt::AlignTop);
    mainLay->addSpacing(15);
    mainLay->addWidget(header,0, Qt::AlignHCenter);
    mainLay->addSpacing(15);
    mainLay->addWidget(contentWidget);
    mainLay->addStretch();
    setLayout(mainLay);

    connect(closeBtn, &QPushButton::clicked, this, &CovarianceMatrix::onClose);
    connect(saveBtn, &QPushButton::clicked, this, &CovarianceMatrix::onSaveClicked);
    connect(cb, &QComboBox::currentTextChanged, this, &CovarianceMatrix::onBaselineChanged);
    if (cb->count() > 0) {
        onBaselineChanged(cb->currentText());
    }
}

void CovarianceMatrix::onBaselineChanged(const QString &baselineId)
{
    if (!currentBaselineId.isEmpty()) {
        for (auto &bl : tempBaselines) {
            if (bl.baselineId == currentBaselineId) {
                bl.cov[0][0] = xxspin->value();
                bl.cov[0][1] = xyspin->value();
                bl.cov[0][2] = xzspin->value();
                bl.cov[1][1] = yyspin->value();
                bl.cov[1][2] = yzspin->value();
                bl.cov[2][2] = zzspin->value();

                bl.cov[1][0] = bl.cov[0][1];
                bl.cov[2][0] = bl.cov[0][2];
                bl.cov[2][1] = bl.cov[1][2];
                break;
            }
        }
    }

    for (auto &bl : tempBaselines) {
        if (bl.baselineId == baselineId) {
            xxspin->setValue(bl.cov[0][0]);
            xyspin->setValue(bl.cov[0][1]);
            xzspin->setValue(bl.cov[0][2]);
            yyspin->setValue(bl.cov[1][1]);
            yzspin->setValue(bl.cov[1][2]);
            zzspin->setValue(bl.cov[2][2]);
            break;
        }
    }
    currentBaselineId = baselineId;
}

void CovarianceMatrix::onClose()
{
    reject();
}

void CovarianceMatrix::onSaveClicked()
{
    onBaselineChanged(currentBaselineId);
    ctx->baselines = tempBaselines;
    accept();
}

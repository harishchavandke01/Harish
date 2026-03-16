#include "rinexconversion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDebug>
#include <QGroupBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include "../../Utils/BaselineUtils/baselineutils.h"
#include "../../Utils/custommessagebox.h"
#include "../../backend/baselineprocessing.h"


RinexConversion::RinexConversion(QWidget *parent): QWidget(parent)
{
    setUpFilesLayout();
    setUpOptionsLayout();

    mainWidget = new QWidget(this);
    mainWidget->setObjectName("rnxMainWidget");

    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(5);
    mainLayout->addWidget(filesInput, 0, Qt::AlignLeft);
    mainLayout->addWidget(convertOptions, 1);

    setLayout(mainLayout);
}

void RinexConversion::setUpFilesLayout()
{
    rinexConversionLabel = new QLabel("RINEX Conversion");
    rinexConversionLabel->setObjectName("rinexConversionLabel");
    rinexConversionLabel->setStyleSheet("font-size:22px; font-weight:bold;");

    inputUbxLabel = new QLabel("UBX input file:");
    inputUbxLabel->setObjectName("inputUbxLabel");

    ubxInputFileLE = new QLineEdit();
    ubxInputFileLE->setReadOnly(true);
    ubxInputFileLE->setObjectName("ubxInputFileLE");

    ubxPickBtn = new QPushButton();
    ubxPickBtn->setObjectName("ubxPickBtn");
    ubxPickBtn->setToolTip("Select UBX file");
    ubxPickBtn->setIcon(QIcon(":/images/images/link.svg"));

    QWidget *ubxWidget = new QWidget();
    QHBoxLayout *ubxl = new QHBoxLayout(ubxWidget);
    ubxl->setContentsMargins(0,0,0,0);
    ubxl->setSpacing(6);
    ubxl->addWidget(ubxInputFileLE, 1);
    ubxl->addWidget(ubxPickBtn, 0, Qt::AlignLeft);

    inputDirLabel = new QLabel("Output directory:");
    inputDirLabel->setObjectName("inputDirLabel");

    outputDirLE = new QLineEdit();
    outputDirLE->setObjectName("outputDirLE");
    outputDirLE->setReadOnly(true);

    outputDirPickBtn = new QPushButton();
    outputDirPickBtn->setObjectName("outputDirPickBtn");
    outputDirPickBtn->setToolTip("Select output directory");
    outputDirPickBtn->setIcon(QIcon(":/images/images/link.svg"));

    QWidget *outDirWidget = new QWidget();
    QHBoxLayout *outDirLayout = new QHBoxLayout(outDirWidget);
    outDirLayout->setContentsMargins(0,0,0,0);
    outDirLayout->setSpacing(6);
    outDirLayout->addWidget(outputDirLE, 1);
    outDirLayout->addWidget(outputDirPickBtn, 0, Qt::AlignLeft);

    rinexObsLabel = new QLabel("RINEX observation file:");
    rinexObsLabel->setObjectName("rinexObsLabel");

    obsPathLE = new QLineEdit();
    obsPathLE->setObjectName("obsPathLE");

    rinexNavLabel = new QLabel("RINEX navigation file:");
    rinexNavLabel->setObjectName("rinexNavLabel");

    navPathLE = new QLineEdit();
    navPathLE->setObjectName("navPathLE");

    formatLabel = new QLabel("RINEX version:");
    formatLabel->setObjectName("formatLabel");

    formatCB = new QComboBox();
    formatCB->addItem("RINEX 3.04");

    convertBtn = new QPushButton("Convert");
    convertBtn->setObjectName("convertBtn");

    filesInput = new QWidget();
    filesInput->setObjectName("rnxFilesInput");

    QVBoxLayout *flay = new QVBoxLayout(filesInput);
    flay->setContentsMargins(16,16,16,16);
    flay->setSpacing(6);

    flay->addWidget(rinexConversionLabel, 0, Qt::AlignCenter);
    flay->addSpacing(14);

    flay->addWidget(inputUbxLabel);
    flay->addWidget(ubxWidget);
    flay->addSpacing(8);

    flay->addWidget(inputDirLabel);
    flay->addWidget(outDirWidget);
    flay->addSpacing(8);

    flay->addWidget(rinexObsLabel);
    flay->addWidget(obsPathLE);
    flay->addSpacing(8);

    flay->addWidget(rinexNavLabel);
    flay->addWidget(navPathLE);
    flay->addSpacing(8);

    flay->addWidget(formatLabel);
    flay->addWidget(formatCB);
    flay->addSpacing(8);

    flay->addWidget(convertBtn, 0, Qt::AlignRight);
    flay->addStretch();

    connect(ubxPickBtn, &QPushButton::clicked, this, &RinexConversion::onUbxClicked);
    connect(outputDirPickBtn, &QPushButton::clicked, this, &RinexConversion::onOutputDirClicked);
    connect(convertBtn, &QPushButton::clicked, this, &RinexConversion::onConvertBtnClicked);
}

void RinexConversion::setUpOptionsLayout()
{
    convertOptions = new QWidget();
    convertOptions->setObjectName("rnxConvertOptions");
    QVBoxLayout *olay = new QVBoxLayout(convertOptions);
    olay->setContentsMargins(50,16,50,16);
    olay->setSpacing(10);


    heading = new QLabel("Convert Settings");
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");

    QGroupBox *logsbox = new QGroupBox("Logs Duration");
    logsbox->setAlignment(Qt::AlignLeft);
    logsbox->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );

    QGridLayout *logslay = new QGridLayout();
    logslay->setContentsMargins(10, 10, 10, 10);
    logslay->setVerticalSpacing(5);
    logslay->setHorizontalSpacing(10);

    tscheckbox = new CustomCheckBox("Time Start (IST)", false);
    logslay->addWidget(tscheckbox, 0, 0);
    QDateTime defaultdt(QDate(2000, 1, 1), QTime(0, 0, 0));
    ts = new QDateTimeEdit(defaultdt);
    ts->setObjectName("dtedit");
    ts->setFixedHeight(25);
    ts->setDisplayFormat("dd-MM-yyyy HH:mm:ss");
    ts->setCalendarPopup(true);
    ts->setEnabled(false);
    logslay->addWidget(ts, 1, 0);
    connect(tscheckbox, &CustomCheckBox::toggled, this, [&]() {
        ts->setEnabled(tscheckbox->isChecked());
    });
    techeckbox = new CustomCheckBox("Time End (IST)", false);
    logslay->addWidget(techeckbox, 0, 1);
    te = new QDateTimeEdit(defaultdt);
    te->setObjectName("dtedit");
    te->setFixedHeight(25);
    te->setDisplayFormat("dd-MM-yyyy HH:mm:ss");
    te->setCalendarPopup(true);
    te->setEnabled(false);
    logslay->addWidget(te, 1, 1);
    connect(techeckbox, &CustomCheckBox::toggled, this, [&]() {
        te->setEnabled(techeckbox->isChecked());
    });
    ticheckbox = new CustomCheckBox("Time Interval (Secs)", false);
    logslay->addWidget(ticheckbox, 0, 2);
    ti = new QComboBox();
    ti->setFixedHeight(25);
    ti->addItems({"0", "0.05", "0.1", "0.2", "0.25", "0.5", "1", "2", "5", "10", "15", "30", "60"});
    ti->setEnabled(false);
    logslay->addWidget(ti, 1, 2);
    connect(ticheckbox, &CustomCheckBox::toggled, this, [&]() {
        ti->setEnabled(ticheckbox->isChecked());
    });
    logsbox->setLayout(logslay);


    QGridLayout *glay = new QGridLayout();
    glay->setColumnStretch(0, 1);
    glay->setColumnStretch(1, 1);
    glay->setColumnStretch(2, 1);
    glay->setColumnStretch(3, 1);
    glay->setColumnStretch(4, 1);
    glay->setContentsMargins(0, 0, 0, 0);
    glay->setVerticalSpacing(2);
    glay->setHorizontalSpacing(10);

    lab1 = new QLabel("Format : ");
    lab1->setFixedHeight(25);
    lab1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab1->setStyleSheet("font-size: 12px;");
    glay->addWidget(lab1, 0, 0);
    format = new QComboBox();
    format->setStyleSheet("padding-left: 4px;");
    format->setFixedHeight(25);
    format->addItem("UBX");
    glay->addWidget(format,1,0);

    lab2 = new QLabel("Receiver : ");
    lab2->setFixedHeight(25);
    lab2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab2->setStyleSheet("font-size: 12px; ");
    glay->addWidget(lab2, 0, 1);
    receiver = new QComboBox();
    receiver->addItems({"PPK", "Surveypod"});
    receiver->setStyleSheet("padding-left: 4px;");
    receiver->setFixedHeight(25);
    glay->addWidget(receiver, 1, 1);

    lab3 = new QLabel("Marker Name : ");
    lab3->setFixedHeight(25);
    lab3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab3->setStyleSheet("font-size: 12px;");
    glay->addWidget(lab3, 0, 2);
    mname = new QLineEdit();
    mname->setStyleSheet( "QLineEdit { color: black; padding-left: 4px; }" "QLineEdit::placeholder { color: gray; }" );
    mname->setFixedHeight(25);
    QRegularExpression rx("^[A-Za-z0-9 _\\-\\.\\+\\!\\@\\#\\$\\%\\^\\&\\*\\=\\:\\;\\,\\.\\<\\>\\?\\|]*$");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    mname->setValidator(validator);
    mname->setPlaceholderText("Not allowed chars : ~(){}[]`\"'\\/");
    glay->addWidget(mname, 1, 2);

    QString leStyle = R"(
        QLineEdit {
            padding-left: 4px;
            color: #202020;
        }
        QLineEdit::placeholder {
            color: #9e9e9e;
        }
        )";

    lab4 = new QLabel("Antenna Name : ");
    lab4->setFixedHeight(25);
    lab4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab4->setStyleSheet("font-size: 12px;");
    glay->addWidget(lab4, 0, 3);
    antname = new QLineEdit();
    antname->setStyleSheet(leStyle);
    antname->setFixedHeight(25);
    antname->setValidator(validator);
    antname->setPlaceholderText("Not allowed chars : ~(){}[]`\"'\\/");
    glay->addWidget(antname, 1, 3);

    lab5 = new QLabel("Sep Nav : ");
    lab5->setFixedHeight(25);
    lab5->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab5->setStyleSheet("font-size: 12px;");
    glay->addWidget(lab5, 0, 4);
    sepnav = new CustomCheckBox("Sep Nav", false);
    glay->addWidget(sepnav, 1, 4);
    QWidget * contentGridWidget = new QWidget();
    contentGridWidget->setLayout(glay);


    QGroupBox *posgroup = new QGroupBox("Approx Pos XYZ");
    posgroup->setAlignment(Qt::AlignLeft);
    posgroup->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );

    posx = new QLineEdit();
    posx->setStyleSheet(leStyle);
    posx->setFixedHeight(25);
    posx->setPlaceholderText("X");
    posy = new QLineEdit();
    posy->setStyleSheet(leStyle);
    posy->setFixedHeight(25);
    posy->setPlaceholderText("Y");
    posz = new QLineEdit();
    posz->setStyleSheet(leStyle);
    posz->setFixedHeight(25);
    posz->setPlaceholderText("Z");
    QGridLayout *posl = new QGridLayout();
    posl->setContentsMargins(10, 10, 10, 10);
    posl->setHorizontalSpacing(10);
    posl->addWidget(posx, 0, 0);
    posl->addWidget(posy, 0, 1);
    posl->addWidget(posz, 0, 2);
    posgroup->setLayout(posl);

    QGroupBox *polegroup = new QGroupBox("Pole Pos H/E/N");
    polegroup->setAlignment(Qt::AlignLeft);
    polegroup->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );
    poleh = new QLineEdit();
    poleh->setStyleSheet(leStyle);
    poleh->setFixedHeight(25);
    poleh->setPlaceholderText("Height");
    polee = new QLineEdit();
    polee->setStyleSheet(leStyle);
    polee->setFixedHeight(25);
    polee->setPlaceholderText("Easting");
    polen = new QLineEdit();
    polen->setStyleSheet(leStyle);
    polen->setFixedHeight(25);
    polen->setPlaceholderText("Northing");
    QGridLayout *polel = new QGridLayout();
    polel->setContentsMargins(10, 10, 10, 10);
    polel->setHorizontalSpacing(10);
    polel->addWidget(poleh, 0, 0);
    polel->addWidget(polee, 0, 1);
    polel->addWidget(polen, 0, 2);
    polegroup->setLayout(polel);

    QWidget *widget1 = new QWidget();
    QHBoxLayout *hbl1 = new QHBoxLayout(widget1);
    hbl1->setContentsMargins(0,0,0,0);
    hbl1->setSpacing(20);
    hbl1->addWidget(posgroup);
    hbl1->addWidget(polegroup);




    QGroupBox *corrgroup = new QGroupBox("Correction Option");
    corrgroup->setAlignment(Qt::AlignLeft);
    corrgroup->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );
    phase = new CustomCheckBox("Phase Shift", false);
    halfc = new CustomCheckBox("Half cyc", false);
    sort = new CustomCheckBox("Sort", false);
    iono = new CustomCheckBox("Iono corr", false);
    timec = new CustomCheckBox("Time corr", false);
    leap = new CustomCheckBox("Leap sec", false);

    QHBoxLayout *corrlay = new QHBoxLayout();
    corrlay->setContentsMargins(5, 5, 5, 5);
    corrlay->addWidget(phase);
    corrlay->addSpacing(5);
    corrlay->addWidget(halfc);
    corrlay->addSpacing(5);
    corrlay->addWidget(sort);
    corrlay->addSpacing(5);
    corrlay->addWidget(iono);
    corrlay->addSpacing(5);
    corrlay->addWidget(timec);
    corrlay->addSpacing(5);
    corrlay->addWidget(leap);
    corrgroup->setLayout(corrlay);

    QGroupBox *ssgroup = new QGroupBox("Satellite System");
    ssgroup->setAlignment(Qt::AlignLeft);
    ssgroup->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );
    gps = new CustomCheckBox("GPS", true);
    glonass = new CustomCheckBox("Glonass", true);
    galileo = new CustomCheckBox("Galileo", true);
    qzss = new CustomCheckBox("QZSS", true);
    beidou = new CustomCheckBox("Beidou", true);
    sbs = new CustomCheckBox("SBS", true);
    navic = new CustomCheckBox("NavIC", true);

    QHBoxLayout *sslay = new QHBoxLayout();
    sslay->setContentsMargins(5, 5, 5, 5);
    sslay->addWidget(gps);
    sslay->addSpacing(5);
    sslay->addWidget(glonass);
    sslay->addSpacing(5);
    sslay->addWidget(galileo);
    sslay->addSpacing(5);
    sslay->addWidget(qzss);
    sslay->addSpacing(5);
    sslay->addWidget(beidou);
    sslay->addSpacing(5);
    sslay->addWidget(sbs);
    sslay->addSpacing(5);
    sslay->addWidget(navic);
    ssgroup->setLayout(sslay);


    QGroupBox *signalsgroup = new QGroupBox("GNSS Signals");
    signalsgroup->setAlignment(Qt::AlignLeft);
    signalsgroup->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );
    L1_G1_E1_B1 = new CustomCheckBox("L1/G1/E1/B1", true);
    L2_G2_E5b_B2b = new CustomCheckBox("L2/G2/E5b/B2b", true);
    L5_G3_E5a_B2a = new CustomCheckBox("L5/G3/E5a/B2a", true);
    L6_E6_B3 = new CustomCheckBox("L6/E6/B3", true);
    E5ab_B1C = new CustomCheckBox("E5ab/B1C", true);
    B2ab = new CustomCheckBox("B2ab", true);
    QGridLayout *signallay = new QGridLayout();
    signallay->setContentsMargins(5, 5, 5, 5);
    signallay->setHorizontalSpacing(10);
    signallay->setVerticalSpacing(5);
    signallay->addWidget(L1_G1_E1_B1, 0, 0);
    signallay->addWidget(L2_G2_E5b_B2b, 0, 1);
    signallay->addWidget(L5_G3_E5a_B2a, 0, 2);
    signallay->addWidget(L6_E6_B3, 1, 0);
    signallay->addWidget(E5ab_B1C, 1, 1);
    signallay->addWidget(B2ab, 1, 2);
    mask = new QPushButton("Mask");
    mask->setCursor(Qt::PointingHandCursor);
    mask->setStyleSheet("QPushButton { border: 1px solid gray; color:white; background: black;  min-height: 25px; max-width:60px; border-radius: 5px; } QPushButton:hover { background-color : gray; }");
    signallay->addWidget(mask, 0, 3);
    signalsgroup->setLayout(signallay);

    connect(mask, &QPushButton::clicked, this, [=] {
        signalmask->exec();
    });

    connect(L1_G1_E1_B1, &CustomCheckBox::toggled, this, [=](bool v) {
        signalmask->setVal(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                           L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                           E5ab_B1C->isChecked(), B2ab->isChecked());
    });

    connect(L2_G2_E5b_B2b, &CustomCheckBox::toggled, this, [=](bool v) {
        signalmask->setVal(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                           L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                           E5ab_B1C->isChecked(), B2ab->isChecked());
    });

    connect(L5_G3_E5a_B2a, &CustomCheckBox::toggled, this, [=](bool v) {
        signalmask->setVal(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                           L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                           E5ab_B1C->isChecked(), B2ab->isChecked());
    });

    connect(L6_E6_B3, &CustomCheckBox::toggled, this, [=](bool v) {
        signalmask->setVal(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                           L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                           E5ab_B1C->isChecked(), B2ab->isChecked());
    });

    connect(E5ab_B1C, &CustomCheckBox::toggled, this, [=](bool v) {
        signalmask->setVal(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                           L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                           E5ab_B1C->isChecked(), B2ab->isChecked());
    });

    connect(B2ab, &CustomCheckBox::toggled, this, [=](bool v) {
        signalmask->setVal(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                           L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                           E5ab_B1C->isChecked(), B2ab->isChecked());
    });

    QGridLayout *bottomlay = new QGridLayout();
    bottomlay->setContentsMargins(0, 0, 0, 0);

    round = new CustomCheckBox("Time Rounding", false);
    round->setStyleSheet("padding-left: 4px;");
    round->setFixedHeight(25);
    bottomlay->addWidget(round, 0, 0);

    QGroupBox *exsatgroup = new QGroupBox("Exclude Satellites");
    exsatgroup->setAlignment(Qt::AlignLeft);
    exsatgroup->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    margin-top: 12px;"
        "    border: 1px solid lightgray;"
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    top: 0px;"
        "    padding: 0 3px;"
        "} "
        );
    QHBoxLayout *hlay = new QHBoxLayout();
    hlay->setContentsMargins(5, 5, 5, 5);
    exsat = new QLineEdit();
    exsat->setStyleSheet("color:gray;padding-left: 4px;");
    exsat->setFixedHeight(25);
    exsat->setPlaceholderText("G01-32, R01-24, E01-36, J01-10, S01-39, C01-63, I01-14");
    hlay->addWidget(exsat);
    exsatgroup->setLayout(hlay);
    bottomlay->addWidget(exsatgroup, 0, 1, 1, 3);

    QWidget *bottomwid = new QWidget();
    bottomwid->setLayout(bottomlay);

    setOption();

    std::vector<bool> maskval = setMask();
    signalmask = new SignalsMask(L1_G1_E1_B1->isChecked(), L2_G2_E5b_B2b->isChecked(),
                                 L5_G3_E5a_B2a->isChecked(), L6_E6_B3->isChecked(),
                                 E5ab_B1C->isChecked(), B2ab->isChecked(), maskval);

    QWidget *bwid = new QWidget();
    bwid->setStyleSheet("border: none;");
    QHBoxLayout *blay = new QHBoxLayout();
    blay->setContentsMargins(10, 10, 10, 10);
    blay->addStretch(1);
    savebtn = new QPushButton("Save");
    savebtn->setObjectName("convertOpBtns");
    savebtn->setCursor(Qt::PointingHandCursor);
    savebtn->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");
    blay->addWidget(savebtn);
    resetbtn = new QPushButton("Reset");
    resetbtn->setObjectName("convertOpBtns");
    resetbtn->setCursor(Qt::PointingHandCursor);
    resetbtn->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");
    blay->addWidget(resetbtn);
    blay->addStretch(1);
    bwid->setLayout(blay);

    connect(savebtn, &QPushButton::clicked, this, [&]() {
        saveOption();
        CustomMessageBox *mb = new CustomMessageBox("INFO", "Saved successfully.", "OK");
        mb->exec();
    });
    connect(resetbtn, &QPushButton::clicked, this, [&]() {
        setOption(false);
        CustomMessageBox *mb = new CustomMessageBox("INFO", "Reset successfully.", "OK");
        mb->exec();
    });


    olay->addWidget(heading,0,Qt::AlignHCenter);
    olay->addSpacing(10);
    olay->addWidget(logsbox,0);
    olay->addWidget(contentGridWidget);
    olay->addWidget(widget1);
    olay->addWidget(corrgroup);
    olay->addWidget(ssgroup);
    olay->addWidget(signalsgroup);
    olay->addWidget(bottomwid);
    olay->addWidget(bwid,0, Qt::AlignRight);
    olay->addStretch();
}

void RinexConversion::setProjectFolder(const QString &_projectFolder)
{
    projectFolder=_projectFolder;
    outputDirLE->setText(projectFolder+"/observations");
}

void RinexConversion::onUbxClicked()
{
    QString file = QFileDialog::getOpenFileName(
        this, tr("Select UBX file"), projectFolder, tr("UBX Files (*.ubx);"));
    if (!file.isEmpty())
        ubxInputFileLE->setText(file);

    QString fileName = QFileInfo(file).completeBaseName();
    obsPathLE->setText(outputDirLE->text()+"/"+fileName+".obs");
    navPathLE->setText(outputDirLE->text()+"/"+fileName+".nav");
}

void RinexConversion::onOutputDirClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select output directory"), projectFolder);
    if (!dir.isEmpty())
        outputDirLE->setText(dir);

    QString fileName = QFileInfo(ubxInputFileLE->text()).completeBaseName();
    if(!fileName.isEmpty()){
        obsPathLE->setText(dir+"/"+fileName+".obs");
        navPathLE->setText(dir+"/"+fileName+".nav");
    }
}

void RinexConversion::onConvertBtnClicked()
{
    if(ubxInputFileLE->text().isEmpty()) {
        CustomMessageBox *mb = new CustomMessageBox("ERROR", "UBX file not selected.\nPlease select a file.", "OK");
        mb->exec();
        return;
    }

    QString obsfile = QFileInfo(obsPathLE->text()).fileName();
    QString navfile = QFileInfo(navPathLE->text()).fileName();


    QDateTime st, et;
    if(tscheckbox->isChecked()) {
        st = ts->dateTime();
    }
    if(techeckbox->isChecked()) {
        et = te->dateTime();
    }
    double tint = NULL;
    if(ticheckbox->isChecked()) {
        tint = ti->currentText().toDouble();
    }

    saveOption();
    if(receiver->currentText() == "Surveypod") {
        if(mname->text() == "") {
            CustomMessageBox *mb = new CustomMessageBox("ERROR", "Marker name is mandatory for Surveypod.", "OK");
            mb->exec();
            return;
        }
        if(poleh->text() == "") {
            CustomMessageBox *mb = new CustomMessageBox("ERROR", "Pole Height is mandatory for Surveypod.", "OK");
            mb->exec();
            return;
        }
        if(polee->text() == "") {
            CustomMessageBox *mb = new CustomMessageBox("ERROR", "Pole Easting is mandatory for Surveypod.", "OK");
            mb->exec();
            return;
        }
        if(polen->text() == "") {
            CustomMessageBox *mb = new CustomMessageBox("ERROR", "Pole Northing is mandatory for Surveypod.", "OK");
            mb->exec();
            return;
        }
    }

    BaselineProcessing *baselineProcessing = new BaselineProcessing();
    baselineProcessing->RinexConversion(ubxInputFileLE->text().replace("/","\\").toStdString(),
                                   outputDirLE->text().replace("/","\\").toStdString(),
                                   obsfile.toStdString(), navfile.toStdString(), st, et, tint);

    connect(baselineProcessing, &BaselineProcessing::Done, this, [&, obsfile]() {
        BaselineUtils *util = new BaselineUtils();
        std::pair<std::pair<int, QString>, ObsData*> od = util->ReadObsFile(outputDirLE->text()+"\\"+obsfile);
        QDateTime st = od.second->ISTStartTime;
        QString y = QString::number(st.date().year()%100);
        if(y == "0") {
            CustomMessageBox *mb = new CustomMessageBox("ERROR", "No valid conversion data found.\nPlease check your input data.", "OK");
            mb->exec();
            return;
        }
        QFileInfo info(obsfile);
        QString bname = info.baseName();

        auto Rename = [](QString src, QString tar) -> void {
            if(!QFile::exists(src)) return;
            if(QFile::exists(tar)) QFile::remove(tar);
            QFile::rename(src, tar);
        };

        Rename(outputDirLE->text() + "\\" + obsfile,
               outputDirLE->text() + "\\" + bname + "." + y + "o");

        QString ofile = bname + "." + y + "o";

        Rename(outputDirLE->text() + "\\" + bname + ".nav",
               outputDirLE->text() + "\\" + bname + "." + y + "n");

        Rename(outputDirLE->text() + "\\" + bname + ".gnav",
               outputDirLE->text() + "\\" + bname + "." + y + "g");

        Rename(outputDirLE->text() + "\\" + bname + ".hnav",
               outputDirLE->text() + "\\" + bname + "." + y + "h");

        Rename(outputDirLE->text() + "\\" + bname + ".qnav",
               outputDirLE->text() + "\\" + bname + "." + y + "q");

        Rename(outputDirLE->text() + "\\" + bname + ".lnav",
               outputDirLE->text() + "\\" + bname + "." + y + "l");

        Rename(outputDirLE->text() + "\\" + bname + ".cnav",
               outputDirLE->text() + "\\" + bname + "." + y + "c");

        Rename(outputDirLE->text() + "\\" + bname + ".inav",
               outputDirLE->text() + "\\" + bname + "." + y + "i");
    });

}

void RinexConversion::setOption(bool val) {
    ConvertUserPreference *userPreference = new ConvertUserPreference();
    if(val) {
        BaselineUtils *util = new BaselineUtils();
        UserPreference *userPreference1 = util->LoadConfig(userPreference);
        userPreference = dynamic_cast<ConvertUserPreference*>(userPreference1);
    }
    gps->setVal(userPreference->GPS);
    glonass->setVal(userPreference->GLO);
    galileo->setVal(userPreference->GAL);
    qzss->setVal(userPreference->QZSS);
    beidou->setVal(userPreference->BEI);
    sbs->setVal(userPreference->SBS);
    navic->setVal(userPreference->NavIC);
    sepnav->setVal(userPreference->sepnav);
    receiver->setCurrentText(userPreference->receiver);
    mname->setText(userPreference->marker);
    antname->setText(userPreference->antenna);
    posx->setText(userPreference->appx != 1e9 ? QString::number(userPreference->appx) : "");
    posy->setText(userPreference->appy != 1e9 ? QString::number(userPreference->appy) : "");
    posz->setText(userPreference->appz != 1e9 ? QString::number(userPreference->appz) : "");
    poleh->setText(userPreference->poleh != 1e9 ? QString::number(userPreference->poleh) : "");
    polee->setText(userPreference->polee != 1e9 ? QString::number(userPreference->polee) : "");
    polen->setText(userPreference->polen != 1e9 ? QString::number(userPreference->polen) : "");
    phase->setVal(userPreference->phase);
    halfc->setVal(userPreference->halfc);
    sort->setVal(userPreference->sort);
    iono->setVal(userPreference->iono);
    timec->setVal(userPreference->timecorr);
    leap->setVal(userPreference->leap);
    L1_G1_E1_B1->setVal(userPreference->sigband1);
    L2_G2_E5b_B2b->setVal(userPreference->sigband2);
    L5_G3_E5a_B2a->setVal(userPreference->sigband3);
    L6_E6_B3->setVal(userPreference->sigband4);
    E5ab_B1C->setVal(userPreference->sigband5);
    B2ab->setVal(userPreference->sigband6);
    round->setVal(userPreference->round);
    exsat->setText(userPreference->exsat);
    update();
}

std::vector<bool> RinexConversion::setMask() {
    ConvertUserPreference *userPreference = new ConvertUserPreference();
    BaselineUtils *util = new BaselineUtils();
    UserPreference *userPreference1 = util->LoadConfig(userPreference);
    userPreference = dynamic_cast<ConvertUserPreference*>(userPreference1);
    QStringList list = {
        "g1c", "g1p", "g1w", "g1y", "g1m", "g1n", "g1s", "g1l", "g1x",
        "g2c", "g2d", "g2s", "g2l", "g2x", "g2p", "g2w", "g2y", "g2m", "g2n",
        "g5i", "g5q", "g5x",
        "r1c", "r1p", "r2c", "r2p", "r3i", "r3q", "r3x", "r4a", "r4b", "r4x",
        "r6a", "r6b", "r6x",
        "e1c", "e1a", "e1b", "e1x", "e1z",
        "e5i", "e5q", "e5x", "e6a", "e6b", "e6c", "e6x", "e6z",
        "e7i", "e7q", "e7x", "e8i", "e8q", "e8x",
        "q1c", "q1s", "q1l", "q1x", "q1z", "q1e", "q1b", "q2s", "q2l", "q2x",
        "q5i", "q5q", "q5x", "q5d", "q5p", "q5z", "q6s", "q6l", "q6x", "q6e", "q6z",
        "b2i", "b2q", "b2x", "b7i", "b7q", "b7x", "b6i", "b6q", "b6x",
        "b1d", "b1p", "b1s", "b1x", "b1l", "b1z", "b5d", "b5p", "b5x",
        "b7d", "b7p", "b7z", "b8d", "b8p", "b8x", "b6d", "b6p", "b6z",
        "i5a", "i5b", "i5c", "i5x", "i9a", "i9b", "i9c", "i1x", "i1d",
        "s1c", "s5i", "s5q", "s5x"
    };
    std::vector<bool> maskval;
    for(auto &i : list) {
        if(userPreference->gnsssignals.find(i) != userPreference->gnsssignals.end()) {
            maskval.push_back(userPreference->gnsssignals[i]);
        } else maskval.push_back(true);
    }
    return maskval;
}

void RinexConversion::saveOption() {
    ConvertUserPreference *userPreference = new ConvertUserPreference();
    BaselineUtils *util = new BaselineUtils();
    UserPreference *userPreference1 = util->LoadConfig(userPreference);
    userPreference = dynamic_cast<ConvertUserPreference*>(userPreference1);

    userPreference->GPS = gps->isChecked();
    userPreference->GLO = glonass->isChecked();
    userPreference->GAL = galileo->isChecked();
    userPreference->QZSS = qzss->isChecked();
    userPreference->BEI = beidou->isChecked();
    userPreference->SBS = sbs->isChecked();
    userPreference->NavIC = navic->isChecked();
    userPreference->sepnav = sepnav->isChecked();
    userPreference->receiver = receiver->currentText();
    userPreference->marker = mname->text();
    userPreference->antenna = antname->text();
    userPreference->appx = posx->text() == "" ? 1e9 : posx->text().toDouble();
    userPreference->appy = posy->text() == "" ? 1e9 : posy->text().toDouble();
    userPreference->appz = posz->text() == "" ? 1e9 : posz->text().toDouble();
    userPreference->poleh = poleh->text() == "" ? 1e9 : poleh->text().toDouble();
    userPreference->polee = polee->text() == "" ? 1e9 : polee->text().toDouble();
    userPreference->polen = polen->text() == "" ? 1e9 : polen->text().toDouble();
    userPreference->phase = phase->isChecked();
    userPreference->halfc = halfc->isChecked();
    userPreference->sort = sort->isChecked();
    userPreference->iono = iono->isChecked();
    userPreference->timecorr = timec->isChecked();
    userPreference->leap = leap->isChecked();
    userPreference->sigband1 = L1_G1_E1_B1->isChecked();
    userPreference->sigband2 = L2_G2_E5b_B2b->isChecked();
    userPreference->sigband3 = L5_G3_E5a_B2a->isChecked();
    userPreference->sigband4 = L6_E6_B3->isChecked();
    userPreference->sigband5 = E5ab_B1C->isChecked();
    userPreference->sigband6 = B2ab->isChecked();
    userPreference->round = round->isChecked();
    userPreference->exsat = exsat->text();

    QStringList list = {
        "g1c", "g1p", "g1w", "g1y", "g1m", "g1n", "g1s", "g1l", "g1x",
        "g2c", "g2d", "g2s", "g2l", "g2x", "g2p", "g2w", "g2y", "g2m", "g2n",
        "g5i", "g5q", "g5x",
        "r1c", "r1p", "r2c", "r2p", "r3i", "r3q", "r3x", "r4a", "r4b", "r4x",
        "r6a", "r6b", "r6x",
        "e1c", "e1a", "e1b", "e1x", "e1z",
        "e5i", "e5q", "e5x", "e6a", "e6b", "e6c", "e6x", "e6z",
        "e7i", "e7q", "e7x", "e8i", "e8q", "e8x",
        "q1c", "q1s", "q1l", "q1x", "q1z", "q1e", "q1b", "q2s", "q2l", "q2x",
        "q5i", "q5q", "q5x", "q5d", "q5p", "q5z", "q6s", "q6l", "q6x", "q6e", "q6z",
        "b2i", "b2q", "b2x", "b7i", "b7q", "b7x", "b6i", "b6q", "b6x",
        "b1d", "b1p", "b1s", "b1x", "b1l", "b1z", "b5d", "b5p", "b5x",
        "b7d", "b7p", "b7z", "b8d", "b8p", "b8x", "b6d", "b6p", "b6z",
        "i5a", "i5b", "i5c", "i5x", "i9a", "i9b", "i9c", "i1x", "i1d",
        "s1c", "s5i", "s5q", "s5x"
    };

    for(auto &j : list) {
        if(signalmask) {
            if(signalmask->mp.find(j) != signalmask->mp.end()) {
                userPreference->gnsssignals[j] = signalmask->mp[j]->isChecked();
            }
        }
    }

    util->SaveConfig(userPreference);
}



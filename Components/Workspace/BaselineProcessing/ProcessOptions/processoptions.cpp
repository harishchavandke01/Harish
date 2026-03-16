#include "processoptions.h"
#include <QHBoxLayout>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QFileDialog>
#include "../../../Utils/BaselineUtils/baselinedata.h"
#include "../../../Utils/BaselineUtils/baselineutils.h"
#include "../../../Utils/custommessagebox.h"
ProcessOptions::ProcessOptions()
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(580, 530);
    setObjectName("processOptions");

    icon = new QLabel();
    icon->setPixmap(QPixmap(":/images/images/surveypod.png"));
    icon->setFixedSize(20,20);
    icon->setScaledContents(true);

    title = new QLabel("Baseline Processing");
    title->setObjectName("guidetitle");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/cross.svg"));
    closeBtn->setIconSize(QSize(16,16));
    closeBtn->setFixedSize(22,22);
    closeBtn->setFlat(true);
    closeBtn->setAutoDefault(false);
    closeBtn->setObjectName("guideclosebtn");
    closeBtn->setStyleSheet("QPushButton { background-color:white; } QPushButton:hover { background-color: #dddddd; }");

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(5,0,5,0);
    hlayout->addWidget(icon);
    hlayout->addWidget(title);
    hlayout->addStretch();
    hlayout->addWidget(closeBtn);

    topBar = new QWidget();
    topBar->setLayout(hlayout);
    topBar->setObjectName("guideTitleBar");
    topBar->setStyleSheet("background-color: #404040; border: none;");

    heading = new QLabel("Process Options");
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");

    QGridLayout *glay = new QGridLayout();
    glay->setAlignment(Qt::AlignCenter);
    glay->setHorizontalSpacing(10);
    glay->setVerticalSpacing(15);
    glay->setContentsMargins(10, 10, 10, 10);

    lab1 = new QLabel("Output Folder : ");
    lab1->setFixedHeight(30);
    lab1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab1->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab1, 0, 0);

    outputDir = new QLineEdit();
    outputDir->setObjectName("outputDir");
    outputDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    outputDir->setEnabled(false);
    glay->addWidget(outputDir, 0, 1, 1, 3);

    lab2 = new QLabel("Position Mode : ");
    lab2->setFixedHeight(30);
    lab2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab2->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab2, 2, 0);

    pmode = new QComboBox();
    pmode->setFixedHeight(30);
    pmode->addItems({"Static"});
    glay->addWidget(pmode, 2, 1);

    lab3 = new QLabel("Frequencies : ");
    lab3->setFixedHeight(30);
    lab3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab3->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab3, 2, 2);

    freq = new QComboBox();
    freq->setFixedHeight(30);
    freq->addItems({"L1", "L1 + L2", "L1 + L2 + L3", "L1 + L2 + L3 + L4"});
    glay->addWidget(freq, 2, 3);

    lab4 = new QLabel("Filter Type : ");
    lab4->setFixedHeight(30);
    lab4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab4->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab4, 3, 0);

    filter = new QComboBox();
    filter->setFixedHeight(30);
    filter->addItems({"Forward", "Backward", "Combined", "Combined No Phase"});
    glay->addWidget(filter, 3, 1);

    lab5 = new QLabel("Elevation Type : ");
    lab5->setFixedHeight(30);
    lab5->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab5->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab5, 3, 2);

    ele = new QComboBox();
    ele->setFixedHeight(30);
    ele->addItems({"Ellipsoidal", "MSL EGM 96", "MSL EGM 08"});
    glay->addWidget(ele, 3, 3);

    lab6 = new QLabel("Integer Ambiguity ");
    lab6->setFixedHeight(30);
    lab6->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab6->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab6, 4, 0);

    lab7 = new QLabel("GPS : ");
    lab7->setFixedHeight(30);
    lab7->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab7->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab7, 4, 1);

    lab8 = new QLabel("Glonass : ");
    lab8->setFixedHeight(30);
    lab8->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab8->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab8, 4, 2);

    lab9 = new QLabel("Beidou : ");
    lab9->setFixedHeight(30);
    lab9->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab9->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab9, 4, 3);

    lab8_1 = new QLabel("Resolution : ");
    lab8_1->setFixedHeight(30);
    lab8_1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab8_1->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab8_1, 5, 0);

    gpsamb = new QComboBox();
    gpsamb->setFixedHeight(30);
    gpsamb->addItems({"Off", "Continuous", "Instantaneous", "Fix and Hold"});
    glay->addWidget(gpsamb, 5, 1);

    gloamb = new QComboBox();
    gloamb->setFixedHeight(30);
    gloamb->addItems({"Off", "On", "Autocal", "Fix and Hold"});
    glay->addWidget(gloamb, 5, 2);

    beiamb = new QComboBox();
    beiamb->setFixedHeight(30);
    beiamb->addItems({"Off", "On"});
    glay->addWidget(beiamb, 5, 3);

    lab10 = new QLabel("Elevation Mask : ");
    lab10->setFixedHeight(30);
    lab10->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab10->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab10, 6, 0);

    elemask = new QComboBox();
    elemask->setFixedHeight(30);
    elemask->addItems({"0", "5", "10", "15", "20", "25", "30", "35"});
    glay->addWidget(elemask, 6, 1);

    lab11 = new QLabel("Satellite System : ");
    lab11->setFixedHeight(30);
    lab11->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab11->setStyleSheet("font-size: 14px; font-weight: bold;");
    glay->addWidget(lab11, 7, 0);

    gps = new CustomCheckBox("GPS", true);
    glay->addWidget(gps, 7, 1);
    glonass = new CustomCheckBox("GLONASS", true);
    glay->addWidget(glonass, 7, 2);
    galileo = new CustomCheckBox("GALILEO", true);
    glay->addWidget(galileo, 7 ,3);
    qzss = new CustomCheckBox("QZSS", true);
    glay->addWidget(qzss, 8, 1);
    beidou = new CustomCheckBox("BEIDOU", true);
    glay->addWidget(beidou, 8, 2);
    sbs = new CustomCheckBox("SBS", true);
    glay->addWidget(sbs, 8, 3);

    setOption();
    savebtn = new QPushButton("Save");
    savebtn->setObjectName("processOpBtns");
    savebtn->setCursor(Qt::PointingHandCursor);
    savebtn->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");
    glay->addWidget(savebtn, 9, 1);

    resetbtn = new QPushButton("Reset");
    resetbtn->setObjectName("processOpBtns");
    resetbtn->setCursor(Qt::PointingHandCursor);
    resetbtn->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");
    glay->addWidget(resetbtn, 9, 2);

    contentWidget = new QWidget();
    contentWidget->setLayout(glay);
    contentWidget->setStyleSheet("border: none;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 10);
    layout->setSpacing(0);
    layout->addWidget(topBar);
    layout->addSpacing(5);
    layout->addWidget(heading, 0, Qt::AlignHCenter);
    layout->addSpacing(5);
    layout->addWidget(contentWidget);
    layout->addStretch();

    connect(closeBtn, &QPushButton::clicked, this, &ProcessOptions::Close);
    connect(savebtn, &QPushButton::clicked, this, [&]() {
        PostProcessUserPreference *userPreference = new PostProcessUserPreference();
        userPreference->GPS = gps->isChecked();
        userPreference->GLO = glonass->isChecked();
        userPreference->GAL = galileo->isChecked();
        userPreference->QZSS = qzss->isChecked();
        userPreference->BEI = beidou->isChecked();
        userPreference->SBS = sbs->isChecked();
        if(pmode->currentText() == "Static") userPreference->posMode = PosMode::Static;
        else userPreference->posMode = PosMode::Kinematic;

        if(freq->currentText() == "L1") userPreference->frequency = Frequency::L1;
        else if(freq->currentText() == "L1 + L2") userPreference->frequency = Frequency::L1_L2;
        else if(freq->currentText() == "L1 + L2 + L3") userPreference->frequency = Frequency::L1_L2_L3;
        else userPreference->frequency = Frequency::L1_L2_L3_L4;

        qDebug()<<freq->currentText()<<"in the UI";

        if(filter->currentText() == "Forward") userPreference->filterType = FilterType::Forward;
        else if(filter->currentText() == "Backward") userPreference->filterType = FilterType::Backward;
        else if(filter->currentText() == "Combined") userPreference->filterType = FilterType::Combined;
        else userPreference->filterType = FilterType::Combined_no_phase_reset;

        if(ele->currentText() == "Ellipsoidal") userPreference->elevationType = ElevationType::Ellipsoidal;
        else if(ele->currentText() == "MSL EGM 96") userPreference->elevationType = ElevationType::MSL_EGM96;
        else userPreference->elevationType = ElevationType::MSL_ESM08;

        if(gpsamb->currentText() == "Off") userPreference->gpsAmbiguity = GPSAmbiguity::Off_gps;
        else if(gpsamb->currentText() == "Continuous") userPreference->gpsAmbiguity = GPSAmbiguity::Continuous_gps;
        else if(gpsamb->currentText() == "Instantaneous") userPreference->gpsAmbiguity = GPSAmbiguity::Instantaneous_gps;
        else userPreference->gpsAmbiguity = GPSAmbiguity::Fix_and_hold_gps;

        if(gloamb->currentText() == "Off") userPreference->gloAmbiguity = GlonassAmbiguity::Off_glo;
        else if(gloamb->currentText() == "On") userPreference->gloAmbiguity = GlonassAmbiguity::On_glo;
        else if(gloamb->currentText() == "Autocal") userPreference->gloAmbiguity = GlonassAmbiguity::Autocal_glo;
        else userPreference->gloAmbiguity = GlonassAmbiguity::Fix_and_hold_glo;

        if(beiamb->currentText() == "Off") userPreference->bdsAmbiguity = BDSAmbiguity::Off_bds;
        else userPreference->bdsAmbiguity = BDSAmbiguity::On_bds;

        userPreference->elevationMask = elemask->currentText().toInt();

        BaselineUtils *util = new BaselineUtils();
        util->SaveConfig(userPreference);
        CustomMessageBox *mb = new CustomMessageBox("INFO", "Saved successfully.", "OK",this);
        mb->exec();
    });
    connect(resetbtn, &QPushButton::clicked, this, [&]() {
        setOption(false);
        CustomMessageBox *mb = new CustomMessageBox("INFO", "Reset successfully.", "OK", this);
        mb->exec();
    });
}

void ProcessOptions::setOption(bool val) {
    PostProcessUserPreference *userPreference = new PostProcessUserPreference();
    if(val) {
        BaselineUtils *util = new BaselineUtils();
        UserPreference *userPreference1 = util->LoadConfig(userPreference);
        userPreference = dynamic_cast<PostProcessUserPreference*>(userPreference1);
    }
    gps->setVal(userPreference->GPS);
    glonass->setVal(userPreference->GLO);
    galileo->setVal(userPreference->GAL);
    qzss->setVal(userPreference->QZSS);
    beidou->setVal(userPreference->BEI);
    sbs->setVal(userPreference->SBS);
    elemask->setCurrentText(QString::number(userPreference->elevationMask));
    pmode->setCurrentIndex(userPreference->posMode);
    freq->setCurrentIndex(userPreference->frequency);
    filter->setCurrentIndex(userPreference->filterType);
    ele->setCurrentIndex(userPreference->elevationType);
    gpsamb->setCurrentIndex(userPreference->gpsAmbiguity);
    gloamb->setCurrentIndex(userPreference->gloAmbiguity);
    beiamb->setCurrentIndex(userPreference->bdsAmbiguity);
    delete userPreference;
    update();
}

void ProcessOptions::setOp(QString dir) {
    outputDir->setText(dir);
}

void ProcessOptions::Close() {
    this->close();
}

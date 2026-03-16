#include "editoptions.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDoubleValidator>
#include <QFileDialog>
#include "../../../Utils/BaselineUtils/baselineutils.h"
#include "../../../Utils/custommessagebox.h"

EditOptions::EditOptions(const QString &_obsPath, QWidget *parent) : obsPath(_obsPath), QDialog{parent}
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(520, 560);
    setObjectName("editOptions");

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

    connect(closeBtn, &QPushButton::clicked, this, &EditOptions::Close);
    QString leStyle = R"(
        QLineEdit {
            padding-left: 4px;
            color: #202020;
        }
        QLineEdit::placeholder {
            color: #9e9e9e;
        }
        )";

    heading = new QLabel("Edit Options");
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");

    QWidget *contentWidget = new QWidget();
    QGridLayout *glay = new QGridLayout(contentWidget);
    glay->setContentsMargins(15,10,15,10);

    QRegularExpression rx("^[A-Za-z0-9 _\\-\\.\\+\\!\\@\\#\\$\\%\\^\\&\\*\\=\\:\\;\\,\\.\\<\\>\\?\\|]*$");
    QValidator *validator = new QRegularExpressionValidator(rx, this);

    QDoubleValidator * numberValidator = new QDoubleValidator();
    numberValidator->setNotation(QDoubleValidator::StandardNotation);
    numberValidator->setDecimals(12);
    numberValidator->setBottom(-1e9);
    numberValidator->setTop(1e9);

    lb1 = new QLabel("Marker Name : ");
    markerName = new QLineEdit();
    markerName->setPlaceholderText("Not allowed chars : ~(){}[]`\"'\\/");
    markerName->setValidator(validator);

    lb2 = new QLabel("Marker Number : ");
    markerNumber = new QLineEdit();

    lb3 = new QLabel("Marker Type : ");
    markerType = new QLineEdit();

    lb4 = new QLabel("Observer : ");
    observer = new QLineEdit();

    lb5 = new QLabel("Antenna Type : ");
    antennaName = new QLineEdit();
    antennaName->setStyleSheet(leStyle);
    antennaName->setPlaceholderText("Not allowed chars : ~(){}[]`\"'\\/");
    antennaName->setValidator(validator);

    lb6 = new QLabel("Receiver : ");
    receiver = new QLineEdit();
    // receiver->addItems({"Surveypod","PPK", "Other"});

    QGroupBox *posgroup = new QGroupBox("Approx Pos XYZ");
    posgroup->setStyleSheet(
        "QGroupBox { "
        "    font-size: 12px; "
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
    posx->setValidator(numberValidator);
    posy = new QLineEdit();
    posy->setStyleSheet(leStyle);
    posy->setFixedHeight(25);
    posy->setPlaceholderText("Y");
    posy->setValidator(numberValidator);
    posz = new QLineEdit();
    posz->setStyleSheet(leStyle);
    posz->setFixedHeight(25);
    posz->setPlaceholderText("Z");
    posz->setValidator(numberValidator);
    QGridLayout *posl = new QGridLayout();
    posl->setContentsMargins(10, 10, 10, 10);
    posl->setHorizontalSpacing(10);
    posl->addWidget(posx, 0, 0);
    posl->addWidget(posy, 0, 1);
    posl->addWidget(posz, 0, 2);
    posgroup->setLayout(posl);

    QGroupBox *polegroup = new QGroupBox("Pole Pos H/E/N");
    polegroup->setStyleSheet(
        "QGroupBox { "
        "    font-size: 12px; "
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
    poleh->setValidator(numberValidator);
    polee = new QLineEdit();
    polee->setStyleSheet(leStyle);
    polee->setFixedHeight(25);
    polee->setPlaceholderText("Easting");
    polee->setValidator(numberValidator);
    polen = new QLineEdit();
    polen->setStyleSheet(leStyle);
    polen->setFixedHeight(25);
    polen->setPlaceholderText("Northing");
    polen->setValidator(numberValidator);
    QGridLayout *polel = new QGridLayout();
    polel->setContentsMargins(10, 10, 10, 10);
    polel->setHorizontalSpacing(10);
    polel->addWidget(poleh, 0, 0);
    polel->addWidget(polee, 0, 1);
    polel->addWidget(polen, 0, 2);
    polegroup->setLayout(polel);


    QGroupBox *baseStationGroup = new QGroupBox("Base Station Coordinates");
    baseStationGroup->setStyleSheet(
        "QGroupBox { "
        "    font-size: 12px; "
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
    lat = new QLineEdit();
    lat->setStyleSheet(leStyle);
    lat->setFixedHeight(27);
    lat->setPlaceholderText("Latitude");
    lat->setValidator(numberValidator);
    lon = new QLineEdit();
    lon->setStyleSheet(leStyle);
    lon->setFixedHeight(27);
    lon->setPlaceholderText("Longitude");
    lon->setValidator(numberValidator);
    height = new QLineEdit();
    height->setStyleSheet(leStyle);
    height->setFixedHeight(27);
    height->setPlaceholderText("Height");
    height->setValidator(numberValidator);
    baseJson = new QPushButton("Base Json");
    baseJson->setStyleSheet("QPushButton { border: 1px solid gray; color:white; background: black;  min-height: 25px; min-width:60px; max-width:80px; border-radius: 5px; } QPushButton:hover { background-color : gray; }");
    QGridLayout *basel = new QGridLayout();
    basel->setContentsMargins(10, 10, 10, 10);
    basel->setHorizontalSpacing(10);
    basel->addWidget(lat, 0, 0);
    basel->addWidget(lon, 0, 1);
    basel->addWidget(height, 0, 2);
    basel->addWidget(baseJson,1,0,1,3,Qt::AlignRight);
    baseStationGroup->setLayout(basel);
    baseStationGroup->setToolTip(
        "Leave fields empty or set to 0 to use base coordinates "
        "from the RINEX header.");

    QWidget *bwid = new QWidget();
    bwid->setStyleSheet("border: none;");
    QHBoxLayout *blay = new QHBoxLayout(bwid);
    blay->setContentsMargins(0, 20, 0, 0);

    save = new QPushButton("Save");
    save->setObjectName("editOpBtns");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");
    blay->addWidget(save);

    reset = new QPushButton("Reset");
    reset->setObjectName("editOpBtns");
    reset->setCursor(Qt::PointingHandCursor);
    reset->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");
    blay->addWidget(reset);

    glay->addWidget(lb1, 0, 0);
    glay->addWidget(lb2, 0, 1);

    glay->addWidget(markerName,   1, 0);
    glay->addWidget(markerNumber, 1, 1);

    glay->addWidget(lb3, 2, 0);
    glay->addWidget(lb4, 2, 1);

    glay->addWidget(markerType, 3, 0);
    glay->addWidget(observer,   3, 1);

    glay->addWidget(lb5, 4, 0);
    glay->addWidget(lb6, 4, 1);

    glay->addWidget(antennaName, 5, 0);
    glay->addWidget(receiver,    5, 1);

    glay->addWidget(posgroup,6,0,1,2);
    glay->addWidget(polegroup,7,0,1,2);

    glay->addWidget(baseStationGroup,8,0,1,2);

    glay->addWidget(bwid, 9,0,1,2,Qt::AlignRight);


    QVBoxLayout *mainl = new QVBoxLayout();
    mainl->setContentsMargins(0,0,0,0);
    mainl->addWidget(topBar,0,Qt::AlignTop);
    mainl->addSpacing(10);
    mainl->addWidget(heading,0,Qt::AlignHCenter);
    mainl->addWidget(contentWidget);
    mainl->addStretch();
    setLayout(mainl);

    onResetClicked();

    connect(baseJson, &QPushButton::clicked, this, [&]() {
        QString file = QFileDialog::getOpenFileName(nullptr, "Select Base Json", "", "Json File (*.json)");
        if(file != "") {
            setbasedata(file);
        }
    });
    connect(save, &QPushButton::clicked, this, &EditOptions::onSaveClicked);
    connect(reset, &QPushButton::clicked, this, &EditOptions::onResetClicked);
}

void EditOptions::setbasedata(QString file) {
    BaselineUtils *util = new BaselineUtils();
    std::pair<std::pair<int, QString>, BaseJsonData*> data = util->ReadBaseJsonFile(file);
    lat->setText(QString::number(data.second->latitude, 'f', 10));
    lon->setText(QString::number(data.second->longitude, 'f', 10));
    height->setText(QString::number(data.second->elevation));
}

void EditOptions::Close()
{
    this->close();
}

void EditOptions::onSaveClicked()
{
    QJsonObject header;
    QString udpatedMarkerName = markerName->text().trimmed();
    header["marker_name"]   = udpatedMarkerName;
    header["marker_number"] = markerNumber->text().trimmed();
    header["marker_type"]   = markerType->text().trimmed();
    header["observer"]      = observer->text().trimmed();
    header["antenna_type"]  = antennaName->text().trimmed();
    header["receiver_type"]      = receiver->text();

    QJsonObject posxyz;
    posxyz["X"] = posx->text().toDouble();
    posxyz["Y"] = posy->text().toDouble();
    posxyz["Z"] = posz->text().toDouble();
    header["finalposxyz"] = posxyz;

    QJsonObject polehen;
    polehen["H"] = poleh->text().toDouble();
    polehen["E"] = polee->text().toDouble();
    polehen["N"] = polen->text().toDouble();
    header["poleHEN"] = polehen;

    QJsonObject basellh;
    basellh["height"] = height->text().toDouble();
    basellh["latitude"] = lat->text().toDouble();
    basellh["longitude"] = lon->text().toDouble();
    header["basellh"] = basellh;
    utils->updateRinexHeader(obsPath, header);

    CustomMessageBox *mb = new CustomMessageBox("INFO","All values saved successfully.","OK",this);
    mb->exec();
    emit pointIdEdited(obsPath, udpatedMarkerName);
}

void EditOptions::onResetClicked()
{
    QJsonObject data = utils->readRinexHeader(obsPath);
    markerName->setText(data["marker_name"].toString());
    markerNumber->setText(data["marker_number"].toString());
    markerType->setText(data["marker_type"].toString());
    observer->setText(data["observer"].toString());
    antennaName->setText(data["antenna_type"].toString());
    receiver->setText(data["receiver_type"].toString());

    auto xyz = data["finalposxyz"].toObject();
    posx->setText(QString::number(xyz["X"].toDouble(),'f',4));
    posy->setText(QString::number(xyz["Y"].toDouble(),'f',4));
    posz->setText(QString::number(xyz["Z"].toDouble(),'f',4));

    auto hen = data["poleHEN"].toObject();
    poleh->setText(QString::number(hen["H"].toDouble(),'f',4));
    polee->setText(QString::number(hen["E"].toDouble(),'f',4));
    polen->setText(QString::number(hen["N"].toDouble(),'f',4));

    auto basellh = data["basellh"].toObject();
    lat->setText(QString::number(basellh["latitude"].toDouble(),'g',10));
    lon->setText(QString::number(basellh["longitude"].toDouble(),'g',10));
    height->setText(QString::number(basellh["height"].toDouble(),'g',4));
}

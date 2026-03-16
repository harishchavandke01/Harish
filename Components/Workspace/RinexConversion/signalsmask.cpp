#include "signalsmask.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include "../../Utils/BaselineUtils/baselineutils.h"

SignalsMask::SignalsMask(bool s1, bool s2, bool s3, bool s4, bool s5, bool s6, std::vector<bool> maskval) {
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("border: 2px solid black;");
    setFixedSize(680, 450);

    icon = new QLabel();
    icon->setPixmap(QPixmap(":/images/images/surveypod.png"));
    icon->setFixedSize(20,20);
    icon->setScaledContents(true);

    title = new QLabel("Signal Mask");
    title->setObjectName("guidetitle");

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon(":/images/images/cross.svg"));
    closeBtn->setIconSize(QSize(16,16));
    closeBtn->setFixedSize(22,22);
    closeBtn->setFlat(true);
    closeBtn->setAutoDefault(false);
    closeBtn->setObjectName("guideclosebtn");
    closeBtn->setStyleSheet("QPushButton { background-color:white; } QPushButton:hover { background-color: #dddddd; }");
    connect(closeBtn, &QPushButton::clicked, this, &SignalsMask::Close);

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

    QGroupBox *gpsbox = new QGroupBox("GPS");
    gpsbox->setAlignment(Qt::AlignLeft);
    gpsbox->setStyleSheet(
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

    int i = 0;

    QGridLayout *gpslay = new QGridLayout();
    gpslay->setContentsMargins(5, 5, 5, 5);
    gpslay->setSpacing(0);
    mp["g1c"] = new CustomCheckBox("1C", maskval[i++], s1);
    mp["g1p"] = new CustomCheckBox("1P", maskval[i++], s1);
    mp["g1w"] = new CustomCheckBox("1W", maskval[i++], s1);
    mp["g1y"] = new CustomCheckBox("1Y", maskval[i++], s1);
    mp["g1m"] = new CustomCheckBox("1M", maskval[i++], s1);
    mp["g1n"] = new CustomCheckBox("1N", maskval[i++], s1);
    mp["g1s"] = new CustomCheckBox("1S", maskval[i++], s1);
    mp["g1l"] = new CustomCheckBox("1L", maskval[i++], s1);
    mp["g1x"] = new CustomCheckBox("1X", maskval[i++], s1);
    mp["g2c"] = new CustomCheckBox("2C", maskval[i++], s2);
    mp["g2d"] = new CustomCheckBox("2D", maskval[i++], s2);
    mp["g2s"] = new CustomCheckBox("2S", maskval[i++], s2);
    mp["g2l"] = new CustomCheckBox("2L", maskval[i++], s2);
    mp["g2x"] = new CustomCheckBox("2X", maskval[i++], s2);
    mp["g2p"] = new CustomCheckBox("2P", maskval[i++], s2);
    mp["g2w"] = new CustomCheckBox("2W", maskval[i++], s2);
    mp["g2y"] = new CustomCheckBox("2Y", maskval[i++], s2);
    mp["g2m"] = new CustomCheckBox("2M", maskval[i++], s2);
    mp["g2n"] = new CustomCheckBox("2N", maskval[i++], s2);
    mp["g5i"] = new CustomCheckBox("5I", maskval[i++], s3);
    mp["g5q"] = new CustomCheckBox("5Q", maskval[i++], s3);
    mp["g5x"] = new CustomCheckBox("5X", maskval[i++], s3);
    gpslay->addWidget(mp["g1c"], 0, 0);
    gpslay->addWidget(mp["g1p"], 0, 1);
    gpslay->addWidget(mp["g1w"], 0, 2);
    gpslay->addWidget(mp["g1y"], 0, 3);
    gpslay->addWidget(mp["g1m"], 0, 4);
    gpslay->addWidget(mp["g1n"], 0, 5);
    gpslay->addWidget(mp["g1s"], 0, 6);
    gpslay->addWidget(mp["g1l"], 0, 7);
    gpslay->addWidget(mp["g1x"], 0, 8);
    gpslay->addWidget(mp["g2c"], 1, 0);
    gpslay->addWidget(mp["g2d"], 1, 1);
    gpslay->addWidget(mp["g2s"], 1, 2);
    gpslay->addWidget(mp["g2l"], 1, 3);
    gpslay->addWidget(mp["g2x"], 1, 4);
    gpslay->addWidget(mp["g2p"], 1, 5);
    gpslay->addWidget(mp["g2w"], 1, 6);
    gpslay->addWidget(mp["g2y"], 1, 7);
    gpslay->addWidget(mp["g2m"], 1, 8);
    gpslay->addWidget(mp["g2n"], 1, 9);
    gpslay->addWidget(mp["g5i"], 2, 0);
    gpslay->addWidget(mp["g5q"], 2, 1);
    gpslay->addWidget(mp["g5x"], 2, 2);
    gpsbox->setLayout(gpslay);

    QGroupBox *globox = new QGroupBox("Glonass");
    globox->setAlignment(Qt::AlignLeft);
    globox->setStyleSheet(
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

    QGridLayout *glolay = new QGridLayout();
    glolay->setContentsMargins(5, 5, 5, 5);
    glolay->setSpacing(0);
    mp["r1c"] = new CustomCheckBox("1C", maskval[i++], s1);
    mp["r1p"] = new CustomCheckBox("1P", maskval[i++], s1);
    mp["r2c"] = new CustomCheckBox("2C", maskval[i++], s2);
    mp["r2p"] = new CustomCheckBox("2P", maskval[i++], s2);
    mp["r3i"] = new CustomCheckBox("3I", maskval[i++], s3);
    mp["r3q"] = new CustomCheckBox("3Q", maskval[i++], s3);
    mp["r3x"] = new CustomCheckBox("3X", maskval[i++], s3);
    mp["r4a"] = new CustomCheckBox("4A", maskval[i++], s1);
    mp["r4b"] = new CustomCheckBox("4B", maskval[i++], s1);
    mp["r4x"] = new CustomCheckBox("4X", maskval[i++], s1);
    mp["r6a"] = new CustomCheckBox("6A", maskval[i++], s2);
    mp["r6b"] = new CustomCheckBox("6B", maskval[i++], s2);
    mp["r6x"] = new CustomCheckBox("6X", maskval[i++], s2);
    glolay->addWidget(mp["r1c"], 0, 0);
    glolay->addWidget(mp["r1p"], 0, 1);
    glolay->addWidget(mp["r2c"], 0, 2);
    glolay->addWidget(mp["r2p"], 0, 3);
    glolay->addWidget(mp["r3i"], 0, 4);
    glolay->addWidget(mp["r3q"], 0, 5);
    glolay->addWidget(mp["r3x"], 0, 6);
    glolay->addWidget(mp["r4a"], 0, 7);
    glolay->addWidget(mp["r4b"], 0, 8);
    glolay->addWidget(mp["r4x"], 0, 9);
    glolay->addWidget(mp["r6a"], 1, 0);
    glolay->addWidget(mp["r6b"], 1, 1);
    glolay->addWidget(mp["r6x"], 1, 2);
    globox->setLayout(glolay);

    QGroupBox *galbox = new QGroupBox("Galileo");
    galbox->setAlignment(Qt::AlignLeft);
    galbox->setStyleSheet(
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

    QGridLayout *gallay = new QGridLayout();
    gallay->setContentsMargins(5, 5, 5, 5);
    gallay->setSpacing(0);
    mp["e1c"] = new CustomCheckBox("1C", maskval[i++], s1);
    mp["e1a"] = new CustomCheckBox("1A", maskval[i++], s1);
    mp["e1b"] = new CustomCheckBox("1B", maskval[i++], s1);
    mp["e1x"] = new CustomCheckBox("1X", maskval[i++], s1);
    mp["e1z"] = new CustomCheckBox("1Z", maskval[i++], s1);
    mp["e5i"] = new CustomCheckBox("5I", maskval[i++], s3);
    mp["e5q"] = new CustomCheckBox("5Q", maskval[i++], s3);
    mp["e5x"] = new CustomCheckBox("5X", maskval[i++], s3);
    mp["e6a"] = new CustomCheckBox("6A", maskval[i++], s4);
    mp["e6b"] = new CustomCheckBox("6B", maskval[i++], s4);
    mp["e6c"] = new CustomCheckBox("6C", maskval[i++], s4);
    mp["e6x"] = new CustomCheckBox("6X", maskval[i++], s4);
    mp["e6z"] = new CustomCheckBox("6Z", maskval[i++], s4);
    mp["e7i"] = new CustomCheckBox("7I", maskval[i++], s2);
    mp["e7q"] = new CustomCheckBox("7Q", maskval[i++], s2);
    mp["e7x"] = new CustomCheckBox("7X", maskval[i++], s2);
    mp["e8i"] = new CustomCheckBox("8I", maskval[i++], s5);
    mp["e8q"] = new CustomCheckBox("8Q", maskval[i++], s5);
    mp["e8x"] = new CustomCheckBox("8X", maskval[i++], s5);
    gallay->addWidget(mp["e1c"], 0, 0);
    gallay->addWidget(mp["e1a"], 0, 1);
    gallay->addWidget(mp["e1b"], 0, 2);
    gallay->addWidget(mp["e1x"], 0, 3);
    gallay->addWidget(mp["e1z"], 0, 4);
    gallay->addWidget(mp["e5i"], 1, 0);
    gallay->addWidget(mp["e5q"], 1, 1);
    gallay->addWidget(mp["e5x"], 1, 2);
    gallay->addWidget(mp["e6a"], 1, 3);
    gallay->addWidget(mp["e6b"], 1, 4);
    gallay->addWidget(mp["e6c"], 1, 5);
    gallay->addWidget(mp["e6x"], 1, 6);
    gallay->addWidget(mp["e6z"], 1, 7);
    gallay->addWidget(mp["e7i"], 2, 0);
    gallay->addWidget(mp["e7q"], 2, 1);
    gallay->addWidget(mp["e7x"], 2, 2);
    gallay->addWidget(mp["e8i"], 2, 3);
    gallay->addWidget(mp["e8q"], 2, 4);
    gallay->addWidget(mp["e8x"], 2, 5);
    galbox->setLayout(gallay);

    QGroupBox *qzssbox = new QGroupBox("QZSS");
    qzssbox->setAlignment(Qt::AlignLeft);
    qzssbox->setStyleSheet(
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

    QGridLayout *qzsslay = new QGridLayout();
    qzsslay->setContentsMargins(5, 5, 5, 5);
    qzsslay->setSpacing(0);
    mp["q1c"] = new CustomCheckBox("1C", maskval[i++], s1);
    mp["q1s"] = new CustomCheckBox("1S", maskval[i++], s1);
    mp["q1l"] = new CustomCheckBox("1L", maskval[i++], s1);
    mp["q1x"] = new CustomCheckBox("1X", maskval[i++], s1);
    mp["q1z"] = new CustomCheckBox("1Z", maskval[i++], s1);
    mp["q1e"] = new CustomCheckBox("1E", maskval[i++], s1);
    mp["q1b"] = new CustomCheckBox("1B", maskval[i++], s1);
    mp["q2s"] = new CustomCheckBox("2S", maskval[i++], s2);
    mp["q2l"] = new CustomCheckBox("2L", maskval[i++], s2);
    mp["q2x"] = new CustomCheckBox("2X", maskval[i++], s2);
    mp["q5i"] = new CustomCheckBox("5I", maskval[i++], s3);
    mp["q5q"] = new CustomCheckBox("5Q", maskval[i++], s3);
    mp["q5x"] = new CustomCheckBox("5X", maskval[i++], s3);
    mp["q5d"] = new CustomCheckBox("5D", maskval[i++], s3);
    mp["q5p"] = new CustomCheckBox("5P", maskval[i++], s3);
    mp["q5z"] = new CustomCheckBox("5Z", maskval[i++], s3);
    mp["q6s"] = new CustomCheckBox("6S", maskval[i++], s4);
    mp["q6l"] = new CustomCheckBox("6L", maskval[i++], s4);
    mp["q6x"] = new CustomCheckBox("6X", maskval[i++], s4);
    mp["q6e"] = new CustomCheckBox("6E", maskval[i++], s4);
    mp["q6z"] = new CustomCheckBox("6Z", maskval[i++], s4);
    qzsslay->addWidget(mp["q1c"], 0, 0);
    qzsslay->addWidget(mp["q1s"], 0, 1);
    qzsslay->addWidget(mp["q1l"], 0, 2);
    qzsslay->addWidget(mp["q1x"], 0, 3);
    qzsslay->addWidget(mp["q1z"], 0, 4);
    qzsslay->addWidget(mp["q1e"], 0, 5);
    qzsslay->addWidget(mp["q1b"], 0, 6);
    qzsslay->addWidget(mp["q2s"], 0, 7);
    qzsslay->addWidget(mp["q2l"], 0, 8);
    qzsslay->addWidget(mp["q2x"], 0, 9);
    qzsslay->addWidget(mp["q5i"], 1, 0);
    qzsslay->addWidget(mp["q5q"], 1, 1);
    qzsslay->addWidget(mp["q5x"], 1, 2);
    qzsslay->addWidget(mp["q5d"], 1, 3);
    qzsslay->addWidget(mp["q5p"], 1, 4);
    qzsslay->addWidget(mp["q5z"], 1, 5);
    qzsslay->addWidget(mp["q6s"], 1, 6);
    qzsslay->addWidget(mp["q6l"], 1, 7);
    qzsslay->addWidget(mp["q6x"], 1, 8);
    qzsslay->addWidget(mp["q6e"], 1, 9);
    qzsslay->addWidget(mp["q6z"], 1, 10);
    qzssbox->setLayout(qzsslay);

    QGroupBox *bdsbox = new QGroupBox("BDS");
    bdsbox->setAlignment(Qt::AlignLeft);
    bdsbox->setStyleSheet(
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

    QGridLayout *bdslay = new QGridLayout();
    bdslay->setContentsMargins(5, 5, 5, 5);
    bdslay->setSpacing(0);
    mp["b2i"] = new CustomCheckBox("2I", maskval[i++], s1);
    mp["b2q"] = new CustomCheckBox("2Q", maskval[i++], s1);
    mp["b2x"] = new CustomCheckBox("2X", maskval[i++], s1);
    mp["b7i"] = new CustomCheckBox("7I", maskval[i++], s2);
    mp["b7q"] = new CustomCheckBox("7Q", maskval[i++], s2);
    mp["b7x"] = new CustomCheckBox("7X", maskval[i++], s2);
    mp["b6i"] = new CustomCheckBox("6I", maskval[i++], s4);
    mp["b6q"] = new CustomCheckBox("6Q", maskval[i++], s4);
    mp["b6x"] = new CustomCheckBox("6X", maskval[i++], s4);
    mp["b1d"] = new CustomCheckBox("1D", maskval[i++], s5);
    mp["b1p"] = new CustomCheckBox("1P", maskval[i++], s5);
    mp["b1x"] = new CustomCheckBox("1X", maskval[i++], s5);
    mp["b1s"] = new CustomCheckBox("1S", maskval[i++], s5);
    mp["b1l"] = new CustomCheckBox("1L", maskval[i++], s5);
    mp["b1z"] = new CustomCheckBox("1Z", maskval[i++], s5);
    mp["b5d"] = new CustomCheckBox("5D", maskval[i++], s3);
    mp["b5p"] = new CustomCheckBox("5P", maskval[i++], s3);
    mp["b5x"] = new CustomCheckBox("5X", maskval[i++], s3);
    mp["b7d"] = new CustomCheckBox("7D", maskval[i++], s2);
    mp["b7p"] = new CustomCheckBox("7P", maskval[i++], s2);
    mp["b7z"] = new CustomCheckBox("7Z", maskval[i++], s2);
    mp["b8d"] = new CustomCheckBox("8D", maskval[i++], s6);
    mp["b8p"] = new CustomCheckBox("8P", maskval[i++], s6);
    mp["b8x"] = new CustomCheckBox("8X", maskval[i++], s6);
    mp["b6d"] = new CustomCheckBox("6D", maskval[i++], s4);
    mp["b6p"] = new CustomCheckBox("6P", maskval[i++], s4);
    mp["b6z"] = new CustomCheckBox("6Z", maskval[i++], s4);
    bdslay->addWidget(mp["b2i"], 0, 0);
    bdslay->addWidget(mp["b2q"], 0, 1);
    bdslay->addWidget(mp["b2x"], 0, 2);
    bdslay->addWidget(mp["b7i"], 0, 3);
    bdslay->addWidget(mp["b7q"], 0, 4);
    bdslay->addWidget(mp["b7x"], 0, 5);
    bdslay->addWidget(mp["b6i"], 0, 6);
    bdslay->addWidget(mp["b6q"], 0, 7);
    bdslay->addWidget(mp["b6x"], 0, 8);
    bdslay->addWidget(mp["b1d"], 1, 0);
    bdslay->addWidget(mp["b1p"], 1, 1);
    bdslay->addWidget(mp["b1x"], 1, 2);
    bdslay->addWidget(mp["b1s"], 1, 3);
    bdslay->addWidget(mp["b1l"], 1, 4);
    bdslay->addWidget(mp["b1z"], 1, 5);
    bdslay->addWidget(mp["b5d"], 1, 6);
    bdslay->addWidget(mp["b5p"], 1, 7);
    bdslay->addWidget(mp["b5x"], 1, 8);
    bdslay->addWidget(mp["b7d"], 2, 0);
    bdslay->addWidget(mp["b7p"], 2, 1);
    bdslay->addWidget(mp["b7z"], 2, 2);
    bdslay->addWidget(mp["b8d"], 2, 3);
    bdslay->addWidget(mp["b8p"], 2, 4);
    bdslay->addWidget(mp["b8x"], 2, 5);
    bdslay->addWidget(mp["b6d"], 2, 6);
    bdslay->addWidget(mp["b6p"], 2, 7);
    bdslay->addWidget(mp["b6z"], 2, 8);
    bdsbox->setLayout(bdslay);

    QGroupBox *navicbox = new QGroupBox("NavIC");
    navicbox->setAlignment(Qt::AlignLeft);
    navicbox->setStyleSheet(
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

    QGridLayout *naviclay = new QGridLayout();
    naviclay->setContentsMargins(5, 5, 5, 5);
    naviclay->setSpacing(0);
    mp["i5a"] = new CustomCheckBox("5A", maskval[i++], s1);
    mp["i5b"] = new CustomCheckBox("5B", maskval[i++], s1);
    mp["i5c"] = new CustomCheckBox("5C", maskval[i++], s1);
    mp["i5x"] = new CustomCheckBox("5X", maskval[i++], s1);
    mp["i9a"] = new CustomCheckBox("9A", maskval[i++], s2);
    mp["i9b"] = new CustomCheckBox("9B", maskval[i++], s2);
    mp["i9c"] = new CustomCheckBox("9C", maskval[i++], s2);
    mp["i1x"] = new CustomCheckBox("1X", maskval[i++], s3);
    mp["i1d"] = new CustomCheckBox("1D", maskval[i++], s2);
    naviclay->addWidget(mp["i5a"], 0, 0);
    naviclay->addWidget(mp["i5b"], 0, 1);
    naviclay->addWidget(mp["i5c"], 0, 2);
    naviclay->addWidget(mp["i5x"], 0, 3);
    naviclay->addWidget(mp["i9a"], 0, 4);
    naviclay->addWidget(mp["i9b"], 0, 5);
    naviclay->addWidget(mp["i9c"], 0, 6);
    naviclay->addWidget(mp["i1x"], 0, 7);
    naviclay->addWidget(mp["i1d"], 0, 8);
    navicbox->setLayout(naviclay);

    QGroupBox *sbasbox = new QGroupBox("SBAS");
    sbasbox->setAlignment(Qt::AlignLeft);
    sbasbox->setStyleSheet(
        "QGroupBox { "
        "    min-width: 250px; "
        "    max-width: 250px; "
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

    QGridLayout *sbaslay = new QGridLayout();
    sbaslay->setContentsMargins(5, 5, 5, 5);
    sbaslay->setSpacing(0);
    mp["s1c"] = new CustomCheckBox("1C", maskval[i++], s1);
    mp["s5i"] = new CustomCheckBox("5I", maskval[i++], s3);
    mp["s5q"] = new CustomCheckBox("5Q", maskval[i++], s3);
    mp["s5x"] = new CustomCheckBox("5X", maskval[i++], s3);
    sbaslay->addWidget(mp["s1c"], 0, 0);
    sbaslay->addWidget(mp["s5i"], 0, 1);
    sbaslay->addWidget(mp["s5q"], 0 ,2);
    sbaslay->addWidget(mp["s5x"], 0, 3);
    sbasbox->setLayout(sbaslay);

    QWidget *brow = new QWidget();
    QHBoxLayout *blay = new QHBoxLayout();
    blay->setContentsMargins(0, 0, 0, 0);
    blay->addWidget(sbasbox);
    blay->addStretch(1);

    set = new QPushButton("Set", this);
    set->setObjectName("singleMaskBtn");
    set->setCursor(Qt::PointingHandCursor);
    set->setStyleSheet("QPushButton { background-color: black; min-width: 80px; max-width: 80px; } QPushButton:hover {background-color: gray;}");
    blay->addWidget(set, 0, Qt::AlignBottom);
    blay->addSpacing(5);

    unset = new QPushButton("Unset", this);
    unset->setObjectName("singleMaskBtn");
    unset->setCursor(Qt::PointingHandCursor);
    unset->setStyleSheet("QPushButton { background-color: black; min-width: 80px; max-width: 80px; } QPushButton:hover {background-color: gray;}");
    blay->addWidget(unset, 0, Qt::AlignBottom);
    blay->addSpacing(5);

    saveBtn = new QPushButton("Save", this);
    saveBtn->setObjectName("singleMaskBtn");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background-color: #00b894; min-width: 80px; max-width: 80px; } QPushButton:hover {background-color: #00d2a8;}");
    blay->addWidget(saveBtn, 0, Qt::AlignBottom);

    brow->setLayout(blay);

    QWidget *w = new QWidget();
    w->setStyleSheet("border: none;");
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setContentsMargins(10, 10, 10, 10);
    lay->addWidget(gpsbox);
    lay->addSpacing(5);
    lay->addWidget(globox);
    lay->addSpacing(5);
    lay->addWidget(galbox);
    lay->addSpacing(5);
    lay->addWidget(qzssbox);
    lay->addSpacing(5);
    lay->addWidget(bdsbox);
    lay->addSpacing(5);
    lay->addWidget(navicbox);
    lay->addSpacing(5);
    lay->addWidget(brow);
    w->setLayout(lay);
    QScrollArea *area = new QScrollArea();
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    area->setStyleSheet(R"(
        QScrollArea {
            border-top: none;
        }
        QScrollBar {
            border: none;
        }
        QScrollBar:vertical {
            background: #f0f0f0;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #888;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            background: #f0f0f0;
            height: 10px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: #888;
            min-width: 20px;
            border-radius: 4px;
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )");
    area->setWidget(w);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(topBar);
    layout->addWidget(area);

    connect(saveBtn, &QPushButton::clicked, this, [=]() {
        ConvertUserPreference *userPreference = new ConvertUserPreference();
        BaselineUtils *util = new BaselineUtils();
        UserPreference *userPreference1 = util->LoadConfig(userPreference);
        userPreference = dynamic_cast<ConvertUserPreference*>(userPreference1);
        for(auto &i : list) {
            userPreference->gnsssignals[i] = mp[i]->isChecked();
        }
        util->SaveConfig(userPreference);
        Close();
    });

    connect(set, &QPushButton::clicked, this, [this]() {
        for(auto &i : list) {
            mp[i]->setVal(true);
        }
        update();
    });

    connect(unset, &QPushButton::clicked, this, [this]() {
        for(auto &i : list) {
            mp[i]->setVal(false);
        }
        update();
    });
}

void SignalsMask::setVal(bool s1, bool s2, bool s3, bool s4, bool s5, bool s6) {
    mp["g1c"]->setEnabled(s1);
    mp["g1p"]->setEnabled(s1);
    mp["g1w"]->setEnabled(s1);
    mp["g1y"]->setEnabled(s1);
    mp["g1m"]->setEnabled(s1);
    mp["g1n"]->setEnabled(s1);
    mp["g1s"]->setEnabled(s1);
    mp["g1l"]->setEnabled(s1);
    mp["g1x"]->setEnabled(s1);
    mp["g2c"]->setEnabled(s2);
    mp["g2d"]->setEnabled(s2);
    mp["g2s"]->setEnabled(s2);
    mp["g2l"]->setEnabled(s2);
    mp["g2x"]->setEnabled(s2);
    mp["g2p"]->setEnabled(s2);
    mp["g2w"]->setEnabled(s2);
    mp["g2y"]->setEnabled(s2);
    mp["g2m"]->setEnabled(s2);
    mp["g2n"]->setEnabled(s2);
    mp["g5i"]->setEnabled(s3);
    mp["g5q"]->setEnabled(s3);
    mp["g5x"]->setEnabled(s3);

    mp["r1c"]->setEnabled(s1);
    mp["r1p"]->setEnabled(s1);
    mp["r2c"]->setEnabled(s2);
    mp["r2p"]->setEnabled(s2);
    mp["r3i"]->setEnabled(s3);
    mp["r3q"]->setEnabled(s3);
    mp["r3x"]->setEnabled(s3);
    mp["r4a"]->setEnabled(s1);
    mp["r4b"]->setEnabled(s1);
    mp["r4x"]->setEnabled(s1);
    mp["r6a"]->setEnabled(s2);
    mp["r6b"]->setEnabled(s2);
    mp["r6x"]->setEnabled(s2);

    mp["e1c"]->setEnabled(s1);
    mp["e1a"]->setEnabled(s1);
    mp["e1b"]->setEnabled(s1);
    mp["e1x"]->setEnabled(s1);
    mp["e1z"]->setEnabled(s1);
    mp["e5i"]->setEnabled(s3);
    mp["e5q"]->setEnabled(s3);
    mp["e5x"]->setEnabled(s3);
    mp["e6a"]->setEnabled(s4);
    mp["e6b"]->setEnabled(s4);
    mp["e6c"]->setEnabled(s4);
    mp["e6x"]->setEnabled(s4);
    mp["e6z"]->setEnabled(s4);
    mp["e7i"]->setEnabled(s2);
    mp["e7q"]->setEnabled(s2);
    mp["e7x"]->setEnabled(s2);
    mp["e8i"]->setEnabled(s5);
    mp["e8q"]->setEnabled(s5);
    mp["e8x"]->setEnabled(s5);

    mp["q1c"]->setEnabled(s1);
    mp["q1s"]->setEnabled(s1);
    mp["q1l"]->setEnabled(s1);
    mp["q1x"]->setEnabled(s1);
    mp["q1z"]->setEnabled(s1);
    mp["q1e"]->setEnabled(s1);
    mp["q1b"]->setEnabled(s1);
    mp["q2s"]->setEnabled(s2);
    mp["q2l"]->setEnabled(s2);
    mp["q2x"]->setEnabled(s2);
    mp["q5i"]->setEnabled(s3);
    mp["q5q"]->setEnabled(s3);
    mp["q5x"]->setEnabled(s3);
    mp["q5d"]->setEnabled(s3);
    mp["q5p"]->setEnabled(s3);
    mp["q5z"]->setEnabled(s3);
    mp["q6s"]->setEnabled(s4);
    mp["q6l"]->setEnabled(s4);
    mp["q6x"]->setEnabled(s4);
    mp["q6e"]->setEnabled(s4);
    mp["q6z"]->setEnabled(s4);

    mp["b2i"]->setEnabled(s1);
    mp["b2q"]->setEnabled(s1);
    mp["b2x"]->setEnabled(s1);
    mp["b7i"]->setEnabled(s2);
    mp["b7q"]->setEnabled(s2);
    mp["b7x"]->setEnabled(s2);
    mp["b6i"]->setEnabled(s4);
    mp["b6q"]->setEnabled(s4);
    mp["b6x"]->setEnabled(s4);
    mp["b1d"]->setEnabled(s5);
    mp["b1p"]->setEnabled(s5);
    mp["b1x"]->setEnabled(s5);
    mp["b1s"]->setEnabled(s5);
    mp["b1l"]->setEnabled(s5);
    mp["b1z"]->setEnabled(s5);
    mp["b5d"]->setEnabled(s3);
    mp["b5p"]->setEnabled(s3);
    mp["b5x"]->setEnabled(s3);
    mp["b7d"]->setEnabled(s2);
    mp["b7p"]->setEnabled(s2);
    mp["b7z"]->setEnabled(s2);
    mp["b8d"]->setEnabled(s6);
    mp["b8p"]->setEnabled(s6);
    mp["b8x"]->setEnabled(s6);
    mp["b6d"]->setEnabled(s4);
    mp["b6p"]->setEnabled(s4);
    mp["b6z"]->setEnabled(s4);

    mp["i5a"]->setEnabled(s1);
    mp["i5b"]->setEnabled(s1);
    mp["i5c"]->setEnabled(s1);
    mp["i5x"]->setEnabled(s1);
    mp["i9a"]->setEnabled(s2);
    mp["i9b"]->setEnabled(s2);
    mp["i9c"]->setEnabled(s2);
    mp["i1x"]->setEnabled(s3);
    mp["i1d"]->setEnabled(s2);

    mp["s1c"]->setEnabled(s1);
    mp["s5i"]->setEnabled(s3);
    mp["s5q"]->setEnabled(s3);
    mp["s5x"]->setEnabled(s3);
}

void SignalsMask::Close() {
    this->close();
}

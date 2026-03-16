#include "stationreferenceselection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QRadioButton>
#include <QButtonGroup>
#include "../../../Utils/custommessagebox.h"
StationReferenceSelection::StationReferenceSelection(const QString &pointId, const QVector<StationRealization> &grp, QWidget *parent): QDialog{parent}, m_pointId(pointId), m_grp(grp)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(580, 350);
    setObjectName("StationReferenceSelection");

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

    heading = new QLabel(QString("Select referece for %1").arg(pointId));
    heading->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");

    table = new QTableWidget();
    setUpTable();
    populateTable();

    saveBtn = new QPushButton("Save");
    saveBtn->setObjectName("srsBtns");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background-color: #00b894; } QPushButton:hover {background-color: #00d2a8;}");

    cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("srsBtns");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background-color: black; } QPushButton:hover {background-color: gray;}");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);

    QWidget * contentWidget = new QWidget();
    QVBoxLayout * lay = new QVBoxLayout(contentWidget);
    lay->setContentsMargins(15,10,15,10);
    lay->addWidget(table);
    lay->addLayout(btnLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(topBar,0, Qt::AlignTop);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(heading,0, Qt::AlignHCenter);
    mainLayout->addWidget(contentWidget);
    mainLayout->addStretch();
    setLayout(mainLayout);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, [this](){
        int checkedId = radioGroup->checkedId();
        if(checkedId<0){
            CustomMessageBox *mb = new CustomMessageBox("ERROR","Reference required\nPlease select one reference coordinate.","OK");
            mb->exec();
            return;
        }
        m_selectedObsPath = m_grp[checkedId].obsPath;
        accept();
    });

    connect(table, &QTableWidget::cellClicked, this, [this](int row, int){
        if(radioGroup && radioGroup->button(row)){
            radioGroup->button(row)->setChecked(true);
        }
    });

    connect(radioGroup, QOverload<int>::of(&QButtonGroup::idClicked),this, [this](int id){
        if(id >= 0){
            table->selectRow(id);
            table->setCurrentCell(id, 1);
        }
    });
}

QString StationReferenceSelection::selectedObsPath() const
{
    return m_selectedObsPath;
}

void StationReferenceSelection::setUpTable()
{
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({
        "Select",
        "File",
        "X (ecef)",
        "Y (ecef)",
        "Z (ecef)"
    });
    table->setStyleSheet(R"(
        QTableWidget::item:selected {
            background-color: #00d2a8;;
            color: white;
        }
        QTableWidget::item:selected:!active {
            background-color: #00b894;;
        }
        QTableWidget QScrollBar {
            background: transparent;
            border: none;
        }
        QTableWidget QScrollBar:vertical {
            width: 6px;
            margin: 2px 0 2px 0;
        }

        QTableWidget QScrollBar::handle:vertical {
            background: #2c2c2c;
            min-height: 25px;
            border-radius: 3px;
        }

        QTableWidget QScrollBar::handle:vertical:hover {
            background: #000000;
        }

        QTableWidget QScrollBar::add-line:vertical,
        QTableWidget QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QTableWidget QScrollBar::add-page:vertical,
        QTableWidget QScrollBar::sub-page:vertical {
            background: transparent;
        }

        QTableWidget QScrollBar:horizontal {
            height: 6px;
            margin: 0 2px 0 2px;
        }

        QTableWidget QScrollBar::handle:horizontal {
            background: #2c2c2c;
            min-width: 25px;
            border-radius: 3px;
        }

        QTableWidget QScrollBar::handle:horizontal:hover {
            background: #000000;
        }

        QTableWidget QScrollBar::add-line:horizontal,
        QTableWidget QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        QTableWidget QScrollBar::add-page:horizontal,
        QTableWidget QScrollBar::sub-page:horizontal {
            background: transparent;
        }
    )");

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    QHeaderView *h = table->horizontalHeader();
    h->setStretchLastSection(true);
}

void StationReferenceSelection::populateTable()
{
    table->setRowCount(m_grp.size());
    radioGroup = new QButtonGroup(this);
    radioGroup->setExclusive(true);
    for (int i = 0; i < m_grp.size(); ++i) {
        const StationRealization &s = m_grp[i];
        QRadioButton *rb = new QRadioButton();
        if (i == 0)
            rb->setChecked(true);
        radioGroup->addButton(rb, i);

        QWidget *radioWidget = new QWidget();
        QHBoxLayout *hl = new QHBoxLayout(radioWidget);
        hl->addWidget(rb);
        hl->setAlignment(Qt::AlignCenter);
        hl->setContentsMargins(0,0,0,0);

        table->setCellWidget(i, 0, radioWidget);
        table->setItem(i, 1, new QTableWidgetItem(QFileInfo(s.obsPath).fileName()));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(s.X, 'f', 4)));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(s.Y, 'f', 4)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(s.Z, 'f', 4)));
    }
}


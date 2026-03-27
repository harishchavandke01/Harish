#include "workspace.h"
#include <QStackedWidget>
#include <QVBoxLayout>

Workspace::Workspace(QMainWindow *parent)
    : QMainWindow{parent}
{
    setUpMenuBar();
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    mainStackedWidget = new QStackedWidget(this);
    layout->addWidget(mainStackedWidget);

    projectContext = new ProjectContext(this);

    rinex = new RinexConversion();
    process = new BaselineProcessUI(projectContext);
    adjustment = new NetworkAdjustment(projectContext);
    mainStackedWidget->addWidget(rinex);
    mainStackedWidget->addWidget(process);
    mainStackedWidget->addWidget(adjustment);
}

void Workspace::setUpMenuBar()
{
    QMenuBar *mb = new QMenuBar(this);
    setMenuBar(mb);

    projectMenu = mb->addMenu(tr("Project"));
    QAction *actBackToStart = projectMenu->addAction(tr("Back"));
    QAction *actCloseProject = projectMenu->addAction(tr("Close project"));
    QAction *actSelectFile = projectMenu->addAction(tr("Select File"));

    processingMenu = mb->addMenu(tr("Processing"));
    QAction *actConvert = processingMenu->addAction(tr("Convert (UBX → OBS)"));
    QAction *actBaseline = processingMenu->addAction(tr("Baseline processing"));
    QAction *actNetworkAdj = processingMenu->addAction(tr("Network adjustments"));

    connect(actBackToStart, &QAction::triggered, this, [this]() {
        CustomMessageBox mb("ERROR", "This tool is not implemented yet", "OK", this);
        mb.exec();
    });
    connect(actCloseProject, &QAction::triggered, this, [this]() {
        CustomMessageBox mb("ERROR", "This tool is not implemented yet", "OK", this);
        mb.exec();
    });

    connect(actSelectFile, &QAction::triggered, this, [this]() {
        CustomMessageBox mb("ERROR", "This tool is not implemented yet", "OK", this);
        mb.exec();
    });


    connect(actConvert, &QAction::triggered, this, [this]() {
        mainStackedWidget->setCurrentWidget(rinex);
    });
    connect(actBaseline, &QAction::triggered, this, [this]() {
        mainStackedWidget->setCurrentWidget(process);
    });
    connect(actNetworkAdj, &QAction::triggered, this, [this]() {
        mainStackedWidget->setCurrentWidget(adjustment);
    });
}

void Workspace::setProjectFolder(const QString &_projectFolder)
{
    projectFolder=_projectFolder;
    rinex->setProjectFolder(projectFolder);
    process->setProjectFolder(projectFolder);
}


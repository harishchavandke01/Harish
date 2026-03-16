#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QWidget>
#include <QString>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QStackedWidget>
#include "RinexConversion/rinexconversion.h"
#include "BaselineProcessing/baselineprocessui.h"
#include "NetworkAdjustment/networkadjustment.h"
#include "../Context/projectcontext.h"
#include "../Utils/custommessagebox.h"
class Workspace : public QMainWindow
{
    Q_OBJECT
public:
    explicit Workspace(QMainWindow *parent=nullptr);
    void setProjectFolder(const QString &_projectFolder);

private:
    QStackedWidget *mainStackedWidget = nullptr;
    RinexConversion *rinex = nullptr;
    BaselineProcessUI *process = nullptr;
    NetworkAdjustment * adjustment = nullptr;
    ProjectContext * projectContext = nullptr;
private:
    QString projectFolder;

    QMenuBar *menuBar = nullptr;
    QMenu *projectMenu = nullptr;
    QMenu *processingMenu = nullptr;

    void setUpMenuBar();

signals:
};

#endif // WORKSPACE_H

#ifndef NETWORKADJUSTMENT_H
#define NETWORKADJUSTMENT_H

#include <QWidget>
#include "../../Context/projectcontext.h"
#include "AdjustOptions/adjustoptions.h"
#include "../BaselineProcessing/ChartView/chartview.h"
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QChart>

class NetworkAdjustment : public QWidget
{
    Q_OBJECT

private:
    ProjectContext *projectContext;

public:
    explicit NetworkAdjustment(ProjectContext *_projectContext,QWidget *parent = nullptr);

signals:


private:
    AdjustmentOptions adjOptions;

    QLabel *heading;
    QPushButton *runAdjustment;
    QPushButton *setBases;
    QPushButton *options;
    QPushButton *report;

    QWidget *leftWidget;
    QWidget *centralWidget;
    QChart *chart;
    ChartView *chartView;

    void loadDataFromProjectContext();
    void drawNetWorkPreview();
    bool validateNetwork();
    void buildNetworkModel();
    bool solveLeastSquares();

private slots:
    void onRunAdjustmentClicked();


protected:
    void showEvent(QShowEvent *event) override;
};

#endif // NETWORKADJUSTMENT_H

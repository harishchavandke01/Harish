#ifndef GENERATENETWORKADJUSTMENTREPORT_H
#define GENERATENETWORKADJUSTMENTREPORT_H

#include <QWidget>
#include <QString>

class GenerateNetworkAdjustmentReport : public QWidget
{
    Q_OBJECT
public:
    explicit GenerateNetworkAdjustmentReport(QWidget *parent = nullptr);
    void savePDF(QString pdfPath);

signals:
};

#endif // GENERATENETWORKADJUSTMENTREPORT_H

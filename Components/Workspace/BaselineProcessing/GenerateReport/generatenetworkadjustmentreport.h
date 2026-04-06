#ifndef GENERATENETWORKADJUSTMENTREPORT_H
#define GENERATENETWORKADJUSTMENTREPORT_H

#include <QWidget>
#include <QString>
#include "../../Components/Context/projectcontext.h"

class GenerateNetworkAdjustmentReport : public QWidget
{
    Q_OBJECT
public:
    explicit GenerateNetworkAdjustmentReport(QWidget *parent = nullptr);
    static bool savePDF(const ProjectContext *ctx, const AdjustmentOptions &options, const QString &filePath);

signals:

private:
    static QString buildHTML(const ProjectContext *ctx, const AdjustmentOptions &options);
};

#endif // GENERATENETWORKADJUSTMENTREPORT_H

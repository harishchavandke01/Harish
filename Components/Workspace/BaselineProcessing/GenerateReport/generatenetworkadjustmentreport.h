#ifndef GENERATENETWORKADJUSTMENTREPORT_H
#define GENERATENETWORKADJUSTMENTREPORT_H

#include <QWidget>
#include <QString>
#include "../../../Context/projectcontext.h"

class GenerateNetworkAdjustmentReport : public QWidget
{
    Q_OBJECT
public:
    explicit GenerateNetworkAdjustmentReport(QWidget *parent = nullptr);

    /**
     * Generate a TBC-style Network Adjustment PDF report.
     * @param ctx     ProjectContext containing all adjustment results
     * @param options AdjustmentOptions used during the adjustment
     * @param filePath Full output file path (must end in .pdf)
     * @return true on success
     */
    static bool savePDF(const ProjectContext *ctx,
                        const AdjustmentOptions &options,
                        const QString &filePath);

signals:

private:
    static QString buildHTML(const ProjectContext *ctx,
                             const AdjustmentOptions &options);
};

#endif // GENERATENETWORKADJUSTMENTREPORT_H

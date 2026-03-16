#ifndef GENERATEREPORT_H
#define GENERATEREPORT_H

#include <QObject>
#include <QStringList>
#include "../../../Utils/ProcessUtils/processutils.h"
#include "../../../Utils/utils.h"
class GenerateReport : public QObject
{
    Q_OBJECT
public:
    explicit GenerateReport(QObject *parent = nullptr);
    void getReport(const QMap<QString, PosData> &posData, const QString &projectFolder);

private:
    ProcessUtils *processUtils;
    Utils *utils;
    QString getHTMLcode(QMap<QString, PosData> posData, QJsonObject &rinexData);
    void savePDF(QString HTMLBody, const QString &_outPath);

    QString generateResidualGraphSvg(const SatelliteData &sat, int width, int height);
    QString generateTrackingSummarySvg(const  QMap<QString, SatelliteData> &map, QDateTime start, QDateTime end, int width, int height);
    QString svgToPngBase64(const QString &svgContent, int width, int height);
    static bool naturalSortComparator(const QString &s1, const QString &s2);
signals:
    void reportGenerationSuccessfull();
    void reportGenerationFailed(const QString &err);
};

#endif // GENERATEREPORT_H

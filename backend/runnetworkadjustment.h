#ifndef RUNNETWORKADJUSTMENT_H
#define RUNNETWORKADJUSTMENT_H

#include <QObject>
#include <QMap>
#include <QVector>
#include "../../../Context/projectcontext.h"

class RunNetworkAdjustment : public QObject
{
    Q_OBJECT
public:
    explicit RunNetworkAdjustment(SubnetworkInfo &info, const QMap<QString, ProjectStation>&stations,
                                  const QVector<ProjectBaseline> &baselines, const AdjustmentOptions &options, QObject *parent = nullptr);

public slots:
    void ExecuteAdjust();

signals:
    void finished(SubnetworkResult result);
    void failed(int subnetworkIndex, QString reason);
    void statusUpdate(QString message);

private:
    SubnetworkInfo m_info;
    QMap<QString, ProjectStation> m_stations;
    QVector<ProjectBaseline> m_baselines;
    AdjustmentOptions m_options;
};

#endif // RUNNETWORKADJUSTMENT_H

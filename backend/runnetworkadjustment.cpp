#include "runnetworkadjustment.h"
#include <QThread>
#include <QDateTime>
#include <limits>

RunNetworkAdjustment::RunNetworkAdjustment(SubnetworkInfo &info, const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines, const AdjustmentOptions &options, QObject *parent)
    :QObject{parent}, m_info(info), m_stations(stations), m_baselines(baselines), m_options(options)
{}

void RunNetworkAdjustment::ExecuteAdjust()
{
    if(m_baselines.isEmpty()){
        emit failed(m_info.index, "No baselines in subnetwork");
        QThread::currentThread()->quit();
        return;
    }

    emit statusUpdate(QString("Adjusting subnetwork %1...").arg(m_info.index));
    SubnetworkResult result;
    result.subnetworkIndex = m_info.index;
    result.constrained = m_info.isConstrained;
    result.usedCovariance = m_options.useCovariance;
    result.adjustedAt = QDateTime::currentDateTimeUtc();
    result.sigma0 = std::numeric_limits<double>::quiet_NaN();
    result.rms3D = std::numeric_limits<double>::quiet_NaN();
    result.dof = 0;
    result.chiSquarePassed = false;

    // ── PHASE 3: LSSolver will be invoked here
    // LSSolver solver(m_info, m_stations, m_baselines, m_options);
    // result = solver.solve();


    // Phase 2 placeholder: return unadjusted (identity) positions so the
    // threading infrastructure is complete and testable end-to-end.


    bool anyStation = false;
    for(const QString &uid : m_info.stationUIDs){
        if(!m_stations.contains(uid)){
            continue;
        }

        const ProjectStation &st = m_stations[uid];
        result.adjustedECEF[uid] = QVector3D(
            static_cast<float>(st.ecef.X),
            static_cast<float>(st.ecef.Y),
            static_cast<float>(st.ecef.Z));
        anyStation = true;
    }

    result.success = anyStation;
    emit finished(result);
    QThread::currentThread()->quit();
}

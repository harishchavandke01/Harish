#include "runnetworkadjustment.h"
#include "../backend/lssolver.h"
#include <QThread>

RunNetworkAdjustment::RunNetworkAdjustment(const SubnetworkInfo &info,const QMap<QString, ProjectStation> &stations,const QVector<ProjectBaseline> &baselines,const AdjustmentOptions &options,QObject *parent)
    : QObject{parent}, m_info(info), m_stations(stations), m_baselines(baselines), m_options(options)
{}

void RunNetworkAdjustment::ExecuteAdjust()
{
    if (m_baselines.isEmpty()) {
        emit failed(m_info.index, "No baselines in subnetwork.");
        QThread::currentThread()->quit();
        return;
    }
    qDebug()<<"\nNetwork Info";
    qDebug()<<"Index : "<<m_info.index<<" isConstrined : "<<m_info.isConstrained<<" ";
    qDebug()<<"Fixed UIDs";
    for(QString str: m_info.fixedUIDs){
        qDebug()<<str<<" ";
    }
    qDebug()<<"station UIDs";
    for(QString str : m_info.stationUIDs){
        qDebug()<<str<<" ";
    }

    qDebug()<<"baselineIndices";
    for(int i : m_info.baselineIndices){
        qDebug()<<i<<" ";
    }

    emit statusUpdate(QString("Adjusting subnetwork %1...").arg(m_info.index));

    LSSolver solver(m_info, m_stations, m_baselines, m_options);
    SubnetworkResult result = solver.solve();

    for (QString &msg : result.iterationLog) {
        emit statusUpdate(msg);
    }

    if (!result.success) {
        emit failed(m_info.index, "Adjustment did not produce a valid solution.");
        QThread::currentThread()->quit();
        return;
    }

    emit finished(result);
    QThread::currentThread()->quit();
}

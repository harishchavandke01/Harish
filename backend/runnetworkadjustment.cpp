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

    qDebug()<<"--------Network adjustment result--------";
    qDebug()<<"Success = "<<result.success;
    qDebug()<<"Subnetwork index = "<<result.subnetworkIndex;
    qDebug()<<"Constrained = "<<result.constrained;
    qDebug()<<"Used covariance = "<<result.usedCovariance;
    qDebug()<<"Adjusted At:" << result.adjustedAt.toString(Qt::ISODate);

    qDebug() << "\n--- GLOBAL STATISTICS ---";
    qDebug() << "Degrees of Freedom (DOF):" << result.dof;
    qDebug() << "Reference Factor (Sigma0):" << QString::number(result.sigma0, 'f', 4);
    qDebug() << "3D RMS:" << QString::number(result.rms3D * 1000.0, 'f', 1) << "mm"; // Converted to mm
    qDebug() << "Chi-Square Value (vTPv):" << QString::number(result.chiSquareValue, 'f', 4);
    qDebug() << "Chi-Square Test:" << (result.chiSquarePassed ? "PASSED" : "FAILED");

    qDebug() << "\n--- ADJUSTED COORDINATES & CORRECTIONS ---";
    for (auto it = result.adjustedECEF.constBegin(); it != result.adjustedECEF.constEnd(); ++it) {
        QString uid = it.key();

        // 1. Change QVector3D to Vector3d64
        Vector3d64 ecef = it.value();
        Vector3d64 corr = result.stationCorrections.value(uid, Vector3d64(0,0,0));

        qDebug() << "Station:" << uid
                 << "\n  | Final XYZ:"
                 // 2. Remove the parentheses from x(), y(), z()
                 << QString::number(ecef.x, 'f', 4)
                 << QString::number(ecef.y, 'f', 4)
                 << QString::number(ecef.z, 'f', 4)
                 << "\n  | Shift dX, dY, dZ:"
                 << QString::number(corr.x, 'f', 4)
                 << QString::number(corr.y, 'f', 4)
                 << QString::number(corr.z, 'f', 4);
    }

    qDebug() << "\n--- ERROR ELLIPSES (STATION PRECISIONS) ---";
    for (auto it = result.stationPrecisions.constBegin(); it != result.stationPrecisions.constEnd(); ++it) {
        QString uid = it.key();
        const StationPrecision &prec = it.value();
        qDebug() << "Station:" << uid
                 << "| 1-Sigma E,N,U:" << QString::number(prec.sigmaE, 'f', 4)
                 << QString::number(prec.sigmaN, 'f', 4)
                 << QString::number(prec.sigmaU, 'f', 4)
                 << "| Semi-Major:" << QString::number(prec.semiMajor, 'f', 4)
                 << "| Semi-Minor:" << QString::number(prec.semiMinor, 'f', 4)
                 << "| Azimuth:" << QString::number(prec.ellipseAzimuthDeg, 'f', 1) << "deg"
                 << "| 95% Horiz:" << QString::number(prec.horizPrecision95, 'f', 4)
                 << "| 95% Vert:" << QString::number(prec.vertPrecision95, 'f', 4);
    }

    qDebug() << "\n--- BASELINE RESIDUALS (TAU TEST) ---";
    for (const auto &res : result.residuals) {
        qDebug() << "Baseline:" << res.base << "->" << res.rover
                 << "| vX, vY, vZ:" << QString::number(res.vX, 'f', 4)
                 << QString::number(res.vY, 'f', 4)
                 << QString::number(res.vZ, 'f', 4)
                 << "| Tau Max:" << QString::number(res.standardizedResidual, 'f', 3)
                 << "| Tau Test:" << (res.tauFailed ? "FAILED (BLUNDER)" : "PASSED");
    }


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

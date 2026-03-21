#ifndef PROJECTCONTEXT_H
#define PROJECTCONTEXT_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QVector3D>
#include <QMatrix3x3>
#include <QDateTime>
#include "../Utils/ProcessUtils/processutils.h"

//Baseline processing
struct ProjectStation {
    QString uid;
    QString stationId;
    QString obsPath;
    QString singlePosPath;
    EcefPos ecef;
    GeoPos  geo;
    double easting = NAN;
    double northing = NAN;
    double orthometric = NAN;
    bool isBase = false;
    bool isFixed = false;
};

struct ProjectBaseline {
    QString baselineId;
    QString from;
    QString to;
    QString fromStationId;
    QString toStationId;
    QString posPath;
    double length = NAN;
    double dX = 0, dY = 0, dZ = 0;
    double cov[3][3] = {};
    double rms = NAN;
    QDateTime startTime;
    QString solutionType;
    double sigma0 = NAN;
    double dof = 0;
};

// Loop closure
struct LoopInfo {
    QString loopId;
    QVector<QString> stations;
    QVector<QString> baselineIds;
    struct VectorInfo {
        QString baselineId;
        QString from, to;
        QString solutionType;
        double length = NAN;
        QDateTime startTime;
    };
    QVector<VectorInfo> vectors;
    QVector3D misclosureXYZ;
    double horizError = NAN;
    double vertError= NAN;
    double error3D = NAN;
    double length = NAN;
    double ppm = NAN;
    bool passed = false;
};

struct LoopClosure {
    QVector<QString> stations;
    QVector3D misclosure;
    double length;
    double closureError;
};

struct LoopReport {
    QVector<LoopInfo> loops;
    int numLoops = 0, numPassed = 0, numFailed = 0;
    double best3D = NAN, bestHoriz = NAN, bestVert = NAN, bestPPM = NAN;
    double worst3D = NAN, worstHoriz = NAN, worstVert = NAN, worstPPM = NAN;
    double avgLength = NAN, avg3D = NAN, avgHoriz = NAN, avgVert = NAN, avgPPM = NAN;
    double stdLength = NAN, std3D = NAN, stdHoriz = NAN, stdVert = NAN, stdPPM = NAN;
    struct Occupation {
        QString stationId;
        QVector<QString> observations;
        QVector<QDateTime> startTimes;
        int occurrences = 0;
    };
    QVector<Occupation> occupations;
};

//Network adjustment
struct NetworkModel {
    QMap<QString, int> unknownIndex;
    int nUnknowns = 0;
    struct Observation {
        QString base, rover;
        double dX = 0, dY = 0, dZ = 0;
        double comp_dX = 0, comp_dY = 0, comp_dZ = 0;
        double mis_dX  = 0, mis_dY  = 0, mis_dZ  = 0;
        double covariance[3][3] = {};
    };
    QVector<Observation> observations;
};


struct SubnetworkInfo {
    int index = 0;
    QSet<QString> stationUIDs;
    QVector<int> baselineIndices;
    QStringList fixedUIDs;
    bool isConstrained = false;
    bool hasResult= false;
};

struct SubnetworkResult {
    int subnetworkIndex = 0;
    bool success = false;
    bool constrained = true;
    bool usedCovariance = true;
    QMap<QString, QVector3D> adjustedECEF;
    QMap<QString, QVector3D> stationCorrections;

    struct Residual {
        QString base, rover;
        double vX = 0, vY = 0, vZ = 0, vNorm = 0;
        double standardizedResidual = 0;
        bool tauFailed = false;
    };
    QVector<Residual> residuals;

    double sigma0= NAN;
    double rms3D = NAN;
    int dof = 0;
    bool chiSquarePassed = false;
    QDateTime adjustedAt;
};


struct AdjustmentResult {
    QMap<int, SubnetworkResult> subnetworkResults;
    int activeSubnetworkIndex = -1;
    QMap<QString, QVector3D> mergedAdjustedECEF;

    bool anySuccess() const {
        for (auto it = subnetworkResults.constBegin(); it != subnetworkResults.constEnd(); ++it)
            if (it.value().success) return true;
        return false;
    }

    bool hasSubnetworkResult(int idx) const {
        return subnetworkResults.contains(idx) && subnetworkResults.value(idx).success;
    }

    void storeSubnetworkResult(const SubnetworkResult &r) {
        subnetworkResults[r.subnetworkIndex] = r;
        if (r.success) {
            for (auto it = r.adjustedECEF.constBegin(); it != r.adjustedECEF.constEnd(); ++it)
                mergedAdjustedECEF[it.key()] = it.value();
            activeSubnetworkIndex = r.subnetworkIndex;
        }
    }

    bool success = false;
    bool constrained   = true;
    bool usedCovariance = true;
    QMap<QString, QVector3D> adjustedECEF;
    QMap<QString, QVector3D> stationCorrections;
    struct LegacyResidual { QString base, rover; double vX,vY,vZ,vNorm; };
    QVector<LegacyResidual> residuals;
    double sigma0 = NAN;
    double rms3D  = NAN;
    int  dof    = 0;
};

struct NetworkReport {
    QVector<LoopClosure> loops;
    QVector<AdjustmentResult::LegacyResidual> residuals;
    double maxResidual  = NAN;
    double meanResidual = NAN;
    QString adjustmentSummary;
};

struct AdjustmentOptions {
    bool useCovariance = true;   // use full 3×3 cov from batchls
    double aPrioriScalar = 1.0;    // multiplies all weights (1.0 = as-is)
    double defaultSigmaH = 0.010;  // m — fallback horizontal sigma
    double defaultSigmaV = 0.020;  // m — fallback vertical sigma
};


class ProjectContext : public QObject {
    Q_OBJECT
public:
    explicit ProjectContext(QObject *parent = nullptr);
    QMap<QString, ProjectStation> stations;
    QVector<ProjectBaseline> baselines;

    QVector<LoopClosure> loopClosures;
    NetworkModel networkModel;
    AdjustmentResult adjustmentResult;
    NetworkReport networkReport;
    LoopReport loopReport;

    QVector<SubnetworkInfo> subnetworks;

signals:
    void baselineReady();
    void adjustmentFinished();
};

#endif // PROJECTCONTEXT_H

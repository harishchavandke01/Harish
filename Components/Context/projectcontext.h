#ifndef PROJECTCONTEXT_H
#define PROJECTCONTEXT_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QVector3D>
#include <QMatrix3x3>
#include <QDateTime>
#include "../Utils/ProcessUtils/processutils.h"

//baseline processing
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
    double length;
    double dX, dY, dZ;
    double cov[3][3];
    double rms;

    QDateTime startTime;
    QString solutionType;
    double sigma0;
    double dof;
};

//loop closure
struct LoopInfo{
    QString loopId;
    QVector<QString> stations;
    QVector<QString> baselineIds;
    struct VectorInfo{
        QString baselineId;
        QString from;
        QString to;

        QString solutionType;
        double length = NAN;
        QDateTime startTime;
    };

    QVector<VectorInfo> vectors;
    QVector3D misclosureXYZ;
    double horizError = NAN;
    double vertError  = NAN;
    double error3D    = NAN;
    double length = NAN;
    double ppm = NAN;
    bool passed = false;
};

struct LoopReport
{
    QVector<LoopInfo> loops;
    int numLoops = 0;
    int numPassed = 0;
    int numFailed = 0;

    double best3D = NAN;
    double bestHoriz = NAN;
    double bestVert = NAN;
    double bestPPM = NAN;
    double worst3D = NAN;
    double worstHoriz = NAN;
    double worstVert = NAN;
    double worstPPM = NAN;

    double avgLength = NAN;
    double avg3D = NAN;
    double avgHoriz = NAN;
    double avgVert = NAN;
    double avgPPM = NAN;

    double stdLength = NAN;
    double std3D = NAN;
    double stdHoriz = NAN;
    double stdVert = NAN;
    double stdPPM = NAN;

    struct Occupation
    {
        QString stationId;
        QVector<QString> observations;
        QVector<QDateTime> startTimes;
        int occurrences = 0;
    };
    QVector<Occupation> occupations;
};


//network adjustment
struct NetworkModel
{
    QMap<QString, int> unknownIndex;
    int nUnknowns = 0;
    struct Observation
    {
        QString base;
        QString rover;

        double dX, dY, dZ;
        double comp_dX, comp_dY, comp_dZ;
        double mis_dX, mis_dY, mis_dZ;
        double covariance[3][3];
    };

    QVector<Observation> observations;
};

struct AdjustmentResult
{
    bool success = false;
    QMap<QString, QVector3D> stationCorrections;
    QMap<QString, QVector3D> adjustedECEF;
    struct Residual
    {
        QString base;
        QString rover;

        double vX, vY, vZ;
        double vNorm;
    };
    QVector<Residual> residuals;

    double sigma0 = NAN;
    double rms3D  = NAN;
    int dof = 0;
    bool usedCovariance = true;
    bool constrained = true;
};

struct LoopClosure
{
    QVector<QString> stations;  // loop path
    QVector3D misclosure;       // Σ(dX dY dZ)
    double length;              // loop length
    double closureError;        // magnitude
};

struct NetworkReport
{
    QVector<LoopClosure> loops;
    QVector<AdjustmentResult::Residual> residuals;

    double maxResidual;
    double meanResidual;

    QString adjustmentSummary;
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

signals:
    void baselineReady();
    void adjustmentFinished();
};
#endif // PROJECTCONTEXT_H

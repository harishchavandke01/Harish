#ifndef PROCESSUTILS_H
#define PROCESSUTILS_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <QTime>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <cmath>
#include <QVector>

struct UTMResult {
    int zoneNumber;
    char zoneLetter;
    double easting;
    double northing;
    double altitude;
};

struct EcefPos {
    double X=0.0, Y=0.0, Z=0.0;
};

struct GeoPos {
    double lat = 0.0;
    double lon = 0.0;
    double h   = 0.0;
};

struct SitePosition {
    GeoPos geodetic;
    double orthometric = NAN;
    EcefPos ecef;
    double easting = NAN, northing = NAN;
};

struct EpochRecord {
    QDateTime gpst;
    double lat=0.0, lon=0.0, h=0.0;
    int Q=0; int ns=0;
    double sdn=0.0, sde=0.0, sdu=0.0;
    double sdne=0.0, sdeu=0.0, sdun=0.0;
    double age = 0.0, ratio = 0.0;
};

struct SatResidualPoint{
    QDateTime time;
    double residual; //carrier phase residual in meters
    double elevation; //elevation in degrees
    bool isFixed; // if solution is fixed at this epoch
};

struct SatelliteData {
    QString satId;
    QVector<SatResidualPoint> points;
    double mean = 0.0;
    double stdDev = 0.0;
    double minVal = 0.0;
    double maxVal = 0.0;
};

struct PosData {
    QString posPath;
    QString from;
    QString to;
    QString fileKey;
    QString roverPath;
    QString basePath;
    QString frequency;
    QString elevMask;
    QString ephemeris;
    QDateTime processingStart;
    QDateTime processingStop;
    QDateTime processedTime;

    double pctFixed = 0.0;
    double pctFloat = 0.0;
    double pctSingle = 0.0;
    double pctOther = 0.0;
    QString solutionTypeSummary;

    SitePosition basePosition;
    SitePosition roverPosition;

    double horizontalPrecision = NAN;
    double verticalPrecision   = NAN;
    double RMS  = NAN;

    QString geodeticAzDMS;
    double geodeticAzDeg = NAN;
    double ellipsoidDistance = NAN;
    double deltaHeight = NAN;

    double dE = NAN;
    double dN = NAN;
    double dU = NAN;

    QTime processingDuration;
    double processingIntervalSeconds = NAN;

    double sigma_dE = NAN, sigma_dN = NAN, sigma_dU = NAN;
    double sigma_dist = NAN;
    double sigma_az_arcsec = NAN;
    QString sigma_az_DMS;
    double sigma_dX = NAN, sigma_dY = NAN, sigma_dZ = NAN;

    double cov_dXYZ[3][3] = {};

    QMap<QString, SatelliteData> SatStatsMap;
    struct _InternalAgg {
        quint64 n_epochs = 0;
        QDateTime firstEpoch, lastEpoch;
        double sumIntervalSeconds = 0.0;
        quint64 nIntervals = 0;

        double meanLat = 0.0, M2Lat = 0.0;
        double meanLon = 0.0, M2Lon = 0.0;
        double meanH   = 0.0, M2H   = 0.0;

        double sumVarE = 0.0, sumVarN = 0.0, sumVarU = 0.0;
        double sumCovEN = 0.0, sumCovEU = 0.0, sumCovNU = 0.0;

        quint64 cntFixed = 0, cntFloat = 0, cntSingle = 0, cntOther = 0;

        QDateTime bestEpochTime;
        double bestLat=0, bestLon=0, bestH=0;
        double bestRatio = -1.0;
    } agg;

    void finalize();
};

class ProcessUtils : public QObject
{
    Q_OBJECT
public:
    explicit ProcessUtils(QObject *parent = nullptr);
    bool parsePosFileIntoPosData(const QString &posFilePath, PosData &out, QString &errorOut);
    bool parseSolutionStat(const QString &statFilePath, PosData &out);
    UTMResult WGS84ToUTM(double lat, double lon, double alt);
    void generateKMLFromPosData(const QMap<QString, PosData> &posData,const QString &filePath);

private:
    static void updateAggregates(PosData &p, const EpochRecord &er);
    void consolidateSatellites(PosData &out);
signals:
};

#endif // PROCESSUTILS_H




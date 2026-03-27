#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QMainWindow>
#include <QDateTime>
#include <QPainter>
#include <QDir>
#include <QJsonObject>

struct FileEntry {
    QString obs;
    QString nav;
    QString pointId;

    double X = 0.0, Y = 0.0, Z = 0.0;

    double refX = 0.0, refY = 0.0, refZ = 0.0;
    bool hasReference = false;
    enum Role { Rover, Base } role;
};

struct PropagatedCoord {
    double lat;
    double lon;
    double hgt;
    bool isPrimary;
};

// struct StaticJob {
//     QString roverObs;
//     QString baseObs;
//     QString nav;
//     QString outPos;
//     QDateTime sortTime;
// };

struct StaticJob {
    QString roverObs;
    QString baseObs;
    QString nav;
    QString outPos;

    QDateTime baseStart;
    QDateTime baseEnd;

    QDateTime roverStart;
    QDateTime roverEnd;

    QDateTime overlapStart;
    QDateTime overlapEnd;

    QDateTime sortTime;
};

struct AvgLLH {
    double lat = NAN;
    double lon = NAN;
    double h   = NAN;
    bool valid = false;
};

struct StationPosition {
    QString key;
    QString name;
    bool isBase = false;
    bool hasPos = false;
    double lat = NAN, lon = NAN, h = NAN;
    double ecefX = NAN, ecefY = NAN, ecefZ = NAN;
    double easting = NAN, northing = NAN;
    double orthometric = NAN;
    double x = NAN, y = NAN;
    double ellipsoidDistanceToBase = NAN;
};


struct StationRealization {
    int row;
    QString obsPath;
    QString pointId;
    double X;
    double Y;
    double Z;
};

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);

public:
    std::string dateTimeToString(QDateTime time);
    QDateTime ISTtoGPST(QDateTime istTime);
    QDateTime GPSTtoIST(QDateTime gpstTime);
    QDateTime DateTimeFromPos(QString date, QString time);
    bool parseObsTimeRange(const QString &path, QDateTime &outStart, QDateTime &outEnd) const;
    AvgLLH computeAverageLLHFromPos(const QString &posFile);

    QString getNormalizedPath(const QString &path);
    QString findNavForObs(const QString &obsPath);

    void parseRinexHeaderToJson(const QString &obsPath);
    QJsonObject readRinexHeader(const QString &obsPath = QString());
    void saveRinexHeader(const QString &obsPath, const QJsonObject &headerObj);
    void updateRinexHeader(const QString &obsPath,const QJsonObject &updates);
    void writeRinexHeader(const QJsonObject &root);
    void ecefToGeodetic(double X, double Y, double Z, double &latDeg, double &lonDeg, double &h);
    bool hasKMLViewer();
};

#endif // UTILS_H

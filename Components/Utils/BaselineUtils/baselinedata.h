#ifndef BASELINEDATA_H
#define BASELINEDATA_H

#include <QObject>
#include <QDateTime>

enum PosMode{
    Static,
    Kinematic,
    Single
};

enum GeoidModel{
    ELIP = 0,
    EGM96 = 1,
    EGM08 = 2
};

class Elevation
{
public:
    double Ellipsoidal;
    double ElevationEGM96;
    double ElevationEGM08;
    GeoidModel geoid;
};

class WGS84Coordinate{
public:
    double latitude;
    double longitude;
    Elevation elevation;
};

enum Pointquality{
    NoFix,
    Float,
    Fix,
    single
};

class PosPoint{
public:
    WGS84Coordinate coordinate;
    QDateTime ISTDateTime;
    Pointquality quality;
};

// class PosData{
// public:
//     QDateTime ISTStartTime;
//     QDateTime ISTEndTime;
//     PosMode posMode;
//     WGS84Coordinate basecoordinate;
//     int noFixCount;
//     int fixCount;
//     int floatCount;
//     int singleCount;
//     std::vector<PosPoint*> points;
//     void addPoint(PosPoint *point);
// };

class TempPosData{
public:
    QDateTime ISTStartTime;
    QDateTime ISTEndTime;
    PosMode posMode;
    WGS84Coordinate basecoordinate;
    int noFixCount;
    int fixCount;
    int floatCount;
    int singleCount;
    std::vector<PosPoint*> points;
    void addPoint(PosPoint *point);
};

class BaseJsonData {
public:
    double latitude;
    double longitude;
    double elevation;
};

class ObsData {
public:
    QDateTime ISTStartTime;
    QDateTime ISTEndTime;
    int gps = 0, glo = 0, gal = 0, bei = 0, qzss = 0, sbs = 0;
    BaseJsonData *basedata;
    QDateTime ParseTimeFromObs(QString timestr);
};

// class LogCsvData {
// public:
//     std::vector<QDateTime> logs;
// };

// class ImageData {
// public:
//     QDateTime OriginalTime;
//     QString Name;

//     QDateTime getImageData(QString img);
//     void SetImageData(QString img, PosPoint *point, QString dir);
// };

// class ImageList {
// public:
//     std::vector<ImageData *> imgList;
// };

// class GeoTaggedData {
// public:
//     int totalimg = 0;
//     int taggedimg = 0;
//     int interpolatedimg = 0;
// };


class BaselineData : public QObject
{
    Q_OBJECT
public:
    explicit BaselineData(QObject *parent = nullptr);

signals:
};

#endif // BASELINEDATA_H

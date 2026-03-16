#ifndef BASELINEPROCESSING_H
#define BASELINEPROCESSING_H

#include <QObject>
#include <QDateTime>
#include "../Components/Utils/BaselineUtils/baselineutils.h"

class BaselineProcessing : public QObject
{
    Q_OBJECT
public:
    explicit BaselineProcessing(QObject *parent = nullptr);

signals:
    void Done();
    void Failed();
public:
    void RinexConversion(std::string inputUbxFile, std::string outputDir, std::string obsFileName, std::string navFileName, QDateTime startTime, QDateTime endTime, double interval);

    // void Process(std::string roverObsFile, std::string baseObsFile, std::string navFile, std::string outputFile, QDateTime startTime, QDateTime endTime,
    //                  double interval, bool gps, bool glonass, bool galileo, bool qzss, bool beidou, bool sbs, int elevationMask, PosMode posMode, Frequency freq,
    //                  FilterType filterType, GPSAmbiguity gpsAmbiguity, GlonassAmbiguity gloAmbiguity, BDSAmbiguity bdsAmbiguity, ElevationType eleType,
    //                  BaseLocationType btype, double lat = 0.0, double lon = 0.0, double height = 0.0, bool showProgress = true);

    void Process(std::string roverObsFile, std::string baseObsFile, std::string navFile, std::string outputFile, QDateTime startTime, QDateTime endTime,
                 double interval, bool gps, bool glonass, bool galileo, bool qzss, bool beidou, bool sbs, int elevationMask, PosMode posMode, Frequency freq,
                 FilterType filterType, GPSAmbiguity gpsAmbiguity, GlonassAmbiguity gloAmbiguity, BDSAmbiguity bdsAmbiguity, ElevationType eleType,
                 BaseLocationType btype, double roverPoleHeight, double basePoleHeight, bool showProgress = true,double x = 0.0, double y = 0.0, double z = 0.0);

    std::map<QString, int> satellites;
    std::map<QString, bool> gnsssignals;
    QStringList list = {
        "g1c", "g1p", "g1w", "g1y", "g1m", "g1n", "g1s", "g1l", "g1x",
        "g2c", "g2d", "g2s", "g2l", "g2x", "g2p", "g2w", "g2y", "g2m", "g2n",
        "g5i", "g5q", "g5x",
        "r1c", "r1p", "r2c", "r2p", "r3i", "r3q", "r3x", "r4a", "r4b", "r4x",
        "r6a", "r6b", "r6x",
        "e1c", "e1a", "e1b", "e1x", "e1z",
        "e5i", "e5q", "e5x", "e6a", "e6b", "e6c", "e6x", "e6z",
        "e7i", "e7q", "e7x", "e8i", "e8q", "e8x",
        "q1c", "q1s", "q1l", "q1x", "q1z", "q1e", "q1b", "q2s", "q2l", "q2x",
        "q5i", "q5q", "q5x", "q5d", "q5p", "q5z", "q6s", "q6l", "q6x", "q6e", "q6z",
        "b2i", "b2q", "b2x", "b7i", "b7q", "b7x", "b6i", "b6q", "b6x",
        "b1d", "b1p", "b1s", "b1x", "b1l", "b1z", "b5d", "b5p", "b5x",
        "b7d", "b7p", "b7z", "b8d", "b8p", "b8x", "b6d", "b6p", "b6z",
        "i5a", "i5b", "i5c", "i5x", "i9a", "i9b", "i9c", "i1x", "i1d",
        "s1c", "s5i", "s5q", "s5x"
    };

};

#endif // BASELINEPROCESSING_H

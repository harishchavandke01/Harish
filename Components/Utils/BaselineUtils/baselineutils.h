#ifndef BASELINEUTILS_H
#define BASELINEUTILS_H

#include <QObject>
#include "baselinedata.h"

enum Format{
    UBX=0
};

enum Frequency {
    L1,
    L1_L2,
    L1_L2_L3,
    L1_L2_L3_L4
};

enum FilterType {
    Forward,
    Backward,
    Combined,
    Combined_no_phase_reset
};

enum GPSAmbiguity {
    Off_gps,
    Continuous_gps,
    Instantaneous_gps,
    Fix_and_hold_gps
};

enum GlonassAmbiguity {
    Off_glo,
    On_glo,
    Autocal_glo,
    Fix_and_hold_glo
};

enum BDSAmbiguity {
    Off_bds,
    On_bds
};

enum BaseLocationType {
    Rinex_header,
    Lat_Lon_Height,
    ECEF_XYZ
};

enum ElevationType {
    Ellipsoidal,
    MSL_EGM96,
    MSL_ESM08
};
class UserPreference {
public:
    virtual ~UserPreference() {}
};

class ConvertUserPreference : public UserPreference {
public:
    bool GPS = true;
    bool GLO = true;
    bool GAL = true;
    bool QZSS = true;
    bool BEI = true;
    bool SBS = true;
    bool NavIC = true;
    bool L1 = true;
    bool L2E5 = true;
    Format format = Format::UBX;
    double RinexVersion = 3.04;
    bool sepnav = false;
    QString receiver = "PPK";
    QString marker = "";
    QString antenna = "";
    double appx = 1e9, appy = 1e9, appz = 1e9;
    double poleh = 1e9, polee = 1e9, polen = 1e9;
    bool phase = false, halfc = false, sort = false, iono = false, timecorr = false, leap = false;
    bool sigband1 = true, sigband2 = true, sigband3 = true, sigband4 = true, sigband5 = true, sigband6 = true;
    bool round = false;
    std::map<QString,bool> gnsssignals;
    QString exsat = "";

    ConvertUserPreference() {
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
        for(auto &i : list) {
            gnsssignals[i] = true;
        }
    }
};

class PostProcessUserPreference : public UserPreference {
public:
    bool GPS = true;
    bool GLO = true;
    bool GAL = true;
    bool QZSS = true;
    bool BEI = true;
    bool SBS = true;
    int elevationMask = 0;
    PosMode posMode = PosMode::Kinematic;
    FilterType filterType = FilterType::Combined;
    Frequency frequency = Frequency::L1_L2;
    GPSAmbiguity gpsAmbiguity = GPSAmbiguity::Fix_and_hold_gps;
    GlonassAmbiguity gloAmbiguity = GlonassAmbiguity::Fix_and_hold_glo;
    BDSAmbiguity bdsAmbiguity = BDSAmbiguity::On_bds;
    BaseLocationType baseLocationType = BaseLocationType::Rinex_header;
    ElevationType elevationType = ElevationType::Ellipsoidal;
};


class BaselineUtils : public QObject
{
    Q_OBJECT
public:
    explicit BaselineUtils(QObject *parent = nullptr);

signals:

public:
    UserPreference *LoadConfig(UserPreference *userPreference);
    void SaveConfig(UserPreference *userPreference);
    void GeneratePostConfigFile(PostProcessUserPreference *postUserPreference, ElevationType etype, int elemask, double roverPoleHeight, double basePoleHeight, double x = 0.0, double y = 0.0, double z = 0.0);
    std::pair<std::pair<int, QString>, TempPosData*> ReadPosFile(QString posFile);
    std::pair<std::pair<int, QString>, ObsData*> ReadObsFile(QString obsFile);
    std::pair<std::pair<int, QString>, BaseJsonData*> ReadBaseJsonFile(QString jsonFile);
};

#endif // BASELINEUTILS_H

#include "baselineutils.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include "../utils.h"
#include "../custommessagebox.h"
#include "baselinedata.h"

BaselineUtils::BaselineUtils(QObject *parent)
    : QObject{parent}
{}

PosMode toPosMode(QString s) {
    if(s == "Static") return PosMode::Static;
    else if(s == "Single") return PosMode::Single;
    return PosMode::Kinematic;
}

QString toPosModeString(PosMode p) {
    if(p == PosMode::Static) return "Static";
    else if(p == PosMode::Single) return "Single";
    return "Kinematic";
}

FilterType toFilterType(QString s) {
    if(s == "Forward") return FilterType::Forward;
    else if(s == "Backward") return FilterType::Backward;
    else if(s == "Combined_no_phase_reset") return FilterType::Combined_no_phase_reset;
    else return FilterType::Combined;
}

QString toFilterTypeString(FilterType p) {
    if(p == FilterType::Forward) return "Forward";
    else if(p == FilterType::Backward) return "Backward";
    else if(p == FilterType::Combined_no_phase_reset) return "Combined_no_phase_reset";
    return "Combined";
}

Frequency toFrequency(QString s) {
    if(s == "L1") return Frequency::L1;
    else if(s == "L1_L2") return Frequency::L1_L2;
    else if(s == "L1_L2_L3") return Frequency::L1_L2_L3;
    return Frequency::L1_L2_L3_L4;
}

QString toFrequencyString(Frequency p) {
    if(p == Frequency::L1) return "L1";
    else if(p == Frequency::L1_L2) return "L1_L2";
    else if(p == Frequency::L1_L2_L3) return "L1_L2_L3";
    return "L1_L2_L3_L4";
}

GPSAmbiguity toGpsAmbiguity(QString s) {
    if(s == "Continuous") return GPSAmbiguity::Continuous_gps;
    else if(s == "Instantaneous") return GPSAmbiguity::Instantaneous_gps;
    else if(s == "Off") return GPSAmbiguity::Off_gps;
    else return GPSAmbiguity::Fix_and_hold_gps;
}

QString toGpsAmbiguityString(GPSAmbiguity p) {
    if(p == GPSAmbiguity::Continuous_gps) return "Continuous";
    else if(p == GPSAmbiguity::Instantaneous_gps) return "Instantaneous";
    else if(p == GPSAmbiguity::Off_gps) return "Off";
    else return "Fix_and_hold";
}

GlonassAmbiguity toGloAmbiguity(QString s) {
    if(s == "Autocal") return GlonassAmbiguity::Autocal_glo;
    else if(s == "On") return GlonassAmbiguity::On_glo;
    else if(s == "Off") return GlonassAmbiguity::Off_glo;
    else return GlonassAmbiguity::Fix_and_hold_glo;
}

QString toGloAmbiguityString(GlonassAmbiguity p) {
    if(p == GlonassAmbiguity::Autocal_glo) return "Autocal";
    else if(p == GlonassAmbiguity::On_glo) return "On";
    else if(p == GlonassAmbiguity::Off_glo) return "Off";
    else return "Fix_and_hold";
}

BDSAmbiguity toBdsAmbiguity(QString s) {
    if(s == "Off") return BDSAmbiguity::Off_bds;
    else return BDSAmbiguity::On_bds;
}

QString toBdsAmbiguityString(BDSAmbiguity p) {
    if(p == BDSAmbiguity::Off_bds) return "Off";
    return "On";
}

BaseLocationType toBaseLocationType(QString s) {
    if(s == "Rinex_header") return BaseLocationType::Rinex_header;
    else if(s == "ECEF_XYZ") return BaseLocationType::ECEF_XYZ;
    return BaseLocationType::Lat_Lon_Height;
}

QString toBaseLocationTypeString(BaseLocationType p) {
    if(p == BaseLocationType::Rinex_header) return "Rinex_header";
    else if(p == BaseLocationType::ECEF_XYZ) return "ECEF_XYZ";
    return "Lat_Lon_Height";
}

ElevationType toElevationType(QString s) {
    if(s == "Ellipsoidal") return ElevationType::Ellipsoidal;
    else if(s == "MSL EGM 96") return ElevationType::MSL_EGM96;
    return ElevationType::MSL_ESM08;
}

QString toElevationTypeString(ElevationType p) {
    if(p == ElevationType::Ellipsoidal) return "Ellipsoidal";
    else if(p == ElevationType::MSL_EGM96) return "MSL EGM 96";
    return "MSL EGM 08";
}

UserPreference* BaselineUtils::LoadConfig(UserPreference *userPreference) {
    try {
        if(dynamic_cast<ConvertUserPreference*>(userPreference)) {
            ConvertUserPreference *convUserPreference = new ConvertUserPreference();
            QString jsonFile = QCoreApplication::applicationDirPath() + "/Data/convuserpreference.json";

            if(QFile::exists(jsonFile)) {
                QFile file(jsonFile);
                if(file.open(QIODevice::ReadOnly)) {
                    QByteArray jsonData = file.readAll();
                    file.close();

                    QJsonParseError parseError;
                    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
                    QJsonObject obj = doc.object();

                    std::vector<QString> data = {
                        "GPS", "GLO", "GAL", "QZSS", "BEI", "SBS", "NavIC",
                        "L1", "L2E5", "format", "RinexVersion", "sepnav", "receiver",
                        "marker", "antenna", "appx", "appy", "appz", "poleh", "polee",
                        "polen", "phase", "halfc", "sort", "iono", "timecorr", "leap",
                        "signalsb1", "signalsb2", "signalsb3", "signalsb4", "signalsb5",
                        "signalsb6", "round", "exsat",
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

                    foreach (QString d, data) {
                        QJsonValue val = obj.value(d);
                        if (d == "format") {
                            convUserPreference->format = Format::UBX;
                        } else if (d == "RinexVersion") {
                            convUserPreference->RinexVersion = val.toString().toDouble();
                        } else if (d == "GPS") {
                            convUserPreference->GPS = val.toString() == "1" ? true : false;
                        } else if (d == "GLO") {
                            convUserPreference->GLO = val.toString() == "1" ? true : false;
                        } else if (d == "GAL") {
                            convUserPreference->GAL = val.toString() == "1" ? true : false;
                        } else if (d == "QZSS") {
                            convUserPreference->QZSS = val.toString() == "1" ? true : false;
                        } else if (d == "BEI") {
                            convUserPreference->BEI = val.toString() == "1" ? true : false;
                        } else if (d == "SBS") {
                            convUserPreference->SBS = val.toString() == "1" ? true : false;
                        } else if (d == "NavIC") {
                            convUserPreference->NavIC = val.toString() == "1" ? true : false;
                        } else if (d == "L1") {
                            convUserPreference->L1 = val.toString() == "1" ? true : false;
                        } else if (d == "L2E5") {
                            convUserPreference->L2E5 = val.toString() == "1" ? true : false;
                        } else if (d == "sepnav") {
                            convUserPreference->sepnav = val.toString() == "1" ? true : false;
                        } else if (d == "receiver") {
                            convUserPreference->receiver = val.toString();
                        } else if (d == "marker") {
                            convUserPreference->marker = val.toString();
                        } else if (d == "antenna") {
                            convUserPreference->antenna = val.toString();
                        } else if (d == "appx") {
                            convUserPreference->appx = val.toString().toDouble();
                        } else if (d == "appy") {
                            convUserPreference->appy = val.toString().toDouble();
                        } else if (d == "appz") {
                            convUserPreference->appz = val.toString().toDouble();
                        } else if (d == "poleh") {
                            convUserPreference->poleh = val.toString().toDouble();
                        } else if (d == "polee") {
                            convUserPreference->polee = val.toString().toDouble();
                        } else if (d == "polen") {
                            convUserPreference->polen = val.toString().toDouble();
                        } else if (d == "phase") {
                            convUserPreference->phase = val.toString() == "1" ? true : false;
                        } else if (d == "halfc") {
                            convUserPreference->halfc = val.toString() == "1" ? true : false;
                        } else if (d == "sort") {
                            convUserPreference->sort = val.toString() == "1" ? true : false;
                        } else if (d == "iono") {
                            convUserPreference->iono = val.toString() == "1" ? true : false;
                        } else if (d == "timecorr") {
                            convUserPreference->timecorr = val.toString() == "1" ? true : false;
                        } else if (d == "leap") {
                            convUserPreference->leap = val.toString() == "1" ? true : false;
                        } else if (d == "signalsb1") {
                            convUserPreference->sigband1 = val.toString() == "1" ? true : false;
                        } else if (d == "signalsb2") {
                            convUserPreference->sigband2 = val.toString() == "1" ? true : false;
                        } else if (d == "signalsb3") {
                            convUserPreference->sigband3 = val.toString() == "1" ? true : false;
                        } else if (d == "signalsb4") {
                            convUserPreference->sigband4 = val.toString() == "1" ? true : false;
                        } else if (d == "signalsb5") {
                            convUserPreference->sigband5 = val.toString() == "1" ? true : false;
                        } else if (d == "signalsb6") {
                            convUserPreference->sigband6 = val.toString() == "1" ? true : false;
                        } else if (d == "round") {
                            convUserPreference->round = val.toString() == "1" ? true : false;
                        } else if (d == "exsat") {
                            convUserPreference->exsat = val.toString();
                        } else {
                            convUserPreference->gnsssignals[d] = val.toString() == "1" ? true : false;
                        }
                    }
                }
            }
            return convUserPreference;
        } else {
            PostProcessUserPreference *postUserPreference = new PostProcessUserPreference();
            QString jsonFile = QCoreApplication::applicationDirPath() + "/Data/postuserpreference.json";

            if(QFile::exists(jsonFile)) {
                QFile file(jsonFile);
                if(file.open(QIODevice::ReadOnly)) {
                    QByteArray jsonData = file.readAll();
                    file.close();

                    QJsonParseError parseError;
                    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
                    QJsonObject obj = doc.object();

                    std::vector<QString> data = { "GPS", "GLO", "GAL", "QZSS", "BEI", "SBS", "elevationMask", "posMode", "filterType", "frequency", "gpsAmbiguity", "gloAmbiguity", "bdsAmbiguity", "baseLocationType", "elevationType" };

                    foreach (QString d, data) {
                        QJsonValue val = obj.value(d);
                        if (d == "GPS") {
                            postUserPreference->GPS = val.toString() == "1" ? true : false;
                        } else if (d == "GLO") {
                            postUserPreference->GLO = val.toString() == "1" ? true : false;
                        } else if (d == "GAL") {
                            postUserPreference->GAL = val.toString() == "1" ? true : false;
                        } else if (d == "QZSS") {
                            postUserPreference->QZSS = val.toString() == "1" ? true : false;
                        } else if (d == "BEI") {
                            postUserPreference->BEI = val.toString() == "1" ? true : false;
                        } else if (d == "SBS") {
                            postUserPreference->SBS = val.toString() == "1" ? true : false;
                        } else if (d == "elevationMask") {
                            postUserPreference->elevationMask = val.toString().toInt();
                        } else if (d == "posMode") {
                            postUserPreference->posMode = toPosMode(val.toString());
                        } else if (d == "filterType") {
                            postUserPreference->filterType = toFilterType(val.toString());
                        } else if (d == "frequency") {
                            postUserPreference->frequency = toFrequency(val.toString());
                        } else if (d == "gpsAmbiguity") {
                            postUserPreference->gpsAmbiguity = toGpsAmbiguity(val.toString());
                        } else if (d == "gloAmbiguity") {
                            postUserPreference->gloAmbiguity = toGloAmbiguity(val.toString());
                        } else if (d == "bdsAmbiguity") {
                            postUserPreference->bdsAmbiguity = toBdsAmbiguity(val.toString());
                        } else if (d == "baseLocationType") {
                            postUserPreference->baseLocationType = toBaseLocationType(val.toString());
                        } else if(d == "elevationType") {
                            postUserPreference->elevationType = toElevationType(val.toString());
                        }
                    }
                }
            }
            return postUserPreference;
        }
    }
    catch (const std::runtime_error& e) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
        messageBox->exec();
    }
    catch (const std::exception& e) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
        messageBox->exec();
    }
    catch (...) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", "An Unknown exception has occurred...", "OK");
        messageBox->exec();
    }
    return new UserPreference();
}



void BaselineUtils::SaveConfig(UserPreference *userPreference) {
    try {
        if(dynamic_cast<ConvertUserPreference*>(userPreference)) {
            ConvertUserPreference *convUserPreference = dynamic_cast<ConvertUserPreference*>(userPreference);
            QString jsonFile = QCoreApplication::applicationDirPath() + "/Data/convuserpreference.json";

            QJsonObject obj;

            std::vector<QString> data = {
                "GPS", "GLO", "GAL", "QZSS", "BEI", "SBS", "NavIC",
                "L1", "L2E5", "format", "RinexVersion", "sepnav", "receiver",
                "marker", "antenna", "appx", "appy", "appz", "poleh", "polee",
                "polen", "phase", "halfc", "sort", "iono", "timecorr", "leap",
                "signalsb1", "signalsb2", "signalsb3", "signalsb4", "signalsb5",
                "signalsb6", "round", "exsat",
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

            foreach (QString d, data) {
                if (d == "format") {
                    obj[d] = "UBX";
                } else if (d == "RinexVersion") {
                    obj[d] = QString::number(convUserPreference->RinexVersion);
                } else if (d == "GPS") {
                    obj[d] = QString::number(convUserPreference->GPS);
                } else if (d == "GLO") {
                    obj[d] = QString::number(convUserPreference->GLO);
                } else if (d == "GAL") {
                    obj[d] = QString::number(convUserPreference->GAL);
                } else if (d == "QZSS") {
                    obj[d] = QString::number(convUserPreference->QZSS);
                } else if (d == "BEI") {
                    obj[d] = QString::number(convUserPreference->BEI);
                } else if (d == "SBS") {
                    obj[d] = QString::number(convUserPreference->SBS);
                } else if (d == "NavIC") {
                    obj[d] = QString::number(convUserPreference->NavIC);
                } else if (d == "L1") {
                    obj[d] = QString::number(convUserPreference->L1);
                } else if (d == "L2E5") {
                    obj[d] = QString::number(convUserPreference->L2E5);
                } else if (d == "sepnav") {
                    obj[d] = QString::number(convUserPreference->sepnav);
                } else if (d == "receiver") {
                    obj[d] = convUserPreference->receiver;
                } else if (d == "marker") {
                    obj[d] = convUserPreference->marker;
                } else if (d == "antenna") {
                    obj[d] = convUserPreference->antenna;
                } else if (d == "appx") {
                    obj[d] = QString::number(convUserPreference->appx);
                } else if (d == "appy") {
                    obj[d] = QString::number(convUserPreference->appy);
                } else if (d == "appz") {
                    obj[d] = QString::number(convUserPreference->appz);
                } else if (d == "poleh") {
                    obj[d] = QString::number(convUserPreference->poleh);
                } else if (d == "polee") {
                    obj[d] = QString::number(convUserPreference->polee);
                } else if (d == "polen") {
                    obj[d] = QString::number(convUserPreference->polen);
                } else if (d == "phase") {
                    obj[d] = QString::number(convUserPreference->phase);
                } else if (d == "halfc") {
                    obj[d] = QString::number(convUserPreference->halfc);
                } else if (d == "sort") {
                    obj[d] = QString::number(convUserPreference->sort);
                } else if (d == "iono") {
                    obj[d] = QString::number(convUserPreference->iono);
                } else if (d == "timecorr") {
                    obj[d] = QString::number(convUserPreference->timecorr);
                } else if (d == "leap") {
                    obj[d] = QString::number(convUserPreference->leap);
                } else if (d == "signalsb1") {
                    obj[d] = QString::number(convUserPreference->sigband1);
                } else if (d == "signalsb2") {
                    obj[d] = QString::number(convUserPreference->sigband2);
                } else if (d == "signalsb3") {
                    obj[d] = QString::number(convUserPreference->sigband3);
                } else if (d == "signalsb4") {
                    obj[d] = QString::number(convUserPreference->sigband4);
                } else if (d == "signalsb5") {
                    obj[d] = QString::number(convUserPreference->sigband5);
                } else if (d == "signalsb6") {
                    obj[d] = QString::number(convUserPreference->sigband6);
                } else if (d == "round") {
                    obj[d] = QString::number(convUserPreference->round);
                } else if (d == "exsat") {
                    obj[d] = convUserPreference->exsat;
                } else {
                    obj[d] = QString::number(convUserPreference->gnsssignals[d]);
                }
            }

            QJsonDocument jsonDoc(obj);

            QFile filew(jsonFile);
            if(filew.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                filew.write(jsonDoc.toJson());
                filew.close();
            }
        }
        else if(dynamic_cast<PostProcessUserPreference*>(userPreference)) {
            PostProcessUserPreference *postUserPreference = dynamic_cast<PostProcessUserPreference*>(userPreference);
            QString jsonFile = QCoreApplication::applicationDirPath() + "/Data/postuserpreference.json";
            QFile file(jsonFile);

            QJsonObject obj;

            std::vector<QString> data = { "GPS", "GLO", "GAL", "QZSS", "BEI", "SBS", "elevationMask", "posMode", "filterType", "frequency", "gpsAmbiguity", "gloAmbiguity", "bdsAmbiguity", "baseLocationType", "elevationType" };

            foreach (QString d, data) {
                if (d == "GPS") {
                    obj[d] = QString::number(postUserPreference->GPS);
                } else if (d == "GLO") {
                    obj[d] = QString::number(postUserPreference->GLO);
                } else if (d == "GAL") {
                    obj[d] = QString::number(postUserPreference->GAL);
                } else if (d == "QZSS") {
                    obj[d] = QString::number(postUserPreference->QZSS);
                } else if (d == "BEI") {
                    obj[d] = QString::number(postUserPreference->BEI);
                } else if (d == "SBS") {
                    obj[d] = QString::number(postUserPreference->SBS);
                } else if (d == "elevationMask") {
                    obj[d] = QString::number(postUserPreference->elevationMask);
                } else if (d == "posMode") {
                    obj[d] = toPosModeString(postUserPreference->posMode);
                } else if (d == "filterType") {
                    obj[d] = toFilterTypeString(postUserPreference->filterType);
                } else if(d == "frequency") {
                    obj[d] = toFrequencyString(postUserPreference->frequency);
                } else if (d == "gpsAmbiguity") {
                    obj[d] = toGpsAmbiguityString(postUserPreference->gpsAmbiguity);
                } else if (d == "gloAmbiguity") {
                    obj[d] = toGloAmbiguityString(postUserPreference->gloAmbiguity);
                } else if(d == "bdsAmbiguity") {
                    obj[d] = toBdsAmbiguityString(postUserPreference->bdsAmbiguity);
                } else if(d == "baseLocationType") {
                    obj[d] = toBaseLocationTypeString(postUserPreference->baseLocationType);
                } else if(d == "elevationType") {
                    obj[d] = toElevationTypeString(postUserPreference->elevationType);
                }
            }

            QJsonDocument jsonDoc(obj);

            QFile filew(jsonFile);
            if(filew.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                filew.write(jsonDoc.toJson());
                filew.close();
            }
        }
    }
    catch (const std::runtime_error& e) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
        messageBox->exec();
    }
    catch (const std::exception& e) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
        messageBox->exec();
    }
    catch (...) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", "An Unknown exception has occurred...", "OK");
        messageBox->exec();
    }
}

void BaselineUtils::GeneratePostConfigFile(PostProcessUserPreference *postUserPreference, ElevationType etype, int elemask, double roverPoleHeight, double basePoleHeight,double x, double y, double z) {
    try {
        QString confFile = QCoreApplication::applicationDirPath() + "/Data/rtkpost.conf";
        QString posMode, freq, filter, gpsAmbiguity, gloAmbiguity, bdsAmbiguity, x_val, y_val, z_val, baseType, height, geoid;

        if(postUserPreference->posMode == PosMode::Static) {
            posMode = "static";
        } else if(postUserPreference->posMode == PosMode::Kinematic) {
            posMode = "kinematic";
        } else if(postUserPreference->posMode == PosMode::Single) {
            posMode = "single";
        }

        if(postUserPreference->frequency == Frequency::L1) {
            freq = "l1";
        } else if(postUserPreference->frequency == Frequency::L1_L2) {
            freq = "l1+l2";
        } else if(postUserPreference->frequency == Frequency::L1_L2_L3){
            freq = "l1+l2+l5";
        }else if(postUserPreference->frequency == Frequency::L1_L2_L3_L4){
            freq = "l1+l2+l5+l6";
        }

        if(postUserPreference->filterType == FilterType::Forward) {
            filter = "forward";
        } else if(postUserPreference->filterType == FilterType::Backward) {
            filter = "backward";
        } else if(postUserPreference->filterType == FilterType::Combined) {
            filter = "combined";
        } else if(postUserPreference->filterType == FilterType::Combined_no_phase_reset) {
            filter = "combined-nophasereset";
        }

        if(postUserPreference->gpsAmbiguity == GPSAmbiguity::Continuous_gps) {
            gpsAmbiguity = "continuous";
        } else if(postUserPreference->gpsAmbiguity == GPSAmbiguity::Off_gps) {
            gpsAmbiguity = "off";
        } else if(postUserPreference->gpsAmbiguity == GPSAmbiguity::Instantaneous_gps) {
            gpsAmbiguity = "instantaneous";
        } else if(postUserPreference->gpsAmbiguity == GPSAmbiguity::Fix_and_hold_gps) {
            gpsAmbiguity = "fix-and-hold";
        }

        if(postUserPreference->gloAmbiguity == GlonassAmbiguity::Off_glo) {
            gloAmbiguity = "off";
        } else if(postUserPreference->gloAmbiguity == GlonassAmbiguity::On_glo) {
            gloAmbiguity = "on";
        } else if(postUserPreference->gloAmbiguity == GlonassAmbiguity::Autocal_glo) {
            gloAmbiguity = "autocal";
        } else if(postUserPreference->gloAmbiguity == GlonassAmbiguity::Fix_and_hold_glo) {
            gloAmbiguity = "fix-and-hold";
        }

        if(postUserPreference->bdsAmbiguity == BDSAmbiguity::Off_bds) {
            bdsAmbiguity = "off";
        } else if(postUserPreference->bdsAmbiguity == BDSAmbiguity::On_bds) {
            bdsAmbiguity = "on";
        }

        if(postUserPreference->baseLocationType == BaseLocationType::Rinex_header) {
            baseType = "rinexhead";
            x_val = "90";
            y_val = "0";
            z_val = "-6335367.6285";
        } else if(postUserPreference->baseLocationType == BaseLocationType::Lat_Lon_Height) {
            baseType = "llh";
            x_val = QString::number(x,'f',9);
            y_val = QString::number(y,'f',9);
            z_val = QString::number(z,'f',4);
        } else if(postUserPreference->baseLocationType == BaseLocationType::ECEF_XYZ){
            baseType = "xyz";
            x_val = QString::number(x,'f',4);
            y_val = QString::number(y,'f',4);
            z_val = QString::number(z,'f',4);
        }

        if(etype == ElevationType::Ellipsoidal) {
            height = "ellipsoidal";
            geoid = "internal";
        } else if(etype == ElevationType::MSL_EGM96) {
            height = "geodetic";
            geoid = "egm96";
        } else if(etype == ElevationType::MSL_ESM08) {
            height = "geodetic";
            geoid = "egm08_2.5";
        }

        int satellites = 0;
        if(postUserPreference->GPS) satellites += 1;
        if(postUserPreference->SBS) satellites += 2;
        if(postUserPreference->GLO) satellites += 4;
        if(postUserPreference->GAL) satellites += 8;
        if(postUserPreference->QZSS) satellites += 16;
        if(postUserPreference->BEI) satellites += 32;

        QString snrmask = "0";
        QString snr = snrmask + "," + snrmask + "," + snrmask + "," + snrmask + "," + snrmask + "," + snrmask + "," + snrmask + "," + snrmask + "," + snrmask;

        std::vector<std::vector<QString>> configParams = {
            { "pos1-posmode", posMode },     // (0:single,1:dgps,2:kinematic,3:static,4:static-start,5:movingbase,6:fixed,7:ppp-kine,8:ppp-static,9:ppp-fixed)
            { "pos1-frequency", freq },       // (1:l1,2:l1+l2,3:l1+l2+l5,4:l1+l2+l5+l6)
            { "pos1-soltype", filter },       // (0:forward,1:backward,2:combined,3:combined-nophasereset)
            { "pos1-elmask", QString::number(elemask) },              // (deg)
            { "pos1-snrmask_r", "off" },          // (0:off,1:on)
            { "pos1-snrmask_b", "off" },          // (0:off,1:on)
            { "pos1-snrmask_L1", snr},
            { "pos1-snrmask_L2", snr},
            { "pos1-snrmask_L5", snr},
            { "pos1-dynamics", "off" },           // (0:off,1:on)
            { "pos1-tidecorr", "off" },          // (0:off,1:on,2:otl)
            { "pos1-ionoopt", "brdc" },          // (0:off,1:brdc,2:sbas,3:dual-freq,4:est-stec,5:ionex-tec,6:qzs-brdc)
            { "pos1-tropopt", "saas" },          // (0:off,1:saas,2:sbas,3:est-ztd,4:est-ztdgrad)
            { "pos1-sateph", "brdc" },           // (0:brdc,1:precise,2:brdc+sbas,3:brdc+ssrapc,4:brdc+ssrcom)
            { "pos1-posopt1", "off" },           // (0:off,1:on)
            { "pos1-posopt2", "off" },           // (0:off,1:on)
            { "pos1-posopt3", "off" },           // (0:off,1:on,2:precise)
            { "pos1-posopt4", "off" },           // (0:off,1:on)
            { "pos1-posopt5", "on" },            // (0:off,1:on)
            { "pos1-posopt6", "off" },           // (0:off,1:on)
            { "pos1-exclsats", "" },             // (prn ...)
            { "pos1-navsys", QString::number(satellites) },             // (1:gps+2:sbas+4:glo+8:gal+16:qzs+32:bds+64:navic)
            { "pos2-armode", gpsAmbiguity },   // (0:off,1:continuous,2:instantaneous,3:fix-and-hold)
            { "pos2-gloarmode", gloAmbiguity },// (0:off,1:on,2:autocal,3:fix-and-hold)
            { "pos2-bdsarmode", bdsAmbiguity },          // (0:off,1:on)
            { "pos2-arfilter", "on" },           // (0:off,1:on)
            { "pos2-arthres", "3" },
            { "pos2-arthresmin", "3" },
            { "pos2-arthresmax", "3" },
            { "pos2-arthres1", "0.1" },
            { "pos2-arthres2", "0" },
            { "pos2-arthres3", "1e-09" },
            { "pos2-arthres4", "1e-05" },
            { "pos2-varholdamb", "0.1" },        // (cyc^2)
            { "pos2-gainholdamb", "0.01" },
            { "pos2-arlockcnt", "0" },
            { "pos2-minfixsats", "4" },
            { "pos2-minholdsats", "5" },
            { "pos2-mindropsats", "10" },
            { "pos2-arelmask", "15" },           // (deg)
            { "pos2-arminfix", "20" },
            { "pos2-armaxiter", "1" },
            { "pos2-elmaskhold", "15" },         // (deg)
            { "pos2-aroutcnt", "20" },
            { "pos2-maxage", "30" },             // (s)
            { "pos2-syncsol", "off" },           // (0:off,1:on)
            { "pos2-slipthres", "0.05" },        // (m)
            { "pos2-dopthres", "0" },            // (m)
            { "pos2-rejionno", "5" },            // (m)
            { "pos2-rejcode", "30" },            // (m)
            { "pos2-niter", "1" },
            { "pos2-baselen", "0" },             // (m)
            { "pos2-basesig", "0" },             // (m)
            { "out-solformat", "llh" },          // (0:llh,1:xyz,2:enu,3:nmea)
            { "out-outhead", "on" },             // (0:off,1:on)
            { "out-outopt", "on" },              // (0:off,1:on)
            { "out-outvel", "off" },             // (0:off,1:on)
            { "out-timesys", "gpst" },           // (0:gpst,1:utc,2:jst)
            { "out-timeform", "hms" },           // (0:tow,1:hms)
            { "out-timendec", "3" },
            { "out-degform", "deg" },            // (0:deg,1:dms)
            { "out-fieldsep", "" },
            { "out-outsingle", "off" },          // (0:off,1:on)
            { "out-maxsolstd", "0" },            // (m)
            { "out-height", height },        // (0:ellipsoidal,1:geodetic)
            { "out-geoid", geoid },            // (0:internal,1:egm96,2:egm08_2.5,3:egm08_1,4:gsi2000)
            { "out-solstatic", "all" },          // (0:all,1:single)
            { "out-nmeaintv1", "0" },            // (s)
            { "out-nmeaintv2", "0" },            // (s)
            { "out-outstat", "residual" },       // (0:off,1:state,2:residual) for .stat
            { "out-event", "off" },          // for _events.pos
            { "stats-eratio1", "300" },
            { "stats-eratio2", "300" },
            { "stats-eratio5", "300" },
            { "stats-errphase", "0.003" },       // (m)
            { "stats-errphaseel", "0.003" },     // (m)
            { "stats-errphasebl", "0" },         // (m/10km)
            { "stats-errdoppler", "1" },         // (Hz)
            { "stats-snrmax", "52" },            // (dB.Hz)
            { "stats-errsnr", "0" },             // (m)
            { "stats-errrcv", "0" },             // ( )
            { "stats-stdbias", "30" },           // (m)
            { "stats-stdiono", "0.03" },         // (m)
            { "stats-stdtrop", "0.3" },          // (m)
            { "stats-prnaccelh", "3" },          // (m/s^2)
            { "stats-prnaccelv", "1" },          // (m/s^2)
            { "stats-prnbias", "0.0001" },            // (m)
            { "stats-prniono", "0.001" },            // (m)
            { "stats-prntrop", "0.0001" },            // (m)
            {"stats-prnpos" , "0" },           //# (m)
            { "stats-clkstab", " 5e-12" },        // (s/s)
            {"ant1-postype"     ,  "llh" },          //# (0:llh,1:xyz,2:single,3:posfile,4:rinexhead,5:rtcm,6:raw)
            {"ant1-pos1"        ,  "90"  },          //# (deg|m)
            {"ant1-pos2"        ,  "0"    },       //# (deg|m)
            {"ant1-pos3"        ,  "-6335367.6285" }, //# (m|m)
            {"ant1-anttype"     ,  ""  },
            {"ant1-antdele"     ,  "0"},  //          # (m)
            {"ant1-antdeln"     ,  "0"},  //          # (m)
            {"ant1-antdelu"     ,  QString::number(roverPoleHeight, 'f', 4)},  //          # (m)
            {"ant2-postype"     ,  baseType }, //  # (0:llh,1:xyz,2:single,3:posfile,4:rinexhead,5:rtcm,6:raw)
            {"ant2-pos1"        ,  x_val}, //         # (deg|m)
            {"ant2-pos2"        ,  y_val}, //         # (deg|m)
            {"ant2-pos3"        ,  z_val}, //         # (m|m)
            {"ant2-anttype"     ,  "" }, //
            {"ant2-antdele"     ,  "0"}, //        # (m)
            {"ant2-antdeln"     ,  "0"}, //        # (m)
            {"ant2-antdelu"     ,  QString::number(basePoleHeight, 'f', 4)}, //         # (m)
            {"ant2-maxaveep"    ,  "1" },
            {"ant2-initrst"     ,  "on" },//         # (0:off,1:on)
            {"misc-timeinterp"  ,  "off" },//       # (0:off,1:on)
            {"misc-sbasatsel"   ,  "0" },//          # (0:all)
            {"misc-rnxopt1"     ,  ""},
            {"misc-rnxopt2"     ,  ""},
            {"misc-pppopt"      ,  ""},
            {"file-satantfile"  ,  ""},
            {"file-rcvantfile"  ,  ""},
            {"file-staposfile"  ,  ""},
            {"file-geoidfile"   ,  ""},
            {"file-ionofile"    ,  ""},
            {"file-dcbfile"     ,  ""},
            {"file-eopfile"     ,  ""},
            {"file-blqfile"     ,  ""},
            {"file-tempdir"     ,  ""},
            {"file-geexefile"   ,  ""},
            {"file-solstatfile" ,  ""},
            {"file-tracefile"  ,  ""}
        };

        QFile file(confFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for(std::vector<QString> p : configParams) {
                out << p[0] + "=" + p[1] << "\n";
            }
            file.close();
        }
    }
    catch (const std::runtime_error& e) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
        messageBox->exec();
    }
    catch (const std::exception& e) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
        messageBox->exec();
    }
    catch (...) {
        CustomMessageBox *messageBox = new CustomMessageBox("ERROR", "An Unknown exception has occurred...", "OK");
        messageBox->exec();
    }
}

std::pair<std::pair<int, QString>, TempPosData*> BaselineUtils::ReadPosFile(QString posFile) {
    try {
        TempPosData *posData = new TempPosData();
        Utils *util = new Utils();

        QFile file(posFile);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while(!in.atEnd()) {
                QString line = in.readLine();
                if(line.startsWith('%')) {
                    int indexk = line.indexOf(':');
                    QString key, value;
                    if (indexk != -1) {
                        key = line.left(indexk);
                        value = line.mid(indexk+1);
                    }
                    QStringList linedata;
                    linedata.append(key);
                    linedata.append(value);
                    if(linedata.size() >= 2) {
                        if(linedata[0].contains("obs start")) {
                            QStringList valueslinedata = linedata[1].trimmed().split(' ', Qt::SkipEmptyParts);
                            if(valueslinedata.size() >= 2) {
                                QDateTime startTime = util->DateTimeFromPos(valueslinedata[0].trimmed(), valueslinedata[1].trimmed());
                                posData->ISTStartTime = util->GPSTtoIST(startTime);
                            }
                        }
                        if(linedata[0].contains("obs end")) {
                            QStringList valueslinedata = linedata[1].trimmed().split(' ', Qt::SkipEmptyParts);
                            if(valueslinedata.size() >= 2) {
                                QDateTime endTime = util->DateTimeFromPos(valueslinedata[0].trimmed(), valueslinedata[1].trimmed());
                                posData->ISTEndTime = util->GPSTtoIST(endTime);
                            }
                        }
                        if(linedata[0].contains("pos mode")) {
                            QString mode = linedata[1].trimmed();
                            if(mode == "Static") {
                                posData->posMode = PosMode::Static;
                            }
                            if(mode == "Kinematic") {
                                posData->posMode = PosMode::Kinematic;
                            }
                            if(mode == "Single") {
                                posData->posMode = PosMode::Single;
                            }
                        }
                        if(linedata[0].contains("ref pos")) {
                            QStringList valueslinedata = linedata[1].trimmed().split(' ', Qt::SkipEmptyParts);
                            if(valueslinedata.size() >= 3) {
                                posData->basecoordinate.latitude = valueslinedata[0].trimmed().toDouble();
                                posData->basecoordinate.longitude = valueslinedata[1].trimmed().toDouble();
                                posData->basecoordinate.elevation.Ellipsoidal = valueslinedata[2].trimmed().toDouble();
                                posData->basecoordinate.elevation.geoid = GeoidModel::ELIP;
                            }
                        }
                    }
                    continue;
                }
                QStringList values = line.split(' ', Qt::SkipEmptyParts);
                Pointquality qual = Pointquality::NoFix;
                if(values.size() < 6) continue;
                QString q = values[5].trimmed();
                if(q == "1") {
                    qual = Pointquality::Fix;
                } else if(q == "2") {
                    qual = Pointquality::Float;
                } else if(q == "5") {
                    qual = Pointquality::single;
                }
                PosPoint *point = new PosPoint();
                point->coordinate.latitude = values[2].trimmed().toDouble();
                point->coordinate.longitude = values[3].trimmed().toDouble();
                point->coordinate.elevation.Ellipsoidal = values[4].trimmed().toDouble();
                point->coordinate.elevation.geoid = GeoidModel::ELIP;
                point->quality = qual;
                point->ISTDateTime = util->GPSTtoIST(util->DateTimeFromPos(values[0].trimmed(), values[1].trimmed()));
                posData->addPoint(point);
            }
            file.close();
        }
        delete util;
        return {{1, ""}, posData};
    }
    catch (const std::runtime_error& e) {
        return {{0, e.what()}, new TempPosData()};
    }
    catch (const std::exception& e) {
        return {{0, e.what()}, new TempPosData()};
    }
    catch (...) {
        return {{0, "Unknown error occured."}, new TempPosData()};
    }
    return {{0, "Unknown error occured."}, new TempPosData()};
}


std::pair<std::pair<int, QString>, ObsData*> BaselineUtils::ReadObsFile(QString obsFile) {
    try {
        ObsData *data = new ObsData();
        data->basedata = new BaseJsonData();
        QFile file(obsFile);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while(!in.atEnd()) {
                QString line = in.readLine();

                if(line.contains("END OF HEADER")) {
                    break;
                } else if(line.contains("TIME OF FIRST OBS")) {
                    data->ISTStartTime = data->ParseTimeFromObs(line);
                } else if(line.contains("TIME OF LAST OBS")) {
                    data->ISTEndTime = data->ParseTimeFromObs(line);
                } else if(line.contains("SYS / # / OBS TYPES")) {
                    QChar constellation = line[0];
                    QString countString = line.mid(5, 2).trimmed();
                    int count = countString.toInt();

                    if(constellation == 'G') data->gps = count;
                    else if(constellation == 'R') data->glo = count;
                    else if(constellation == 'E') data->gal = count;
                    else if(constellation == 'C') data->bei = count;
                    else if(constellation == 'J') data->qzss = count;
                    else if(constellation == 'S') data->sbs = count;
                } else if(line.contains("APPROX POSITION XYZ")) {
                    QStringList str = line.trimmed().split(' ', Qt::SkipEmptyParts);
                    data->basedata->longitude = str[0].toDouble();
                    data->basedata->latitude = str[1].toDouble();
                    data->basedata->elevation = str[2].toDouble();
                }
            }
        }

        if(data->ISTStartTime.isValid()) {
            Utils *util = new Utils();
            data->ISTStartTime = util->GPSTtoIST(data->ISTStartTime);
        }
        if(data->ISTEndTime.isValid()) {
            Utils *util = new Utils();
            data->ISTEndTime = util->GPSTtoIST(data->ISTEndTime);
        }
        if(data->ISTStartTime.isValid() && data->ISTEndTime.isValid()) {
            return {{1, ""}, data};
        }
        return {{0, "Unable to read observation file."}, new ObsData()};
    }
    catch (const std::runtime_error& e) {
        return {{0, e.what()}, new ObsData()};
    }
    catch (const std::exception& e) {
        return {{0, e.what()}, new ObsData()};
    }
    catch (...) {
        return {{0, "An Unknown error has occured."}, new ObsData()};
    }
    return {{0, "An Unknown error has occured."}, new ObsData()};
}

std::pair<std::pair<int, QString>, BaseJsonData*> BaselineUtils::ReadBaseJsonFile(QString jsonFile) {
    BaseJsonData *data = new BaseJsonData();
    if(QFile::exists(jsonFile)) {
        QFile file(jsonFile);
        if(file.open(QIODevice::ReadOnly)) {
            QByteArray jsonData = file.readAll();
            file.close();

            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
            QJsonObject obj = doc.object();

            data->latitude = obj.value("lat").toDouble();
            data->longitude = obj.value("lng").toDouble();
            data->elevation = obj.value("alt").toDouble();
        }
    }
    return {{1, ""}, data};
}

// #include "processutils.h"
// #include <QFileInfo>
// #include <QtMath>
// #include <QCoreApplication>
// #include "proj.h"
// #include <QDebug>
// #include <QRegularExpression>
// static constexpr double WGS84_A = 6378137.0;
// static constexpr double WGS84_F = 1.0/298.257223563;
// static constexpr double WGS84_E2 = 2*WGS84_F - WGS84_F*WGS84_F;

// ProcessUtils::ProcessUtils(QObject *parent) : QObject{parent}
// {
//     qputenv("PROJ_DATA", QCoreApplication::applicationDirPath().toUtf8());
// }

// static PJ *createVGridPipeline()
// {

//     QString grid = QStringLiteral("proj/us_nga_egm08_25.tif");
//     QString pipeline = QString("+proj=pipeline +step +proj=vgridshift +grids=%1").arg(grid);

//     PJ *P = proj_create(nullptr, pipeline.toLocal8Bit().constData());
//     if (!P) {
//         QString alt = QString("+proj=pipeline +step +proj=vgridshift +grids=\"%1\"").arg(grid);
//         P = proj_create(nullptr, alt.toLocal8Bit().constData());
//     }
//     return P;
// }

// static void destroyPipeline(PJ *P) {
//     if (P) proj_destroy(P);
// }

// static double projEllipsoidalToOrthometric(PJ *P, double latDeg, double lonDeg, double hEllip)
// {
//     if (!P) return std::numeric_limits<double>::quiet_NaN();

//     if (!std::isfinite(latDeg) || !std::isfinite(lonDeg) || !std::isfinite(hEllip)) {
//         return std::numeric_limits<double>::quiet_NaN();
//     }

//     constexpr double DEG2RAD = M_PI / 180.0;
//     auto tryOnce = [&](double inLonDeg, double inLatDeg)->double {
//         PJ_COORD in;
//         in.lpzt.lam = inLonDeg * DEG2RAD;
//         in.lpzt.phi = inLatDeg * DEG2RAD;
//         in.lpzt.z   = hEllip;
//         in.lpzt.t   = 0.0;

//         proj_errno_reset(P);
//         PJ_COORD out = proj_trans(P, PJ_FWD, in);
//         double H = out.lpzt.z;
//         return H;
//     };

//     double H = tryOnce(lonDeg, latDeg);
//     if (std::isfinite(H)) {
//         return H;
//     }
//     return std::numeric_limits<double>::quiet_NaN();
// }

// static double projGeoidUndulation(PJ *P, double latDeg, double lonDeg, double hEllip)
// {
//     double H = projEllipsoidalToOrthometric(P, latDeg, lonDeg, hEllip);
//     if (!std::isfinite(H))
//         return std::numeric_limits<double>::quiet_NaN();
//     return hEllip - H;
// }

// void ProcessUtils::updateAggregates(PosData &p, const EpochRecord &er)
// {
//     if (p.agg.n_epochs == 0) {
//         p.agg.firstEpoch = er.gpst;
//         p.agg.lastEpoch = er.gpst;
//         p.agg.bestEpochTime = er.gpst;
//         p.agg.bestLat = er.lat; p.agg.bestLon = er.lon; p.agg.bestH = er.h;
//         p.agg.bestRatio = er.ratio;
//     } else {
//         qint64 dtSec = p.agg.lastEpoch.secsTo(er.gpst);
//         if (dtSec > 0) { p.agg.sumIntervalSeconds += dtSec; p.agg.nIntervals++; }
//         p.agg.lastEpoch = er.gpst;
//         if (er.ratio > p.agg.bestRatio) {
//             p.agg.bestRatio = er.ratio;
//             p.agg.bestEpochTime = er.gpst;
//             p.agg.bestLat = er.lat; p.agg.bestLon = er.lon; p.agg.bestH = er.h;
//         }
//     }

//     p.agg.n_epochs++;
//     double n = double(p.agg.n_epochs);

//     // lat
//     double delta = er.lat - p.agg.meanLat;
//     p.agg.meanLat += delta / n;
//     p.agg.M2Lat += delta * (er.lat - p.agg.meanLat);

//     // lon
//     delta = er.lon - p.agg.meanLon;
//     p.agg.meanLon += delta / n;
//     p.agg.M2Lon += delta * (er.lon - p.agg.meanLon);

//     // height
//     delta = er.h - p.agg.meanH;
//     p.agg.meanH += delta / n;
//     p.agg.M2H += delta * (er.h - p.agg.meanH);

//     double varN = er.sdn * er.sdn;
//     double varE = er.sde * er.sde;
//     double varU = er.sdu * er.sdu;

//     auto signedSquare = [](double signed_sqrt_val)->double {
//         if (!std::isfinite(signed_sqrt_val)) return NAN;
//         double absval = std::abs(signed_sqrt_val);
//         double cov = absval * absval;
//         return (signed_sqrt_val < 0.0) ? -cov : cov;
//     };

//     double covNE = signedSquare(er.sdne);
//     double covEU = signedSquare(er.sdeu);
//     double covNU = signedSquare(er.sdun);

//     p.agg.sumVarE += varE;
//     p.agg.sumVarN += varN;
//     p.agg.sumVarU += varU;
//     p.agg.sumCovEN += covNE;
//     p.agg.sumCovEU += covEU;
//     p.agg.sumCovNU += covNU;
// }

// static void geodeticToECEF(double latDeg, double lonDeg, double h, double &X, double &Y, double &Z)
// {
//     double lat = qDegreesToRadians(latDeg);
//     double lon = qDegreesToRadians(lonDeg);
//     double sinlat = qSin(lat);
//     double coslat = qCos(lat);
//     double coslon = qCos(lon);
//     double sinlon = qSin(lon);
//     double N = WGS84_A / qSqrt(1.0 - WGS84_E2 * sinlat * sinlat);
//     X = (N + h) * coslat * coslon;
//     Y = (N + h) * coslat * sinlon;
//     Z = (N * (1.0 - WGS84_E2) + h) * sinlat;
// }

// static void buildECEFtoENUrotation(double latDeg, double lonDeg, double R[3][3])
// {
//     double lat = qDegreesToRadians(latDeg);
//     double lon = qDegreesToRadians(lonDeg);
//     double sinLat = qSin(lat), cosLat = qCos(lat);
//     double sinLon = qSin(lon), cosLon = qCos(lon);

//     //east
//     R[0][0] = -sinLon;           R[0][1] =  cosLon;           R[0][2] = 0.0;
//     //north
//     R[1][0] = -sinLat * cosLon;  R[1][1] = -sinLat * sinLon;  R[1][2] =  cosLat;
//     //up
//     R[2][0] =  cosLat * cosLon;  R[2][1] =  cosLat * sinLon;  R[2][2] =  sinLat;
// }

// static void enuFromEcefDelta(const double R[3][3], double dX, double dY, double dZ,
//                              double &E, double &N, double &U)
// {
//     E = R[0][0]*dX + R[0][1]*dY + R[0][2]*dZ;
//     N = R[1][0]*dX + R[1][1]*dY + R[1][2]*dZ;
//     U = R[2][0]*dX + R[2][1]*dY + R[2][2]*dZ;
// }

// static QString degToDMS(double deg)
// {
//     if (qIsNaN(deg)) return QString();
//     double absdeg = qAbs(deg);
//     int d = int(absdeg);
//     double rem = (absdeg - d) * 60.0;
//     int m = int(rem);
//     double s = (rem - m) * 60.0;
//     QString sign = (deg < 0.0) ? "-" : "";
//     return QString("%1%2°%3'%4\"").arg(sign).arg(d).arg(m,2,10,QChar('0')).arg(QString::number(s,'f',1));
// }

// static void geodesicInverseFallback(double lat1, double lon1, double lat2, double lon2, double &azi12Deg, double &distMeters)
// {

//     double X1,Y1,Z1, X2,Y2,Z2;
//     geodeticToECEF(lat1, lon1, 0.0, X1, Y1, Z1);
//     geodeticToECEF(lat2, lon2, 0.0, X2, Y2, Z2);
//     double dX = X2 - X1, dY = Y2 - Y1, dZ = Z2 - Z1;
//     double R[3][3];
//     buildECEFtoENUrotation(lat1, lon1, R);
//     double E,N,U;
//     enuFromEcefDelta(R, dX, dY, dZ, E, N, U);
//     distMeters = qSqrt(E*E + N*N);
//     double azRad = 0.0;
//     if (distMeters > 0.0) azRad = qAtan2(E, N);
//     azi12Deg = qRadiansToDegrees(azRad);
//     if (azi12Deg < 0.0) azi12Deg += 360.0;
// }

// static void matMul3x3(const double A[3][3], const double B[3][3], double out[3][3])
// {
//     for (int i=0;i<3;++i) {
//         for (int j=0;j<3;++j) {
//             double s = 0.0;
//             for (int k=0;k<3;++k) s += A[i][k] * B[k][j];
//             out[i][j] = s;
//         }
//     }
// }

// static void matTranspose3x3(const double A[3][3], double AT[3][3])
// {
//     for(int i=0;i<3;++i)
//         for(int j=0;j<3;++j)
//             AT[i][j] = A[j][i];
// }

// char latitudeToZoneLetter(double latitude) {
//     if (latitude >= 84 || latitude < -80) return 'Z';
//     const char letters[] = {
//         'C','D','E','F','G','H','J','K','L','M',
//         'N','P','Q','R','S','T','U','V','W','X'
//     };
//     int index = int((latitude + 80) / 8);
//     return letters[index];
// }


// UTMResult ProcessUtils::WGS84ToUTM(double lat, double lon, double alt)
// {
//     UTMResult result{};

//     result.zoneNumber = static_cast<int>((lon + 180) / 6) + 1;
//     result.zoneLetter = latitudeToZoneLetter(lat);

//     std::string projStr = "+proj=utm +zone=" + std::to_string(result.zoneNumber)
//                           + " +datum=WGS84 +units=m +no_defs";
//     if (lat < 0) projStr += " +south";

//     PJ_CONTEXT *C = proj_context_create();
//     if (!C) throw std::runtime_error("Failed to create PROJ context");

//     PJ *P = proj_create_crs_to_crs( C, "+proj=longlat +datum=WGS84 +no_defs", projStr.c_str(), nullptr );

//     if (!P) {
//         proj_context_destroy(C);
//         throw std::runtime_error("Failed to create projection");
//     }

//     PJ_COORD coord = proj_coord(lon, lat, alt, 0);
//     PJ_COORD utm   = proj_trans(P, PJ_FWD, coord);

//     result.easting  = utm.xyz.x;
//     result.northing = utm.xyz.y;
//     result.altitude = utm.xyz.z;

//     proj_destroy(P);
//     proj_context_destroy(C);

//     return result;
// }

// static QString arcsecToDMS(double arcsec)
// {
//     if (!qIsFinite(arcsec)) return QString("-");
//     double deg = arcsec / 3600.0;
//     double absdeg = qAbs(deg);
//     int d = int(absdeg);
//     double rem = (absdeg - d) * 60.0;
//     int m = int(rem);
//     double s = (rem - m) * 60.0;
//     QString sign = (deg < 0.0) ? "-" : "";
//     return QString("%1%2°%3'%4\"").arg(sign).arg(d).arg(m, 2, 10, QChar('0')).arg(QString::number(s, 'f', 1));
// }

// bool ProcessUtils::parsePosFileIntoPosData(const QString &posFilePath, PosData &out, QString &errorOut)
// {
//     QFile f(posFilePath);
//     if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         errorOut = QString("Cannot open file: %1").arg(posFilePath);
//         return false;
//     }
//     out.posPath = posFilePath;
//     out.fileKey = QFileInfo(posFilePath).fileName();
//     out.processedTime = QDateTime::currentDateTimeUtc();
//     QTextStream in(&f);
//     QRegularExpression headerRe("^%\\s*([^:]+?)\\s*:\\s*(.*)$");

//     while (!in.atEnd()) {
//         QString line = in.readLine().trimmed();
//         if (line.isEmpty()) continue;

//         if (line.startsWith('%')) {
//             QRegularExpressionMatch m = headerRe.match(line);
//             if (m.hasMatch()) {
//                 QString key = m.captured(1).trimmed();
//                 QString val = m.captured(2).trimmed();

//                 if (key.compare("inp file", Qt::CaseInsensitive) == 0) {
//                     if (out.roverPath.isEmpty())
//                         out.roverPath = val;
//                     else if (out.basePath.isEmpty())
//                         out.basePath = val;
//                 } else if (key.compare("freqs", Qt::CaseInsensitive) == 0) {
//                     out.frequency = val;
//                 } else if (key.compare("elev mask", Qt::CaseInsensitive) == 0) {
//                     out.elevMask = val;
//                 } else if (key.compare("ephemeris", Qt::CaseInsensitive) == 0) {
//                     out.ephemeris = val;
//                 }
//                 else if (key.compare("obs start", Qt::CaseInsensitive) == 0) {

//                     QRegularExpression dtRe("(\\d{4}/\\d{2}/\\d{2})\\s+(\\d{2}:\\d{2}:\\d{2}(?:\\.\\d+)?)");
//                     auto dm = dtRe.match(val);
//                     if (dm.hasMatch()) {
//                         QString d = dm.captured(1);
//                         QString t = dm.captured(2);
//                         QDate date = QDate::fromString(d, "yyyy/MM/dd");
//                         QTime time = QTime::fromString(t.split('.').first(), "HH:mm:ss");
//                         if (date.isValid() && time.isValid()) {
//                             QDateTime dt(date, time, Qt::UTC);
//                             out.processingStart = dt;
//                         }
//                     }
//                 } else if (key.compare("obs end", Qt::CaseInsensitive) == 0) {
//                     QRegularExpression dtRe("(\\d{4}/\\d{2}/\\d{2})\\s+(\\d{2}:\\d{2}:\\d{2}(?:\\.\\d+)?)");
//                     auto dm = dtRe.match(val);
//                     if (dm.hasMatch()) {
//                         QString d = dm.captured(1);
//                         QString t = dm.captured(2);
//                         QDate date = QDate::fromString(d, "yyyy/MM/dd");
//                         QTime time = QTime::fromString(t.split('.').first(), "HH:mm:ss");
//                         if (date.isValid() && time.isValid()) {
//                             QDateTime dt(date, time, Qt::UTC);
//                             out.processingStop = dt;
//                         }
//                     }
//                 } else if (key.compare("ref pos", Qt::CaseInsensitive) == 0) {

//                     QStringList parts = val.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
//                     if (parts.size() >= 3) {
//                         bool ok1, ok2, ok3;
//                         double lat = parts[0].toDouble(&ok1);
//                         double lon = parts[1].toDouble(&ok2);
//                         double h = parts[2].toDouble(&ok3);
//                         if (ok1 && ok2 && ok3) {
//                             out.basePosition.geodetic.lat = lat;
//                             out.basePosition.geodetic.lon = lon;
//                             out.basePosition.geodetic.h = h;
//                         }
//                     }
//                 } else {

//                 }
//             }
//             continue;
//         }

//         QStringList toks = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
//         if (toks.size() < 10) continue;
//         EpochRecord er;

//         QString dateTok = toks.value(0);
//         QString timeTok = toks.value(1);
//         QDate d = QDate::fromString(dateTok, "yyyy/MM/dd");
//         QTime t = QTime::fromString(timeTok.split('.').first(), "HH:mm:ss");
//         if (!d.isValid() || !t.isValid()) {
//             continue;
//         }
//         er.gpst = QDateTime(d, t, Qt::UTC);

//         bool ok;
//         er.lat = toks.value(2).toDouble(&ok); if(!ok) continue;
//         er.lon = toks.value(3).toDouble(&ok); if(!ok) continue;
//         er.h   = toks.value(4).toDouble(&ok); if(!ok) continue;
//         er.Q   = toks.value(5).toInt(&ok); if(!ok) er.Q = 0;
//         er.ns  = toks.value(6).toInt(&ok); if(!ok) er.ns = 0;

//         er.sdn = toks.value(7).toDouble(&ok); if(!ok) er.sdn = 0.0;
//         er.sde = toks.value(8).toDouble(&ok); if(!ok) er.sde = 0.0;
//         er.sdu = toks.value(9).toDouble(&ok); if(!ok) er.sdu = 0.0;

//         if (toks.size() > 10) er.sdne = toks.value(10).toDouble();
//         if (toks.size() > 11) er.sdeu = toks.value(11).toDouble();
//         if (toks.size() > 12) er.sdun = toks.value(12).toDouble();

//         if (toks.size() > 14) {
//             er.age = toks.value(toks.size()-2).toDouble();
//             er.ratio = toks.value(toks.size()-1).toDouble();
//         }

//         switch (er.Q) {
//         case 1: out.agg.cntFixed++; break;
//         case 2: out.agg.cntFloat++; break;
//         case 5: out.agg.cntSingle++; break;
//         default: out.agg.cntOther++; break;
//         }
//         if(er.Q != 1)
//             continue;
//         updateAggregates(out, er);
//     }
//     if (!out.processingStart.isValid() && out.agg.n_epochs > 0) out.processingStart = out.agg.firstEpoch;
//     if (!out.processingStop.isValid() && out.agg.n_epochs > 0) out.processingStop = out.agg.lastEpoch;

//     f.close();
//     return true;
// }

// double calculateRMS(const QString &statFilePath) {
//     QFile file(statFilePath);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return -1.0;

//     QTextStream in(&file);
//     double sumSqPhase = 0.0;
//     double sumWeight = 0.0;
//     long count = 0;
//     bool isEpochFixed = false;

//     while (!in.atEnd()) {
//         QString line = in.readLine().trimmed();
//         if (line.isEmpty()) continue;
//         QStringList parts = line.split(',');
//         if (parts[0] == "$POS" && parts.size() > 3) {
//             isEpochFixed = (parts[3].toInt() == 1);
//             continue;
//         }
//         if (isEpochFixed && parts[0] == "$SAT" && parts.size() > 9) {
//             int vsat = parts[9].toInt();
//             if (vsat == 1) {
//                 bool ok;
//                 double el = parts.at(6).toDouble() * (M_PI / 180.0);
//                 double weight = qSin(el);
//                 double phaseResidual = parts[8].toDouble(&ok);
//                 if (ok) {
//                     sumSqPhase += (phaseResidual * phaseResidual) * (weight * weight);
//                     sumWeight += (weight * weight);
//                 }
//             }
//         }
//     }
//     return (sumWeight > 0) ? qSqrt(sumSqPhase / sumWeight) : 0.0;
// }

// void PosData::finalize()
// {
//     if (agg.n_epochs == 0) {
//         quint64 totalCount = agg.cntFixed + agg.cntFloat + agg.cntSingle + agg.cntOther;
//         if (totalCount > 0) {
//             pctFixed = 100.0 * double(agg.cntFixed) / double(totalCount);
//             pctFloat = 100.0 * double(agg.cntFloat) / double(totalCount);
//             pctSingle = 100.0 * double(agg.cntSingle) / double(totalCount);
//             pctOther = 100.0 * double(agg.cntOther) / double(totalCount);
//             solutionTypeSummary = QString("%1% fixed, %2% float, %3% single").arg(pctFixed,0,'f',1).arg(pctFloat,0,'f',1).arg(pctSingle,0,'f',1);
//         }
//         return;
//     }

//     roverPosition.geodetic.lat = agg.meanLat;
//     roverPosition.geodetic.lon = agg.meanLon;
//     roverPosition.geodetic.h   = agg.meanH;

//     const double n = double(agg.n_epochs);
//     double meanVarE = agg.sumVarE / n;
//     double meanVarN = agg.sumVarN / n;
//     double meanVarU = agg.sumVarU / n;
//     double meanCovEN = agg.sumCovEN / n;
//     double meanCovEU = agg.sumCovEU / n;
//     double meanCovNU = agg.sumCovNU / n;

//     auto clampSmall = [](double v, double eps)->double {
//         if (v >= 0.0) return v;
//         if (v >= -eps) return 0.0;
//         return v;
//     };

//     const double CLAMP_EPS = 1e-12;
//     meanVarE = clampSmall(meanVarE, CLAMP_EPS);
//     meanVarN = clampSmall(meanVarN, CLAMP_EPS);
//     meanVarU = clampSmall(meanVarU, CLAMP_EPS);

//     // horizontalPrecision = qSqrt(fabs(meanVarE + meanVarN));
//     // verticalPrecision   = qSqrt(fabs(meanVarU));

//     processingStart = agg.firstEpoch;
//     processingStop  = agg.lastEpoch;
//     qint64 secs = processingStart.secsTo(processingStop);
//     if (secs < 0) secs = 0;
//     processingDuration = QTime::fromMSecsSinceStartOfDay(int(secs * 1000));
//     processingIntervalSeconds = (agg.nIntervals > 0 ? agg.sumIntervalSeconds / agg.nIntervals : NAN);

//     quint64 total = agg.cntFixed + agg.cntFloat + agg.cntSingle + agg.cntOther;
//     if (total > 0) {
//         pctFixed = 100.0 * double(agg.cntFixed) / double(total);
//         pctFloat = 100.0 * double(agg.cntFloat) / double(total);
//         pctSingle = 100.0 * double(agg.cntSingle) / double(total);
//         pctOther = 100.0 * double(agg.cntOther) / double(total);
//         solutionTypeSummary = QString("%1% fixed, %2% float, %3% single")
//                                   .arg(pctFixed,0,'f',1).arg(pctFloat,0,'f',1).arg(pctSingle,0,'f',1);
//     }

//     bool baseValid = (qIsFinite(basePosition.geodetic.lat) && qIsFinite(basePosition.geodetic.lon));
//     if (!baseValid) return;

//     // ECEF positions
//     double Xb,Yb,Zb, Xr,Yr,Zr;
//     geodeticToECEF(basePosition.geodetic.lat, basePosition.geodetic.lon, basePosition.geodetic.h, Xb, Yb, Zb);
//     geodeticToECEF(roverPosition.geodetic.lat, roverPosition.geodetic.lon, roverPosition.geodetic.h, Xr, Yr, Zr);
//     basePosition.ecef = {Xb,Yb,Zb};
//     roverPosition.ecef = {Xr,Yr,Zr};

//     // ENU baseline
//     double dX = Xr - Xb, dY = Yr - Yb, dZ = Zr - Zb;
//     double R[3][3];
//     buildECEFtoENUrotation(basePosition.geodetic.lat, basePosition.geodetic.lon, R);
//     double e_local, n_local, u_local;
//     enuFromEcefDelta(R, dX, dY, dZ, e_local, n_local, u_local);

//     this->dE = e_local;
//     this->dN = n_local;
//     this->dU = u_local;

//     // Geodesic azimuth and distance
//     double aziDeg = NAN, s12 = NAN;
//     geodesicInverseFallback(basePosition.geodetic.lat, basePosition.geodetic.lon,
//                             roverPosition.geodetic.lat, roverPosition.geodetic.lon,
//                             aziDeg, s12);
//     geodeticAzDeg = aziDeg;
//     ellipsoidDistance = s12;
//     geodeticAzDMS = degToDMS(geodeticAzDeg);

//     RMS = calculateRMS(posPath+".stat");

//     const double targetErrPhase = 0.003;
//     double errorScale = (RMS > 0) ? (RMS / targetErrPhase) : 1.0;
//     double baselineLengthInKm = ellipsoidDistance/1000.0;

//     const double H_FIXED = 0.008;
//     const double V_FIXED = 0.013;
//     const double PPM = 0.000001;

//     double internalH = 2.45 * qSqrt((meanVarE) + (meanVarN)) *errorScale;
//     double internalV = 1.96 * qSqrt(meanVarU)* errorScale;

//     horizontalPrecision = qSqrt(qPow(internalH, 2) +  qPow(H_FIXED, 2) +  qPow(baselineLengthInKm * 1000 * PPM, 2));
//     verticalPrecision   = qSqrt(qPow(internalV, 2) + qPow(V_FIXED, 2) + qPow(baselineLengthInKm * 1000 * PPM, 2));
//     // Global grid pipeline
//     static PJ *g_vgrid_pipeline = nullptr;
//     static bool g_vgrid_attempted = false;

//     if (!g_vgrid_attempted) {
//         g_vgrid_pipeline = createVGridPipeline();
//         g_vgrid_attempted = true;
//     }
//     if (g_vgrid_pipeline) {
//         double Hb = projEllipsoidalToOrthometric( g_vgrid_pipeline, basePosition.geodetic.lat, basePosition.geodetic.lon, basePosition.geodetic.h);
//         double Hr = projEllipsoidalToOrthometric( g_vgrid_pipeline, roverPosition.geodetic.lat, roverPosition.geodetic.lon, roverPosition.geodetic.h);

//         if (qIsFinite(Hb))
//             basePosition.orthometric = Hb;

//         if (qIsFinite(Hr))
//             roverPosition.orthometric = Hr;
//     } else {
//         if (!qIsFinite(basePosition.orthometric))
//             basePosition.orthometric = basePosition.geodetic.h;

//         if (!qIsFinite(roverPosition.orthometric))
//             roverPosition.orthometric = roverPosition.geodetic.h;
//     }

//     deltaHeight = roverPosition.geodetic.h - basePosition.geodetic.h;

//     // Grid coordinates (UTM)
//     ProcessUtils *util = new ProcessUtils();
//     UTMResult utmBase  = util->WGS84ToUTM(basePosition.geodetic.lat, basePosition.geodetic.lon, basePosition.geodetic.h);
//     UTMResult utmRover = util->WGS84ToUTM(roverPosition.geodetic.lat, roverPosition.geodetic.lon, roverPosition.geodetic.h);
//     basePosition.easting  = utmBase.easting;
//     basePosition.northing = utmBase.northing;
//     roverPosition.easting  = utmRover.easting;
//     roverPosition.northing = utmRover.northing;

//     double SigmaDeltaENU[3][3] = {};
//     SigmaDeltaENU[0][0] = meanVarE;
//     SigmaDeltaENU[1][1] = meanVarN;
//     SigmaDeltaENU[2][2] = meanVarU;
//     SigmaDeltaENU[0][1] = SigmaDeltaENU[1][0] = meanCovEN;
//     SigmaDeltaENU[0][2] = SigmaDeltaENU[2][0] = meanCovEU;
//     SigmaDeltaENU[1][2] = SigmaDeltaENU[2][1] = meanCovNU;

//     for (int i=0;i<3;++i) {
//         for (int j=i+1;j<3;++j) {
//             double avg = 0.5*(SigmaDeltaENU[i][j] + SigmaDeltaENU[j][i]);
//             SigmaDeltaENU[i][j] = SigmaDeltaENU[j][i] = avg;
//         }
//     }

//     for (int i=0;i<3;++i) {
//         if (SigmaDeltaENU[i][i] < 0.0 && SigmaDeltaENU[i][i] > -1e-14) SigmaDeltaENU[i][i] = 0.0;
//         if (SigmaDeltaENU[i][i] < -1e-8) {
//             qWarning() << "Warning: large negative ENU variance at index" << i << SigmaDeltaENU[i][i]
//                        << " — check upstream epoch covariance aggregation.";
//             SigmaDeltaENU[i][i] = qAbs(SigmaDeltaENU[i][i]);
//         }
//     }

//     sigma_dE = qSqrt(fabs(SigmaDeltaENU[0][0]));
//     sigma_dN = qSqrt(fabs(SigmaDeltaENU[1][1]));
//     sigma_dU = qSqrt(fabs(SigmaDeltaENU[2][2]));

//     double s = qSqrt(dE*dE + dN*dN);
//     if (s > 0.0) {
//         double grad0 = dE / s; double grad1 = dN / s;
//         double Cov00 = SigmaDeltaENU[0][0], Cov01 = SigmaDeltaENU[0][1];
//         double Cov10 = SigmaDeltaENU[1][0], Cov11 = SigmaDeltaENU[1][1];
//         double var_s = grad0 * (Cov00 * grad0 + Cov01 * grad1) + grad1 * (Cov10 * grad0 + Cov11 * grad1);
//         sigma_dist = qSqrt(fabs(var_s));
//     } else {
//         sigma_dist = 0.0;
//     }

//     double sigmaAzArcsec_grad = std::numeric_limits<double>::infinity();
//     double sigmaAzArcsec_perp = std::numeric_limits<double>::infinity();

//     double r2 = dE*dE + dN*dN;
//     const double R2_EPS = 1e-8;

//     if (r2 > R2_EPS) {
//         double dAE = dN / r2;
//         double dAN = -dE / r2;
//         double Cov00 = SigmaDeltaENU[0][0], Cov01 = SigmaDeltaENU[0][1];
//         double Cov10 = SigmaDeltaENU[1][0], Cov11 = SigmaDeltaENU[1][1];
//         double varAz = dAE * (Cov00 * dAE + Cov01 * dAN) + dAN * (Cov10 * dAE + Cov11 * dAN);
//         double sigmaAzRad = qSqrt(fabs(varAz));
//         sigmaAzArcsec_grad = sigmaAzRad * (180.0 / M_PI) * 3600.0;
//     }

//     if (s > 0.0) {
//         double ux = -dN / s;
//         double uy =  dE / s;
//         double perVar = ux * (SigmaDeltaENU[0][0]*ux + SigmaDeltaENU[0][1]*uy)
//                         + uy * (SigmaDeltaENU[1][0]*ux + SigmaDeltaENU[1][1]*uy);
//         if (perVar < 0.0 && perVar > -1e-14) perVar = 0.0;
//         if (perVar < -1e-8) {
//             perVar = qAbs(perVar);
//         }
//         double sigma_perp = qSqrt(fabs(perVar));
//         double sigmaAlphaRad = sigma_perp / s;
//         sigmaAzArcsec_perp = sigmaAlphaRad * (180.0 / M_PI) * 3600.0;
//     }
//     if (qIsFinite(sigmaAzArcsec_perp)) {
//         sigma_az_arcsec = sigmaAzArcsec_perp;
//     } else if (qIsFinite(sigmaAzArcsec_grad)) {
//         sigma_az_arcsec = sigmaAzArcsec_grad;
//     } else {
//         sigma_az_arcsec = std::numeric_limits<double>::infinity();
//     }

//     double temp[3][3] = {};
//     matMul3x3(SigmaDeltaENU, R, temp);
//     double RT[3][3]; matTranspose3x3(R, RT);
//     double SigmaECEF[3][3] = {};
//     matMul3x3(RT, temp, SigmaECEF);

//     for (int i=0;i<3;++i) {
//         for (int j=i+1;j<3;++j) {
//             double avg = 0.5 * (SigmaECEF[i][j] + SigmaECEF[j][i]);
//             SigmaECEF[i][j] = SigmaECEF[j][i] = avg;
//         }
//         if (SigmaECEF[i][i] < 0.0 && SigmaECEF[i][i] > -1e-14) SigmaECEF[i][i] = 0.0;
//         if (SigmaECEF[i][i] < -1e-8) {
//             qWarning() << "Large negative variance in SigmaECEF["<<i<<"]["<<i<<"] =" << SigmaECEF[i][i]
//                        << " — taking abs to avoid NAN but please investigate upstream.";
//             SigmaECEF[i][i] = qAbs(SigmaECEF[i][i]);
//         }
//     }

//     for (int i=0;i<3;++i) for (int j=0;j<3;++j) cov_dXYZ[i][j] = SigmaECEF[i][j];
//     sigma_dX = qSqrt(fabs(SigmaECEF[0][0]));
//     sigma_dY = qSqrt(fabs(SigmaECEF[1][1]));
//     sigma_dZ = qSqrt(fabs(SigmaECEF[2][2]));

//     sigma_az_DMS = arcsecToDMS(sigma_az_arcsec);
//     delete util;
// }

// bool ProcessUtils::parseSolutionStat(const QString &statFilePath, PosData &out)
// {
//     QFile file(statFilePath);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

//     QTextStream in(&file);
//     bool isFixedEpoch = false;
//     int accepted = 0;

//     auto gpsToDateTime = [](int week, double tow) -> QDateTime {
//         QDateTime epoch(QDate(1980, 1, 6),QTime(0, 0),QTimeZone::UTC);
//         return epoch.addDays(week * 7).addMSecs(qRound64(tow * 1000.0));
//     };

//     while (!in.atEnd()) {
//         QString line = in.readLine();
//         if (line.trimmed().isEmpty()) continue;

//         if (line.startsWith("$POS")) {
//             QStringList parts = line.split(',');
//             if (parts.size() > 3) {
//                 isFixedEpoch = (parts[3].toInt() == 1);
//             }
//             continue;
//         }
//         if (line.startsWith("$SAT")) {
//             if (!isFixedEpoch) continue;

//             QStringList parts = line.split(',');
//             if (parts.size() < 10) continue;
//             if (parts[9].toInt() != 1) continue;

//             int freq = parts[4].toInt();
//             double residual = parts[8].toDouble();

//             if (qAbs(residual) < 1e-9) continue;

//             int week = parts[1].toInt();
//             double tow = parts[2].toDouble();
//             QString satId = parts[3].trimmed();
//             double el = parts[6].toDouble();

//             QString mapKey = QString("%1_L%2").arg(satId).arg(freq);

//             SatResidualPoint pt;
//             pt.time = gpsToDateTime(week, tow);
//             pt.residual = residual;
//             pt.elevation = el;
//             pt.isFixed = true;
//             if (!out.SatStatsMap.contains(mapKey)) {
//                 out.SatStatsMap[mapKey].satId = mapKey;
//             }
//             out.SatStatsMap[mapKey].points.append(pt);
//             accepted++;
//         }
//     }

//     file.close();
//     consolidateSatellites(out);
//     return true;
// }

// void ProcessUtils::consolidateSatellites(PosData &out)
// {
//     const double C1 = 2.5457;
//     const double C2 = 1.5457;

//     QMap<QString, SatelliteData> finalMap;
//     QSet<QString> uniqueSats;
//     for (auto it = out.SatStatsMap.cbegin(); it != out.SatStatsMap.cend(); ++it) {
//         const QString &key = it.key();
//         if (key.contains("_L")) {
//             uniqueSats.insert(key.left(key.indexOf('_')));
//         }
//     }

//     for(const QString &sat : uniqueSats) {
//         QString keyL1 = sat + "_L1";
//         QString keyL2 = sat + "_L2";

//         bool hasL1 = out.SatStatsMap.contains(keyL1);
//         bool hasL2 = out.SatStatsMap.contains(keyL2);

//         SatelliteData combinedSat;
//         combinedSat.satId = sat;
//         if (hasL1 && hasL2) {
//             SatelliteData &d1 = out.SatStatsMap[keyL1];
//             SatelliteData &d2 = out.SatStatsMap[keyL2];
//             int idx1 = 0, idx2 = 0;
//             while(idx1 < d1.points.size() && idx2 < d2.points.size()) {
//                 const auto &p1 = d1.points[idx1];
//                 const auto &p2 = d2.points[idx2];

//                 if(p1.time < p2.time) {
//                     idx1++;
//                 } else if (p2.time < p1.time) {
//                     idx2++;
//                 } else {
//                     SatResidualPoint lcPt = p1;
//                     lcPt.residual = (C1 * p1.residual) - (C2 * p2.residual);
//                     combinedSat.points.append(lcPt);
//                     idx1++; idx2++;
//                 }
//             }
//         }
//         else if (hasL1) {
//             combinedSat.points = out.SatStatsMap[keyL1].points;
//         }
//         else if (hasL2) {
//             combinedSat.points = out.SatStatsMap[keyL2].points;
//         }

//         if (!combinedSat.points.isEmpty()) {

//             double sum = 0, sqSum = 0;
//             for(auto &p : combinedSat.points) sum += p.residual;
//             double initialMean = sum / combinedSat.points.size();

//             for(auto &p : combinedSat.points) {
//                 sqSum += (p.residual - initialMean) * (p.residual - initialMean);
//             }
//             double initialStdDev = (combinedSat.points.size() > 1) ? std::sqrt(sqSum / (combinedSat.points.size() - 1)) : 0.0;
//             double limit = 3.0 * initialStdDev;
//             if (limit < 0.02) limit = 0.02;

//             QVector<SatResidualPoint> cleanPoints;
//             cleanPoints.reserve(combinedSat.points.size());

//             for(auto &p : combinedSat.points) {
//                 if (qAbs(p.residual - initialMean) <= limit) {
//                     cleanPoints.append(p);
//                 }
//             }
//             combinedSat.points = cleanPoints;

//             if (!combinedSat.points.isEmpty()) {
//                 sum = 0; sqSum = 0;
//                 combinedSat.minVal = 1e9;
//                 combinedSat.maxVal = -1e9;

//                 for(auto &p : combinedSat.points) {
//                     sum += p.residual;
//                     if(p.residual < combinedSat.minVal) combinedSat.minVal = p.residual;
//                     if(p.residual > combinedSat.maxVal) combinedSat.maxVal = p.residual;
//                 }

//                 double count = (double)combinedSat.points.size();
//                 combinedSat.mean = sum / count;

//                 for(auto &p : combinedSat.points) {
//                     sqSum += (p.residual - combinedSat.mean) * (p.residual - combinedSat.mean);
//                 }
//                 combinedSat.stdDev = (count > 1) ? std::sqrt(sqSum / (count - 1)) : 0.0;
//                 finalMap.insert(sat, combinedSat);
//             }
//         }
//     }
//     out.SatStatsMap = finalMap;
// }

// void ProcessUtils::generateKMLFromPosData(const QMap<QString, PosData> &posData, const QString &filePath)
// {
//     if (posData.isEmpty())
//         return;

//     QFile file(filePath);
//     if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//         return;

//     QTextStream out(&file);
//     out.setRealNumberNotation(QTextStream::FixedNotation);
//     out.setRealNumberPrecision(8);

//     out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
//     out << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
//     out << "<Document>\n";
//     out << "<name>Surveypod Baselines</name>\n";

//     QString logoPath = QCoreApplication::applicationDirPath() + "/logo.png";
//     if (!QFile::exists(logoPath)){
//         QFile::copy(":/images/images/logo.png", logoPath);
//     }
//     out << "<ScreenOverlay>\n";
//     out << "<name>Surveypod Logo</name>\n";
//     out << "<Icon>\n";
//     out << "<href>file:///" << logoPath.replace("\\", "/") << "</href>\n";
//     out << "</Icon>\n";
//     out << "<overlayXY x=\"0\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>\n";
//     out << "<screenXY x=\"0.02\" y=\"0.98\" xunits=\"fraction\" yunits=\"fraction\"/>\n";
//     out << "<size x=\"0.2\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n";
//     out << "</ScreenOverlay>\n";


//     out << "<Style id=\"baseStyle\">"
//            "<IconStyle><color>ff0000ff</color><scale>1.4</scale>"
//            "<Icon><href>http://maps.google.com/mapfiles/kml/paddle/red-circle.png</href></Icon>"
//            "</IconStyle></Style>\n";

//     out << "<Style id=\"roverStyle\">"
//            "<IconStyle><color>ff00ff00</color><scale>1.2</scale>"
//            "<Icon><href>http://maps.google.com/mapfiles/kml/paddle/grn-circle.png</href></Icon>"
//            "</IconStyle></Style>\n";

//     out << "<Style id=\"lineStyle\">"
//            "<LineStyle><color>ffff0000</color><width>2</width></LineStyle>"
//            "</Style>\n";

//     QSet<QString> addedStations;

//     out << "<Folder><name>Baselines</name>\n";
//     for (auto it = posData.begin(); it != posData.end(); ++it)
//     {
//         const PosData &pd = it.value();
//         double baseLat = pd.basePosition.geodetic.lat;
//         double baseLon = pd.basePosition.geodetic.lon;
//         double baseH = pd.basePosition.geodetic.h;

//         double roverLat = pd.roverPosition.geodetic.lat;
//         double roverLon = pd.roverPosition.geodetic.lon;
//         double roverH = pd.roverPosition.geodetic.h;

//         QString from = pd.from;
//         QString to = pd.to;

//         // BASE MARKER
//         if (!addedStations.contains(from))
//         {
//             addedStations.insert(from);
//             out << "<Placemark>\n";
//             out << "<name>" << from << "</name>\n";
//             out << "<styleUrl>#baseStyle</styleUrl>\n";
//             out << "<description><![CDATA[\n";
//             out << "<b>Latitude:</b> " << baseLat << "<br/>";
//             out << "<b>Longitude:</b> " << baseLon << "<br/>";
//             out << "<b>Height:</b> " << baseH << " m\n";
//             out << "]]></description>\n";
//             out << "<Point><coordinates>" << baseLon << "," << baseLat << "," << baseH << "</coordinates></Point>\n";
//             out << "</Placemark>\n";
//         }
//         // ROVER MARKER
//         if (!addedStations.contains(to))
//         {
//             addedStations.insert(to);
//             out << "<Placemark>\n";
//             out << "<name>" << to << "</name>\n";
//             out << "<styleUrl>#roverStyle</styleUrl>\n";
//             out << "<description><![CDATA[\n";
//             out << "<b>Latitude:</b> " << roverLat << " m<br/>";
//             out << "<b>Longitude:</b> " << roverLon << " m<br/>";
//             out << "<b>Hright:</b> " << roverH << " m<br/>";
//             out << "]]></description>\n";
//             out << "<Point><coordinates>" << roverLon << "," << roverLat << "," << roverH << "</coordinates></Point>\n";
//             out << "</Placemark>\n";
//         }

//         // BASELINE LINE
//         out << "<Placemark>\n";
//         out << "<name>" << from << " → " << to << "</name>\n";
//         out << "<styleUrl>#lineStyle</styleUrl>\n";
//         out << "<description><![CDATA[\n";
//         out << "<b>From:</b> " << from << "<br/>";
//         out << "<b>To:</b> " << to << "<br/>";
//         out << "<b>Ellipsoidal Distance:</b> "<< pd.ellipsoidDistance << " m<br/>";
//         out << "]]></description>\n";
//         out << "<LineString>\n";
//         out << "<tessellate>1</tessellate>\n";
//         out << "<coordinates>\n";
//         out << baseLon  << "," << baseLat  << "," << baseH  << "\n";
//         out << roverLon << "," << roverLat << "," << roverH << "\n";
//         out << "</coordinates>\n";
//         out << "</LineString>\n";
//         out << "</Placemark>\n";
//     }
//     out << "</Folder>\n";
//     out << "</Document>\n";
//     out << "</kml>\n";
//     file.close();
// }














#include "processutils.h"
#include <QFileInfo>
#include <QtMath>
#include <QCoreApplication>
#include "proj.h"
#include <QDebug>
#include <QRegularExpression>
static constexpr double WGS84_A = 6378137.0;
static constexpr double WGS84_F = 1.0/298.257223563;
static constexpr double WGS84_E2 = 2*WGS84_F - WGS84_F*WGS84_F;

ProcessUtils::ProcessUtils(QObject *parent) : QObject{parent}
{
    qputenv("PROJ_DATA", QCoreApplication::applicationDirPath().toUtf8());
}

static PJ *createVGridPipeline()
{

    QString grid = QStringLiteral("proj/us_nga_egm08_25.tif");
    QString pipeline = QString("+proj=pipeline +step +proj=vgridshift +grids=%1").arg(grid);

    PJ *P = proj_create(nullptr, pipeline.toLocal8Bit().constData());
    if (!P) {
        QString alt = QString("+proj=pipeline +step +proj=vgridshift +grids=\"%1\"").arg(grid);
        P = proj_create(nullptr, alt.toLocal8Bit().constData());
    }
    return P;
}

static void destroyPipeline(PJ *P) {
    if (P) proj_destroy(P);
}

static double projEllipsoidalToOrthometric(PJ *P, double latDeg, double lonDeg, double hEllip)
{
    if (!P) return std::numeric_limits<double>::quiet_NaN();

    if (!std::isfinite(latDeg) || !std::isfinite(lonDeg) || !std::isfinite(hEllip)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    constexpr double DEG2RAD = M_PI / 180.0;
    auto tryOnce = [&](double inLonDeg, double inLatDeg)->double {
        PJ_COORD in;
        in.lpzt.lam = inLonDeg * DEG2RAD;
        in.lpzt.phi = inLatDeg * DEG2RAD;
        in.lpzt.z   = hEllip;
        in.lpzt.t   = 0.0;

        proj_errno_reset(P);
        PJ_COORD out = proj_trans(P, PJ_FWD, in);
        double H = out.lpzt.z;
        return H;
    };

    double H = tryOnce(lonDeg, latDeg);
    if (std::isfinite(H)) {
        return H;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

static double projGeoidUndulation(PJ *P, double latDeg, double lonDeg, double hEllip)
{
    double H = projEllipsoidalToOrthometric(P, latDeg, lonDeg, hEllip);
    if (!std::isfinite(H))
        return std::numeric_limits<double>::quiet_NaN();
    return hEllip - H;
}

void ProcessUtils::updateAggregates(PosData &p, const EpochRecord &er)
{
    if (p.agg.n_epochs == 0) {
        p.agg.firstEpoch = er.gpst;
        p.agg.lastEpoch = er.gpst;
        p.agg.bestEpochTime = er.gpst;
        p.agg.bestLat = er.lat; p.agg.bestLon = er.lon; p.agg.bestH = er.h;
        p.agg.bestRatio = er.ratio;
    } else {
        qint64 dtSec = p.agg.lastEpoch.secsTo(er.gpst);
        if (dtSec > 0) { p.agg.sumIntervalSeconds += dtSec; p.agg.nIntervals++; }
        p.agg.lastEpoch = er.gpst;
        if (er.ratio > p.agg.bestRatio) {
            p.agg.bestRatio = er.ratio;
            p.agg.bestEpochTime = er.gpst;
            p.agg.bestLat = er.lat; p.agg.bestLon = er.lon; p.agg.bestH = er.h;
        }
    }

    p.agg.n_epochs++;
    double n = double(p.agg.n_epochs);

    // lat
    double delta = er.lat - p.agg.meanLat;
    p.agg.meanLat += delta / n;
    p.agg.M2Lat += delta * (er.lat - p.agg.meanLat);

    // lon
    delta = er.lon - p.agg.meanLon;
    p.agg.meanLon += delta / n;
    p.agg.M2Lon += delta * (er.lon - p.agg.meanLon);

    // height
    delta = er.h - p.agg.meanH;
    p.agg.meanH += delta / n;
    p.agg.M2H += delta * (er.h - p.agg.meanH);

    double varN = er.sdn * er.sdn;
    double varE = er.sde * er.sde;
    double varU = er.sdu * er.sdu;

    auto signedSquare = [](double signed_sqrt_val)->double {
        if (!std::isfinite(signed_sqrt_val)) return NAN;
        double absval = std::abs(signed_sqrt_val);
        double cov = absval * absval;
        return (signed_sqrt_val < 0.0) ? -cov : cov;
    };

    double covNE = signedSquare(er.sdne);
    double covEU = signedSquare(er.sdeu);
    double covNU = signedSquare(er.sdun);

    p.agg.sumVarE += varE;
    p.agg.sumVarN += varN;
    p.agg.sumVarU += varU;
    p.agg.sumCovEN += covNE;
    p.agg.sumCovEU += covEU;
    p.agg.sumCovNU += covNU;
}

static void geodeticToECEF(double latDeg, double lonDeg, double h, double &X, double &Y, double &Z)
{
    double lat = qDegreesToRadians(latDeg);
    double lon = qDegreesToRadians(lonDeg);
    double sinlat = qSin(lat);
    double coslat = qCos(lat);
    double coslon = qCos(lon);
    double sinlon = qSin(lon);
    double N = WGS84_A / qSqrt(1.0 - WGS84_E2 * sinlat * sinlat);
    X = (N + h) * coslat * coslon;
    Y = (N + h) * coslat * sinlon;
    Z = (N * (1.0 - WGS84_E2) + h) * sinlat;
}

static void buildECEFtoENUrotation(double latDeg, double lonDeg, double R[3][3])
{
    double lat = qDegreesToRadians(latDeg);
    double lon = qDegreesToRadians(lonDeg);
    double sinLat = qSin(lat), cosLat = qCos(lat);
    double sinLon = qSin(lon), cosLon = qCos(lon);

    //east
    R[0][0] = -sinLon;           R[0][1] =  cosLon;           R[0][2] = 0.0;
    //north
    R[1][0] = -sinLat * cosLon;  R[1][1] = -sinLat * sinLon;  R[1][2] =  cosLat;
    //up
    R[2][0] =  cosLat * cosLon;  R[2][1] =  cosLat * sinLon;  R[2][2] =  sinLat;
}

static void enuFromEcefDelta(const double R[3][3], double dX, double dY, double dZ,
                             double &E, double &N, double &U)
{
    E = R[0][0]*dX + R[0][1]*dY + R[0][2]*dZ;
    N = R[1][0]*dX + R[1][1]*dY + R[1][2]*dZ;
    U = R[2][0]*dX + R[2][1]*dY + R[2][2]*dZ;
}

static QString degToDMS(double deg)
{
    if (qIsNaN(deg)) return QString();
    double absdeg = qAbs(deg);
    int d = int(absdeg);
    double rem = (absdeg - d) * 60.0;
    int m = int(rem);
    double s = (rem - m) * 60.0;
    QString sign = (deg < 0.0) ? "-" : "";
    return QString("%1%2°%3'%4\"").arg(sign).arg(d).arg(m,2,10,QChar('0')).arg(QString::number(s,'f',1));
}

static void geodesicInverseFallback(double lat1, double lon1, double lat2, double lon2, double &azi12Deg, double &distMeters)
{

    double X1,Y1,Z1, X2,Y2,Z2;
    geodeticToECEF(lat1, lon1, 0.0, X1, Y1, Z1);
    geodeticToECEF(lat2, lon2, 0.0, X2, Y2, Z2);
    double dX = X2 - X1, dY = Y2 - Y1, dZ = Z2 - Z1;
    double R[3][3];
    buildECEFtoENUrotation(lat1, lon1, R);
    double E,N,U;
    enuFromEcefDelta(R, dX, dY, dZ, E, N, U);
    distMeters = qSqrt(E*E + N*N);
    double azRad = 0.0;
    if (distMeters > 0.0) azRad = qAtan2(E, N);
    azi12Deg = qRadiansToDegrees(azRad);
    if (azi12Deg < 0.0) azi12Deg += 360.0;
}

static void matMul3x3(const double A[3][3], const double B[3][3], double out[3][3])
{
    for (int i=0;i<3;++i) {
        for (int j=0;j<3;++j) {
            double s = 0.0;
            for (int k=0;k<3;++k) s += A[i][k] * B[k][j];
            out[i][j] = s;
        }
    }
}

static void matTranspose3x3(const double A[3][3], double AT[3][3])
{
    for(int i=0;i<3;++i)
        for(int j=0;j<3;++j)
            AT[i][j] = A[j][i];
}

char latitudeToZoneLetter(double latitude) {
    if (latitude >= 84 || latitude < -80) return 'Z';
    const char letters[] = {
        'C','D','E','F','G','H','J','K','L','M',
        'N','P','Q','R','S','T','U','V','W','X'
    };
    int index = int((latitude + 80) / 8);
    return letters[index];
}


UTMResult ProcessUtils::WGS84ToUTM(double lat, double lon, double alt)
{
    UTMResult result{};

    result.zoneNumber = static_cast<int>((lon + 180) / 6) + 1;
    result.zoneLetter = latitudeToZoneLetter(lat);

    std::string projStr = "+proj=utm +zone=" + std::to_string(result.zoneNumber)
                          + " +datum=WGS84 +units=m +no_defs";
    if (lat < 0) projStr += " +south";

    PJ_CONTEXT *C = proj_context_create();
    if (!C) throw std::runtime_error("Failed to create PROJ context");

    PJ *P = proj_create_crs_to_crs( C, "+proj=longlat +datum=WGS84 +no_defs", projStr.c_str(), nullptr );

    if (!P) {
        proj_context_destroy(C);
        throw std::runtime_error("Failed to create projection");
    }

    PJ_COORD coord = proj_coord(lon, lat, alt, 0);
    PJ_COORD utm   = proj_trans(P, PJ_FWD, coord);

    result.easting  = utm.xyz.x;
    result.northing = utm.xyz.y;
    result.altitude = utm.xyz.z;

    proj_destroy(P);
    proj_context_destroy(C);

    return result;
}

static QString arcsecToDMS(double arcsec)
{
    if (!qIsFinite(arcsec)) return QString("-");
    double deg = arcsec / 3600.0;
    double absdeg = qAbs(deg);
    int d = int(absdeg);
    double rem = (absdeg - d) * 60.0;
    int m = int(rem);
    double s = (rem - m) * 60.0;
    QString sign = (deg < 0.0) ? "-" : "";
    return QString("%1%2°%3'%4\"").arg(sign).arg(d).arg(m, 2, 10, QChar('0')).arg(QString::number(s, 'f', 1));
}

bool ProcessUtils::parsePosFileIntoPosData(const QString &posFilePath, PosData &out, QString &errorOut)
{
    QFile f(posFilePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorOut = QString("Cannot open file: %1").arg(posFilePath);
        return false;
    }
    out.posPath = posFilePath;
    out.fileKey = QFileInfo(posFilePath).fileName();
    out.processedTime = QDateTime::currentDateTimeUtc();
    QTextStream in(&f);
    QRegularExpression headerRe("^%\\s*([^:]+?)\\s*:\\s*(.*)$");

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        if (line.startsWith('%')) {
            QRegularExpressionMatch m = headerRe.match(line);
            if (m.hasMatch()) {
                QString key = m.captured(1).trimmed();
                QString val = m.captured(2).trimmed();

                if (key.compare("inp file", Qt::CaseInsensitive) == 0) {
                    if (out.roverPath.isEmpty())
                        out.roverPath = val;
                    else if (out.basePath.isEmpty())
                        out.basePath = val;
                } else if (key.compare("freqs", Qt::CaseInsensitive) == 0) {
                    out.frequency = val;
                } else if (key.compare("elev mask", Qt::CaseInsensitive) == 0) {
                    out.elevMask = val;
                } else if (key.compare("ephemeris", Qt::CaseInsensitive) == 0) {
                    out.ephemeris = val;
                }
                else if (key.compare("obs start", Qt::CaseInsensitive) == 0) {

                    QRegularExpression dtRe("(\\d{4}/\\d{2}/\\d{2})\\s+(\\d{2}:\\d{2}:\\d{2}(?:\\.\\d+)?)");
                    auto dm = dtRe.match(val);
                    if (dm.hasMatch()) {
                        QString d = dm.captured(1);
                        QString t = dm.captured(2);
                        QDate date = QDate::fromString(d, "yyyy/MM/dd");
                        QTime time = QTime::fromString(t.split('.').first(), "HH:mm:ss");
                        if (date.isValid() && time.isValid()) {
                            QDateTime dt(date, time, Qt::UTC);
                            out.processingStart = dt;
                        }
                    }
                } else if (key.compare("obs end", Qt::CaseInsensitive) == 0) {
                    QRegularExpression dtRe("(\\d{4}/\\d{2}/\\d{2})\\s+(\\d{2}:\\d{2}:\\d{2}(?:\\.\\d+)?)");
                    auto dm = dtRe.match(val);
                    if (dm.hasMatch()) {
                        QString d = dm.captured(1);
                        QString t = dm.captured(2);
                        QDate date = QDate::fromString(d, "yyyy/MM/dd");
                        QTime time = QTime::fromString(t.split('.').first(), "HH:mm:ss");
                        if (date.isValid() && time.isValid()) {
                            QDateTime dt(date, time, Qt::UTC);
                            out.processingStop = dt;
                        }
                    }
                } else if (key.compare("ref pos", Qt::CaseInsensitive) == 0) {

                    QStringList parts = val.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    if (parts.size() >= 3) {
                        bool ok1, ok2, ok3;
                        double lat = parts[0].toDouble(&ok1);
                        double lon = parts[1].toDouble(&ok2);
                        double h = parts[2].toDouble(&ok3);
                        if (ok1 && ok2 && ok3) {
                            out.basePosition.geodetic.lat = lat;
                            out.basePosition.geodetic.lon = lon;
                            out.basePosition.geodetic.h = h;
                        }
                    }
                }
            }
            continue;
        }

        QStringList toks = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (toks.size() < 10) continue;
        EpochRecord er;

        QString dateTok = toks.value(0);
        QString timeTok = toks.value(1);
        QDate d = QDate::fromString(dateTok, "yyyy/MM/dd");
        QTime t = QTime::fromString(timeTok.split('.').first(), "HH:mm:ss");
        if (!d.isValid() || !t.isValid()) {
            continue;
        }
        er.gpst = QDateTime(d, t, Qt::UTC);

        bool ok;
        er.lat = toks.value(2).toDouble(&ok); if(!ok) continue;
        er.lon = toks.value(3).toDouble(&ok); if(!ok) continue;
        er.h   = toks.value(4).toDouble(&ok); if(!ok) continue;
        er.Q   = toks.value(5).toInt(&ok); if(!ok) er.Q = 0;
        er.ns  = toks.value(6).toInt(&ok); if(!ok) er.ns = 0;

        er.sdn = toks.value(7).toDouble(&ok); if(!ok) er.sdn = 0.0;
        er.sde = toks.value(8).toDouble(&ok); if(!ok) er.sde = 0.0;
        er.sdu = toks.value(9).toDouble(&ok); if(!ok) er.sdu = 0.0;

        if (toks.size() > 10) er.sdne = toks.value(10).toDouble();
        if (toks.size() > 11) er.sdeu = toks.value(11).toDouble();
        if (toks.size() > 12) er.sdun = toks.value(12).toDouble();

        if (toks.size() > 14) {
            er.age = toks.value(toks.size()-2).toDouble();
            er.ratio = toks.value(toks.size()-1).toDouble();
        }

        switch (er.Q) {
        case 1: out.agg.cntFixed++; break;
        case 2: out.agg.cntFloat++; break;
        case 5: out.agg.cntSingle++; break;
        default: out.agg.cntOther++; break;
        }
        if(er.Q != 1)
            continue;
        updateAggregates(out, er);
    }
    if (!out.processingStart.isValid() && out.agg.n_epochs > 0) out.processingStart = out.agg.firstEpoch;
    if (!out.processingStop.isValid() && out.agg.n_epochs > 0) out.processingStop = out.agg.lastEpoch;

    f.close();
    return true;
}

void PosData::finalize()
{
    if (agg.n_epochs == 0) {
        quint64 totalCount = agg.cntFixed + agg.cntFloat + agg.cntSingle + agg.cntOther;
        if (totalCount > 0) {
            pctFixed = 100.0 * double(agg.cntFixed) / double(totalCount);
            pctFloat = 100.0 * double(agg.cntFloat) / double(totalCount);
            pctSingle = 100.0 * double(agg.cntSingle) / double(totalCount);
            pctOther = 100.0 * double(agg.cntOther) / double(totalCount);
            solutionTypeSummary = QString("%1% fixed, %2% float, %3% single").arg(pctFixed,0,'f',1).arg(pctFloat,0,'f',1).arg(pctSingle,0,'f',1);
        }
        return;
    }

    roverPosition.geodetic.lat = agg.meanLat;
    roverPosition.geodetic.lon = agg.meanLon;
    roverPosition.geodetic.h   = agg.meanH;

    const double n = double(agg.n_epochs);
    double meanVarE = agg.sumVarE / n;
    double meanVarN = agg.sumVarN / n;
    double meanVarU = agg.sumVarU / n;
    double meanCovEN = agg.sumCovEN / n;
    double meanCovEU = agg.sumCovEU / n;
    double meanCovNU = agg.sumCovNU / n;

    auto clampSmall = [](double v, double eps)->double {
        if (v >= 0.0) return v;
        if (v >= -eps) return 0.0;
        return v;
    };

    const double CLAMP_EPS = 1e-12;
    meanVarE = clampSmall(meanVarE, CLAMP_EPS);
    meanVarN = clampSmall(meanVarN, CLAMP_EPS);
    meanVarU = clampSmall(meanVarU, CLAMP_EPS);

    // horizontalPrecision = qSqrt(fabs(meanVarE + meanVarN));
    // verticalPrecision   = qSqrt(fabs(meanVarU));

    processingStart = agg.firstEpoch;
    processingStop  = agg.lastEpoch;
    qint64 secs = processingStart.secsTo(processingStop);
    if (secs < 0) secs = 0;
    processingDuration = QTime::fromMSecsSinceStartOfDay(int(secs * 1000));
    processingIntervalSeconds = (agg.nIntervals > 0 ? agg.sumIntervalSeconds / agg.nIntervals : NAN);

    quint64 total = agg.cntFixed + agg.cntFloat + agg.cntSingle + agg.cntOther;
    if (total > 0) {
        pctFixed = 100.0 * double(agg.cntFixed) / double(total);
        pctFloat = 100.0 * double(agg.cntFloat) / double(total);
        pctSingle = 100.0 * double(agg.cntSingle) / double(total);
        pctOther = 100.0 * double(agg.cntOther) / double(total);
        solutionTypeSummary = QString("%1% fixed, %2% float, %3% single")
                                  .arg(pctFixed,0,'f',1).arg(pctFloat,0,'f',1).arg(pctSingle,0,'f',1);
    }

    bool baseValid = (qIsFinite(basePosition.geodetic.lat) && qIsFinite(basePosition.geodetic.lon));
    if (!baseValid) return;

    // ECEF positions
    double Xb,Yb,Zb, Xr,Yr,Zr;
    geodeticToECEF(basePosition.geodetic.lat, basePosition.geodetic.lon, basePosition.geodetic.h, Xb, Yb, Zb);
    geodeticToECEF(roverPosition.geodetic.lat, roverPosition.geodetic.lon, roverPosition.geodetic.h, Xr, Yr, Zr);
    basePosition.ecef = {Xb,Yb,Zb};
    roverPosition.ecef = {Xr,Yr,Zr};

    // ENU baseline
    double dX = Xr - Xb, dY = Yr - Yb, dZ = Zr - Zb;
    double R[3][3];
    buildECEFtoENUrotation(basePosition.geodetic.lat, basePosition.geodetic.lon, R);
    double e_local, n_local, u_local;
    enuFromEcefDelta(R, dX, dY, dZ, e_local, n_local, u_local);

    this->dE = e_local;
    this->dN = n_local;
    this->dU = u_local;

    // Geodesic azimuth and distance
    double aziDeg = NAN, s12 = NAN;
    geodesicInverseFallback(basePosition.geodetic.lat, basePosition.geodetic.lon,
                            roverPosition.geodetic.lat, roverPosition.geodetic.lon,
                            aziDeg, s12);
    geodeticAzDeg = aziDeg;
    ellipsoidDistance = s12;
    geodeticAzDMS = degToDMS(geodeticAzDeg);

    const double targetErrPhase = 0.003;
    double errorScale = (RMS > 0) ? (RMS / targetErrPhase) : 1.0;
    double baselineLengthInKm = ellipsoidDistance/1000.0;

    const double H_FIXED = 0.008;
    const double V_FIXED = 0.013;
    const double PPM = 0.000001;

    double internalH = 2.45 * qSqrt((meanVarE) + (meanVarN)) *errorScale;
    double internalV = 1.96 * qSqrt(meanVarU)* errorScale;

    horizontalPrecision = qSqrt(qPow(internalH, 2) +  qPow(H_FIXED, 2) +  qPow(baselineLengthInKm * 1000 * PPM, 2));
    verticalPrecision   = qSqrt(qPow(internalV, 2) + qPow(V_FIXED, 2) + qPow(baselineLengthInKm * 1000 * PPM, 2));
    // Global grid pipeline
    static PJ *g_vgrid_pipeline = nullptr;
    static bool g_vgrid_attempted = false;

    if (!g_vgrid_attempted) {
        g_vgrid_pipeline = createVGridPipeline();
        g_vgrid_attempted = true;
    }
    if (g_vgrid_pipeline) {
        double Hb = projEllipsoidalToOrthometric( g_vgrid_pipeline, basePosition.geodetic.lat, basePosition.geodetic.lon, basePosition.geodetic.h);
        double Hr = projEllipsoidalToOrthometric( g_vgrid_pipeline, roverPosition.geodetic.lat, roverPosition.geodetic.lon, roverPosition.geodetic.h);

        if (qIsFinite(Hb))
            basePosition.orthometric = Hb;

        if (qIsFinite(Hr))
            roverPosition.orthometric = Hr;
    } else {
        if (!qIsFinite(basePosition.orthometric))
            basePosition.orthometric = basePosition.geodetic.h;

        if (!qIsFinite(roverPosition.orthometric))
            roverPosition.orthometric = roverPosition.geodetic.h;
    }

    deltaHeight = roverPosition.geodetic.h - basePosition.geodetic.h;

    // Grid coordinates (UTM)
    ProcessUtils *util = new ProcessUtils();
    UTMResult utmBase  = util->WGS84ToUTM(basePosition.geodetic.lat, basePosition.geodetic.lon, basePosition.geodetic.h);
    UTMResult utmRover = util->WGS84ToUTM(roverPosition.geodetic.lat, roverPosition.geodetic.lon, roverPosition.geodetic.h);
    basePosition.easting  = utmBase.easting;
    basePosition.northing = utmBase.northing;
    roverPosition.easting  = utmRover.easting;
    roverPosition.northing = utmRover.northing;

    double SigmaDeltaENU[3][3] = {};
    SigmaDeltaENU[0][0] = meanVarE;
    SigmaDeltaENU[1][1] = meanVarN;
    SigmaDeltaENU[2][2] = meanVarU;
    SigmaDeltaENU[0][1] = SigmaDeltaENU[1][0] = meanCovEN;
    SigmaDeltaENU[0][2] = SigmaDeltaENU[2][0] = meanCovEU;
    SigmaDeltaENU[1][2] = SigmaDeltaENU[2][1] = meanCovNU;

    for (int i=0;i<3;++i) {
        for (int j=i+1;j<3;++j) {
            double avg = 0.5*(SigmaDeltaENU[i][j] + SigmaDeltaENU[j][i]);
            SigmaDeltaENU[i][j] = SigmaDeltaENU[j][i] = avg;
        }
    }

    for (int i=0;i<3;++i) {
        if (SigmaDeltaENU[i][i] < 0.0 && SigmaDeltaENU[i][i] > -1e-14) SigmaDeltaENU[i][i] = 0.0;
        if (SigmaDeltaENU[i][i] < -1e-8) {
            qWarning() << "Warning: large negative ENU variance at index" << i << SigmaDeltaENU[i][i]
                       << " — check upstream epoch covariance aggregation.";
            SigmaDeltaENU[i][i] = qAbs(SigmaDeltaENU[i][i]);
        }
    }

    sigma_dE = qSqrt(fabs(SigmaDeltaENU[0][0]));
    sigma_dN = qSqrt(fabs(SigmaDeltaENU[1][1]));
    sigma_dU = qSqrt(fabs(SigmaDeltaENU[2][2]));

    double s = qSqrt(dE*dE + dN*dN);
    if (s > 0.0) {
        double grad0 = dE / s; double grad1 = dN / s;
        double Cov00 = SigmaDeltaENU[0][0], Cov01 = SigmaDeltaENU[0][1];
        double Cov10 = SigmaDeltaENU[1][0], Cov11 = SigmaDeltaENU[1][1];
        double var_s = grad0 * (Cov00 * grad0 + Cov01 * grad1) + grad1 * (Cov10 * grad0 + Cov11 * grad1);
        sigma_dist = qSqrt(fabs(var_s));
    } else {
        sigma_dist = 0.0;
    }

    double sigmaAzArcsec_grad = std::numeric_limits<double>::infinity();
    double sigmaAzArcsec_perp = std::numeric_limits<double>::infinity();

    double r2 = dE*dE + dN*dN;
    const double R2_EPS = 1e-8;

    if (r2 > R2_EPS) {
        double dAE = dN / r2;
        double dAN = -dE / r2;
        double Cov00 = SigmaDeltaENU[0][0], Cov01 = SigmaDeltaENU[0][1];
        double Cov10 = SigmaDeltaENU[1][0], Cov11 = SigmaDeltaENU[1][1];
        double varAz = dAE * (Cov00 * dAE + Cov01 * dAN) + dAN * (Cov10 * dAE + Cov11 * dAN);
        double sigmaAzRad = qSqrt(fabs(varAz));
        sigmaAzArcsec_grad = sigmaAzRad * (180.0 / M_PI) * 3600.0;
    }

    if (s > 0.0) {
        double ux = -dN / s;
        double uy =  dE / s;
        double perVar = ux * (SigmaDeltaENU[0][0]*ux + SigmaDeltaENU[0][1]*uy)
                        + uy * (SigmaDeltaENU[1][0]*ux + SigmaDeltaENU[1][1]*uy);
        if (perVar < 0.0 && perVar > -1e-14) perVar = 0.0;
        if (perVar < -1e-8) {
            perVar = qAbs(perVar);
        }
        double sigma_perp = qSqrt(fabs(perVar));
        double sigmaAlphaRad = sigma_perp / s;
        sigmaAzArcsec_perp = sigmaAlphaRad * (180.0 / M_PI) * 3600.0;
    }
    if (qIsFinite(sigmaAzArcsec_perp)) {
        sigma_az_arcsec = sigmaAzArcsec_perp;
    } else if (qIsFinite(sigmaAzArcsec_grad)) {
        sigma_az_arcsec = sigmaAzArcsec_grad;
    } else {
        sigma_az_arcsec = std::numeric_limits<double>::infinity();
    }

    double temp[3][3] = {};
    matMul3x3(SigmaDeltaENU, R, temp);
    double RT[3][3]; matTranspose3x3(R, RT);
    double SigmaECEF[3][3] = {};
    matMul3x3(RT, temp, SigmaECEF);

    for (int i=0;i<3;++i) {
        for (int j=i+1;j<3;++j) {
            double avg = 0.5 * (SigmaECEF[i][j] + SigmaECEF[j][i]);
            SigmaECEF[i][j] = SigmaECEF[j][i] = avg;
        }
        if (SigmaECEF[i][i] < 0.0 && SigmaECEF[i][i] > -1e-14) SigmaECEF[i][i] = 0.0;
        if (SigmaECEF[i][i] < -1e-8) {
            qWarning() << "Large negative variance in SigmaECEF["<<i<<"]["<<i<<"] =" << SigmaECEF[i][i]
                       << " — taking abs to avoid NAN but please investigate upstream.";
            SigmaECEF[i][i] = qAbs(SigmaECEF[i][i]);
        }
    }

    for (int i=0;i<3;++i) for (int j=0;j<3;++j) cov_dXYZ[i][j] = SigmaECEF[i][j];
    sigma_dX = qSqrt(fabs(SigmaECEF[0][0]));
    sigma_dY = qSqrt(fabs(SigmaECEF[1][1]));
    sigma_dZ = qSqrt(fabs(SigmaECEF[2][2]));

    sigma_az_DMS = arcsecToDMS(sigma_az_arcsec);
    delete util;
}

bool ProcessUtils::parseSolutionStat(const QString &statFilePath, PosData &out)
{
    QFile file(statFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    bool isFixedEpoch = false;
    int accepted = 0;
    double sumSqPhase = 0.0;
    double sumWeight  = 0.0;

    auto gpsToDateTime = [](int week, double tow) -> QDateTime {
        QDateTime epoch(QDate(1980, 1, 6), QTime(0, 0), QTimeZone::UTC);
        return epoch.addDays(week * 7).addMSecs(qRound64(tow * 1000.0));
    };

    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.trimmed().isEmpty())
            continue;
        if (line.startsWith("$POS"))
        {
            QStringList parts = line.split(',');
            if (parts.size() > 3)
                isFixedEpoch = (parts[3].toInt() == 1);
            continue;
        }
        if (line.startsWith("$SAT"))
        {
            if (!isFixedEpoch)
                continue;

            QStringList parts = line.split(',');
            if (parts.size() < 10)
                continue;

            if (parts[9].toInt() != 1)
                continue;

            bool ok;
            int week      = parts[1].toInt();
            double tow    = parts[2].toDouble();
            QString satId = parts[3].trimmed();
            int freq      = parts[4].toInt();
            double elDeg  = parts[6].toDouble();
            double residual = parts[8].toDouble(&ok);

            if (!ok)
                continue;

            if (qAbs(residual) < 1e-9)
                continue;

            double elRad = elDeg * (M_PI / 180.0);
            double weight = qSin(elRad);

            sumSqPhase += (residual * residual) * (weight * weight);
            sumWeight  += (weight * weight);

            QString mapKey = QString("%1_L%2").arg(satId).arg(freq);
            SatResidualPoint pt;
            pt.time = gpsToDateTime(week, tow);
            pt.residual = residual;
            pt.elevation = elDeg;
            pt.isFixed = true;

            if (!out.SatStatsMap.contains(mapKey))
                out.SatStatsMap[mapKey].satId = mapKey;
            out.SatStatsMap[mapKey].points.append(pt);
            accepted++;
        }
    }

    file.close();
    if (sumWeight > 0.0)
        out.RMS = qSqrt(sumSqPhase / sumWeight);
    else
        out.RMS = 0.0;

    consolidateSatellites(out);
    return true;
}

void ProcessUtils::consolidateSatellites(PosData &out)
{
    const double C1 = 2.5457;
    const double C2 = 1.5457;

    QMap<QString, SatelliteData> finalMap;
    QSet<QString> uniqueSats;
    for (auto it = out.SatStatsMap.cbegin(); it != out.SatStatsMap.cend(); ++it) {
        const QString &key = it.key();
        if (key.contains("_L")) {
            uniqueSats.insert(key.left(key.indexOf('_')));
        }
    }

    for(const QString &sat : uniqueSats) {
        QString keyL1 = sat + "_L1";
        QString keyL2 = sat + "_L2";

        bool hasL1 = out.SatStatsMap.contains(keyL1);
        bool hasL2 = out.SatStatsMap.contains(keyL2);

        SatelliteData combinedSat;
        combinedSat.satId = sat;
        if (hasL1 && hasL2) {
            SatelliteData &d1 = out.SatStatsMap[keyL1];
            SatelliteData &d2 = out.SatStatsMap[keyL2];
            int idx1 = 0, idx2 = 0;
            while(idx1 < d1.points.size() && idx2 < d2.points.size()) {
                const auto &p1 = d1.points[idx1];
                const auto &p2 = d2.points[idx2];

                if(p1.time < p2.time) {
                    idx1++;
                } else if (p2.time < p1.time) {
                    idx2++;
                } else {
                    SatResidualPoint lcPt = p1;
                    lcPt.residual = (C1 * p1.residual) - (C2 * p2.residual);
                    combinedSat.points.append(lcPt);
                    idx1++; idx2++;
                }
            }
        }
        else if (hasL1) {
            combinedSat.points = out.SatStatsMap[keyL1].points;
        }
        else if (hasL2) {
            combinedSat.points = out.SatStatsMap[keyL2].points;
        }

        if (!combinedSat.points.isEmpty()) {

            double sum = 0, sqSum = 0;
            for(auto &p : combinedSat.points) sum += p.residual;
            double initialMean = sum / combinedSat.points.size();

            for(auto &p : combinedSat.points) {
                sqSum += (p.residual - initialMean) * (p.residual - initialMean);
            }
            double initialStdDev = (combinedSat.points.size() > 1) ? std::sqrt(sqSum / (combinedSat.points.size() - 1)) : 0.0;
            double limit = 3.0 * initialStdDev;
            if (limit < 0.02) limit = 0.02;

            QVector<SatResidualPoint> cleanPoints;
            cleanPoints.reserve(combinedSat.points.size());

            for(auto &p : combinedSat.points) {
                if (qAbs(p.residual - initialMean) <= limit) {
                    cleanPoints.append(p);
                }
            }
            combinedSat.points = cleanPoints;

            if (!combinedSat.points.isEmpty()) {
                sum = 0; sqSum = 0;
                combinedSat.minVal = 1e9;
                combinedSat.maxVal = -1e9;

                for(auto &p : combinedSat.points) {
                    sum += p.residual;
                    if(p.residual < combinedSat.minVal) combinedSat.minVal = p.residual;
                    if(p.residual > combinedSat.maxVal) combinedSat.maxVal = p.residual;
                }

                double count = (double)combinedSat.points.size();
                combinedSat.mean = sum / count;

                for(auto &p : combinedSat.points) {
                    sqSum += (p.residual - combinedSat.mean) * (p.residual - combinedSat.mean);
                }
                combinedSat.stdDev = (count > 1) ? std::sqrt(sqSum / (count - 1)) : 0.0;
                finalMap.insert(sat, combinedSat);
            }
        }
    }
    out.SatStatsMap = finalMap;
}

void ProcessUtils::generateKMLFromPosData(const QMap<QString, PosData> &posData, const QString &filePath)
{
    if (posData.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(8);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
    out << "<Document>\n";
    out << "<name>Surveypod Baselines</name>\n";

    QString logoPath = QCoreApplication::applicationDirPath() + "/logo.png";
    if (!QFile::exists(logoPath)){
        QFile::copy(":/images/images/logo.png", logoPath);
    }
    out << "<ScreenOverlay>\n";
    out << "<name>Surveypod Logo</name>\n";
    out << "<Icon>\n";
    out << "<href>file:///" << logoPath.replace("\\", "/") << "</href>\n";
    out << "</Icon>\n";
    out << "<overlayXY x=\"0\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>\n";
    out << "<screenXY x=\"0.02\" y=\"0.98\" xunits=\"fraction\" yunits=\"fraction\"/>\n";
    out << "<size x=\"0.2\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n";
    out << "</ScreenOverlay>\n";


    out << "<Style id=\"baseStyle\">"
           "<IconStyle><color>ff0000ff</color><scale>1.4</scale>"
           "<Icon><href>http://maps.google.com/mapfiles/kml/paddle/red-circle.png</href></Icon>"
           "</IconStyle></Style>\n";

    out << "<Style id=\"roverStyle\">"
           "<IconStyle><color>ff00ff00</color><scale>1.2</scale>"
           "<Icon><href>http://maps.google.com/mapfiles/kml/paddle/grn-circle.png</href></Icon>"
           "</IconStyle></Style>\n";

    out << "<Style id=\"lineStyle\">"
           "<LineStyle><color>ffff0000</color><width>2</width></LineStyle>"
           "</Style>\n";

    QSet<QString> addedStations;

    out << "<Folder><name>Baselines</name>\n";
    for (auto it = posData.begin(); it != posData.end(); ++it)
    {
        const PosData &pd = it.value();
        double baseLat = pd.basePosition.geodetic.lat;
        double baseLon = pd.basePosition.geodetic.lon;
        double baseH = pd.basePosition.geodetic.h;

        double roverLat = pd.roverPosition.geodetic.lat;
        double roverLon = pd.roverPosition.geodetic.lon;
        double roverH = pd.roverPosition.geodetic.h;

        QString from = pd.from;
        QString to = pd.to;

        // BASE MARKER
        if (!addedStations.contains(from))
        {
            addedStations.insert(from);
            out << "<Placemark>\n";
            out << "<name>" << from << "</name>\n";
            out << "<styleUrl>#baseStyle</styleUrl>\n";
            out << "<description><![CDATA[\n";
            out << "<b>Latitude:</b> " << baseLat << "<br/>";
            out << "<b>Longitude:</b> " << baseLon << "<br/>";
            out << "<b>Height:</b> " << baseH << " m\n";
            out << "]]></description>\n";
            out << "<Point><coordinates>" << baseLon << "," << baseLat << "," << baseH << "</coordinates></Point>\n";
            out << "</Placemark>\n";
        }
        // ROVER MARKER
        if (!addedStations.contains(to))
        {
            addedStations.insert(to);
            out << "<Placemark>\n";
            out << "<name>" << to << "</name>\n";
            out << "<styleUrl>#roverStyle</styleUrl>\n";
            out << "<description><![CDATA[\n";
            out << "<b>Latitude:</b> " << roverLat << " m<br/>";
            out << "<b>Longitude:</b> " << roverLon << " m<br/>";
            out << "<b>Hright:</b> " << roverH << " m<br/>";
            out << "]]></description>\n";
            out << "<Point><coordinates>" << roverLon << "," << roverLat << "," << roverH << "</coordinates></Point>\n";
            out << "</Placemark>\n";
        }

        // BASELINE LINE
        out << "<Placemark>\n";
        out << "<name>" << from << " → " << to << "</name>\n";
        out << "<styleUrl>#lineStyle</styleUrl>\n";
        out << "<description><![CDATA[\n";
        out << "<b>From:</b> " << from << "<br/>";
        out << "<b>To:</b> " << to << "<br/>";
        out << "<b>Ellipsoidal Distance:</b> "<< pd.ellipsoidDistance << " m<br/>";
        out << "]]></description>\n";
        out << "<LineString>\n";
        out << "<tessellate>1</tessellate>\n";
        out << "<coordinates>\n";
        out << baseLon  << "," << baseLat  << "," << baseH  << "\n";
        out << roverLon << "," << roverLat << "," << roverH << "\n";
        out << "</coordinates>\n";
        out << "</LineString>\n";
        out << "</Placemark>\n";
    }
    out << "</Folder>\n";
    out << "</Document>\n";
    out << "</kml>\n";
    file.close();
}

#include "utils.h"
#include <QScreen>
#include <QGuiApplication>
#include <QTimeZone>
#include <QRegularExpression>
#include <QSettings>

static constexpr double WGS84_A  = 6378137.0;
static constexpr double WGS84_F  = 1.0 / 298.257223563;
static constexpr double WGS84_E2 = 2*WGS84_F - WGS84_F*WGS84_F;

Utils::Utils(QObject *parent)
    : QObject{parent}
{}

std::string Utils::dateTimeToString(QDateTime time) {
    QString timeqstring = time.toString("yyyy/MM/dd HH:mm:ss");
    return timeqstring.toStdString();
}

QDateTime Utils::ISTtoGPST(QDateTime istTime) {
    QDateTime utcTime = istTime.addSecs(-19800);
    QDateTime gpstTime = utcTime.addSecs(18);
    return gpstTime;
}

QDateTime Utils::GPSTtoIST(QDateTime gpstTime) {
    QDateTime utcTime = gpstTime.addSecs(-18);
    QDateTime istTime = utcTime.addSecs(19800);
    return istTime;
}

QDateTime Utils::DateTimeFromPos(QString date, QString time) {
    QString dateTimeStr = date + " " + time;
    QString format = "yyyy/MM/dd HH:mm:ss.z";
    QDateTime dateTime = QDateTime::fromString(dateTimeStr, format);
    return dateTime;
}

void Utils::ecefToGeodetic(double X, double Y, double Z, double &latDeg, double &lonDeg, double &h)
{
    const double a  = WGS84_A;
    const double e2 = WGS84_E2;
    const double b  = a * (1.0 - WGS84_F);
    const double ep2 = (a*a - b*b) / (b*b);
    lonDeg = qRadiansToDegrees(std::atan2(Y, X));
    double p = std::sqrt(X*X + Y*Y);
    double theta = std::atan2(Z * a, p * b);
    double sinTheta = std::sin(theta);
    double cosTheta = std::cos(theta);
    double lat = std::atan2( Z + ep2 * b * sinTheta*sinTheta*sinTheta, p - e2 * a * cosTheta*cosTheta*cosTheta );
    double sinLat = std::sin(lat);
    double N = a / std::sqrt(1.0 - e2 * sinLat*sinLat);
    h = p / std::cos(lat) - N;
    latDeg = qRadiansToDegrees(lat);
}

bool Utils::parseObsTimeRange(const QString &path, QDateTime &outStart, QDateTime &outEnd) const
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringView v(line);

        if (line.contains("TIME OF FIRST OBS")) {
            int Y = v.mid(0, 6).toInt();
            int M = v.mid(6, 6).toInt();
            int D = v.mid(12, 6).toInt();
            int h = v.mid(18, 6).toInt();
            int m = v.mid(24, 6).toInt();
            double s = v.mid(30, 13).toDouble();
            outStart = QDateTime(QDate(Y, M, D), QTime(h, m, int(floor(s + 0.5))));
            outStart.setTimeZone(QTimeZone::UTC);
        }

        if (line.contains("TIME OF LAST OBS")) {
            QStringView v(line);
            int Y = v.mid(0, 6).toInt();
            int M = v.mid(6, 6).toInt();
            int D = v.mid(12, 6).toInt();
            int h = v.mid(18, 6).toInt();
            int m = v.mid(24, 6).toInt();
            double s = v.mid(30, 13).toDouble();
            outEnd = QDateTime(QDate(Y, M, D), QTime(h, m, int(floor(s + 0.5))));
            outEnd.setTimeZone(QTimeZone::UTC);
            break;
        }
    }
    f.close();
    return outStart.isValid() && outEnd.isValid();
}

AvgLLH Utils::computeAverageLLHFromPos(const QString &posFile)
{
    QFile file(posFile);
    AvgLLH avg;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return avg;

    QTextStream in(&file);

    double slat = 0, slon = 0, sh = 0;
    int count = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("%"))
            continue;

        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 4)
            continue;

        double lat = parts[2].toDouble();
        double lon = parts[3].toDouble();
        double h = parts[4].toDouble();

        slat += lat;
        slon += lon;
        sh   += h;
        count++;
    }

    if (count > 0) {
        avg.lat = slat / (double)count;
        avg.lon = slon / (double)count;
        avg.h   = sh / (double)count;
        avg.valid = true;
    }
    return avg;
}

QString Utils::getNormalizedPath(const QString &path)
{
    QFileInfo fi(path);
    QString canon = fi.canonicalFilePath();
    return canon.isEmpty() ? fi.absoluteFilePath() : canon;
}

QString Utils::findNavForObs(const QString &obsPath)
{
    QFileInfo obsInfo(obsPath);
    QString obsFolder = obsInfo.absolutePath();
    QString baseName  = obsInfo.completeBaseName();

    QDir dir(obsFolder);
    QStringList filters;
    filters << baseName + ".*n";

    QFileInfoList navFiles = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name );
    if (!navFiles.isEmpty())
        return navFiles.first().absoluteFilePath();
    return QString();
}

static QString formatRinexTime(const QString &line){
    int year   = line.mid(0,6).trimmed().toInt();
    int month  = line.mid(6,6).trimmed().toInt();
    int day    = line.mid(12,6).trimmed().toInt();
    int hour   = line.mid(18,6).trimmed().toInt();
    int minute = line.mid(24,6).trimmed().toInt();
    double sec = line.mid(30,13).trimmed().toDouble();
    QString sys = line.mid(48,3).trimmed();
    return QString("%1-%2-%3 %4:%5:%6 %7").arg(year,   4, 10, QChar('0')).arg(month,  2, 10, QChar('0')).arg(day,    2, 10, QChar('0')).arg(hour,   2, 10, QChar('0')).arg(minute, 2, 10, QChar('0')).arg(sec, 0, 'f', 3).arg(sys);
}

void Utils::parseRinexHeaderToJson(const QString &obsPath)
{
    QFile file(obsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    QJsonObject header;
    QJsonObject posxyz;
    QJsonObject polehen;
    QJsonObject basellh;
    QJsonObject finalposxyz;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.contains("END OF HEADER"))
            break;

        if (line.contains("MARKER NAME"))
            header["marker_name"] = line.left(60).trimmed();

        else if (line.contains("MARKER NUMBER"))
            header["marker_number"] = line.left(60).trimmed();

        else if (line.contains("MARKER TYPE"))
            header["marker_type"] = line.left(60).trimmed();

        else if (line.contains("OBSERVER / AGENCY")) {
            QString data = line.left(60).simplified();
            header["observer"] = data;
        }

        else if (line.contains("ANT # / TYPE")) {
            QString data = line.left(60);
            QString serial = data.mid(0, 20).trimmed();
            QString rest = data.mid(20).trimmed();

            QStringList tokens = rest.split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);

            QString type;
            QString radome;
            if (tokens.size() >= 1)
                type = tokens[0];
            if (tokens.size() >= 2)
                radome = tokens[1];

            header["antenna_serial"] = serial;
            header["antenna_type"]   = type;
            header["antenna_radome"] = radome;
        }

        else if (line.contains("REC # / TYPE / VERS")) {
            header["receiver_serial"] = line.mid(0, 20).trimmed();
            header["receiver_type"]   = line.mid(20, 20).trimmed();
            header["receiver_vers"]   = line.mid(40, 20).trimmed();
        }

        else if (line.contains("APPROX POSITION XYZ")) {
            posxyz["X"] = line.mid(0, 14).trimmed().toDouble();
            posxyz["Y"] = line.mid(14, 14).trimmed().toDouble();
            posxyz["Z"] = line.mid(28, 14).trimmed().toDouble();

            finalposxyz["X"] = line.mid(0, 14).trimmed().toDouble();
            finalposxyz["Y"] = line.mid(14, 14).trimmed().toDouble();
            finalposxyz["Z"] = line.mid(28, 14).trimmed().toDouble();
        }

        else if (line.contains("ANTENNA: DELTA H/E/N")) {
            polehen["H"] = line.mid(0, 14).trimmed().toDouble();
            polehen["E"] = line.mid(14, 14).trimmed().toDouble();
            polehen["N"] = line.mid(28, 14).trimmed().toDouble();
        }

        else if (line.contains("TIME OF FIRST OBS")) {
            header["time_first_obs"] = formatRinexTime(line);
        }

        else if (line.contains("TIME OF LAST OBS")) {
            header["time_last_obs"] = formatRinexTime(line);
        }
    }
    basellh["latitude"]  = 0;
    basellh["longitude"] = 0;
    basellh["height"]    = 0;

    header["obsPath"] = obsPath;
    header["isChecked"] = true;
    header["posxyz"] = posxyz;
    header["poleHEN"] = polehen;
    header["basellh"] = basellh;
    header["finalposxyz"] = finalposxyz;
    saveRinexHeader(obsPath, header);
}

QJsonObject Utils::readRinexHeader(const QString &obsPath)
{
    QString jsonPath = QCoreApplication::applicationDirPath() + "/Data/rinex_headers.json";

    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return {};

    QJsonObject root = doc.object();

    if (obsPath.isEmpty())
        return root;

    QFileInfo fi(obsPath);
    QString key = fi.fileName();

    return root.value(key).toObject();
}


void Utils::saveRinexHeader(const QString &obsPath, const QJsonObject &headerObj)
{
    QFileInfo fi(obsPath);
    QString key = fi.fileName();

    QString jsonPath = QCoreApplication::applicationDirPath() + "/Data/rinex_headers.json";
    QFile file(jsonPath);
    QJsonObject root;

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        root = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }

    root[key] = headerObj;

    file.open(QIODevice::WriteOnly);
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

void Utils::updateRinexHeader(const QString &obsPath,const QJsonObject &updates)
{
    QJsonObject root = readRinexHeader();
    if (root.isEmpty())
        return;

    QFileInfo fi(obsPath);
    QString key = fi.fileName();

    QJsonObject obj = root.value(key).toObject();
    for (auto it = updates.begin(); it != updates.end(); ++it) {
        obj[it.key()] = it.value();
    }
    root[key] = obj;
    writeRinexHeader(root);
}

void Utils::writeRinexHeader(const QJsonObject &root)
{
    QString jsonPath = QCoreApplication::applicationDirPath() + "/Data/rinex_headers.json";

    QFile file(jsonPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

bool Utils::hasKMLViewer()
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CLASSES_ROOT\\.kml", QSettings::NativeFormat);
    QString fileType = settings.value(".").toString();
    return !fileType.isEmpty();
#else
    return true;
#endif
}

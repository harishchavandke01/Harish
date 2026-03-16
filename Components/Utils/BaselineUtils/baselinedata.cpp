#include "baselinedata.h"
#include <QRegularExpression>
#include <QRegularExpression>
#include <QTimeZone>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "../custommessagebox.h"
BaselineData::BaselineData(QObject *parent): QObject{parent}
{}

void TempPosData::addPoint(PosPoint *p){
    points.push_back(p);
    if(p->quality == Pointquality::NoFix) noFixCount += 1;
    else if(p->quality == Pointquality::Float) floatCount += 1;
    else if(p->quality == Pointquality::Fix) fixCount += 1;
    else if(p->quality == Pointquality::single) singleCount += 1;
}

QDateTime ObsData::ParseTimeFromObs(QString timestr){
    QDateTime time;
    try{
        static const QRegularExpression regex("\\S+");
        QRegularExpressionMatchIterator matches = regex.globalMatch(timestr);

        QStringList parts;
        while(matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            parts << match.captured(0);
        }

        QString sec;
        if(parts[5][1] == '.') {
            sec = parts[5].mid(0,5);
        } else {
            sec = parts[5].mid(0,6);
        }

        if(parts[1].size() == 1) parts[1] = "0"+parts[1];
        if(parts[2].size() == 1) parts[2] = "0"+parts[2];
        if(parts[3].size() == 1) parts[3] = "0"+parts[3];
        if(parts[4].size() == 1) parts[4] = "0"+parts[4];

        QString dateTimeString = QString("%1-%2-%3 %4:%5:%6").arg(parts[0], parts[1], parts[2], parts[3], parts[4], sec);
        QStringList formats = {"yyyy-MM-dd HH:mm:ss.zzz", "yyyy-MM-dd HH:mm:s.zzz"};


        for(QString &format : formats) {
            time = QDateTime::fromString(dateTimeString, format);
            if(time.isValid()) {
                time.setTimeZone(QTimeZone("UTC"));
                break;
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
    return time;
}

// QDateTime ImageData::getImageData(QString img) {
//     QDateTime dateTime;
//     try {
//         Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(img.toStdString());
//         image->readMetadata();
//         Exiv2::ExifData &exifdata = image->exifData();
//         auto it = exifdata.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
//         std::string datestr = it->toString();
//         dateTime = QDateTime::fromString(QString::fromStdString(datestr), "yyyy:MM:dd HH:mm:ss");
//     }
//     catch (const std::runtime_error& e) {
//         CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
//         messageBox->exec();
//     }
//     catch (const std::exception& e) {
//         CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
//         messageBox->exec();
//     }
//     catch (...) {
//         CustomMessageBox *messageBox = new CustomMessageBox("ERROR", "An Unknown exception has occurred...", "OK");
//         messageBox->exec();
//     }
//     return dateTime;
// }


// Exiv2::Value::UniquePtr convertToDMS(double decimalDegrees) {
//     double absVal = std::fabs(decimalDegrees);

//     int degrees = static_cast<int>(absVal);
//     double minutesFull = (absVal - degrees) * 60.0;
//     int minutes = static_cast<int>(minutesFull);
//     double seconds = (minutesFull - minutes) * 60.0;

//     Exiv2::Value::UniquePtr value =
//         Exiv2::Value::create(Exiv2::unsignedRational);

//     int secNum = static_cast<int>(seconds * 10000);
//     int secDen = 10000;

//     value->read( std::to_string(degrees) + "/1 " + std::to_string(minutes) + "/1 " + std::to_string(secNum) + "/" + std::to_string(secDen) );

//     return value;
// }


// void ImageData::SetImageData(QString img, PosPoint *point, QString dir) {
//     try {
//         Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(img.toStdString());
//         image->readMetadata();
//         Exiv2::ExifData &exifdata = image->exifData();

//         exifdata["Exif.GPSInfo.GPSLatitude"] = *convertToDMS(point->coordinate.latitude);
//         exifdata["Exif.GPSInfo.GPSLatitudeRef"] = (point->coordinate.latitude >= 0.0f) ? "N" : "S";

//         exifdata["Exif.GPSInfo.GPSLongitude"] = *convertToDMS(point->coordinate.longitude);
//         exifdata["Exif.GPSInfo.GPSLongitudeRef"] = (point->coordinate.longitude >= 0.0f) ? "E" : "W";

//         auto altValue = Exiv2::Value::create(Exiv2::unsignedRational);
//         double altitude = std::fabs(point->coordinate.elevation.Ellipsoidal);
//         altValue->read( std::to_string(static_cast<uint32_t>(altitude * 100)) + "/100" );
//         exifdata["Exif.GPSInfo.GPSAltitude"] = *altValue;
//         auto altRefValue = Exiv2::Value::create(Exiv2::unsignedByte);
//         altRefValue->read(
//             (point->coordinate.elevation.Ellipsoidal >= 0.0) ? "0" : "1"
//             );
//         exifdata["Exif.GPSInfo.GPSAltitudeRef"] = *altRefValue;


//         QString dateTimeStr = point->ISTDateTime.toString("yyyy.MM.dd HH:mm:ss");
//         exifdata["Exif.Photo.DateTimeDigitized"] = dateTimeStr.toStdString();

//         std::string focalstr = exifdata["Exif.Photo.FocalLength"].toString();
//         double focalLength = 0.0f;
//         if(focalstr.find('/') != std::string::npos) {
//             std::istringstream iss(focalstr);
//             std::string numeratorStr, denominatorStr;
//             if(std::getline(iss, numeratorStr, '/') && std::getline(iss, denominatorStr)) {
//                 float numerator = std::stof(numeratorStr);
//                 float denominator = std::stof(denominatorStr);
//                 if (denominator != 0.0f) {
//                     focalLength = numerator / denominator;
//                 }
//             }
//         }
//         if(focalLength == 0.0f) {
//             auto value = Exiv2::Value::create(Exiv2::unsignedRational);
//             value->read("25/1");
//             exifdata["Exif.Photo.FocalLength"] = *value;
//         }

//         std::string focalLength35mmStr;
//         auto itFocal35mm = exifdata.findKey(Exiv2::ExifKey("Exif.Photo.FocalLengthIn35mmFilm"));
//         if(itFocal35mm != exifdata.end()) {
//             focalLength35mmStr = itFocal35mm->toString();
//         }

//         if(focalLength35mmStr == "0") {
//             exifdata["Exif.Photo.FocalLengthIn35mmFilm"] = static_cast<uint16_t>(38);
//         }

//         float fNumber = 0.0f;
//         auto itFNumber = exifdata.findKey(Exiv2::ExifKey("Exif.Photo.FNumber"));
//         if(itFNumber != exifdata.end()) {
//             auto rational = itFNumber->toRational();
//             if(rational.second != 0) {
//                 fNumber = static_cast<float>(rational.first) / rational.second;
//             }
//         }

//         if(fNumber == 0.0f) {
//             auto value = Exiv2::Value::create(Exiv2::unsignedRational);
//             value->read("56/10");
//             exifdata["Exif.Photo.FNumber"] = *value;
//         }

//         QString directoryPath = QFileInfo(img).absolutePath();
//         QString taggedFolderPath = QDir(directoryPath).filePath(dir);

//         QDir dir;
//         if(!dir.exists(taggedFolderPath)) {
//             dir.mkpath(taggedFolderPath);
//         }

//         QString newImagePath = QDir(taggedFolderPath).filePath(QFileInfo(img).fileName());
//         QFile::copy(img, newImagePath);

//         Exiv2::Image::UniquePtr newImage = Exiv2::ImageFactory::open(newImagePath.toStdString());
//         newImage->readMetadata();
//         newImage->setExifData(exifdata);
//         newImage->writeMetadata();
//     }
//     catch (const std::runtime_error& e) {
//         CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
//         messageBox->exec();
//     }
//     catch (const std::exception& e) {
//         CustomMessageBox *messageBox = new CustomMessageBox("ERROR", e.what(), "OK");
//         messageBox->exec();
//     }
//     catch (...) {
//         CustomMessageBox *messageBox = new CustomMessageBox("ERROR", "An Unknown exception has occurred...", "OK");
//         messageBox->exec();
//     }
// }



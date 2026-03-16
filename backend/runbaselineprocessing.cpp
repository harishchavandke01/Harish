#include "runbaselineprocessing.h"
#include "baselineprocessing.h"
#include "QThread"
#include <QTimer>
#include <QDir>
#include <sstream>
#include "../../Components/Utils/BaselineUtils/baselinedata.h"
#include "../../Components/Utils/custommessagebox.h"
#include "../../Components/Utils/utils.h"
#include "baselinewrapper.h"


RunBaselineProcessing::RunBaselineProcessing(std::string _args, std::string _ubxfile, std::string _outdir, std::string _obsfile, std::string _navfile, std::string _logfile, QObject *parent)
    : QObject{parent}
{
    args = _args;
    ubxFile = _ubxfile;
    outDir = _outdir;
    obsFile = _obsfile;
    navFile = _navfile;
    logFile = _logfile;
}

RunBaselineProcessing::RunBaselineProcessing(std::string _args, std::string _conffile, std::string _posfile, std::string _baseobsfile, std::string _roverobsfile, std::string _navfile, std::string _logfile, QObject *parent) {
    args = _args;
    confFile = _conffile;
    posFile = _posfile;
    baseobsFile = _baseobsfile;
    roverobsFile = _roverobsfile;
    navFile = _navfile;
    logFile = _logfile;
}

RunBaselineProcessing::RunBaselineProcessing(CustomProgressBar *pb, QObject *parent) {
    progressbar = pb;
}

// RunBaselineProcessing::RunBaselineProcessing(QString _imagesDir, QString _firstImg, QString _lastImage, ImageList *_data, CustomProgressBar *pb) {
//     imagesDir = _imagesDir;
//     firstImg = _firstImg;
//     lastImg = _lastImage;
//     imagedata = _data;
//     progressbar = pb;
// }

// RunBaselineProcessing::RunBaselineProcessing(QString logcsvfile, LogCsvData *data, CustomProgressBar *pb) {
//     logfile = logcsvfile;
//     logdata = data;
//     progressbar = pb;
// }

// RunBaselineProcessing::RunBaselineProcessing(ImageList *_imgdata, PosData* _posdata, PosData *_poseventdata, int idx, QString _tdir, bool interpolate, GeoTaggedData *data, CustomProgressBar *pb) {
//     imagedata = _imgdata;
//     posdata = _posdata;
//     poseventdata = _poseventdata;
//     index = idx;
//     taggedDir = _tdir;
//     interpolation = interpolate;
//     geodata = data;
//     progressbar = pb;
// }

void RunBaselineProcessing::ExecuteConvert() {
    try {
        std::vector<std::string> argsstr;
        argsstr.push_back("SurveyaanConvert");

        std::istringstream iss(args);
        std::string token;
        while(iss >> token) {
            argsstr.push_back(token);
        }

        QFileInfo info(QString::fromStdString(navFile));
        QString fname = info.baseName();

        argsstr.push_back("-d");
        argsstr.push_back(outDir);
        argsstr.push_back("-o");
        argsstr.push_back(obsFile);
        argsstr.push_back("-n");
        argsstr.push_back(navFile);
        if(args.find("-y R") == std::string::npos) {
            argsstr.push_back("-g");
            argsstr.push_back((fname + ".gnav").toStdString());
        }
        if(args.find("-y E") == std::string::npos) {
            argsstr.push_back("-h");
            argsstr.push_back((fname + ".hnav").toStdString());
        }
        if(args.find("-y J") == std::string::npos) {
            argsstr.push_back("-q");
            argsstr.push_back((fname + ".qnav").toStdString());
        }
        argsstr.push_back("-l");
        argsstr.push_back((fname + ".lnav").toStdString());
        if(args.find("-y C") == std::string::npos) {
            argsstr.push_back("-b");
            argsstr.push_back((fname + ".cnav").toStdString());
        }
        if(args.find("-y S") == std::string::npos) {
            argsstr.push_back("-i");
            argsstr.push_back((fname + ".inav").toStdString());
        }
        argsstr.push_back("-s");
        argsstr.push_back((fname + ".sbs").toStdString());
        argsstr.push_back(ubxFile);

        std::vector<std::vector<char>> argBuffers;
        std::vector<char*> argv;

        for(auto& s : argsstr) {
            argBuffers.emplace_back(s.begin(), s.end());
            argBuffers.back().push_back('\0');
            argv.push_back(argBuffers.back().data());
        }

        int argc = static_cast<int>(argv.size());

        BaselineWrapper *bwrap = new BaselineWrapper();
        int res = bwrap->ConvertData(argc, argv.data(), logFile);
        if(res == 0) {
            emit Success();
        } else {
            emit Failed();
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

void RunBaselineProcessing::ExecuteProgress() {
    int i = 0;
    while(!stop && progressbar->getCurrentProgress() != 100) {
        if(progressbar->isclosed) {
            return;
        }
        QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, i));
        i = i+1;
        if(i > 98) i = 99;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void RunBaselineProcessing::ExecutePostProcess() {
    try {
        std::vector<std::string> argsstr;
        argsstr.push_back("SurveyaanPostProcess");

        std::istringstream iss(args);
        std::string token;
        while(iss >> token) {
            argsstr.push_back(token);
        }

        argsstr.push_back("-k");
        argsstr.push_back(confFile);
        argsstr.push_back("-o");
        argsstr.push_back(posFile);
        argsstr.push_back(roverobsFile);
        argsstr.push_back(baseobsFile);
        argsstr.push_back(navFile);

        std::vector<std::vector<char>> argBuffers;
        std::vector<char*> argv;

        for(auto& s : argsstr) {
            argBuffers.emplace_back(s.begin(), s.end());
            argBuffers.back().push_back('\0');
            argv.push_back(argBuffers.back().data());
        }

        int argc = static_cast<int>(argv.size());

        BaselineWrapper *bwrap = new BaselineWrapper();
        int res = bwrap->RtkPost(argc, argv.data(), logFile);
        if(res == 0) {
            emit Success();
        }
        else {
            emit Failed();
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

// void RunBaselineProcessing::ExecuteReadingImages() {
//     QDir dir(imagesDir);
//     dir.setFilter(QDir::Files | QDir::NoSymLinks);
//     dir.setSorting(QDir::Name | QDir::IgnoreCase);

//     QStringList nameFilters;
//     nameFilters << "*.jpg";
//     dir.setNameFilters(nameFilters);

//     QStringList imageslist = dir.entryList();
//     for(int i = 0 ; i < imageslist.size() ; i++) {
//         imageslist[i] = imagesDir + "/" + imageslist[i];
//     }

//     int indexfirst = imageslist.indexOf(firstImg);
//     int indexlast = imageslist.indexOf(lastImg);

//     if(indexfirst == -1) emit Failed();
//     if(indexlast == -1) emit Failed();

//     if(indexfirst > indexlast) {
//         int total = indexlast+imageslist.size()-indexfirst+1;
//         int curr = 0;
//         for(int i = indexfirst ; i < imageslist.size() ; i++) {
//             ImageData *img = new ImageData();
//             img->Name = imageslist[i];
//             img->OriginalTime = img->getImageData(imageslist[i]);
//             imagedata->imgList.push_back(img);
//             curr++;
//             if(progressbar->isclosed) {
//                 emit Stopped();
//                 return;
//             }
//             int per = curr*100/total;
//             QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         }
//         for(int i = 0 ; i <= indexlast ; i++) {
//             ImageData *img = new ImageData();
//             img->Name = imageslist[i];
//             img->OriginalTime = img->getImageData(imageslist[i]);
//             imagedata->imgList.push_back(img);
//             curr++;
//             if(progressbar->isclosed) {
//                 emit Stopped();
//                 return;
//             }
//             int per = curr*100/total;
//             QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         }
//     } else {
//         int total = indexlast-indexfirst+1;
//         int curr = 0;
//         for(int i = indexfirst ; i <= indexlast ; i++) {
//             ImageData *img = new ImageData();
//             img->Name = imageslist[i];
//             img->OriginalTime = img->getImageData(imageslist[i]);
//             imagedata->imgList.push_back(img);
//             curr++;
//             if(progressbar->isclosed) {
//                 emit Stopped();
//                 return;
//             }
//             int per = curr*100/total;
//             QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         }
//     }
//     std::sort(imagedata->imgList.begin(), imagedata->imgList.end(), [](const ImageData *x, const ImageData *y) {
//         return x->OriginalTime < y->OriginalTime;
//     });

//     emit Success();
// }

// void RunBaselineProcessing::ExecuteReadingLog() {
//     QFile file(logfile);
//     Utils *util = new Utils();
//     file.open(QIODevice::ReadOnly | QIODevice::Text);

//     int total = 0;
//     QTextStream tot(&file);
//     while(!tot.atEnd()) {
//         QString line = tot.readLine().trimmed();
//         if(line.isEmpty()) continue;
//         total++;
//     }

//     if(total == 0) {
//         delete util;
//         file.close();
//         emit Success();
//         return;
//     }

//     file.seek(0);
//     int count = 0;
//     QTextStream in(&file);
//     while(!in.atEnd()) {
//         QString line = in.readLine().trimmed();

//         if(line.isEmpty()) continue;
//         QStringList columns = line.split(',');
//         if(columns.isEmpty()) continue;
//         QString dateStr = columns[0].trimmed();

//         QString format = "dd_MM_yyyy_HH_mm_ss_zzz";
//         QDateTime dateTime = QDateTime::fromString(dateStr, format);

//         if(dateTime.isValid()) {
//             QDateTime istTime = util->GPSTtoIST(dateTime);
//             logdata->logs.push_back(istTime);
//         }

//         count++;
//         if(progressbar->isclosed) {
//             delete util;
//             file.close();
//             emit Stopped();
//             return;
//         }
//         int per = count*100/total;
//         QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }

//     delete util;
//     file.close();
//     emit Success();
// }

// PosPoint* RunBaselineProcessing::GetClosestPoint(QDateTime time) {
//     double offsets[] = {0.6, 0.4, 0.8};
//     auto comp = [](const PosPoint* a, const PosPoint* b) {
//         return a->ISTDateTime < b->ISTDateTime;
//     };
//     for(double offset : offsets) {
//         QDateTime desiredTime = QDateTime(QDate(time.date().year(), time.date().month(), time.date().day()),
//                                           QTime(time.time().hour(), time.time().minute(), time.time().second(), time.time().msec())).addMSecs(offset);
//         PosPoint key;
//         key.ISTDateTime = desiredTime;

//         auto it = std::lower_bound(posdata->points.begin(), posdata->points.end(), &key, comp);

//         if (it != posdata->points.end() && (*it)->ISTDateTime == desiredTime) {
//             return *it;
//         }
//     }

//     PosPoint key;
//     key.ISTDateTime = time;

//     auto it = std::lower_bound(posdata->points.begin(), posdata->points.end(), &key, comp);

//     if (it != posdata->points.end() && (*it)->ISTDateTime == time) {
//         return *it;
//     }

//     int fallbackIndex = std::distance(posdata->points.begin(), it);

//     if (fallbackIndex == 0) {
//         return posdata->points[0];
//     }
//     else if (fallbackIndex == static_cast<int>(posdata->points.size())) {
//         return posdata->points.back();
//     }
//     else {
//         QDateTime lowerTime = posdata->points[fallbackIndex - 1]->ISTDateTime;
//         QDateTime higherTime = posdata->points[fallbackIndex]->ISTDateTime;

//         qint64 diffToLower = std::abs(lowerTime.msecsTo(time));
//         qint64 diffToHigher = std::abs(higherTime.msecsTo(time));

//         return (diffToLower <= diffToHigher) ? posdata->points[fallbackIndex - 1] : posdata->points[fallbackIndex];
//     }
// }

// void RunBaselineProcessing::ExecuteGeoTagging() {
//     QVector<QStringList> csvdata;
//     int total = imagedata->imgList.size();
//     int lastidx = total-1;
//     int lastimgidx = index;
//     int lastposidx = 0;
//     geodata->totalimg = total;

//     Utils *util = new Utils();
//     csvdata.append({"Image name", "image index", "image time", "image diff", "pos Index", "pos Point time", "pos diff"});
//     csvdata.append({QFileInfo(imagedata->imgList[index]->Name).fileName(), QString::number(index), imagedata->imgList[index]->OriginalTime.toString("HH:mm:ss"), "0", "0",
//                     util->ISTtoGPST(poseventdata->points[0]->ISTDateTime).toString("HH:mm:ss"), "0"});
//     imagedata->imgList[index]->SetImageData(imagedata->imgList[index]->Name, poseventdata->points[0], taggedDir);
//     geodata->taggedimg++;
//     index++;
//     int count = 1;
//     int per = count*100/total;
//     if(progressbar->isclosed) {
//         emit Stopped();
//         return;
//     }
//     QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//     std::this_thread::sleep_for(std::chrono::milliseconds(10));

//     for(int i = 1 ; i < poseventdata->points.size() ; i++) {
//         if(index > lastidx) break;

//         qint64 msecs = imagedata->imgList[index]->OriginalTime.msecsTo(imagedata->imgList[lastimgidx]->OriginalTime);
//         double imgdiff = std::abs(msecs / 1000.0);

//         msecs = poseventdata->points[i]->ISTDateTime.msecsTo(poseventdata->points[lastposidx]->ISTDateTime);
//         double posdiff = std::abs(msecs / 1000.0);

//         if(posdiff < 0.1) {
//             csvdata.append({"", "", "", QString::number(imgdiff), QString::number(i), util->ISTtoGPST(poseventdata->points[i]->ISTDateTime).toString("HH:mm:ss.fff"),
//                             QString::number(posdiff)});
//             continue;
//         }

//         if(std::abs(imgdiff-posdiff) <= 1) {
//             lastimgidx = index;
//             lastposidx = i;
//             csvdata.append({QFileInfo(imagedata->imgList[index]->Name).fileName(), QString::number(index), imagedata->imgList[index]->OriginalTime.toString("HH:mm:ss"),
//                             QString::number(imgdiff), QString::number(i), util->ISTtoGPST(poseventdata->points[i]->ISTDateTime).toString("HH:mm:ss.fff"), QString::number(posdiff)});
//             imagedata->imgList[index]->SetImageData(imagedata->imgList[index]->Name, poseventdata->points[i], taggedDir);
//             index++;
//             geodata->taggedimg++;
//             count++;
//             per = count*100/total;
//             if(progressbar->isclosed) {
//                 emit Stopped();
//                 return;
//             }
//             QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             continue;
//         }

//         if(imgdiff > posdiff) {
//             csvdata.append({"", "", "", QString::number(imgdiff), QString::number(i), util->ISTtoGPST(poseventdata->points[i]->ISTDateTime).toString("HH:mm:ss.fff"), QString::number(posdiff)});
//             continue;
//         }

//         if(imgdiff < posdiff) {
//             PosPoint* pt = GetClosestPoint(poseventdata->points[lastposidx]->ISTDateTime.addSecs(imgdiff));
//             if(std::abs(imgdiff - (pt->ISTDateTime.secsTo(poseventdata->points[lastposidx]->ISTDateTime))) <= 1 && interpolation) {
//                 csvdata.append({QFileInfo(imagedata->imgList[index]->Name).fileName(), QString::number(index), imagedata->imgList[index]->OriginalTime.toString("HH:mm:ss"),
//                                 QString::number(imgdiff), "", "", QString::number(posdiff), util->ISTtoGPST(pt->ISTDateTime).toString("HH:mm:ss.fff")});
//                 imagedata->imgList[index]->SetImageData(imagedata->imgList[index]->Name, pt, taggedDir);
//                 geodata->interpolatedimg++;
//                 count++;
//                 per = count*100/total;
//                 if(progressbar->isclosed) {
//                     emit Stopped();
//                     return;
//                 }
//                 QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, per));
//                 std::this_thread::sleep_for(std::chrono::milliseconds(100));
//             }
//             else {
//                 csvdata.append({QFileInfo(imagedata->imgList[index]->Name).fileName(), QString::number(index), imagedata->imgList[index]->OriginalTime.toString("HH:mm:ss"),
//                                 QString::number(imgdiff), "", "", QString::number(posdiff)});
//             }
//             index++;
//             i--;
//             continue;
//         }
//     }

//     if(progressbar->isclosed) {
//         emit Stopped();
//         return;
//     }

//     QMetaObject::invokeMethod(progressbar, "setStatus", Qt::QueuedConnection, Q_ARG(QString, "Writing tagged data CSV file..."));

//     QString newCsvPath = QDir(taggedDir).filePath("result.csv");

//     QFile file(newCsvPath);
//     file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

//     QTextStream out(&file);
//     out.setEncoding(QStringConverter::Utf8);

//     for(QStringList& row : csvdata) {
//         out << row.join(',') << '\n';
//     }

//     file.close();

//     emit Success();
// }

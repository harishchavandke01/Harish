#include "baselineprocessing.h"
#include "../Components/Utils/BaselineUtils/baselinedata.h"
#include "../Components/Utils/custommessagebox.h"
#include "../Components/Utils/customprogressbar.h"
#include "../Components/Utils/utils.h"
#include "runbaselineprocessing.h"
#include <QThread>
#include <QDir>
#include <QTimer>
#include <QFileInfo>
#include <QFile>

BaselineProcessing::BaselineProcessing(QObject *parent) : QObject{parent}
{
    auto addRange = [&](char system, int start, int end){
        for(int i=start;i<=end; i++){
            satellites[QString("%1%2").arg(system).arg(i, 2, 10, QChar('0'))] = i;
        }
    };
    addRange('G',  1, 32);
    addRange('R',  1, 24);
    addRange('E',  1, 36);
    addRange('J',  1, 10);
    addRange('S',  1, 39);
    addRange('C',  1, 63);
    addRange('I',  1, 14);
}

void BaselineProcessing::RinexConversion(std::string inputUbxFile, std::string outputDir, std::string obsFilename, std::string navFilename,
                                    QDateTime startTime, QDateTime endTime, double interval) {

    ConvertUserPreference *userPreference = new ConvertUserPreference();
    BaselineUtils *util = new BaselineUtils();
    UserPreference *userPreference1 = util->LoadConfig(userPreference);
    userPreference = dynamic_cast<ConvertUserPreference*>(userPreference1);

    std::string args = "-r ubx -f 5 -od -os";
    if(!startTime.isNull()) {
        Utils *util = new Utils();
        std::string stime = util->dateTimeToString(startTime);
        args += " -ts " + stime;
    }
    if(!endTime.isNull()) {
        Utils *util = new Utils();
        std::string etime = util->dateTimeToString(endTime);
        args += " -te " + etime;
    }
    if(interval != NULL) {
        args += " -ti " + std::to_string(interval);
    }

    if(!userPreference->GPS) {
        args += " -y G";
    }
    if(!userPreference->GLO) {
        args += " -y R";
    }
    if(!userPreference->GAL) {
        args += " -y E";
    }
    if(!userPreference->QZSS) {
        args += " -y J";
    }
    if(!userPreference->BEI) {
        args += " -y C";
    }
    if(!userPreference->SBS) {
        args += " -y S";
    }
    if(!userPreference->NavIC) {
        args += " -y I";
    }
    QString curr = "";
    for(auto &j : userPreference->exsat) {
        if(j == ' ') continue;
        else if(j == ',') {
            if(curr != "" && satellites.find(curr) != satellites.end()) {
                args += " -x " + curr.toStdString();
            }
            curr = "";
        } else curr += j;
    }
    if(curr != "" && satellites.find(curr) != satellites.end()) {
        args += " -x " + curr.toStdString();
    }

    if(userPreference->sepnav) {
        args += " -sp";
    }
    if(userPreference->round) {
        args += " -ro -TADJ=1";
    }
    args += " -hr " + userPreference->receiver.toStdString();
    if(userPreference->marker != "") {
        args += " -hm " + userPreference->marker.toStdString() + "~";
    }
    if(userPreference->antenna != "") {
        args += " -ha " + userPreference->antenna.toStdString() + "~";
    }
    if(userPreference->appx != 1e9 || userPreference->appy != 1e9 || userPreference->appz != 1e9) {
        args += " -hp " + QString::number(userPreference->appx).toStdString() + "/" +
                QString::number(userPreference->appy).toStdString() + "/" +
                QString::number(userPreference->appz).toStdString();
    }
    if(userPreference->poleh != 1e9 || userPreference->polee != 1e9 || userPreference->polen != 1e9) {
        args += " -hd " + QString::number(userPreference->receiver == "PPK" ?
                                              userPreference->poleh : userPreference->poleh+0.117).toStdString() + "/" +
                QString::number(userPreference->polee).toStdString() + "/" +
                QString::number(userPreference->polen).toStdString();
    }
    if(userPreference->iono) {
        args += " -oi";
    }
    if(userPreference->timecorr) {
        args += " -ot";
    }
    if(userPreference->leap) {
        args += " -ol";
    }
    if(userPreference->halfc) {
        args += " -halfc";
    }
    if(userPreference->sort) {
        args += " -sortsats";
    }
    if(userPreference->phase) {
        args += " -ps";
    }

    for(auto &i : list) {
        if(i[0] == 'g') {
            if(i[1] == '1') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '2') gnsssignals[i] = userPreference->sigband2;
            else if(i[1] == '5') gnsssignals[i] = userPreference->sigband3;
        } else if(i[0] == 'r') {
            if(i[1] == '1' || i[1] == '4') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '2' || i[1] == '6') gnsssignals[i] = userPreference->sigband2;
            else if(i[1] == '3') gnsssignals[i] = userPreference->sigband3;
        } else if(i[0] == 'e') {
            if(i[1] == '1') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '7') gnsssignals[i] = userPreference->sigband2;
            else if(i[1] == '5') gnsssignals[i] = userPreference->sigband3;
            else if(i[1] == '6') gnsssignals[i] = userPreference->sigband4;
            else if(i[1] == '8') gnsssignals[i] = userPreference->sigband5;
        } else if(i[0] == 'q') {
            if(i[1] == '1') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '2') gnsssignals[i] = userPreference->sigband2;
            else if(i[1] == '5') gnsssignals[i] = userPreference->sigband3;
            else if(i[1] == '6') gnsssignals[i] = userPreference->sigband4;
        } else if(i[0] == 'b') {
            if(i[1] == '2') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '7') gnsssignals[i] = userPreference->sigband2;
            else if(i[1] == '5') gnsssignals[i] = userPreference->sigband3;
            else if(i[1] == '6') gnsssignals[i] = userPreference->sigband4;
            else if(i[1] == '1') gnsssignals[i] = userPreference->sigband5;
            else if(i[1] == '8') gnsssignals[i] = userPreference->sigband6;
        } else if(i[0] == 'i') {
            if(i[1] == '5') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '9') gnsssignals[i] = userPreference->sigband2;
            else if(i[1] == '1') {
                if(i[2] == 'd') gnsssignals[i] = userPreference->sigband2;
                else if(i[2] == 'x') gnsssignals[i] = userPreference->sigband3;
            }
        } else if(i[0] == 's') {
            if(i[1] == '1') gnsssignals[i] = userPreference->sigband1;
            else if(i[1] == '5') gnsssignals[i] = userPreference->sigband3;
        }
    }
    QString mask = "";
    for(auto &i : userPreference->gnsssignals) {
        if(!i.second) {
            if(mask != "") mask += ',';
            if(i.first[0] == 'g') mask += 'G';
            else if(i.first[0] == 'r') mask += 'R';
            else if(i.first[0] == 'e') mask += 'E';
            else if(i.first[0] == 'q') mask += 'J';
            else if(i.first[0] == 's') mask += 'S';
            else if(i.first[0] == 'b') mask += 'C';
            else if(i.first[0] == 'i') mask += 'I';

            mask += 'L';
            mask += i.first[1];
            mask += i.first[2].toUpper();
        } else {
            if(!gnsssignals[i.first]) {
                if(mask != "") mask += ',';
                if(i.first[0] == 'g') mask += 'G';
                else if(i.first[0] == 'r') mask += 'R';
                else if(i.first[0] == 'e') mask += 'E';
                else if(i.first[0] == 'q') mask += 'J';
                else if(i.first[0] == 's') mask += 'S';
                else if(i.first[0] == 'b') mask += 'C';
                else if(i.first[0] == 'i') mask += 'I';

                mask += 'L';
                mask += i.first[1];
                mask += i.first[2].toUpper();
            }
        }
    }
    if(mask != "") {
        args += " -nomask " + mask.toStdString();
    }

    QString dirPath = QCoreApplication::applicationDirPath() + "/Data";
    std::string logfile = dirPath.toStdString();
    QDir dir;
    if(!dir.exists(dirPath)) {
        dir.mkpath(dirPath);
    }

    logfile += "/conv_debug_log.txt";

    CustomProgressBar *progressbar = new CustomProgressBar(1);
    progressbar->setAttribute(Qt::WA_DeleteOnClose);
    progressbar->setModal(true);
    progressbar->show();

    QMetaObject::invokeMethod(progressbar, "setStatus", Qt::QueuedConnection, Q_ARG(QString, "Converting UBX to OBS file..."));

    RunBaselineProcessing *proc = new RunBaselineProcessing(args, inputUbxFile, outputDir, obsFilename, navFilename, logfile);
    QThread *thread = new QThread();

    proc->moveToThread(thread);

    RunBaselineProcessing *proc2 = new RunBaselineProcessing(progressbar);
    QThread *thread2 = new QThread();
    proc2->moveToThread(thread2);

    connect(thread, &QThread::started, proc, &RunBaselineProcessing::ExecuteConvert);

    connect(proc, &RunBaselineProcessing::Success, this, [=]() {
        proc2->Stop();
        QMetaObject::invokeMethod(progressbar, "setStatus", Qt::QueuedConnection, Q_ARG(QString, "Writing OBS file..."));
        QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, 100));
        QTimer::singleShot(1000, progressbar, &CustomProgressBar::close);
        QTimer::singleShot(1000, this, [=]() {
            CustomMessageBox *messagebox = new CustomMessageBox("INFO", "Data has been converted successfully", "OK");
            messagebox->exec();
            emit Done();
        });
    });

    connect(proc, &RunBaselineProcessing::Failed, this, [=]() {
        proc2->Stop();
        QMetaObject::invokeMethod(progressbar, "setStatus", Qt::QueuedConnection, Q_ARG(QString, "Failed to process data..."));
        QMetaObject::invokeMethod(progressbar, "updateCurrent", Qt::QueuedConnection, Q_ARG(int, 100));
        QTimer::singleShot(1000, progressbar, &CustomProgressBar::close);
        QTimer::singleShot(1000, this, [=]() {
            CustomMessageBox *messagebox = new CustomMessageBox("ERROR", "An error has occured. \nPlease check your data again", "OK");
            messagebox->exec();
        });
    });

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    connect(thread2, &QThread::started, proc2, &RunBaselineProcessing::ExecuteProgress);
    connect(thread2, &QThread::finished, thread2, &QObject::deleteLater);

    thread->start();
    thread2->start();
}

void BaselineProcessing::Process(std::string roverObsFile, std::string baseObsFile, std::string navFile, std::string outputFile,
                                 QDateTime startTime, QDateTime endTime, double interval, bool gps, bool glonass, bool galileo, bool qzss, bool beidou, bool sbs,
                                 int elevationMask, PosMode posMode, Frequency freq, FilterType filterType, GPSAmbiguity gpsAmbiguity,
                                 GlonassAmbiguity gloAmbiguity, BDSAmbiguity bdsAmbiguity, ElevationType eleType, BaseLocationType btype,
                                 double roverPoleHeight, double basePoleHeight,bool showProgress, double x, double y, double z)
{
    PostProcessUserPreference *postUserPreference = new PostProcessUserPreference();
    postUserPreference->GPS = gps;
    postUserPreference->GLO = glonass;
    postUserPreference->GAL = galileo;
    postUserPreference->QZSS = qzss;
    postUserPreference->BEI = beidou;
    postUserPreference->SBS = sbs;
    postUserPreference->elevationMask = elevationMask;
    postUserPreference->posMode = posMode;
    postUserPreference->filterType = filterType;
    postUserPreference->frequency = freq;
    postUserPreference->gpsAmbiguity = gpsAmbiguity;
    postUserPreference->gloAmbiguity = gloAmbiguity;
    postUserPreference->bdsAmbiguity = bdsAmbiguity;
    postUserPreference->baseLocationType = btype;

    BaselineUtils *baseUtils = new BaselineUtils();
    if(posMode != PosMode::Single)
        baseUtils->SaveConfig(postUserPreference);

    std::string args = "";
    if(!startTime.isNull()) {
        Utils *util = new Utils();
        std::string stime = util->dateTimeToString(startTime);
        args += " -ts " + stime;
    }
    if(!endTime.isNull()) {
        Utils *util = new Utils();
        std::string etime = util->dateTimeToString(endTime);
        args += " -te " + etime;
    }
    if(interval != NULL) {
        args += " -ti " + std::to_string(interval);
    }

    QString dirPath = QCoreApplication::applicationDirPath() + "/Data";
    std::string logfile = dirPath.toStdString();
    QDir dir;
    if(!dir.exists(dirPath)) {
        dir.mkpath(dirPath);
    }
    logfile += "/post_debug_log.txt";
    std::string confFile = dirPath.toStdString() + "/rtkpost.conf";
    if(btype == BaseLocationType::Rinex_header) baseUtils->GeneratePostConfigFile(postUserPreference, eleType, elevationMask, roverPoleHeight, basePoleHeight);
    else baseUtils->GeneratePostConfigFile(postUserPreference, eleType, elevationMask,roverPoleHeight,basePoleHeight, x, y, z);

    CustomProgressBar *progressbar = nullptr;
    RunBaselineProcessing *proc2 = nullptr;
    QThread *thread2 = nullptr;

    if (showProgress) {
        progressbar = new CustomProgressBar(1);
        progressbar->setAttribute(Qt::WA_DeleteOnClose);
        progressbar->setModal(true);
        progressbar->show();
    }
    RunBaselineProcessing *proc = new RunBaselineProcessing(args, confFile, outputFile, baseObsFile, roverObsFile, navFile, logfile);

    QThread *thread = new QThread();
    proc->moveToThread(thread);

    connect(thread, &QThread::started, proc, &RunBaselineProcessing::ExecutePostProcess);

    connect(proc, &RunBaselineProcessing::Success, this, [=]() {
        if (showProgress && progressbar) {
            progressbar->updateCurrent(100);
            QTimer::singleShot(500, progressbar, &QWidget::close);
        }
        emit Done();
    });

    connect(proc, &RunBaselineProcessing::Failed, this, [=]() {
        if (showProgress && progressbar) {
            progressbar->updateCurrent(100);
            QTimer::singleShot(500, progressbar, &QWidget::close);
        }
        emit Failed();
    });

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

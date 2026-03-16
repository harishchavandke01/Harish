#ifndef RUNBASELINEPROCESSING_H
#define RUNBASELINEPROCESSING_H

#include <QWidget>
#include <QLibrary>
#include "../../Components/Utils/customprogressbar.h"
#include "../../Components/Utils/BaselineUtils/baselinedata.h"
class RunBaselineProcessing : public QObject
{
    Q_OBJECT
public:
    explicit RunBaselineProcessing(CustomProgressBar *pb, QObject *parent = nullptr);
    explicit RunBaselineProcessing(std::string _args, std::string _ubxfile, std::string _outdir, std::string _obsfile, std::string _navfile, std::string _logfile, QObject *parent = nullptr);
    explicit RunBaselineProcessing(std::string _args, std::string _conffile, std::string _posfile, std::string _baseobsfile, std::string _roverobsfile, std::string _navfile, std::string _logfile, QObject *parent = nullptr);


    void ExecuteConvert();
    void ExecuteProgress();
    void ExecutePostProcess();

signals:
    void Success();
    void Failed();
    void Stopped();

public:
    void Stop() {
        stop = true;
    }

private:
    std::string args;
    std::string navFile;
    std::string logFile;
    char res[1024];
    // Convert
    std::string ubxFile;
    std::string outDir;
    std::string obsFile;
    // Post Process
    std::string confFile;
    std::string posFile;
    std::string baseobsFile;
    std::string roverobsFile;

    CustomProgressBar *progressbar;
    bool stop = false;
};

#endif // RUNBASELINEPROCESSING_H

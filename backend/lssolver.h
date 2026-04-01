#ifndef LSSOLVER_H
#define LSSOLVER_H

#include <QObject>
#include <cmath>
#include <vector>
#include <map>
#include <string>
#include "../../../Context/projectcontext.h"

class LSSolver : public QObject
{
    Q_OBJECT

public:
    explicit LSSolver(const SubnetworkInfo &_info, const QMap<QString, ProjectStation> &_stations, const QVector<ProjectBaseline> &_baselines, const AdjustmentOptions &_options);
    ~LSSolver();
    SubnetworkResult solve();

private:
    SubnetworkInfo info;
    AdjustmentOptions adjustmentOptions;
    std::vector<ProjectBaseline> baselines;
    std::map<std::string, ProjectStation> stations;

    std::map<std::string, int> stationIndexMap; // stations uid -> starting matrix column

    int numUnknowns; // Total columns in the Design Matrix (A)
    int numFixed; // Number of fixed control stations
    int numObs;  // Total rows in the Design Matrix (A)

    std::vector<std::string> log;
    void logMessage(const std::string *message);
    void buildStateMapping(); //maps station uids -> matrix columns
    void applyAdjustments(const std::vector<double> &adjustments); // applies the calculated coords correction to the stations
};

#endif //LSSOLVER_H

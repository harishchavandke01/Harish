#ifndef STATISTICALTESTS_H
#define STATISTICALTESTS_H

#include <QObject>
#include <map>
#include <vector>
#include <string>
#include "../../../Context/projectcontext.h"
#include "eigenwrapper.h"

class StatisticalTests : public QObject
{
    Q_OBJECT
public:
    explicit StatisticalTests(QObject *parent = nullptr);
    // It populates the statistical fields inside the SubnetworkResult struct.
    static void evaluate(const std::map<std::string, ProjectStation> &stations,
                         const std::vector<ProjectBaseline> &baselines,
                         const std::map<std::string, int> &stationIndexMap,
                         EigenWrapper &mathEngine,
                         SubnetworkResult &result,
                         std::vector<std::string> &log);

private:
    // Calculates the critical value for the Chi-Square test based on Degrees of Freedom
    static double calculateChiSquareCritical(int dof, double alpha);

    // Calculates the critical values for the Tau test (blunder detection)
    static double calculateTauCritical(int dof, double significance);

signals:
};

#endif // STATISTICALTESTS_H

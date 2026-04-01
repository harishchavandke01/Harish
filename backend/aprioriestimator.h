#ifndef APRIORIESTIMATOR_H
#define APRIORIESTIMATOR_H

#include <map>
#include <vector>
#include <string>
#include "../../../Context/projectcontext.h"

class APrioriEstimator : public QObject
{
    Q_OBJECT
public:
    explicit APrioriEstimator(QObject *parent = nullptr);
    static bool compute(std::map<std::string, ProjectStation> &stations, const std::vector<ProjectBaseline> &baselines, std::vector<std::string> &log);

signals:
};

#endif // APRIORIESTIMATOR_H

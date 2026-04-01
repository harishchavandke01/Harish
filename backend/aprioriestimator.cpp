#include "aprioriestimator.h"
#include <cmath>

APrioriEstimator::APrioriEstimator(QObject *parent)
    : QObject{parent}
{}


bool APrioriEstimator::compute(std::map<std::string, ProjectStation> &stations, const std::vector<ProjectBaseline> &baselines, std::vector<std::string> &log)
{
    int missingCount = 0;
    for(auto &st :  stations){
        ProjectStation &station = st.second;
        if(std::isnan(station.ecef.X) || std::isnan(station.ecef.Y) || std::isnan(station.ecef.Z)){
            missingCount++;
        }
    }

    if(missingCount==0){
        log.push_back("All stations have the initial ECEF coordinates, skiping the a priori computation");
        return true;
    }

    log.push_back("Estimating the a priori ECEF coordiantes.");
    bool progressMade = true;
    while (missingCount > 0 && progressMade) {
        progressMade = false;
        for (const ProjectBaseline &bl : baselines) {
            std::string fromUID = bl.fromStationId.toStdString();
            std::string toUID = bl.toStationId.toStdString();

            if (stations.find(fromUID) == stations.end() || stations.find(toUID) == stations.end()) {
                continue;
            }

            ProjectStation &stFrom = stations[fromUID];
            ProjectStation &stTo = stations[toUID];

            bool fromValid = !std::isnan(stFrom.ecef.X);
            bool toValid   = !std::isnan(stTo.ecef.X);

            if (fromValid && !toValid) {
                stTo.ecef.X = stFrom.ecef.X + bl.dX;
                stTo.ecef.Y = stFrom.ecef.Y + bl.dY;
                stTo.ecef.Z = stFrom.ecef.Z + bl.dZ;

                missingCount--;
                progressMade = true;
                log.push_back("Propagated coordinates forward to station: " + stTo.stationId.toStdString());
            }
            else if (!fromValid && toValid) {
                stFrom.ecef.X = stTo.ecef.X - bl.dX;
                stFrom.ecef.Y = stTo.ecef.Y - bl.dY;
                stFrom.ecef.Z = stTo.ecef.Z - bl.dZ;

                missingCount--;
                progressMade = true;
                log.push_back("Propagated coordinates backward to station: " + stFrom.stationId.toStdString());
            }
        }
    }
    if (missingCount > 0) {
        log.push_back("ERROR: Could not compute initial coordinates for " + std::to_string(missingCount) + " station(s).");
        log.push_back("The subnetwork is disconnected. Ensure all float stations are linked to a fixed control point.");
        return false;
    }

    log.push_back("A priori coordinate estimation completed successfully.");
    return true;
}

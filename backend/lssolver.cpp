#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "lssolver.h"
#include "aprioriestimator.h"
#include "matrixbuilder.h"
#include "eigenwrapper.h"
#include "statisticaltests.h"

LSSolver::LSSolver(const SubnetworkInfo &_info, const QMap<QString, ProjectStation> &_stations, const QVector<ProjectBaseline> &_baselines, const AdjustmentOptions &_options)
    : info(_info), adjustmentOptions(_options), numFixed(0), numObs(0), numUnknowns(0)
{
    for(auto it = _stations.constBegin(); it!=_stations.constEnd();it++){
        std::string uid = it.key().toStdString();
        stations[uid] = it.value();
    }

    baselines.reserve(_baselines.size());
    for(const ProjectBaseline &pb : _baselines){
        baselines.push_back(pb);
    }
    numObs = static_cast<int>(baselines.size()) * 3; //every 3D baseline provide 3 observations (dx, dy, dz)
}

SubnetworkResult LSSolver::solve()
{
    SubnetworkResult result;
    result.subnetworkIndex = info.index;
    result.success = false;

    //phase1
    buildStateMapping();
    // DELEGATE TO MODULE 2
    if (!APrioriEstimator::compute(stations, baselines, log)) {
        for (const std::string &msg : log) {
            result.iterationLog.append(QString::fromStdString(msg));
        }
        return result;
    }

    // phase 2
    int iteration = 1;
    int maxIterations = 15;
    bool converged = false;
    double convergenceLimit = 1.0e-5;
    double currentRms = 0.0;

    std::string startMsg = "Starting non linear least square adjustment...";
    logMessage(&startMsg);

    EigenWrapper mathEngine;

    while(!converged && iteration <= maxIterations){
        Eigen::SparseMatrix<double> A, Cov;
        Eigen::VectorXd b;
        Eigen::MatrixXd F;

        if (!MatrixBuilder::build(stations, baselines, stationIndexMap, adjustmentOptions,
                                  numUnknowns, numFixed, numObs, A, Cov, b, F)) {
            std::string errMsg = "Matrix building failed at iteration " + std::to_string(iteration);
            logMessage(&errMsg);
            break;
        }
        std::vector<double> x_adjustments;
        if (!mathEngine.solveSystem(A, Cov, b, F, x_adjustments)) {
            std::string errMsg = "Mathematical solver failed at iteration " + std::to_string(iteration);
            logMessage(&errMsg);
            break;
        }

        applyAdjustments(x_adjustments);

        //check covergence (RMS of the correction)
        double sumEq = 0.0;
        for(double val : x_adjustments){
            sumEq+=(val * val);
        }

        currentRms = std::sqrt(sumEq/x_adjustments.size());
        std::string iterMsg = "Iteration " + std::to_string(iteration) + " | RMS: " + std::to_string(currentRms) + " m";
        logMessage(&iterMsg);
        if(currentRms < convergenceLimit){
            converged = true;
            std::string convMsg = "Network convered";
            logMessage(&convMsg);
        }
        else{
            iteration++;
        }
    }

    //phase 3 quality control and packaging
    if(converged){
        // DELEGATE TO MODULE 5:
        StatisticalTests::evaluate(stations, baselines, stationIndexMap, mathEngine, result, log);

        for(const auto &st : stations){
            const ProjectStation &station = st.second;
            QString uid = QString::fromStdString(st.first);
            result.adjustedECEF.insert(uid, QVector3D(station.ecef.X, station.ecef.Y, station.ecef.Z));
        }
        result.success = true;
    }
    else{
        std::string failMsg = "Adjustment failed to converge.";
        logMessage(&failMsg);
    }

    // Convert the Standard C++ log back to Qt format for UI reading
    for (const std::string &msg : log) {
        result.iterationLog.append(QString::fromStdString(msg));
    }
    return result;
}

void LSSolver::logMessage(const std::string *message)
{
    if(message) log.push_back(*message);
}

//state mapping
void LSSolver::buildStateMapping()
{
    numUnknowns = 0;
    numFixed = 0;
    stationIndexMap.clear();

    for(const auto &st : stations){
        const ProjectStation &station = st.second;
        if(station.isFixed){
            numFixed++;
        }

        stationIndexMap[st.first] = numUnknowns; // Map the UID to the current column index
        numUnknowns+=3; //3 columns for X,Y Z
    }

    std::string msg1 = "State mapping complete: Mapped " + std::to_string(stations.size()) + " stations to " + std::to_string(numUnknowns) + " matrix columns.";
    logMessage(&msg1);

    std::string msg2 = "Found " + std::to_string(numFixed) + " fixed control point(s).";
    logMessage(&msg2);
}

void LSSolver::applyAdjustments(const std::vector<double> &adjustments)
{
    for(auto &st: stations){
        ProjectStation &station = st.second;
        int colIdx = stationIndexMap[st.first];
        station.ecef.X += adjustments[colIdx];
        station.ecef.Y += adjustments[colIdx + 1];
        station.ecef.Z += adjustments[colIdx + 2];
    }
}

#include "statisticaltests.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

StatisticalTests::StatisticalTests(QObject *parent)
    : QObject{parent}
{}

void StatisticalTests::evaluate(const std::map<std::string, ProjectStation> &stations,
                                const std::vector<ProjectBaseline> &baselines,
                                const std::map<std::string, int> &stationIndexMap,
                                EigenWrapper &mathEngine,
                                SubnetworkResult &result,
                                std::vector<std::string> &log)
{
    log.push_back("Running post adjustment statistical analysis...");

    int numObs = static_cast<int>(baselines.size()) * 3;
    int numUnknowns = static_cast<int>(stations.size()) * 3;
    int numFixed = 0;

    // 1. CALCULATE DEGREES OF FREEDOM (DOF)
    for(const auto &val : stations){
        const ProjectStation &station = val.second;
        if(station.isFixed) numFixed++;
    }

    result.dof = numObs - (numUnknowns - (numFixed * 3));

    if(result.dof <= 0){
        log.push_back("WARNING: Degrees of Freedom is <= 0. Statistical tests cannot be performed.");
        result.success = true; // Technically converged, but statistically meaningless
        return;
    }

    // 2. CALCULATE RESIDUALS (v) AND EXACT WEIGHTED SUM OF SQUARES (v^T * P * v)
    double sumSqResiduals = 0.0;
    double weightedSumSq = 0.0;

    for(const ProjectBaseline &bl : baselines){
        std::string fromUID = bl.fromStationId.toStdString();
        std::string toUID = bl.toStationId.toStdString();

        if(stations.find(fromUID) == stations.end() || stations.find(toUID) == stations.end()){
            continue;
        }

        const ProjectStation &stFrom = stations.at(fromUID);
        const ProjectStation &stTo   = stations.at(toUID);

        // v = computed - observed
        double compX = stTo.ecef.X - stFrom.ecef.X;
        double compY = stTo.ecef.Y - stFrom.ecef.Y;
        double compZ = stTo.ecef.Z - stFrom.ecef.Z;

        double vX = compX - bl.dX;
        double vY = compY - bl.dY;
        double vZ = compZ - bl.dZ;

        double vNorm = std::sqrt(vX*vX + vY*vY + vZ*vZ);
        sumSqResiduals += (vNorm * vNorm);

        // Populate Frontend Struct
        SubnetworkResult::Residual res;
        res.base = bl.fromStationId;
        res.rover = bl.toStationId;
        res.vX = vX;
        res.vY = vY;
        res.vZ = vZ;
        res.vNorm = vNorm;

        // --- SALSA EXACT WEIGHTING MATH ---
        // 1. Put the residuals into an Eigen 3x1 vector
        Eigen::Vector3d vVec(vX, vY, vZ);

        // 2. Extract the 3x3 RTKLIB covariance matrix for this specific baseline
        Eigen::Matrix3d covMatrix;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                covMatrix(i, j) = bl.cov[i][j];
            }
        }

        // 3. Rigorously invert the matrix to find the Weight (P)
        double det = covMatrix.determinant();

        // Ensure the matrix is actually mathematically invertible (not all zeros)
        if (std::abs(det) > 1e-15) {
            Eigen::Matrix3d P = covMatrix.inverse();
            // True Weighted Sum of Squares: v^T * P * v
            double localWSS = vVec.transpose() * P * vVec;
            weightedSumSq += localWSS;
        } else {
            // If the covariance matrix is empty (all zeros), treat it as an identity weight matrix
            weightedSumSq += (vX*vX + vY*vY + vZ*vZ);
        }

        result.residuals.append(res);
    }

    // Calculate A Posteriori Variance Factor (Sigma Zero Squared)
    double apv = weightedSumSq / result.dof;
    result.sigma0 = std::sqrt(apv);
    result.rms3D = std::sqrt(sumSqResiduals / baselines.size());
    log.push_back("A Posteriori Variance Factor (Sigma0): " + std::to_string(result.sigma0));

    // 3. GLOBAL CHI-SQUARE TEST
    double alpha = 0.05; // 95% Confidence Level
    double chiCriticalUpper = calculateChiSquareCritical(result.dof, alpha / 2.0);
    double chiCriticalLower = calculateChiSquareCritical(result.dof, 1.0 - (alpha / 2.0));

    result.chiSquareValue = weightedSumSq;

    if (result.chiSquareValue >= chiCriticalLower && result.chiSquareValue <= chiCriticalUpper) {
        result.chiSquarePassed = true;
        log.push_back("Global Chi-Square Test: PASSED");
    } else {
        result.chiSquarePassed = false;
        log.push_back("Global Chi-Square Test: FAILED (Check for blunders or poor apriori weights)");
    }

    // 4. INVERT NORMAL MATRIX & CALCULATE ERROR ELLIPSES (Qxx)
    Eigen::SparseMatrix<double> Qxx;
    if (!mathEngine.invertNormalMatrix(Qxx)) {
        log.push_back("Failed to invert Normal Matrix. Cannot calculate error ellipses.");
        return;
    }

    // Scale Qxx by the A Posteriori Variance to get the final Covariance Matrix
    Qxx = Qxx * apv;

    // Extract 3x3 blocks for each station
    for (const auto &val : stations) {
        // CORRECTED ITERATION SYNTAX
        const std::string &uid = val.first;
        const ProjectStation &st = val.second;

        if (st.isFixed) continue; // Fixed stations have zero error

        int colIdx = stationIndexMap.at(uid);

        // Extract ECEF Covariance 3x3 matrix from Qxx
        Eigen::Matrix3d covECEF;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                covECEF(r, c) = Qxx.coeff(colIdx + r, colIdx + c);
            }
        }

        // --- Rotate ECEF to ENU (Local Horizon) ---
        // Requires Latitude and Longitude in Radians
        double latRad = st.geo.lat * (M_PI / 180.0);
        double lonRad = st.geo.lon * (M_PI / 180.0);

        Eigen::Matrix3d R;
        R(0, 0) = -std::sin(lonRad);
        R(0, 1) =  std::cos(lonRad);
        R(0, 2) =  0.0;

        R(1, 0) = -std::sin(latRad) * std::cos(lonRad);
        R(1, 1) = -std::sin(latRad) * std::sin(lonRad);
        R(1, 2) =  std::cos(latRad);

        R(2, 0) =  std::cos(latRad) * std::cos(lonRad);
        R(2, 1) =  std::cos(latRad) * std::sin(lonRad);
        R(2, 2) =  std::sin(latRad);

        // C_ENU = R * C_ECEF * R^T
        Eigen::Matrix3d covENU = R * covECEF * R.transpose();

        // --- Calculate Ellipse Parameters ---
        StationPrecision prec;
        prec.sigmaE = std::sqrt(std::abs(covENU(0, 0)));
        prec.sigmaN = std::sqrt(std::abs(covENU(1, 1)));
        prec.sigmaU = std::sqrt(std::abs(covENU(2, 2)));

        // Eigenvalues of the 2x2 Horizontal Covariance yield Semi-Major/Minor axes
        Eigen::Matrix2d covHoriz = covENU.block<2, 2>(0, 0);
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> eigenSolver(covHoriz);

        if (eigenSolver.info() == Eigen::Success) {
            Eigen::Vector2d eigenvalues = eigenSolver.eigenvalues();
            Eigen::Matrix2d eigenvectors = eigenSolver.eigenvectors();

            // Eigenvalues are sorted ascending. Max is semi-major squared.
            prec.semiMinor = std::sqrt(std::abs(eigenvalues(0)));
            prec.semiMajor = std::sqrt(std::abs(eigenvalues(1)));

            // Azimuth of semi-major axis (from North, clockwise)
            double xVec = eigenvectors(0, 1); // East component of major axis
            double yVec = eigenvectors(1, 1); // North component of major axis
            double azRad = std::atan2(xVec, yVec);
            if (azRad < 0) azRad += 2.0 * M_PI;

            prec.ellipseAzimuthDeg = azRad * (180.0 / M_PI);
        }

        // Store precision in Qt Result
        result.stationPrecisions.insert(QString::fromStdString(uid), prec);
    }

    log.push_back("Statistical Analysis Complete. Error Ellipses generated.");
}

// CRITICAL VALUE APPROXIMATIONS
double StatisticalTests::calculateChiSquareCritical(int dof, double alpha)
{
    // Uses the Wilson-Hilferty transformation for robust approximation
    // Z is the standard normal quantile. For 95% (two-tailed), Z is approx 1.96.
    double Z = 1.96;
    if (alpha > 0.5) Z = -1.96; // Lower bound

    double d = static_cast<double>(dof);
    double term = 1.0 - (2.0 / (9.0 * d)) + Z * std::sqrt(2.0 / (9.0 * d));

    return d * std::pow(term, 3);
}

double StatisticalTests::calculateTauCritical(int dof, double significance)
{
    // Tau is mathematically undefined for degrees of freedom <= 1
    if (dof <= 1) return 3.0;

    // 1. Get the standard normal Z-score based on the significance level
    // SALSA defaults to 95% confidence (alpha = 0.05, two-tailed)
    double Z = 1.95996;
    if (significance <= 0.01) Z = 2.57583; // 99% confidence fallback

    double nu = static_cast<double>(dof);
    double df_t = nu - 1.0;

    // 2. Approximate the Student's t-critical value from the Z-score
    // This is a standard approximation used in geodetic engines for t-distributions
    double t = Z + (std::pow(Z, 3) + Z) / (4.0 * df_t);

    // 3. Pope's Exact Tau Formula
    // tau = (sqrt(DOF) * t) / sqrt(DOF - 1 + t^2)
    double tau = (std::sqrt(nu) * t) / std::sqrt(nu - 1.0 + (t * t));

    return tau;
}

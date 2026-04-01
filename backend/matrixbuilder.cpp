#include "matrixbuilder.h"

MatrixBuilder::MatrixBuilder(QObject *parent)
{}

bool MatrixBuilder::build(const std::map<std::string, ProjectStation> &stations, const std::vector<ProjectBaseline> &baselines,
                          const std::map<std::string, int> &stationIndexMap, const AdjustmentOptions &adjustmentOptions,
                          int numUnknows, int numFixed, int numObs, Eigen::SparseMatrix<double> &A, Eigen::SparseMatrix<double> &Cov,
                          Eigen::VectorXd &b, Eigen::MatrixXd &F)
{
    //A = Design Matrix (rows: observations, columns : Unknowns)
    //Cov = Covariance matrix (rows: observations, Columns: Observations)
    //b = misclosure vector (rows: obervations, columns : 1)
    //F = constrained matrix (rows: fixed stations *3, columns: Unknowns)

    A.resize(numObs, numUnknows);
    Cov.resize(numObs, numObs);
    b.resize(numObs);
    F.setZero(numFixed * 3, numUnknows);

    //prepare eigen triplets for sparse allocation (triplet (Row, Column, Value)
    std::vector<Eigen::Triplet<double>> tripletListA;
    std::vector<Eigen::Triplet<double>> tripletListCov;

    //reserve memory to prevent reallocation overhead (optimization for large network)
    tripletListA.reserve(numObs*2);
    tripletListCov.reserve(numObs * 3);
    int row = 0 ; // tracks the current observation row index

    //build matrices A, b and Cov
    for(const ProjectBaseline &bl : baselines){
        std::string fromUID=  bl.fromStationId.toStdString();
        std::string toUID = bl.toStationId.toStdString();

        if(stations.find(fromUID) == stations.end() || stations.find(toUID) == stations.end()){
            continue;
        }

        const ProjectStation &stFrom = stations.at(fromUID);
        const ProjectStation &stTo = stations.at(toUID);

        int colFrom = stationIndexMap.at(fromUID);
        int colTo = stationIndexMap.at(toUID);

        //calcualte misclosure vector (b)
        double compX = stTo.ecef.X - stFrom.ecef.X;
        double compY = stTo.ecef.Y - stFrom.ecef.Y;
        double compZ = stTo.ecef.Z - stFrom.ecef.Z;

        b(row) = bl.dX - compX;
        b(row+1) = bl.dY - compY;
        b(row+2) = bl.dZ - compZ;

        //build Design matrix (A)
        // For 3D GNSS baselines, partial derivatives are exactly -1.0 (From) and +1.0 (To)
        // X components
        tripletListA.push_back(Eigen::Triplet<double>(row, colFrom, -1.0));
        tripletListA.push_back(Eigen::Triplet<double>(row, colTo, 1.0));

        // Y components
        tripletListA.push_back(Eigen::Triplet<double>(row+1, colFrom+1, -1.0));
        tripletListA.push_back(Eigen::Triplet<double>(row+1, colTo+1, 1.0));

        //Z component
        tripletListA.push_back(Eigen::Triplet<double>(row+2, colFrom+2, -1.0));
        tripletListA.push_back(Eigen::Triplet<double>(row+2, colTo+2, 1.0));

        //build covariance matrix
        double scalar = adjustmentOptions.aPrioriScalar;
        for(int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                double covValue = 0.0;
                if(adjustmentOptions.useCovariance){
                    covValue = bl.cov[i][j] * scalar;
                }
                else if(i==j){
                    covValue= (i==2)?(adjustmentOptions.defaultSigmaV * adjustmentOptions.defaultSigmaV)
                                    :(adjustmentOptions.defaultSigmaH * adjustmentOptions.defaultSigmaH);
                }

                if(covValue != 0.0){
                    tripletListCov.push_back(Eigen::Triplet<double>(row+i, row+j, covValue));
                }
            }
        }
        row+=3;
    }

    //compile the sparse matrices from the triplet lists (Eigen builds it optimally here)
    A.setFromTriplets(tripletListA.begin(), tripletListA.end());
    Cov.setFromTriplets(tripletListCov.begin(), tripletListCov.end());

    // build constraint matrix F for fixed stations
    int fRow = 0;
    for(const auto &st: stations){
        const ProjectStation &station = st.second;
        if(station.isFixed){
            int colIndex = stationIndexMap.at(st.first);
            //Tp freeze a station in the Null space, place a 1.0 in its matrix columns
            F(fRow, colIndex) = 1.0;
            F(fRow+1, colIndex+1) = 1.0;
            F(fRow+2, colIndex+2) = 1.0;
            fRow+=3;
        }
    }
    return true;
}


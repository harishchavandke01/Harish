#ifndef MATRIXBUILDER_H
#define MATRIXBUILDER_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <QObject>
#include <vector>
#include <map>
#include <string>
#include "../../../Context/projectcontext.h"

class MatrixBuilder : public QObject
{
    Q_OBJECT
public:
    explicit MatrixBuilder(QObject *parent = nullptr);
    static bool build(const std::map<std::string, ProjectStation> &stations,
                      const std::vector<ProjectBaseline> &baselines,
                      const std::map<std::string, int> &stationIndexMap,
                      const AdjustmentOptions &adjustmentOptions,
                      int numUnknows, int numFixed, int numObs,
                      Eigen::SparseMatrix<double> &A,//design matrix
                      Eigen::SparseMatrix<double> &Cov, //Covariance/ weight matrix
                      Eigen::VectorXd &b, // misclosure vector
                      Eigen::MatrixXd &F // constrint matrix (fixed stations
                      );
signals:
};

#endif // MATRIXBUILDER_H

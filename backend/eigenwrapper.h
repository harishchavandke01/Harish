#ifndef EIGENWRAPPER_H
#define EIGENWRAPPER_H


#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/IterativeLinearSolvers>
#include <QObject>
#include <vector>

class EigenWrapper : public QObject
{
    Q_OBJECT
public:
    explicit EigenWrapper(QObject *parent = nullptr);

    //takes the raw, unwhitened matrices from the Matrix Builder and calculates the final coordinates corrections

    bool solveSystem(const Eigen::SparseMatrix<double> &A, const Eigen::SparseMatrix<double> &Cov, const Eigen::VectorXd &b,
                     const Eigen::MatrixXd &F, std::vector<double> &x_adjustments);

    //called by statistical Tests after success
    bool invertNormalMatrix(Eigen::SparseMatrix<double> &Qxx);

private:
    Eigen::SparseMatrix<double> m_N;

    //uses Cholesky Decomposition (L * L^T) to whiten the design matrix and misclosure vector
    bool whitenSystem(const Eigen::SparseMatrix<double> &A, const Eigen::SparseMatrix<double> &Cov,
                      const Eigen::VectorXd &b, Eigen::SparseMatrix<double> &PA, Eigen::VectorXd &Pb);

    //uses singular value Decomposition (SVD) on the constrained matrix (F) tp find the null space (V0)
    bool computeNullSpace(const Eigen::MatrixXd &F, Eigen::MatrixXd &V0);

signals:
};

#endif //EIGENWRAPPER_H

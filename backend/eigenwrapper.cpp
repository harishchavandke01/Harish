#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "eigenwrapper.h"
#include <iostream>
EigenWrapper::EigenWrapper(QObject *parent)
    : QObject{parent}
{}

bool EigenWrapper::solveSystem(const Eigen::SparseMatrix<double> &A, const Eigen::SparseMatrix<double> &Cov,
                    const Eigen::VectorXd &b, const Eigen::MatrixXd &F, std::vector<double> &x_adjustments)
{
    Eigen::SparseMatrix<double> PA;
    Eigen::VectorXd Pb;

    //step1 whites the system (apply the covariance weights) this gives us PA = Cov^-1 * A and Pb = Cov^-1 *b
    if(!whitenSystem(A,Cov, b, PA,Pb)){
        return false; //covariance matrix is invalid (not positive definite
    }

    //step2 form the full normal equations
    //N = A^T * P * A
    //u = A^T * P * b

    m_N = (A.transpose() * PA).pruned();// .pruned() cleans up near-zero noise in the sparse matrix
    Eigen::VectorXd u = A.transpose() * Pb;

    //step3: compute the null space (SVD on the constraint Matrix F)
    Eigen::MatrixXd V0;
    if(!computeNullSpace(F, V0)){
        return false; // network is mathematically over-constrained
    }

    //step 4 project noraml equations into the Null space
    //this mathematically "locks" the fixed stations so they cannot adjusted
    // N_proj = V0^T * N * V0;
    Eigen::MatrixXd N_proj = V0.transpose() * m_N * V0;
    Eigen::VectorXd u_proj = V0.transpose() * u;

    // step 5 solve the project system (N_proj * y = u_proj)
    // since N_proj is symmetric positive-definite, Cholesky (LLT) is the fastest/safest solver
    Eigen::LLT<Eigen::MatrixXd> llt(N_proj);
    if(llt.info() != Eigen::Success){
        return false;// matrix singularity detected (network geometry is flawed)
    }

    Eigen::VectorXd y = llt.solve(u_proj);

    //step 6 recover the full adjustment vector
    // x = V0 * y
    Eigen::VectorXd x = V0 * y;

    //convert Eigen::VectorXs back to standard vector<double> for the LSSolver
    x_adjustments.assign(x.data(), x.data()+ x.size());
    return true;
}

//stochastic whitening (cholesky decomposition)
bool EigenWrapper::whitenSystem(const Eigen::SparseMatrix<double> &A, const Eigen::SparseMatrix<double> &Cov,
                const Eigen::VectorXd &b, Eigen::SparseMatrix<double> &PA, Eigen::VectorXd &Pb)
{
    //To build the normal equations, we need the weight matrix (P = Cov^-1)
    //Instead of manually inverting a massive covarinace matrix, we use the Eigens
    //simplicialLLT (Cholesky) to safely solve for PA and Pb
    Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> cholesky;
    cholesky.compute(Cov);
    if(cholesky.info() != Eigen::Success){
        return false;
    }

    // solve the systems : Cov * PA = A and Cov *Pb = b
    PA = cholesky.solve(A);
    Pb = cholesky.solve(b);
    if(cholesky.info() != Eigen::Success){
        return false;
    }
    return true;
}

//constraint handling - SVD (Singular value Decomposition)
bool EigenWrapper::computeNullSpace(const Eigen::MatrixXd &F, Eigen::MatrixXd &V0)
{
    int numUnknowns = F.cols();
    int numConstraints  = F.rows();

    //if there are no fixed stations (Free network ), the Null space is just the Identity matrix
    if(numConstraints == 0){
        V0 = Eigen::MatrixXd::Identity(numUnknowns, numUnknowns);
        return true;
    }

    //performs SVD on the constraint matrix F (F = U * sigma * V^T)
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(F, Eigen::ComputeFullV);

    //Determine the rank if the constraint matrix
    svd.setThreshold(1e-7);
    int rank = svd.rank();

    int degreesOfFreedom = numUnknowns - rank;
    //saftey check : are all stations fixed (Nothing to adjust!)
    if(degreesOfFreedom <= 0){
        return false;
    }

    //The Null space (V0) consists of the last 'degreesOfFreedom' columns of V
    V0 = svd.matrixV().rightCols(degreesOfFreedom);
    return true;
}

//post processing - invert normal matrix
bool EigenWrapper::invertNormalMatrix(Eigen::SparseMatrix<double> &Qxx)
{
    // this is called by module 5 to calculate the Error ellipses
    //we invert the saved normal matrix (m_N) by solving the N * Qxx = I
    Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> llt(m_N);
    if(llt.info() != Eigen::Success){
        return false;
    }

    //create a sparse identity matrix
    Eigen::SparseMatrix<double> I(m_N.rows(), m_N.cols());
    I.setIdentity();

    //Qxx represent the a posterior covariance matrix
    Qxx = llt.solve(I);
    return (llt.info() == Eigen::Success);
}



// #ifndef LSSOLVER_H
// #define LSSOLVER_H

// #include <QMap>
// #include <QVector>
// #include <QString>
// #include <Eigen/Dense>
// #include "../../../Context/projectcontext.h"

// class LSSolver
// {
// public:
//     explicit LSSolver(const SubnetworkInfo &info, const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline>&baselines, const AdjustmentOptions &options);
//     SubnetworkResult solve();

// private:
//     SubnetworkInfo m_info;
//     QMap<QString, ProjectStation> m_stations;
//     QVector<ProjectBaseline> m_baselines;
//     AdjustmentOptions m_options;

//     QMap<QString, QString> m_canonicalUid;
//     QMap<QString, QString> m_uidToCanonical;
//     QMap<QString, int> m_freeIdx;
//     int m_nFree = 0;

//     QMap<QString, Eigen::Vector3d> m_pos;
//     QVector<ProjectBaseline> m_active;
//     QVector<QString> m_log;

//     void buildCanonicalMap();
//     void initPositions();
//     void filterBaselines();

//     bool runInnerLoop(Eigen::VectorXd &dx_final, Eigen::MatrixXd &N_inv, Eigen::MatrixXd &A_final, Eigen::VectorXd &w_final, Eigen::MatrixXd &P_final, int outerIter);
//     void buildSystem(const QVector<ProjectBaseline> &baselines, Eigen::MatrixXd &A, Eigen::VectorXd &w, Eigen::MatrixXd &P) const;

//     Eigen::Matrix3d setupErrorCovariance(const ProjectBaseline &bl) const;
//     Eigen::Matrix3d weightBlock(const ProjectBaseline &bl) const;
//     bool hasValidCovariance(const ProjectBaseline &bl) const;

//     double tauCritical(int dof) const;
//     double chi2Approx(double alpha, int dof) const;

//     void propagatePrecisions(const Eigen::MatrixXd &N_inv, double sigma0_sq, SubnetworkResult &result) const;
//     void populateResult(const Eigen::VectorXd &v, const Eigen::MatrixXd &P, const Eigen::MatrixXd &Q_vv, const QVector<ProjectBaseline> &active, double sigma0, int dof, SubnetworkResult &result) const;
// };

// #endif // LSSOLVER_H





#ifndef LSSOLVER_H
#define LSSOLVER_H

#include <QMap>
#include <QVector>
#include <QString>
#include <Eigen/Dense>
#include "../../../Context/projectcontext.h"

class LSSolver
{
public:
    explicit LSSolver(const SubnetworkInfo &info, const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline>&baselines, const AdjustmentOptions &options);
    SubnetworkResult solve();

private:
    SubnetworkInfo m_info;
    QMap<QString, ProjectStation> m_stations;
    QVector<ProjectBaseline> m_baselines;
    AdjustmentOptions m_options;

    QMap<QString, QString> m_canonicalUid;
    QMap<QString, QString> m_uidToCanonical;
    QMap<QString, int> m_freeIdx;
    int m_nFree = 0;

    QMap<QString, Eigen::Vector3d> m_pos;
    QVector<ProjectBaseline> m_active;
    QVector<QString> m_log;

    void buildCanonicalMap();
    void initPositions();
    void filterBaselines();

    bool runInnerLoop(Eigen::VectorXd &dx_final, Eigen::MatrixXd &N_inv, Eigen::MatrixXd &A_final, Eigen::VectorXd &w_final, Eigen::MatrixXd &P_final, int outerIter, int &iterations);
    void buildSystem(const QVector<ProjectBaseline> &baselines, Eigen::MatrixXd &A, Eigen::VectorXd &w, Eigen::MatrixXd &P) const;

    Eigen::Matrix3d setupErrorCovariance(const ProjectBaseline &bl) const;
    Eigen::Matrix3d weightBlock(const ProjectBaseline &bl) const;
    bool hasValidCovariance(const ProjectBaseline &bl) const;

    double tauCritical(int dof) const;
    double chi2Approx(double alpha, int dof) const;

    void propagatePrecisions(const Eigen::MatrixXd &N_inv, double sigma0_sq, SubnetworkResult &result) const;
    void populateResult(const Eigen::VectorXd &v, const Eigen::MatrixXd &P, const Eigen::MatrixXd &Q_vv, const QVector<ProjectBaseline> &active, double sigma0, int dof, SubnetworkResult &result) const;
};

#endif // LSSOLVER_H

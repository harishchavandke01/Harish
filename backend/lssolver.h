#ifndef LSSOLVER_H
#define LSSOLVER_H

#include <QMap>
#include <QVector>
#include <QString>
#include <Eigen/Dense>
#include "../../../Context/projectcontext.h"

// LSSolver — least-squares network adjustment.
//
// Receives copies of all input data and returns a fully populated
// SubnetworkResult. Never touches ProjectContext directly.
//
// Algorithm (two nested loops, matching TBC behaviour):
//   Outer loop  – outlier rejection: removes the worst failing baseline
//                 and re-runs the inner loop from scratch (max 5 passes).
//   Inner loop  – convergence: iterates A/w/P → δx → position update
//                 until ‖δx‖∞ < 1e-6 m (max 10 passes).
//
// After convergence:
//   σ₀, DOF, chi-square test, tau test, propagated station precisions and
//   error ellipses are all computed and stored in the result.

class LSSolver
{
public:
    explicit LSSolver(const SubnetworkInfo                &info,
                      const QMap<QString, ProjectStation> &stations,
                      const QVector<ProjectBaseline>      &baselines,
                      const AdjustmentOptions             &options);

    SubnetworkResult solve();

private:
    // ── Input (value copies, thread-safe) ────────────────────────────────────
    SubnetworkInfo                m_info;
    QMap<QString, ProjectStation> m_stations;
    QVector<ProjectBaseline>      m_baselines;
    AdjustmentOptions             m_options;

    // ── Derived maps ─────────────────────────────────────────────────────────
    // stationId → canonical uid (first lexicographic uid for that stationId)
    QMap<QString, QString> m_canonicalUid;
    // any uid in the subnetwork → its canonical uid
    QMap<QString, QString> m_uidToCanonical;
    // canonical uid → column-block index in the unknown vector (free stations only)
    QMap<QString, int> m_freeIdx;
    int m_nFree = 0;

    // ── Mutable state ────────────────────────────────────────────────────────
    // canonical uid → current ECEF position (updated each inner iteration)
    QMap<QString, Eigen::Vector3d> m_pos;
    // baselines still active (self-loops removed; outliers removed per outer loop)
    QVector<ProjectBaseline> m_active;
    // human-readable log entries
    QVector<QString> m_log;

    // ── Setup helpers ────────────────────────────────────────────────────────
    void buildCanonicalMap();
    void initPositions();
    void filterBaselines();

    // ── Core solver ──────────────────────────────────────────────────────────
    // Returns true on convergence, fills dx_final and N_inv.
    bool runInnerLoop(Eigen::VectorXd       &dx_final,
                      Eigen::MatrixXd       &N_inv,
                      Eigen::MatrixXd       &A_final,
                      Eigen::VectorXd       &w_final,
                      Eigen::MatrixXd       &P_final,
                      int                    outerIter);

    void buildSystem(const QVector<ProjectBaseline> &baselines,
                     Eigen::MatrixXd &A,
                     Eigen::VectorXd &w,
                     Eigen::MatrixXd &P) const;

    // ── Weight helpers ───────────────────────────────────────────────────────
    Eigen::Matrix3d weightBlock(const ProjectBaseline &bl) const;
    bool hasValidCovariance(const ProjectBaseline &bl) const;

    // ── Statistical helpers ──────────────────────────────────────────────────
    double tauCritical(int dof) const;
    double chi2Approx(double alpha, int dof) const;  // Wilson–Hilferty approximation

    // ── Post-solve ───────────────────────────────────────────────────────────
    void propagatePrecisions(const Eigen::MatrixXd &N_inv,
                             double sigma0_sq,
                             SubnetworkResult &result) const;

    void populateResult(const Eigen::VectorXd &v,
                        const Eigen::MatrixXd &P,
                        const Eigen::MatrixXd &Q_vv,
                        const QVector<ProjectBaseline> &active,
                        double sigma0,
                        int    dof,
                        SubnetworkResult &result) const;
};

#endif // LSSOLVER_H

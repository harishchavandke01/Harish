#include "lssolver.h"
#include <cmath>
#include <algorithm>
#include <QDebug>

static constexpr double CONV_EPS = 1e-6;
static constexpr int MAX_INNER = 10;
static constexpr int MAX_OUTER = 5;
static constexpr double SCALE_95 = 1.960;
static constexpr double PI = 3.14159265358979323846;

LSSolver::LSSolver(const SubnetworkInfo &info,const QMap<QString, ProjectStation> &stations,const QVector<ProjectBaseline>&baselines,const AdjustmentOptions &options): m_info(info), m_stations(stations), m_baselines(baselines), m_options(options)
{
    buildCanonicalMap();
    initPositions();
    filterBaselines();
}

void LSSolver::buildCanonicalMap()
{
    QMap<QString, QVector<QString>> byId;
    for (const QString &uid : m_info.stationUIDs) {
        if (!m_stations.contains(uid)) continue;
        byId[m_stations[uid].stationId].append(uid);
    }
    for (auto it = byId.begin(); it != byId.end(); ++it) {
        QVector<QString> &uids = it.value();
        std::sort(uids.begin(), uids.end());
        m_canonicalUid[it.key()] = uids.first();
    }
    for (auto it = byId.constBegin(); it != byId.constEnd(); ++it) {
        const QString &canon = m_canonicalUid[it.key()];
        for (const QString &uid : it.value())
            m_uidToCanonical[uid] = canon;
    }
    m_nFree = 0;
    for (auto it = m_canonicalUid.constBegin(); it != m_canonicalUid.constEnd(); ++it) {
        const QString &uid = it.value();
        if (m_stations.contains(uid) && !m_stations[uid].isFixed)
            m_freeIdx[uid] = m_nFree++;
    }
}

void LSSolver::initPositions()
{
    for (auto it = m_canonicalUid.constBegin(); it != m_canonicalUid.constEnd(); ++it) {
        const QString &uid = it.value();
        if (m_stations.contains(uid)) {
            const EcefPos &e = m_stations[uid].ecef;
            m_pos[uid] = Eigen::Vector3d(e.X, e.Y, e.Z);
        }
    }
}

void LSSolver::filterBaselines()
{
    for (const ProjectBaseline &bl : m_baselines) {
        if (bl.from == bl.to) continue;
        QString fromC = m_uidToCanonical.value(bl.from, bl.from);
        QString toC   = m_uidToCanonical.value(bl.to,   bl.to);
        if (!m_pos.contains(fromC) || !m_pos.contains(toC)) continue;
        if (fromC == toC) continue;
        m_active.append(bl);
    }
}

bool LSSolver::hasValidCovariance(const ProjectBaseline &bl) const
{
    if (bl.cov[0][0] <= 0.0 || bl.cov[1][1] <= 0.0 || bl.cov[2][2] <= 0.0)
        return false;
    Eigen::Matrix3d C;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            C(i, j) = bl.cov[i][j];
    return std::abs(C.determinant()) > 1e-30;
}

Eigen::Matrix3d LSSolver::weightBlock(const ProjectBaseline &bl) const
{
    if (m_options.useCovariance && hasValidCovariance(bl)) {
        Eigen::Matrix3d C;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                C(i, j) = m_options.aPrioriScalar * bl.cov[i][j];
        return C.inverse();
    }
    // Fallback: diagonal weight from default sigmas.
    double wH = 1.0 / (m_options.aPrioriScalar * m_options.defaultSigmaH * m_options.defaultSigmaH);
    double wV = 1.0 / (m_options.aPrioriScalar * m_options.defaultSigmaV * m_options.defaultSigmaV);
    Eigen::Matrix3d W = Eigen::Matrix3d::Zero();
    W(0, 0) = wH;
    W(1, 1) = wH;
    W(2, 2) = wV;
    return W;
}

void LSSolver::buildSystem(const QVector<ProjectBaseline> &baselines, Eigen::MatrixXd &A, Eigen::VectorXd &w, Eigen::MatrixXd &P) const
{
    int n_obs = baselines.size() * 3;
    A = Eigen::MatrixXd::Zero(n_obs, 3 * m_nFree);
    w = Eigen::VectorXd::Zero(n_obs);
    P = Eigen::MatrixXd::Zero(n_obs, n_obs);

    for (int k = 0; k < baselines.size(); ++k) {
        const ProjectBaseline &bl = baselines[k];
        int row = k * 3;
        QString fromC = m_uidToCanonical.value(bl.from, bl.from);
        QString toC   = m_uidToCanonical.value(bl.to,   bl.to);

        Eigen::Vector3d pos_from = m_pos.value(fromC, Eigen::Vector3d::Zero());
        Eigen::Vector3d pos_to   = m_pos.value(toC, Eigen::Vector3d::Zero());

        Eigen::Vector3d obs(bl.dX, bl.dY, bl.dZ);
        w.segment(row, 3) = obs - (pos_to - pos_from);

        if (m_freeIdx.contains(fromC)) {
            int col = m_freeIdx[fromC] * 3;
            A.block(row, col, 3, 3) = -Eigen::Matrix3d::Identity();
        }
        if (m_freeIdx.contains(toC)) {
            int col = m_freeIdx[toC] * 3;
            A.block(row, col, 3, 3) =  Eigen::Matrix3d::Identity();
        }
        P.block(row, row, 3, 3) = weightBlock(bl);
    }
}

bool LSSolver::runInnerLoop(Eigen::VectorXd &dx_final, Eigen::MatrixXd &N_inv, Eigen::MatrixXd &A_final, Eigen::VectorXd &w_final, Eigen::MatrixXd &P_final, int outerIter)
{
    if (m_nFree == 0) {
        int n_obs = m_active.size() * 3;
        dx_final = Eigen::VectorXd::Zero(0);
        N_inv = Eigen::MatrixXd::Zero(0, 0);
        buildSystem(m_active, A_final, w_final, P_final);
        return true;
    }

    bool converged = false;
    for (int iter = 0; iter < MAX_INNER; ++iter) {
        Eigen::MatrixXd A;
        Eigen::VectorXd w;
        Eigen::MatrixXd P;
        buildSystem(m_active, A, w, P);

        Eigen::MatrixXd N = A.transpose() * P * A;
        Eigen::VectorXd c = A.transpose() * P * w;

        Eigen::VectorXd dx;
        if (m_info.isConstrained) {
            Eigen::LDLT<Eigen::MatrixXd> ldlt(N);
            if (ldlt.info() != Eigen::Success) {
                m_log.append(QString("  [outer %1, inner %2] LDLT decomposition failed.").arg(outerIter).arg(iter + 1));
                return false;
            }
            dx = ldlt.solve(c);
        } else {
            dx = N.completeOrthogonalDecomposition().solve(c);
        }

        for (auto it = m_freeIdx.constBegin(); it != m_freeIdx.constEnd(); ++it) {
            m_pos[it.key()] += dx.segment(it.value() * 3, 3);
        }

        double maxDx = dx.cwiseAbs().maxCoeff();
        m_log.append(QString("  [outer %1, inner %2] max|δx| = %3 m").arg(outerIter).arg(iter + 1).arg(maxDx, 0, 'e', 3));

        dx_final = dx;
        buildSystem(m_active, A_final, w_final, P_final);
        if (m_info.isConstrained) {
            N_inv = Eigen::LDLT<Eigen::MatrixXd>(A_final.transpose() * P_final * A_final)
            .solve(Eigen::MatrixXd::Identity(3 * m_nFree, 3 * m_nFree));
        } else {
            N_inv = (A_final.transpose() * P_final * A_final)
            .completeOrthogonalDecomposition()
                .pseudoInverse();
        }

        if (maxDx < CONV_EPS) {
            converged = true;
            break;
        }
    }
    return converged;
}

double LSSolver::chi2Approx(double alpha, int dof) const
{
    double z = (alpha < 0.5) ? -1.959964 : 1.959964;
    double k = static_cast<double>(dof);
    double t = 1.0 - 2.0 / (9.0 * k) + z * std::sqrt(2.0 / (9.0 * k));
    return std::max(0.0, k * t * t * t);
}

double LSSolver::tauCritical(int dof) const
{
    if (dof <= 0) return 1.96;
    double d = static_cast<double>(dof);
    return 1.96 * std::sqrt(d / (d + 0.26 * 1.96 * 1.96));
}

void LSSolver::propagatePrecisions(const Eigen::MatrixXd &N_inv,
                                   double sigma0_sq,
                                   SubnetworkResult &result) const
{
    for (auto it = m_freeIdx.constBegin(); it != m_freeIdx.constEnd(); ++it) {
        const QString &uid = it.key();
        int idx = it.value();

        if (!m_stations.contains(uid)) continue;
        const ProjectStation &st = m_stations[uid];
        Eigen::Matrix3d Cxyz = sigma0_sq * N_inv.block(idx * 3, idx * 3, 3, 3);

        double lat = st.geo.lat * PI / 180.0;
        double lon = st.geo.lon * PI / 180.0;
        double sLat = std::sin(lat), cLat = std::cos(lat);
        double sLon = std::sin(lon), cLon = std::cos(lon);

        Eigen::Matrix3d R;
        R(0, 0) = -sLon; R(0, 1) =  cLon; R(0, 2) = 0.0;
        R(1, 0) = -sLat * cLon;  R(1, 1) = -sLat * sLon;  R(1, 2) = cLat;
        R(2, 0) =  cLat * cLon;  R(2, 1) =  cLat * sLon;  R(2, 2) = sLat;

        Eigen::Matrix3d Cenu = R * Cxyz * R.transpose();

        double sigE = std::sqrt(std::max(0.0, Cenu(0, 0)));
        double sigN = std::sqrt(std::max(0.0, Cenu(1, 1)));
        double sigU = std::sqrt(std::max(0.0, Cenu(2, 2)));

        Eigen::Matrix2d Ch = Cenu.block<2, 2>(0, 0);
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> eig(Ch);

        double semiMinor = std::sqrt(std::max(0.0, eig.eigenvalues()(0)));
        double semiMajor = std::sqrt(std::max(0.0, eig.eigenvalues()(1)));

        Eigen::Vector2d evec = eig.eigenvectors().col(1);
        double azRad = std::atan2(evec(0), evec(1));
        double azDeg = azRad * 180.0 / PI;
        if (azDeg < 0.0) azDeg += 180.0;

        StationPrecision sp;
        sp.sigmaE = sigE;
        sp.sigmaN = sigN;
        sp.sigmaU = sigU;
        sp.semiMajor = semiMajor;
        sp.semiMinor = semiMinor;
        sp.ellipseAzimuthDeg = azDeg;
        sp.horizPrecision95 = SCALE_95 * semiMajor;
        sp.vertPrecision95 = SCALE_95 * sigU;

        result.stationPrecisions[uid] = sp;
    }
}

void LSSolver::populateResult(const Eigen::VectorXd &v, const Eigen::MatrixXd &P, const Eigen::MatrixXd &Q_vv, const QVector<ProjectBaseline> &active, double sigma0, int    dof, SubnetworkResult&result) const
{
    double tau_crit = tauCritical(dof);

    for (int k = 0; k < active.size(); ++k) {
        const ProjectBaseline &bl = active[k];
        int row = k * 3;

        SubnetworkResult::Residual r;
        r.base  = bl.fromStationId;
        r.rover = bl.toStationId;
        r.vX = v(row);
        r.vY = v(row + 1);
        r.vZ = v(row + 2);
        r.vNorm = v.segment(row, 3).norm();

        auto tau_for = [&](int i) -> double {
            double q = Q_vv(row + i, row + i);
            if (q <= 0.0) return 0.0;
            double sig_vi = sigma0 * std::sqrt(q);
            return (sig_vi > 1e-12) ? v(row + i) / sig_vi : 0.0;
        };

        r.tauX = tau_for(0);
        r.tauY = tau_for(1);
        r.tauZ = tau_for(2);
        r.standardizedResidual = std::max({std::abs(r.tauX), std::abs(r.tauY), std::abs(r.tauZ)});
        r.tauFailed = (r.standardizedResidual > tau_crit);
        result.residuals.append(r);
    }
}

SubnetworkResult LSSolver::solve()
{
    SubnetworkResult result;
    result.subnetworkIndex = m_info.index;
    result.constrained = m_info.isConstrained;
    result.usedCovariance  = m_options.useCovariance;
    result.adjustedAt  = QDateTime::currentDateTimeUtc();

    if (m_active.isEmpty()) {
        m_log.append("No active baselines — adjustment aborted.");
        result.iterationLog = m_log;
        return result;
    }

    QMap<QString, Eigen::Vector3d> pos_initial = m_pos;

    for (int outer = 0; outer < MAX_OUTER; ++outer) {

        int n_obs = m_active.size() * 3;
        int n_unknowns = m_nFree * 3;
        int dof = n_obs - n_unknowns;

        m_log.append(QString("Outer iteration %1: %2 baselines, DOF=%3").arg(outer + 1).arg(m_active.size()).arg(dof));

        if (dof < 0) {
            m_log.append("  Insufficient observations (DOF < 0) — stopping.");
            break;
        }
        m_pos = pos_initial;

        Eigen::VectorXd dx_final;
        Eigen::MatrixXd N_inv, A_final, P_final;
        Eigen::VectorXd w_final;

        bool converged = runInnerLoop(dx_final, N_inv, A_final, w_final, P_final, outer + 1);
        if (!converged)
            m_log.append("Inner loop did not fully converge (max iterations reached).");

        Eigen::VectorXd v;
        if (m_nFree > 0)
            v = A_final * dx_final - w_final;
        else
            v = -w_final;

        double vtpv = 0.0;
        for (int k = 0; k < m_active.size(); ++k) {
            Eigen::Vector3d vk = v.segment(k * 3, 3);
            Eigen::Matrix3d Pk = P_final.block(k * 3, k * 3, 3, 3);
            vtpv += (double)(vk.transpose() * Pk * vk);
        }

        double sigma0 = (dof > 0) ? std::sqrt(vtpv / dof) : 0.0;
        double sigma0_sq = sigma0 * sigma0;
        double rms3D = (dof > 0) ? std::sqrt(vtpv / n_obs) : 0.0;

        m_log.append(QString("  σ₀ = %1  vᵀPv = %2  DOF = %3").arg(sigma0, 0, 'f', 4).arg(vtpv, 0, 'f', 4).arg(dof));

        bool chi_passed = false;
        if (dof > 0) {
            double lo = chi2Approx(0.025, dof);
            double hi = chi2Approx(0.975, dof);
            chi_passed = (vtpv >= lo && vtpv <= hi);
            m_log.append(QString("  χ² bounds [%1, %2]  vᵀPv=%3  → %4").arg(lo, 0, 'f', 3).arg(hi, 0, 'f', 3).arg(vtpv, 0, 'f', 4).arg(chi_passed ? "PASSED" : "FAILED"));
        }

        //  Cofactor matrix of residuals (for tau test)

        Eigen::MatrixXd Q_vv = Eigen::MatrixXd::Zero(n_obs, n_obs);
        for (int k = 0; k < m_active.size(); ++k) {
            Eigen::Matrix3d Pk = P_final.block(k * 3, k * 3, 3, 3);
            Q_vv.block(k * 3, k * 3, 3, 3) = Pk.inverse();  // Q_ll diagonal block
        }
        if (m_nFree > 0) {
            Q_vv -= A_final * N_inv * A_final.transpose();
        }

        //  Tau test
        if (dof <= 0) {
            m_log.append("  DOF=0: tau test skipped.");
            result.sigma0 = sigma0;
            result.rms3D = rms3D;
            result.chiSquareValue = vtpv;
            result.dof= dof;
            result.chiSquarePassed = chi_passed;
            result.success = true;
            populateResult(v, P_final, Q_vv, m_active, sigma0, dof, result);
            propagatePrecisions(N_inv, sigma0_sq, result);
            break;
        }

        double tau_crit = tauCritical(dof);
        m_log.append(QString("  τ_crit = %1  (DOF=%2)").arg(tau_crit, 0, 'f', 3).arg(dof));

        int    worst_k    = -1;
        double worst_tau  = 0.0;

        for (int k = 0; k < m_active.size(); ++k) {
            for (int c = 0; c < 3; ++c) {
                int i = k * 3 + c;
                double q = Q_vv(i, i);
                if (q <= 0.0) continue;
                double sig_vi = sigma0 * std::sqrt(q);
                double tau_i  = (sig_vi > 1e-12) ? std::abs(v(i) / sig_vi) : 0.0;
                if (tau_i > worst_tau) {
                    worst_tau = tau_i;
                    worst_k   = k;
                }
            }
        }

        bool tau_pass = (worst_tau <= tau_crit);
        m_log.append(QString("  Max |τ| = %1 (baseline %2) → %3")
                         .arg(worst_tau, 0, 'f', 3)
                         .arg(worst_k >= 0 ? m_active[worst_k].fromStationId
                                                 + "→" + m_active[worst_k].toStationId
                                           : "—")
                         .arg(tau_pass ? "PASS" : "FAIL"));

        if (tau_pass || outer == MAX_OUTER - 1) {
            // All tau tests pass (or we've exhausted outer iterations).
            result.sigma0          = sigma0;
            result.rms3D           = rms3D;
            result.chiSquareValue  = vtpv;
            result.dof             = dof;
            result.chiSquarePassed = chi_passed;
            result.success         = true;
            populateResult(v, P_final, Q_vv, m_active, sigma0, dof, result);
            propagatePrecisions(N_inv, sigma0_sq, result);
            break;
        }

        // Remove the worst baseline and restart the outer loop.
        QString removed = m_active[worst_k].fromStationId
                          + " → " + m_active[worst_k].toStationId;
        m_log.append(QString("  Removing baseline %1 (|τ|=%2 > %3), restarting.")
                         .arg(removed).arg(worst_tau, 0, 'f', 3).arg(tau_crit, 0, 'f', 3));
        m_active.remove(worst_k);

        // Guard: if we've removed too many baselines DOF would go negative.
        int new_dof = (m_active.size() * 3) - (m_nFree * 3);
        if (new_dof < 0) {
            m_log.append("  Removing further baselines would leave DOF < 0 — stopping.");
            break;
        }
    }

    // ── Write adjusted positions into result ──────────────────────────────────
    if (result.success) {
        for (auto it = m_canonicalUid.constBegin(); it != m_canonicalUid.constEnd(); ++it) {
            const QString &uid = it.value();
            if (!m_pos.contains(uid)) continue;

            const Eigen::Vector3d &pos = m_pos[uid];
            QVector3D ecef_f(static_cast<float>(pos.x()),
                             static_cast<float>(pos.y()),
                             static_cast<float>(pos.z()));

            // Write to ALL uids that share this station (aliases).
            for (auto ait = m_uidToCanonical.constBegin();
                 ait != m_uidToCanonical.constEnd(); ++ait) {
                if (ait.value() == uid) {
                    result.adjustedECEF[ait.key()] = ecef_f;

                    // Correction vector (canonical initial vs final).
                    if (pos_initial.contains(uid)) {
                        Eigen::Vector3d corr = pos - pos_initial[uid];
                        result.stationCorrections[ait.key()] =
                            QVector3D(static_cast<float>(corr.x()),
                                      static_cast<float>(corr.y()),
                                      static_cast<float>(corr.z()));
                    }
                }
            }
        }
    }

    result.iterationLog = m_log;
    return result;
}

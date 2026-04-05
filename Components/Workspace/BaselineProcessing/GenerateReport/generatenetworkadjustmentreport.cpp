#include "generatenetworkadjustmentreport.h"

#include <QTextDocument>
#include <QPrinter>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <cmath>
#include <algorithm>

#include "../../../Utils/ProcessUtils/processutils.h"

GenerateNetworkAdjustmentReport::GenerateNetworkAdjustmentReport(QWidget *parent)
    : QWidget{parent}
{}

// ---------------------------------------------------------------------------
// ECEF → geodetic (WGS-84 iterative Bowring method)
// Returns lat/lon in degrees, h in metres (ellipsoidal)
// ---------------------------------------------------------------------------
static void ecef2geo(double X, double Y, double Z,
                     double &latDeg, double &lonDeg, double &h)
{
    const double a  = 6378137.0;
    const double f  = 1.0 / 298.257223563;
    const double e2 = 2.0 * f - f * f;

    double p = std::sqrt(X * X + Y * Y);
    lonDeg   = std::atan2(Y, X) * 180.0 / M_PI;

    if (p < 1e-10) {
        latDeg = (Z >= 0.0) ? 90.0 : -90.0;
        h = std::fabs(Z) - a * std::sqrt(1.0 - e2);
        return;
    }

    double lat = std::atan2(Z, p * (1.0 - e2));
    for (int i = 0; i < 10; ++i) {
        double sinLat = std::sin(lat);
        double N = a / std::sqrt(1.0 - e2 * sinLat * sinLat);
        lat = std::atan2(Z + e2 * N * sinLat, p);
    }
    double sinLat = std::sin(lat);
    double cosLat = std::cos(lat);
    double N = a / std::sqrt(1.0 - e2 * sinLat * sinLat);
    h = (cosLat > 1e-10) ? p / cosLat - N
                          : std::fabs(Z) / sinLat - N * (1.0 - e2);
    latDeg = lat * 180.0 / M_PI;
}

// ---------------------------------------------------------------------------
// Format helpers
// ---------------------------------------------------------------------------
static QString fmt(double v, int decimals = 4)
{
    if (!std::isfinite(v)) return QStringLiteral("—");
    return QString::number(v, 'f', decimals);
}

static QString fmtDMS(double deg)
{
    if (!std::isfinite(deg)) return QStringLiteral("—");
    int sign = (deg < 0) ? -1 : 1;
    double absDeg = std::fabs(deg);
    int d = (int)absDeg;
    double rem = (absDeg - d) * 60.0;
    int m = (int)rem;
    double s = (rem - m) * 60.0;
    return QString("%1%2° %3' %4\"")
               .arg(sign < 0 ? "-" : "")
               .arg(d)
               .arg(m, 2, 10, QChar('0'))
               .arg(QString::number(s, 'f', 5));
}

// ---------------------------------------------------------------------------
// Colour-coded cell helpers
// ---------------------------------------------------------------------------
static QString tdPass(bool passed)
{
    return passed
        ? QStringLiteral("<td style='color:#1a7a42;font-weight:bold;'>PASSED ✓</td>")
        : QStringLiteral("<td style='color:#c0392b;font-weight:bold;'>FAILED ✗</td>");
}

static QString tdTau(bool failed)
{
    return failed
        ? QStringLiteral(" style='color:#c0392b;font-weight:bold;'")
        : QStringLiteral("");
}

// ---------------------------------------------------------------------------
// HTML builder
// ---------------------------------------------------------------------------
QString GenerateNetworkAdjustmentReport::buildHTML(const ProjectContext *ctx,
                                                    const AdjustmentOptions &options)
{
    const AdjustmentResult &ar = ctx->adjustmentResult;

    // -----------------------------------------------------------------------
    // CSS
    // -----------------------------------------------------------------------
    QString css = R"(
<style>
body  { font-family:'Segoe UI',Arial,sans-serif; font-size:9pt; color:#1a1a1a; }
h1    { font-size:16pt; color:#1b3a6b; margin-bottom:2px; }
h2    { font-size:11pt; color:#1b3a6b; margin:14px 0 4px 0;
        border-bottom:2px solid #1b3a6b; padding-bottom:3px; }
p     { margin:3px 0; }
table { width:100%; border-collapse:collapse; margin-bottom:10px; font-size:8.5pt; }
th    { background:#1b3a6b; color:#fff; padding:5px 8px; text-align:left; }
td    { border:1px solid #c8d0da; padding:4px 8px; vertical-align:middle; }
tr:nth-child(even) td { background:#f4f7fb; }
.kv   { width:50%; }
.kv td{ border:none; padding:2px 8px; }
.note { font-size:8pt; color:#555; font-style:italic; margin-top:6px; }
.chi-pass { color:#1a7a42; font-weight:bold; }
.chi-fail { color:#c0392b; font-weight:bold; }
</style>
)";

    QString html;
    html.reserve(1 << 17);   // pre-allocate ~128 KB

    // -----------------------------------------------------------------------
    // Header / banner
    // -----------------------------------------------------------------------
    html += css;
    html += QStringLiteral(
        "<h1>Network Adjustment Report</h1>"
        "<p style='font-size:8pt;color:#555;margin-bottom:10px;'>"
        "Generated: %1 UTC | Software: SurveyPod (Nibrus Technologies)</p>")
            .arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss"));

    // -----------------------------------------------------------------------
    // 1. Adjustment Configuration
    // -----------------------------------------------------------------------
    html += QStringLiteral("<h2>1. Adjustment Configuration</h2>");
    html += QStringLiteral("<table class='kv'>"
        "<tr><td><b>Adjustment Type</b></td><td>%1</td></tr>"
        "<tr><td><b>Use Covariance Matrices</b></td><td>%2</td></tr>"
        "<tr><td><b>A Priori Scalar</b></td><td>%3</td></tr>"
        "<tr><td><b>Default Horizontal &sigma;</b></td><td>%4 m</td></tr>"
        "<tr><td><b>Default Vertical &sigma;</b></td><td>%5 m</td></tr>"
        "</table>")
            .arg(ar.subnetworkResults.isEmpty() ? "—"
                 : (ar.subnetworkResults.constBegin()->constrained
                        ? "Constrained (Control Points Fixed)"
                        : "Free Network (Minimum Constraint)"))
            .arg(options.useCovariance ? "Yes" : "No")
            .arg(QString::number(options.aPrioriScalar, 'f', 4))
            .arg(QString::number(options.defaultSigmaH, 'f', 4))
            .arg(QString::number(options.defaultSigmaV, 'f', 4));

    // -----------------------------------------------------------------------
    // 2. Control (Fixed) Stations
    // -----------------------------------------------------------------------
    html += QStringLiteral("<h2>2. Control Stations (Fixed)</h2>");

    // Collect unique fixed stations
    QMap<QString, const ProjectStation *> fixedStations;
    for (auto it = ctx->stations.constBegin(); it != ctx->stations.constEnd(); ++it) {
        if (it->isFixed)
            fixedStations.insert(it->stationId, &(*it));
    }

    if (fixedStations.isEmpty()) {
        html += QStringLiteral("<p>No fixed control stations — Free Network adjustment.</p>");
    } else {
        html += QStringLiteral(
            "<table>"
            "<tr><th>Station</th><th>Latitude</th><th>Longitude</th>"
            "<th>Ellip. Ht (m)</th><th>Easting (m)</th><th>Northing (m)</th>"
            "<th>ECEF X (m)</th><th>ECEF Y (m)</th><th>ECEF Z (m)</th></tr>");
        for (auto it = fixedStations.constBegin(); it != fixedStations.constEnd(); ++it) {
            const ProjectStation *s = it.value();
            html += QString("<tr>"
                "<td>%1</td><td>%2</td><td>%3</td>"
                "<td>%4</td><td>%5</td><td>%6</td>"
                "<td>%7</td><td>%8</td><td>%9</td>"
                "</tr>")
                .arg(s->stationId)
                .arg(fmtDMS(s->geo.lat))
                .arg(fmtDMS(s->geo.lon))
                .arg(fmt(s->geo.h, 4))
                .arg(fmt(s->easting, 3))
                .arg(fmt(s->northing, 3))
                .arg(fmt(s->ecef.X, 4))
                .arg(fmt(s->ecef.Y, 4))
                .arg(fmt(s->ecef.Z, 4));
        }
        html += QStringLiteral("</table>");
    }

    // -----------------------------------------------------------------------
    // Per-subnetwork sections
    // -----------------------------------------------------------------------
    for (auto sit = ar.subnetworkResults.constBegin();
         sit != ar.subnetworkResults.constEnd(); ++sit)
    {
        const SubnetworkResult &sr = sit.value();
        if (!sr.success) continue;

        QString snetTag = QString("<br/><hr/><h2 style='color:#2c3e50;'>Subnetwork %1 — %2</h2>")
                              .arg(sr.subnetworkIndex)
                              .arg(sr.constrained ? "Constrained" : "Free Network");
        html += snetTag;

        // ------------------------------------------------------------------
        // 3. Adjusted Station Coordinates
        // ------------------------------------------------------------------
        html += QStringLiteral("<h2>3. Adjusted Station Coordinates</h2>");
        html += QStringLiteral(
            "<table>"
            "<tr><th>Station</th>"
            "<th>Latitude</th><th>Longitude</th><th>Ellip. Ht (m)</th>"
            "<th>Easting (m)</th><th>Northing (m)</th>"
            "<th>Adj. X (m)</th><th>Adj. Y (m)</th><th>Adj. Z (m)</th>"
            "<th>dX (mm)</th><th>dY (mm)</th><th>dZ (mm)</th>"
            "</tr>");

        // Collect unique stations in this subnetwork
        QMap<QString, QString> stationIdToUid;  // stationId → first uid
        for (auto it = ar.mergedAdjustedECEF.constBegin();
             it != ar.mergedAdjustedECEF.constEnd(); ++it)
        {
            const QString &uid = it.key();
            if (!ctx->stations.contains(uid)) continue;
            const QString &sid = ctx->stations[uid].stationId;
            if (!stationIdToUid.contains(sid))
                stationIdToUid[sid] = uid;
        }

        ProcessUtils pu;
        for (auto it = stationIdToUid.constBegin(); it != stationIdToUid.constEnd(); ++it) {
            const QString &sid = it.key();
            const QString &uid = it.value();
            const ProjectStation &st = ctx->stations[uid];

            // Adjusted ECEF
            Vector3d64 adj = ar.mergedAdjustedECEF.value(uid, Vector3d64(st.ecef.X, st.ecef.Y, st.ecef.Z));

            // Corrections in mm
            Vector3d64 corr = sr.stationCorrections.value(uid, Vector3d64(0, 0, 0));

            // Convert adjusted ECEF → geodetic
            double latAdj = 0, lonAdj = 0, hAdj = 0;
            ecef2geo(adj.x, adj.y, adj.z, latAdj, lonAdj, hAdj);

            // Adjusted UTM
            double eastAdj = st.easting, northAdj = st.northing;
            try {
                UTMResult utm = pu.WGS84ToUTM(latAdj, lonAdj, hAdj);
                eastAdj  = utm.easting;
                northAdj = utm.northing;
            } catch (...) {}

            html += QString("<tr>"
                "<td>%1</td>"
                "<td>%2</td><td>%3</td><td>%4</td>"
                "<td>%5</td><td>%6</td>"
                "<td>%7</td><td>%8</td><td>%9</td>"
                "<td>%10</td><td>%11</td><td>%12</td>"
                "</tr>")
                .arg(sid)
                .arg(fmtDMS(latAdj))
                .arg(fmtDMS(lonAdj))
                .arg(fmt(hAdj, 4))
                .arg(fmt(eastAdj, 3))
                .arg(fmt(northAdj, 3))
                .arg(fmt(adj.x, 4))
                .arg(fmt(adj.y, 4))
                .arg(fmt(adj.z, 4))
                .arg(fmt(corr.x * 1000.0, 2))
                .arg(fmt(corr.y * 1000.0, 2))
                .arg(fmt(corr.z * 1000.0, 2));
        }
        html += QStringLiteral("</table>");
        html += QStringLiteral("<p class='note'>dX/dY/dZ: coordinate corrections applied during adjustment (mm).</p>");

        // ------------------------------------------------------------------
        // 4. Baseline Residuals
        // ------------------------------------------------------------------
        html += QStringLiteral("<h2>4. Baseline Residuals</h2>");
        html += QStringLiteral(
            "<table>"
            "<tr>"
            "<th>Baseline</th>"
            "<th>Obs &Delta;X (m)</th><th>Obs &Delta;Y (m)</th><th>Obs &Delta;Z (m)</th>"
            "<th>Adj &Delta;X (m)</th><th>Adj &Delta;Y (m)</th><th>Adj &Delta;Z (m)</th>"
            "<th>vX (mm)</th><th>vY (mm)</th><th>vZ (mm)</th>"
            "<th>|v| (mm)</th>"
            "<th>&tau;X</th><th>&tau;Y</th><th>&tau;Z</th>"
            "<th>r<sub>X</sub></th><th>r<sub>Y</sub></th><th>r<sub>Z</sub></th>"
            "<th>Status</th>"
            "</tr>");

        for (const SubnetworkResult::Residual &res : sr.residuals) {
            QString style = res.tauFailed ? " style='background:#fff0f0;'" : "";
            html += QString("<tr%1>"
                "<td>%2 → %3</td>"
                "<td>%4</td><td>%5</td><td>%6</td>"
                "<td>%7</td><td>%8</td><td>%9</td>"
                "<td>%10</td><td>%11</td><td>%12</td>"
                "<td>%13</td>"
                "<td%14>%15</td><td%14>%16</td><td%14>%17</td>"
                "<td>%18</td><td>%19</td><td>%20</td>"
                "%21"
                "</tr>")
                .arg(style)
                .arg(res.base).arg(res.rover)
                // observed
                .arg(fmt(res.obsX, 5)).arg(fmt(res.obsY, 5)).arg(fmt(res.obsZ, 5))
                // adjusted
                .arg(fmt(res.adjX, 5)).arg(fmt(res.adjY, 5)).arg(fmt(res.adjZ, 5))
                // residuals in mm
                .arg(fmt(res.vX * 1000.0, 2))
                .arg(fmt(res.vY * 1000.0, 2))
                .arg(fmt(res.vZ * 1000.0, 2))
                .arg(fmt(res.vNorm * 1000.0, 2))
                // tau
                .arg(tdTau(res.tauFailed))
                .arg(fmt(res.tauX, 3))
                .arg(fmt(res.tauY, 3))
                .arg(fmt(res.tauZ, 3))
                // redundancy
                .arg(fmt(res.redundancyX, 3))
                .arg(fmt(res.redundancyY, 3))
                .arg(fmt(res.redundancyZ, 3))
                // pass/fail
                .arg(res.tauFailed
                     ? QStringLiteral("<td style='color:#c0392b;font-weight:bold;'>FAIL</td>")
                     : QStringLiteral("<td style='color:#1a7a42;font-weight:bold;'>PASS</td>"));
        }
        html += QStringLiteral("</table>");
        html += QStringLiteral(
            "<p class='note'>"
            "r: redundancy number per component (0 = unchecked, 1 = fully controlled). "
            "&tau;: standardised residual; |&tau;| &gt; &tau;<sub>crit</sub> flags an outlier."
            "</p>");

        // ------------------------------------------------------------------
        // 5. Adjustment Statistics
        // ------------------------------------------------------------------
        html += QStringLiteral("<h2>5. Adjustment Statistics</h2>");

        // Compute chi-square bounds for display
        auto chi2Approx = [](double alpha, int dof) -> double {
            if (dof <= 0) return 0.0;
            double z = (alpha < 0.5) ? -1.959964 : 1.959964;
            double k = static_cast<double>(dof);
            double t = 1.0 - 2.0 / (9.0 * k) + z * std::sqrt(2.0 / (9.0 * k));
            return std::max(0.0, k * t * t * t);
        };

        double chi2Lo = chi2Approx(0.025, sr.dof);
        double chi2Hi = chi2Approx(0.975, sr.dof);

        QString chiClass = sr.chiSquarePassed ? "chi-pass" : "chi-fail";
        QString chiStr   = QString("<span class='%1'>%2  [bounds: %3 – %4]</span>")
                               .arg(chiClass)
                               .arg(sr.chiSquarePassed ? "PASSED ✓" : "FAILED ✗")
                               .arg(fmt(chi2Lo, 3))
                               .arg(fmt(chi2Hi, 3));

        html += QString(
            "<table class='kv'>"
            "<tr><td><b>Degrees of Freedom (DOF)</b></td><td>%1</td></tr>"
            "<tr><td><b>Reference Factor (&sigma;<sub>0</sub>)</b></td><td>%2</td></tr>"
            "<tr><td><b>Weighted Sum of Squares (v<sup>T</sup>Pv)</b></td><td>%3</td></tr>"
            "<tr><td><b>3D RMS of Residuals</b></td><td>%4 mm</td></tr>"
            "<tr><td><b>Chi-Square Test (95% confidence)</b></td><td>%5</td></tr>"
            "</table>")
            .arg(sr.dof)
            .arg(fmt(sr.sigma0, 6))
            .arg(fmt(sr.chiSquareValue, 4))
            .arg(fmt(sr.rms3D * 1000.0, 2))
            .arg(chiStr);

        html += QStringLiteral(
            "<p class='note'>"
            "&sigma;<sub>0</sub> = &radic;(v<sup>T</sup>Pv / DOF).  "
            "Value close to 1.0 indicates the a priori weights are consistent with the data."
            "</p>");

        // ------------------------------------------------------------------
        // 6. Station Precisions (95% confidence)
        // ------------------------------------------------------------------
        html += QStringLiteral("<h2>6. Station Precisions (95% Confidence)</h2>");
        html += QStringLiteral(
            "<table>"
            "<tr>"
            "<th>Station</th>"
            "<th>&sigma;<sub>E</sub> (m)</th>"
            "<th>&sigma;<sub>N</sub> (m)</th>"
            "<th>&sigma;<sub>U</sub> (m)</th>"
            "<th>Semi-Major (m)</th>"
            "<th>Semi-Minor (m)</th>"
            "<th>Azimuth (&deg;)</th>"
            "<th>95% Horiz (m)</th>"
            "<th>95% Vert (m)</th>"
            "</tr>");

        for (auto pit = sr.stationPrecisions.constBegin();
             pit != sr.stationPrecisions.constEnd(); ++pit)
        {
            const QString &uid = pit.key();
            const StationPrecision &sp = pit.value();
            QString stId = ctx->stations.contains(uid)
                               ? ctx->stations[uid].stationId : uid;
            html += QString("<tr>"
                "<td>%1</td>"
                "<td>%2</td><td>%3</td><td>%4</td>"
                "<td>%5</td><td>%6</td><td>%7</td>"
                "<td><b>%8</b></td><td><b>%9</b></td>"
                "</tr>")
                .arg(stId)
                .arg(fmt(sp.sigmaE, 5))
                .arg(fmt(sp.sigmaN, 5))
                .arg(fmt(sp.sigmaU, 5))
                .arg(fmt(sp.semiMajor, 5))
                .arg(fmt(sp.semiMinor, 5))
                .arg(fmt(sp.ellipseAzimuthDeg, 1))
                .arg(fmt(sp.horizPrecision95, 5))
                .arg(fmt(sp.vertPrecision95, 5));
        }
        html += QStringLiteral("</table>");
        html += QStringLiteral(
            "<p class='note'>"
            "95% Horizontal = 2.4477 &times; semi-major axis (&chi;<sup>2</sup><sub>2,0.05</sub> = 5.991). "
            "95% Vertical = 1.9600 &times; &sigma;<sub>U</sub> (normal distribution)."
            "</p>");

        // ------------------------------------------------------------------
        // 7. Iteration log
        // ------------------------------------------------------------------
        if (!sr.iterationLog.isEmpty()) {
            html += QStringLiteral("<h2>7. Solver Log</h2><pre style='font-size:7.5pt;color:#333;'>");
            for (const QString &line : sr.iterationLog)
                html += line.toHtmlEscaped() + QStringLiteral("\n");
            html += QStringLiteral("</pre>");
        }
    }

    return html;
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
bool GenerateNetworkAdjustmentReport::savePDF(const ProjectContext *ctx,
                                               const AdjustmentOptions &options,
                                               const QString &filePath)
{
    if (!ctx || filePath.isEmpty()) return false;

    QString htmlBody = buildHTML(ctx, options);
    if (htmlBody.isEmpty()) return false;

    // Ensure output directory exists
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setResolution(300);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    // Banner
    QString bannerHTML = R"(
<table style="width:100%;border:none;margin-bottom:8px;">
  <tr>
    <td style="width:60px;border:none;">
      <img src=':/images/images/surveypod.png' style='max-width:45px;height:auto;'/>
    </td>
    <td style="vertical-align:middle;font-weight:bold;font-size:18px;border:none;">
      Surveypod &mdash; Network Adjustment Report
    </td>
    <td style="text-align:right;font-size:8pt;color:#777;border:none;vertical-align:bottom;">
      Nibrus Technologies Pvt Ltd
    </td>
  </tr>
</table>
<hr style="border:none;border-top:2px solid #1b3a6b;margin:4px 0 12px 0;"/>
)";

    QString fullHTML = QString(
        "<html><head><meta charset='utf-8'/></head>"
        "<body>%1%2</body></html>")
        .arg(bannerHTML, htmlBody);

    QTextDocument doc;
    doc.setHtml(fullHTML);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);

    return true;
}

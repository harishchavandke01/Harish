#include "generatenetworkadjustmentreport.h"
#include <QTextDocument>
#include <QPrinter>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <cmath>
#include <algorithm>

#include "../../../Utils/ProcessUtils/processutils.h"

GenerateNetworkAdjustmentReport::GenerateNetworkAdjustmentReport(QWidget *parent) : QWidget{parent}
{}

static QString fmt(double v, int decimals = 4){
    if(!std::isfinite(v)) return QStringLiteral("-");
    return QString::number(v, 'f', decimals);
}

static QString tdPass(bool passed)
{
    return passed ? QStringLiteral("<td style='color:#1a7a42;font-weight:bold;'>PASSED ✓</td>")
                  : QStringLiteral("<td style='color:#c0392b;font-weight:bold;'>FAILED ✗</td>");
}

static QString tdTau(bool failed)
{
    return failed ? QStringLiteral(" style='color:#c0392b;font-weight:bold;'")
                  : QStringLiteral("");
}

QString GenerateNetworkAdjustmentReport::buildHTML(const ProjectContext *ctx,
                                                   const AdjustmentOptions &options)
{
    const AdjustmentResult &ar = ctx->adjustmentResult;
    QString html;
    html.reserve(1 << 17);

    html += "<div style='text-align:left; font-size:15pt; font-weight:bold; margin-bottom:20pt;'>"
            "Network Adjustment Report</div>";
    html += "<br><br>";
    html += "<p style='font-size:12pt; font-weight:bold;'>Adjustment Configuration</p>";
    html += "<table>";
    html += QString("<tr><td>Adjustment Type</td><td>%1</td></tr>")
                .arg(ar.subnetworkResults.isEmpty() ? "—"
                                                    : (ar.subnetworkResults.constBegin()->constrained
                                                           ? "Constrained (Control Points Fixed)"
                                                           : "Free Network"));
    html += QString("<tr><td>Use Covariance Matrices</td><td>%1</td></tr>")
                .arg(options.useCovariance ? "Yes" : "No");
    html += QString("<tr><td>A Priori Scalar</td><td>%1</td></tr>")
                .arg(QString::number(options.aPrioriScalar, 'f', 4));
    html += QString("<tr><td>Error in Height of Antenna</td><td>%1 m</td></tr>")
                .arg(QString::number(options.antennaHeightError, 'f', 4));
    html += QString("<tr><td>Centering Error</td><td>%1 m</td></tr>")
                .arg(QString::number(options.centeringError, 'f', 4));
    html += "</table>";

    html += "<br><br>";
    html += "<p style='font-size:12pt; font-weight:bold;'>Control Stations (Fixed)</p>";
    html += "<table><tr>"
            "<th>Station</th><th>Latitude</th><th>Longitude</th>"
            "<th>Ellip. Ht</th><th>EGM08</th><th>Easting</th><th>Northing</th></tr>";
    for (auto it = ctx->stations.begin(); it != ctx->stations.end(); ++it) {
        if (!it->isFixed) continue;
        html += QString("<tr>"
                        "<td>%1</td><td>%2</td><td>%3</td>"
                        "<td>%4</td><td>%5</td><td>%6</td><td>%7</td>"
                        "</tr>")
                    .arg(it->stationId)
                    .arg(it->geo.lat)
                    .arg(it->geo.lon)
                    .arg(fmt(it->geo.h,4))
                    .arg(fmt(it->geo.h,4))
                    .arg(fmt(it->easting, 3))
                    .arg(fmt(it->northing, 3));
    }
    html += "</table>";
    html += "<br>";
    for (auto sit = ar.subnetworkResults.begin(); sit != ar.subnetworkResults.end(); ++sit)
    {
        const SubnetworkResult &sr = sit.value();
        if (!sr.success) continue;
        html += "<br><hr><br>";
        html += QString("<p style='font-size:12pt; font-weight:bold;'>Subnetwork %1 — %2</p>")
                    .arg(sr.subnetworkIndex).arg(sr.constrained ? "Constrained" : "Free Network");

        html += "<br>";
        html += "<p style='font-size:12pt; font-weight:bold;'>Adjustment Statistics</p>";
        html += QString("<table>"
                        "<tr><td>Iterations</td><td>%1</td>"
                        "<tr><td>DOF</td><td>%2</td></tr>"
                        "<tr><td>Network reference factor</td><td>%3</td></tr>"
                        "<tr><td>Precision confidence level</td><td>%4</td>"
                        "<tr><td>Chi-square</td><td>%5</td></tr>"
                        "</table>")
                    .arg(sr.iterations)
                    .arg(fmt(sr.dof,2))
                    .arg(fmt(sr.sigma0,6))
                    .arg("95%")
                    .arg(sr.chiSquarePassed ? "PASSED" : "FAILED");

        html +="<br><br>";

        //grid coords
        html += "<p style='font-size:12pt; font-weight:bold;'>Adjusted Grid Coordinates</p>";
        html += "<table style='border-collapse:collapse;'><tr>"
                "<th>Station</th><th>Easting</th><th>Easting error</th><th>Northing</th>"
                "<th>Northing error</th><th>EGM-08</th><th>EGM-08<br>error</th>"
                "</tr>";

        for (auto it = sr.stationCorrections.begin(); it != sr.stationCorrections.end(); ++it)
        {
            QString uid = it.key();
            if (!ctx->stations.contains(uid)) continue;
            const ProjectStation &st = ctx->stations[uid];
            html += QString("<tr>"
                            "<td>%1</td><td>%2</td><td>%3</td><td>%4</td>"
                            "<td>%5</td><td>%6</td>"
                            "<td>%7</td>"
                            "</tr>")
                        .arg(st.stationId)
                        .arg(fmt(st.easting,3))
                        .arg("Eerr")
                        .arg(fmt(st.northing,3))
                        .arg("Nerr")
                        .arg(fmt(st.geo.h, 4))
                        .arg("Elerr");
        }
        html += "</table>";
        html +="<br>";

        //Geodetic coords
        html += "<p style='font-size:12pt; font-weight:bold;'>Adjusted Geodetic Coordinates</p>";
        html += "<table style='border-collapse:collapse;'><tr>"
                "<th>Station</th><th>Latitude</th><th>Lat error</th><th>Longitude</th>"
                "<th>Lon error</th><th>Ellipsoidal</th><th>Ellipsoidal<br>error</th>"
                "</tr>";
        for (auto it = sr.stationCorrections.begin(); it != sr.stationCorrections.end(); ++it)
        {
            QString uid = it.key();
            if (!ctx->stations.contains(uid)) continue;
            const ProjectStation &st = ctx->stations[uid];
            html += QString("<tr>"
                            "<td>%1</td><td>%2</td><td>%3</td><td>%4</td>"
                            "<td>%5</td><td>%6</td>"
                            "<td>%7</td>"
                            "</tr>")
                        .arg(st.stationId)
                        .arg(fmt(st.geo.lat,10))
                        .arg("laterr")
                        .arg(fmt(st.geo.lon,10))
                        .arg("lonrr")
                        .arg(fmt(st.geo.h, 4))
                        .arg("Elerr");
        }
        html += "</table>";
        html +="<br>";

        //ECEF coords
        html += "<p style='font-size:12pt; font-weight:bold;'>Adjusted ECEF Coordinates</p>";
        html += "<table><tr>"
                "<th>Station</th><th>X</th><th>X error</th><th>Y</th>"
                "<th>Y error</th><th>Z</th><th>Z error</th><th>3D Error</th>"
                "</tr>";
        for (auto it = sr.stationCorrections.begin(); it != sr.stationCorrections.end(); ++it)
        {
            QString uid = it.key();
            if (!ctx->stations.contains(uid)) continue;
            const ProjectStation &st = ctx->stations[uid];
            Vector3d64 adj = ar.mergedAdjustedECEF.value(uid, Vector3d64(st.ecef.X, st.ecef.Y, st.ecef.Z));
            html += QString("<tr>"
                            "<td>%1</td><td>%2</td><td>%3</td><td>%4</td>"
                            "<td>%5</td><td>%6</td>"
                            "<td>%7</td><td>%8</td>"
                            "</tr>")
                        .arg(st.stationId)
                        .arg(fmt(adj.x,3))
                        .arg("Xerr")
                        .arg(fmt(adj.y,3))
                        .arg("Yrr")
                        .arg(fmt(adj.z, 3))
                        .arg("Zlerr")
                        .arg("3D error");
        }
        html += "</table>";

        html +="<br><br>";
        html += "<p style='font-size:12pt; font-weight:bold;'>4. Baseline Residuals</p>";

        html += "<table><tr>"
                "<th>Baseline</th>"
                "<th>Obs ΔX</th><th>Obs ΔY</th><th>Obs ΔZ</th>"
                "<th>Adj ΔX</th><th>Adj ΔY</th><th>Adj ΔZ</th>"
                "<th>vX (mm)</th><th>vY (mm)</th><th>vZ (mm)</th>"
                "<th>|v|</th>"
                "<th>τX</th><th>τY</th><th>τZ</th>"
                "<th>rX</th><th>rY</th><th>rZ</th>"
                "<th>Status</th></tr>";

        for (const auto &res : sr.residuals)
        {
            html += QString("<tr>"
                            "<td>%1 → %2</td>"
                            "<td>%3</td><td>%4</td><td>%5</td>"
                            "<td>%6</td><td>%7</td><td>%8</td>"
                            "<td>%9</td><td>%10</td><td>%11</td>"
                            "<td>%12</td>"
                            "<td>%13</td><td>%14</td><td>%15</td>"
                            "<td>%16</td><td>%17</td><td>%18</td>"
                            "<td>%19</td>"
                            "</tr>")
                        .arg(res.base).arg(res.rover)
                        .arg(fmt(res.obsX,5)).arg(fmt(res.obsY,5)).arg(fmt(res.obsZ,5))
                        .arg(fmt(res.adjX,5)).arg(fmt(res.adjY,5)).arg(fmt(res.adjZ,5))
                        .arg(fmt(res.vX*1000,2))
                        .arg(fmt(res.vY*1000,2))
                        .arg(fmt(res.vZ*1000,2))
                        .arg(fmt(res.vNorm*1000,2))
                        .arg(fmt(res.tauX,3))
                        .arg(fmt(res.tauY,3))
                        .arg(fmt(res.tauZ,3))
                        .arg(fmt(res.redundancyX,3))
                        .arg(fmt(res.redundancyY,3))
                        .arg(fmt(res.redundancyZ,3))
                        .arg(res.tauFailed ? "FAIL" : "PASS");
        }

        html += "</table>";

        // ===== 6. PRECISION =====
        html += "<p style='font-size:12pt; font-weight:bold;'>6. Station Precisions</p>";

        html += "<table><tr>"
                "<th>Station</th><th>σE</th><th>σN</th><th>σU</th>"
                "<th>Semi-Major</th><th>Semi-Minor</th><th>Azimuth</th>"
                "<th>95% H</th><th>95% V</th></tr>";

        for (auto it = sr.stationPrecisions.begin(); it != sr.stationPrecisions.end(); ++it)
        {
            QString uid = it.key();
            const StationPrecision &sp = it.value();
            QString id = ctx->stations.contains(uid) ? ctx->stations[uid].stationId : uid;

            html += QString("<tr>"
                            "<td>%1</td>"
                            "<td>%2</td><td>%3</td><td>%4</td>"
                            "<td>%5</td><td>%6</td><td>%7</td>"
                            "<td>%8</td><td>%9</td>"
                            "</tr>")
                        .arg(id)
                        .arg(fmt(sp.sigmaE,5))
                        .arg(fmt(sp.sigmaN,5))
                        .arg(fmt(sp.sigmaU,5))
                        .arg(fmt(sp.semiMajor,5))
                        .arg(fmt(sp.semiMinor,5))
                        .arg(fmt(sp.ellipseAzimuthDeg,1))
                        .arg(fmt(sp.horizPrecision95,5))
                        .arg(fmt(sp.vertPrecision95,5));
        }

        html += "</table>";

        // ===== 7. SOLVER LOG =====
        if (!sr.iterationLog.isEmpty()) {
            html += "<p style='font-size:12pt; font-weight:bold;'>7. Solver Log</p>";
            html += "<pre style='font-size:8pt;'>";
            for (const QString &line : sr.iterationLog)
                html += line + "\n";
            html += "</pre>";
        }
    }

    return html;
}

bool GenerateNetworkAdjustmentReport::savePDF(const ProjectContext *ctx, const AdjustmentOptions &options, const QString &filePath)
{
    if (!ctx || filePath.isEmpty()) return false;
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setResolution(300);
    const QMarginsF marginsMM(10, 10, 10, 10);
    printer.setPageMargins(marginsMM, QPageLayout::Millimeter);

    QString bannerHTML = R"(
        <table style="width:100%; border:none; margin-bottom:10px;">
          <tr>
            <td style="width:80px;">
              <img src=':/images/images/surveypod.png' style='max-width:50px; height:auto;' />
            </td>
            <td style="vertical-align:middle; font-weight:bold; font-size:25px;">
              <b>SurveyPod (Nibrus Technologies Pvt Ltd)</b>
            </td>
          </tr>
        </table>
        <hr style="border:none; border-top:1px solid #ccc; margin:4px 0;" />
        <br/>
    )";

    QString htmlBody = buildHTML(ctx, options);
    if(htmlBody.isEmpty()) return false;

    QString fullHTML = QString(R"(
        <html>
        <head>
            <meta charset="utf-8"/>
            <style>
                body { font-family: "Times New Roman", serif; font-size: 10pt; color: #111; }
                table { width:100%; border-collapse: collapse; margin-bottom: 24pt; }
                th { border:1px solid #444; padding:6px; text-align:center; background:#eee; font-weight:bold; }
                td { border:1px solid #444; padding:6px; text-align:center; vertical-align:middle; }
                hr { border:none; border-top:1px solid #ccc; margin:6pt 0; }
                p { margin:0; }
            </style>
        </head>
        <body>
            %1
            %2
        </body>
        </html>
    )").arg(bannerHTML, htmlBody);

    QTextDocument doc;
    doc.setHtml(fullHTML);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);
    return true;
}

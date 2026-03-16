#include "generateloopclosurereport.h"

#include <QSet>
#include <QTextDocument>
#include <QPrinter>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <cmath>

GenerateLoopClosureReport::GenerateLoopClosureReport(QWidget *parent) : QWidget{parent}
{}

bool GenerateLoopClosureReport::savePDF(const ProjectContext *ctx, const QString &folderPath)
{
    if(!ctx) return false;

    QString htmlBody = buildHTML(ctx->loopReport);
    if(htmlBody.isEmpty()) return false;

    QString outPath = folderPath;
    if(outPath.isEmpty()) return false;

    QDir dir(outPath);
    if(!dir.exists()) dir.mkpath(".");

    outPath += "/loop_closure_report.pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setResolution(300);

    const QMarginsF margins(12, 12, 12, 12);
    printer.setPageMargins(margins, QPageLayout::Millimeter);

    QString headerHTML = R"(
    <table style="width:100%; border:none; margin-bottom:10px;">
      <tr>
        <td style="width:80px; border:none;">
          <img src=':/images/images/surveypod.png' style='max-width:50px; height:auto;' />
        </td>
        <td style="vertical-align:middle; font-weight:bold; font-size:22px; border:none; text-align:left;">
          Surveypod (Nibrus Technologies Pvt Ltd)
        </td>
      </tr>
    </table>
    <hr style="border:none; border-top:1px solid #ccc; margin:4px 0;" />
    <br/>
    )";

    QString fullHTML = QString(R"(
    <html>
    <head>
        <meta charset="utf-8"/>
        <style>
        body { font-family:"Segoe UI", "Times New Roman", serif; font-size:10pt; color:#111; }
        table { width:100%; border-collapse:collapse; margin-bottom:15pt; page-break-inside: avoid; }
        th { border:1px solid #777; padding:5px; text-align:left; background:#f0f0f0; font-weight:bold; }
        td { border:1px solid #777; padding:5px; text-align:left; vertical-align:middle; }
        .no-border td { border:none; padding:2px; }
        hr { border:none; border-top:1px solid #ccc; margin:6pt 0; }
        </style>
    </head>
    <body>
        %1
        %2
    </body>
    </html>
    )").arg(headerHTML, htmlBody);

    QTextDocument doc;
    doc.setHtml(fullHTML);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);

    return true;
}

QString GenerateLoopClosureReport::buildHTML(const LoopReport &report)
{
    auto fmt = [&](double v, int prec = 3)->QString {
        if(!std::isfinite(v)) return "-";
        return QString::number(v, 'f', prec);
    };

    QString html;

    html += "<p style='font-size:14pt; font-weight:bold;'>GNSS Loop Closure Results</p>";
    html += "<p style='font-size:12pt; font-weight:bold; margin-bottom:10px;'>Summary</p>";
    html += QString("<p>Number of Loops: %1 &nbsp;&nbsp;&nbsp; Number Passed: %2 &nbsp;&nbsp;&nbsp; Number Failed: %3</p>")
                .arg(report.numLoops).arg(report.numPassed).arg(report.numFailed);

    html += "<table>";
    html += "<tr><th></th><th>Length (Meter)</th><th>&Delta;3D (Meter)</th>"
            "<th>&Delta;Horiz (Meter)</th><th>&Delta;Vert (Meter)</th><th>PPM</th></tr>";

    html += QString("<tr><td>Best</td><td>-</td><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                .arg(fmt(report.best3D)).arg(fmt(report.bestHoriz, 3)).arg(fmt(report.bestVert, 3)).arg(fmt(report.bestPPM, 3));
    html += QString("<tr><td>Worst</td><td>-</td><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                .arg(fmt(report.worst3D)).arg(fmt(report.worstHoriz)).arg(fmt(report.worstVert)).arg(fmt(report.worstPPM));
    html += QString("<tr><td>Average Loop</td><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                .arg(fmt(report.avgLength)).arg(fmt(report.avg3D)).arg(fmt(report.avgHoriz))
                .arg(fmt(report.avgVert)).arg(fmt(report.avgPPM));
    html += QString("<tr><td>Standard Error</td><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                .arg(fmt(report.stdLength)).arg(fmt(report.std3D)).arg(fmt(report.stdHoriz))
                .arg(fmt(report.stdVert)).arg(fmt(report.stdPPM));
    html += "</table><br/>";

    struct ObsData {
        QString vectorId;
        QString from;
        QString to;
        QString solutionType;
        double length;
        QDateTime startTime;
        int occurrences = 0;
    };

    struct OccDetail {
        QString observationStr;
        QDateTime startTime;
    };

    struct OccData {
        QString point;
        int occurrences = 0;
        QVector<OccDetail> details;
    };

    QMap<QString, ObsData> obsMap;
    QMap<QString, OccData> occMap;
    QSet<QString> uniqueOcc;

    int loopCount = 0;

    if (report.numFailed > 0) {
        html += "<p style='font-size:13pt; font-weight:bold; page-break-before: always;'>Failed Loops</p>";

        for(const LoopInfo &loop : report.loops) {

            if (loop.passed) continue;

            if(loopCount > 0 && loopCount % 2 == 0)
                html += "<div style='page-break-before: always;'></div>";

            html += "<div style='border:1px solid #555; padding:12px; margin-bottom:45px; page-break-inside: avoid;'>";

            html += QString("<p style='font-weight:bold; margin-bottom:2px;'>Loop: %1</p>").arg(loop.loopId);
            html += "<table><tr><th>Vector ID</th><th>From</th><th>To</th><th>Start Time</th></tr>";

            QString vecList;

            for(const auto &vec : loop.vectors) {

                QString dtStr = vec.startTime.isValid() ? vec.startTime.toString("dd-MM-yyyy HH:mm:ss") : "-";

                html += QString("<tr><td>%1 → %2 (%3)</td><td>%1</td><td>%2</td><td>%4</td></tr>")
                            .arg(vec.from).arg(vec.to).arg(vec.baselineId).arg(dtStr);

                vecList += vec.baselineId + "-";

                if(!obsMap.contains(vec.baselineId)) {
                    ObsData od;
                    od.vectorId = QString("%1 → %2 (%3)").arg(vec.from).arg(vec.to).arg(vec.baselineId);
                    od.from = vec.from;
                    od.to = vec.to;
                    od.solutionType = vec.solutionType.isEmpty() ? "Fixed" : vec.solutionType;
                    od.length = vec.length;
                    od.startTime = vec.startTime;
                    obsMap[vec.baselineId] = od;
                }

                obsMap[vec.baselineId].occurrences++;

                QString keyFrom = vec.from + "_" + vec.to + "_" + vec.startTime.toString();

                if(!uniqueOcc.contains(keyFrom)) {

                    uniqueOcc.insert(keyFrom);

                    OccDetail detailFrom;
                    detailFrom.observationStr = QString("%1 → %2").arg(vec.from).arg(vec.to);
                    detailFrom.startTime = vec.startTime;

                    occMap[vec.from].point = vec.from;
                    occMap[vec.from].occurrences++;
                    occMap[vec.from].details.append(detailFrom);
                }

                QString keyTo = vec.to + "_" + vec.from + "_" + vec.startTime.toString();

                if(!uniqueOcc.contains(keyTo)) {

                    uniqueOcc.insert(keyTo);

                    OccDetail detailTo;
                    detailTo.observationStr = QString("%1 → %2").arg(vec.from).arg(vec.to);
                    detailTo.startTime = vec.startTime;

                    occMap[vec.to].point = vec.to;
                    occMap[vec.to].occurrences++;
                    occMap[vec.to].details.append(detailTo);
                }
            }

            vecList.chop(1);

            html += "</table>";

            html += QString(
                        "<table class='no-border' style='margin-bottom: 20px; margin-top: 15px;'>"
                        "<tr><td><b>%1</b></td><td></td><td></td><td></td></tr>"
                        "<tr><td>Length = %2 m </td><td>&Delta;Horizontal = %3 m </td><td>&Delta;Vertical = %4 m </td><td>PPM = %5</td></tr>"
                        "<tr><td>&Delta;3D = %6 m </td><td>&Delta;X = %7 m </td><td>&Delta;Y = %8 m </td><td>&Delta;Z = %9 m</td></tr>"
                        "</table>"
                        ).arg(vecList).arg(fmt(loop.length)).arg(fmt(loop.horizError)).arg(fmt(loop.vertError)).arg(fmt(loop.ppm))
                        .arg(fmt(loop.error3D)).arg(fmt(loop.misclosureXYZ.x())).arg(fmt(loop.misclosureXYZ.y())).arg(fmt(loop.misclosureXYZ.z()));

            html += "</div>";

            loopCount++;
        }
    }

    if (!obsMap.isEmpty()) {
        html += "<div style='page-break-before: always;'></div>";
        html += "<p style='font-size:12pt; font-weight:bold;'>Observations In Failed Loops</p>";
        html += "<table>";
        html += "<tr><th>Vector ID</th><th>From</th><th>To</th><th>Solution Type</th>"
                "<th>Length (Meter)</th><th>Start Time</th><th>No. of Occurrences</th></tr>";

        for(auto it = obsMap.begin(); it != obsMap.end(); ++it) {

            const ObsData &od = it.value();

            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                        .arg(od.vectorId).arg(od.from).arg(od.to).arg(od.solutionType).arg(fmt(od.length))
                        .arg(od.startTime.isValid() ? od.startTime.toString("dd-MM-yyyy HH:mm:ss") : "-")
                        .arg(od.occurrences);
        }

        html += "</table><br/>";
    }

    if (!occMap.isEmpty()) {

        html += "<p style='font-size:12pt; font-weight:bold; page-break-before: always;'>Occupations In Failed Loops</p>";
        html += "<table>";
        html += "<tr><th>Point</th><th>Observations</th><th>Start Time</th><th>No. of Occurrences</th></tr>";

        for(auto it = occMap.begin(); it != occMap.end(); ++it) {

            const OccData &oc = it.value();

            html += QString("<tr style='background-color:#fafafa;'><td><b>%1</b></td><td></td><td></td><td><b>%2</b></td></tr>")
                        .arg(oc.point).arg(oc.occurrences);

            for(const OccDetail &detail : oc.details) {

                html += QString("<tr><td></td><td>%1</td><td>%2</td><td></td></tr>")
                .arg(detail.observationStr)
                    .arg(detail.startTime.isValid() ? detail.startTime.toString("dd-MM-yyyy HH:mm:ss") : "-");
            }
        }

        html += "</table>";
    }

    html += "<br/><hr/>";
    html += QString("<p style='text-align:center; font-size:9pt; color:#555;'>"
                    "Date: %1 &nbsp;&nbsp;&nbsp; Project: %2 &nbsp;&nbsp;&nbsp; Surveypod</p>")
                .arg(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss"))
                .arg("Loop closure report");

    return html;
}

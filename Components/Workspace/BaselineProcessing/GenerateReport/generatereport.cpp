#include "generatereport.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>
#include <QListWidgetItem>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QPainter>
#include <QPrinter>
#include <QStandardPaths>
#include <QBuffer>
#include <QSvgRenderer>
#include <QPainter>
#include <QBuffer>
#include <QImage>
#include <QTextDocument>

GenerateReport::GenerateReport(QObject *parent)
    : QObject{parent}
{
    processUtils = new ProcessUtils();
    utils = new Utils();
}
bool GenerateReport::naturalSortComparator(const QString &s1, const QString &s2)
{
    static const QRegularExpression re(QStringLiteral("([A-Z]+)(\\d+)"));
    auto m1 = re.match(s1);
    auto m2 = re.match(s2);

    if (m1.hasMatch() && m2.hasMatch()) {
        if (m1.captured(1) == m2.captured(1)) {
            return m1.captured(2).toInt() < m2.captured(2).toInt();
        }
        return m1.captured(1) < m2.captured(1);
    }
    return s1 < s2;
}

QString GenerateReport::svgToPngBase64(const QString &svgContent, int width, int height)
{
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QSvgRenderer renderer;
    renderer.load(svgContent.toUtf8());

    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return QString("<img src='data:image/png;base64,%1' style='width:100%; max-width:%2px;' />").arg(QString(byteArray.toBase64())).arg(width);
}

QString GenerateReport::generateResidualGraphSvg(const SatelliteData &sat, int width, int height)
{
    if (sat.points.size() < 2) return "";
    QVector<SatResidualPoint> sortedPoints = sat.points;
    std::sort(sortedPoints.begin(), sortedPoints.end(), [](const SatResidualPoint &a, const SatResidualPoint &b){
        return a.time < b.time;
    });
    int timeZoneOffsetSecs = 19800;

    qint64 startMS = sortedPoints.first().time.toMSecsSinceEpoch();
    qint64 endMS = sortedPoints.last().time.toMSecsSinceEpoch();
    double timeSpan = double(endMS - startMS);
    if (timeSpan < 1000.0) timeSpan = 1000.0;

    double marginL = 50;
    double marginR = 25;
    double marginT = 70;
    double marginB = 35;
    double graphW = width - (marginL + marginR);
    double graphH = height - (marginT + marginB);
    double zeroY = marginT + graphH / 2.0;

    double maxAbs = 0.0;
    for(const auto &p : sortedPoints) if (qAbs(p.residual) > maxAbs) maxAbs = qAbs(p.residual);
    double yLimit = qMax(0.02, maxAbs * 1.1);

    int step = 1;
    if (sortedPoints.size() > 2000) step = sortedPoints.size() / 2000;

    QString svg;
    QTextStream out(&svg);
    out << "<svg width='" << width << "' height='" << height << "' xmlns='http://www.w3.org/2000/svg' version='1.1'>";

    out << "<rect x='0' y='0' width='" << width << "' height='" << height << "' fill='white' />";
    out << "<rect x='" << marginL << "' y='" << marginT << "' width='" << graphW << "' height='" << graphH << "' fill='none' stroke='#000' stroke-width='1' />";

    out << "<line x1='10' y1='10' x2='" << (width - 10) << "' y2='10' stroke='#000' stroke-width='2' />";
    out << "<text x='" << (width / 2) << "' y='35' font-family='Arial' font-size='14' font-weight='bold' text-anchor='middle' fill='#000'>"<< sat.satId << "</text>";

    out << "<text x='" << (width/2) << "' y='55' font-family='Arial' font-size='11' text-anchor='middle' fill='#000'>"
        << "Mean: " << QString::number(sat.mean, 'f', 3) << "m  "
        << "  StdDev: " << QString::number(sat.stdDev, 'f', 3) << "m  "
        << "  Min: " << QString::number(sat.minVal, 'f', 3) << "m   "
        << "  Max: " << QString::number(sat.maxVal, 'f', 3) << "m"
        << "</text>";

    QVector<double> ticks = {yLimit, yLimit/2.0, 0.0, -yLimit/2.0, -yLimit};

    for (double val : ticks) {
        double yPos = zeroY - (val / yLimit) * (graphH / 2.0);
        QString stroke = (qFuzzyCompare(val, 0.0)) ? "#555" : "#ddd";
        QString dash = (qFuzzyCompare(val, 0.0)) ? "5,3" : "0";
        int width = (qFuzzyCompare(val, 0.0)) ? 1 : 1;

        out << "<line x1='" << marginL << "' y1='" << yPos << "' x2='" << (marginL + graphW) << "' y2='" << yPos
            << "' stroke='" << stroke << "' stroke-width='" << width << "' stroke-dasharray='" << dash << "' />";
        QString label = QString::number(val, 'f', 3);
        if (val > 0) label = "+" + label;
        out << "<text x='" << (marginL - 5) << "' y='" << (yPos + 4) << "' text-anchor='end' font-family='Arial' font-size='10'>" << label << "</text>";
    }
    out << "<polyline fill='none' stroke='#009477' stroke-width='0.6' points='";

    for (int i = 0; i < sortedPoints.size(); i += step) {
        const auto &p = sortedPoints.at(i);

        double tOffset = double(p.time.toMSecsSinceEpoch() - startMS);
        double x = marginL + (tOffset / timeSpan) * graphW;
        double normY = (p.residual / yLimit);
        double y = zeroY - (normY * (graphH / 2.0));

        x = qBound(marginL, x, marginL + graphW);
        y = qBound(marginT, y, marginT + graphH);

        out << QString::number(x, 'f', 1) << "," << QString::number(y, 'f', 1) << " ";
    }
    out << "' />";
    int numTicks = 5;
    for(int i = 0; i < numTicks; ++i) {
        double ratio = (double)i / (numTicks - 1);
        double tickX = marginL + (ratio * graphW);
        qint64 tickTimeMS = startMS + (qint64)(ratio * timeSpan);
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(tickTimeMS, QTimeZone::UTC).toLocalTime();
        QString timeLabel = dt.toString("HH:mm");

        QString anchor = "middle";
        if (i == 0) anchor = "start";
        if (i == numTicks - 1) anchor = "end";

        out << "<text x='" << tickX << "' y='" << (height - 5) << "' text-anchor='" << anchor << "' font-family='Arial' font-size='10'>" << timeLabel << "</text>";
        out << "<line x1='" << tickX << "' y1='" << (marginT + graphH) << "' x2='" << tickX << "' y2='" << (marginT + graphH + 5) << "' stroke='#000' stroke-width='1'/>";
    }
    out << "</svg>";
    return svgToPngBase64(svg, width, height);
}

QString GenerateReport::generateTrackingSummarySvg(const QMap<QString, SatelliteData> &map, QDateTime start, QDateTime end, int width, int height)
{
    if (map.isEmpty()) return "";

    QStringList sortedKeys = map.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end(), [](const QString &a, const QString &b){
        return GenerateReport::naturalSortComparator(a, b);
    });

    int marginX = 50;
    int marginY = 30;
    int graphW = width - (2 * marginX);
    int graphH = height - (2 * marginY);
    qint64 totalSeconds = start.secsTo(end);
    if (totalSeconds <= 0) totalSeconds = 1;

    QString svg;
    QTextStream out(&svg);
    out << "<svg width='" << width << "' height='" << height << "' xmlns='http://www.w3.org/2000/svg' version='1.1'>";
    out << "<rect x='" << marginX << "' y='" << marginY << "' width='" << graphW << "' height='" << graphH
        << "' fill='none' stroke='#000' stroke-width='1'/>";
    double rowHeight = (double)graphH / sortedKeys.size();

    for(int i=0; i<sortedKeys.size(); ++i) {
        QString satId = sortedKeys[i];
        double yPos = marginY + (i * rowHeight);
        double barH = qMin(rowHeight * 0.7, 15.0);
        double barY = yPos + (rowHeight - barH) / 2.0;

        out << "<text x='" << (marginX - 5) << "' y='" << (yPos + rowHeight/1.5)
            << "' font-family='Arial' font-size='10' text-anchor='end'>" << satId << "</text>";

        out << "<line x1='" << marginX << "' y1='" << yPos << "' x2='" << (width-marginX) << "' y2='" << yPos
            << "' stroke='#eee' stroke-width='1'/>";

        const SatelliteData &data = map[satId];
        if (!data.points.isEmpty()) {
            qint64 segStart = -1, lastT = -1;

            auto drawRect = [&](qint64 s, qint64 e) {
                double x = marginX + ((double)s / totalSeconds) * graphW;
                double w = ((double)(e - s) / totalSeconds) * graphW;
                out << "<rect x='" << x << "' y='" << barY << "' width='" << qMax(1.0, w) << "' height='" << barH << "' fill='#4CAF50' />";
            };

            for (const auto &p : data.points) {
                qint64 t = start.secsTo(p.time);
                if (segStart == -1) segStart = t;
                else if ((t - lastT) > 300) {
                    drawRect(segStart, lastT);
                    segStart = t;
                }
                lastT = t;
            }
            if (segStart != -1) drawRect(segStart, lastT);
        }
    }
    int timeY = marginY + graphH + 20;
    out << "<text x='" << marginX << "' y='" << timeY << "' font-size='10' text-anchor='start'>" << start.addSecs(5 * 3600 + 30 * 60).toString("HH:mm") << "</text>";
    out << "<text x='" << (width - marginX) << "' y='" << timeY << "' font-size='10' text-anchor='end'>"<< end.addSecs(5 * 3600 + 30 * 60).toString("HH:mm") << "</text>";
    out << "</svg>";

    return svgToPngBase64(svg, width, height);
}

void GenerateReport::getReport(const QMap<QString, PosData> &posData, const QString &projectFolder)
{
    try {
        if (posData.isEmpty()) {
            emit reportGenerationFailed("No processing data found!\nPlease process files.");
            return;
        }
        QJsonObject rinexData = utils->readRinexHeader();
        QString HTMLBody =  getHTMLcode(posData, rinexData);

        savePDF(HTMLBody, projectFolder);
        emit reportGenerationSuccessfull();
    }
    catch (...) {
        emit reportGenerationFailed(QStringLiteral("Unexpected unknown error while preparing report."));
    }
}

void GenerateReport::savePDF(QString HTMLBody, const QString &_outPath)
{
    try{
        QString outPath = _outPath;
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);

        if (outPath.isEmpty()) {
            emit reportGenerationFailed(QStringLiteral("Unable to determine an output folder for the report. \nPlease choose a save location."));
            return;
        }

        QDir outDir(outPath);
        if (!outDir.exists()) {
            if (!outDir.mkpath(".")) {
                emit reportGenerationFailed(QStringLiteral("Unable to create output directory: \n%1").arg(outPath));
                return;
            }
        }
        QFileInfo outDirInfo(outDir.absolutePath());
        if (!outDirInfo.isWritable()) {
            emit reportGenerationFailed(QStringLiteral("Cannot write to directory: \n%1. \nCheck permissions or available disk space.").arg(outPath));
            return;
        }

        outPath += "/baseline_report.pdf";

        printer.setOutputFileName(outPath);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setResolution(300);

        const QMarginsF marginsMM(10, 10, 10, 10);
        printer.setPageMargins(marginsMM, QPageLayout::Millimeter);

        QString headerHTML = R"(

        <table style="width:100%; border:none; margin-bottom:10px;">
          <tr>
            <td style="width:80px;">
              <img src=':/images/images/surveypod.png' style='max-width:50px; height:auto;' />
            </td>
            <td style="vertical-align:middle; font-weight:bold; font-size:25px;">
              <b>Survepod (Nibrus Technologies Pvt Ltd)</b>
            </td>
          </tr>
        </table>
        <hr style="border:none; border-top:1px solid #ccc; margin:4px 0;" />
        <br/>
    )";


        QString titleHTML = R"(
        <div style="text-align:left; font-size:14pt; font-weight:bold; margin-bottom:20pt;">
            Baseline Summary Report
            <br>
        </div>
    )";

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
            %3
        </body>
        </html>
    )").arg(headerHTML, titleHTML, HTMLBody);


        QTextDocument doc;
        doc.setHtml(fullHTML);
        doc.setPageSize(printer.pageRect(QPrinter::Point).size());

        doc.print(&printer);
    }
    catch (...) {
        emit reportGenerationFailed(QStringLiteral("Unexpected unknown error during PDF generation."));
    }
}


QString GenerateReport::getHTMLcode(QMap<QString, PosData> posData, QJsonObject &rinexData)
{
    auto fmtDouble = [&](double v, int prec = 8)->QString {
        if (!std::isfinite(v)) return QStringLiteral("-");
        return QString::number(v, 'f', prec);
    };

    auto fmtDateTime = [&](const QDateTime &dt)->QString {
        if (!dt.isValid()) return QStringLiteral("-");
        return dt.toLocalTime().toString("M/d/yyyy h:mm:ss AP");
    };

    auto fmtTimeOnly = [&](const QDateTime &dt)->QString {
        if (!dt.isValid()) return QStringLiteral("-");
        return dt.toLocalTime().time().toString("hh:mm:ss AP");
    };

    auto fmtDuration = [&](const QTime &t)->QString {
        if (!t.isValid()) return QStringLiteral("-");
        return t.toString("hh:mm:ss");
    };

    auto fmtIntervalSeconds = [&](double s)->QString {
        if (!std::isfinite(s)) return QStringLiteral("-");
        if (qFuzzyCompare(s, qRound64(s)) && qRound64(s) % 60 == 0) {
            qint64 mins = qRound64(s) / 60;
            return QString("%1 %2").arg(mins).arg(mins == 1 ? "Minute" : "Minutes");
        }
        if (qFuzzyCompare(s, qRound64(s))) return QString("%1 s").arg(qRound64(s));
        return QString("%1 s").arg(QString::number(s, 'f', 2));
    };

    auto dashIfEmpty = [&](const QString &s) -> QString {
        return s.trimmed().isEmpty() ? QStringLiteral("-") : s.trimmed();
    };

    auto jsonTriple = [&](const QJsonObject &obj, const QString &k1, const QString &k2, const QString &k3, int prec = 3) -> QString
    {
        if (obj.isEmpty())
            return QStringLiteral("-");
        return QString("(%1, %2, %3)").arg(obj.value(k1).toDouble(), 0, 'f', prec).arg(obj.value(k2).toDouble(), 0, 'f', prec).arg(obj.value(k3).toDouble(), 0, 'f', prec);
    };


    QStringList headers = {"Observation","From","To","Solution Type","H. Prec.\n(Meter)", "V. Prec.\n(Meter)", "Geodetic\nAz.", "Baseline\nlength\n(Meter)", "ΔHeight\n(ellipsoidal)\n(Meter)"};
    QList<QStringList> rows;
    for(auto it = posData.begin(); it != posData.end(); it++){
        QStringList row;
        QString from  = it.value().from;
        QString to  = it.value().to;

        row.push_back(from+" → "+to);
        row.push_back(from);
        row.push_back(to);

        QString tempSolType;
        if(it.value().pctFixed != 0.0)
            tempSolType += QString("Fixed %1%<br/>").arg(QString::number(it.value().pctFixed, 'f', 1));
        if(it.value().pctFloat != 0.0)
            tempSolType += QString("Float %1%<br/>").arg(QString::number(it.value().pctFloat, 'f', 1));
        if(it.value().pctSingle != 0.0)
            tempSolType += QString("Single %1%<br/>").arg(QString::number(it.value().pctSingle, 'f', 1));
        if(it.value().pctOther != 0.0)
            tempSolType += QString("Other %1%<br/>").arg(QString::number(it.value().pctOther, 'f', 1));
        if(tempSolType.isEmpty()) tempSolType = it.value().solutionTypeSummary.isEmpty() ? "-" : it.value().solutionTypeSummary.toHtmlEscaped();

        row.push_back(tempSolType);

        row.push_back(fmtDouble(it.value().horizontalPrecision, 3));
        row.push_back(fmtDouble(it.value().verticalPrecision, 3));
        row.push_back(std::isfinite(it.value().geodeticAzDeg) ? QString::number(it.value().geodeticAzDeg, 'f', 4) : QStringLiteral("-"));
        row.push_back(fmtDouble(it.value().ellipsoidDistance, 3));
        row.push_back(fmtDouble(it.value().deltaHeight, 3));
        rows.push_back(row);
    }

    QString html;
    html += "<html><head><meta charset='utf-8'/></head><body style='font-family:\"Times New Roman\",serif; font-size:10pt; color:#111;'>";
    html += "<div style='width:100%;'>";

    html += "<table style='width:100%; border-collapse:collapse; margin-bottom:10pt; page-break-inside:avoid'>";
    html += "<tr>";
    for (const QString &h : headers) {
        QString headerHtml = h.toHtmlEscaped().replace("\n", "<br/>");
        html += QString("<th style='border:1px solid #444; padding:4px; text-align:center; font-weight:bold; background:#eee; font-size:9pt;'>%1</th>").arg(headerHtml);
    }
    html += "</tr>";

    for (int r = 0; r < rows.size(); ++r) {
        if (r % 2 == 1) html += "<tr style='background:#fbfbfb; page-break-inside: avoid;'>";
        else html += "<tr style='page-break-inside: avoid;'>";
        for (int c = 0; c < rows[r].size(); ++c) {
            QString cellHtml;
            if (c == 3) {
                cellHtml = rows[r][c];
            } else {
                cellHtml = rows[r][c].toHtmlEscaped().replace("\n", "<br/>");
            }
            html += QString("<td style='border:1px solid #444; padding:4px; text-align:center; vertical-align:middle; font-size:8pt; page-break-inside: avoid !important'>%1</td>").arg(cellHtml);
        }
        html += "</tr>";
    }
    html += "</table>";
    html += "<br><br><br><br>";
    for(auto it = posData.begin(); it != posData.end(); it++){
        QString tempHeader = it.key();
        QString startTime = fmtTimeOnly(it.value().processingStart);
        QString stopTime  = fmtTimeOnly(it.value().processingStop);
        html += "<div style='page-break-before:always; page-break-inside:avoid; margin-bottom:40px; width:100%;'>";
        html += QString("<p style='text-align:center; font-size:11pt;'><b style='font-size:13pt;'>%1</b> (%2 - %3)</p>").arg(it.value().from + " → " + it.value().to).arg(startTime).arg(stopTime);
        html += "<hr style='border:none; border-top:1px solid #ccc; margin:0px 0 2px 0;'/>";

        //info
        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:40pt;'>";
        html += QString("<tr><td style='padding:6px; text-align:left; vertical-align:top;  width:35%; font-weight:bold;'>Baseline observation:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(tempHeader.toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Processed:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(fmtDateTime(it.value().processedTime).toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Solution type:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg((it.value().solutionTypeSummary.isEmpty() ? "-" : it.value().solutionTypeSummary).toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Frequency used:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(it.value().frequency.isEmpty() ? QStringLiteral("-") : it.value().frequency.toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Elevation mask:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(it.value().elevMask.isEmpty() ? QStringLiteral("-") : it.value().elevMask.toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Horizontal precision:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1 m</td></tr>").arg(fmtDouble(it.value().horizontalPrecision, 3));
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Vertical precision:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1 m</td></tr>").arg(fmtDouble(it.value().verticalPrecision, 3));
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>RMS:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1 m</td></tr>").arg(fmtDouble(it.value().RMS, 4));
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Ephemeris used:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(it.value().ephemeris.isEmpty() ? QStringLiteral("-") : it.value().ephemeris.toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Processing start time:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(fmtDateTime(it.value().processingStart).toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Processing stop time:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(fmtDateTime(it.value().processingStop).toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Processing duration:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(fmtDuration(it.value().processingDuration).toHtmlEscaped());
        html += QString("<tr><td style='padding:6px; vertical-align:top; text-align:left;  font-weight:bold;'>Processing interval:</td><td style='padding:6px; vertical-align:top; text-align:left; '>%1</td></tr>").arg(fmtIntervalSeconds(it.value().processingIntervalSeconds).toHtmlEscaped());
        html += "</table>";
        html += "</div>";
        html += "<br/>";

        html += "<div style='width:100%; page-break-inside:avoid;  margin-bottom:40px;'>";
        html += "<p style='text-align:left; font-size:12pt; font-weight:bold;'>Vector Components</p>";

        //------From table
        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:20px; border:1px solid #ccc;'>";
        html += QString("<tr style='background-color:#f0f0f0;'><th colspan='6'>From : %1</th></tr>") .arg(it.value().from);

        html += "<tr>"
                "<th colspan='2'>Grid (UTM)</th>"
                "<th colspan='2'>Geodetic</th>"
                "<th colspan='2'>ECEF</th>"
                "</tr>";
        html += QString(
                    "<tr>"
                    "<td >Easting</td><td>%1 m</td>"
                    "<td>Latitude</td><td>%4°</td>"
                    "<td>X</td><td>%7 m</td>"
                    "</tr>"
                    "<tr>"
                    "<td>Northing</td><td>%2 m</td>"
                    "<td>Longitude</td><td>%5°</td>"
                    "<td>Y</td><td>%8 m</td>"
                    "</tr>"
                    "<tr>"
                    "<td>Orthometric H<br>(EGM08)</td><td>%3 m</td>"
                    "<td>Ellipsoidal h</td><td>%6 m</td>"
                    "<td>Z</td><td>%9 m</td>"
                    "</tr>")
                    .arg(fmtDouble(it.value().basePosition.easting, 3))
                    .arg(fmtDouble(it.value().basePosition.northing, 3))
                    .arg(fmtDouble(it.value().basePosition.orthometric, 3))
                    .arg(fmtDouble(it.value().basePosition.geodetic.lat, 9))
                    .arg(fmtDouble(it.value().basePosition.geodetic.lon, 9))
                    .arg(fmtDouble(it.value().basePosition.geodetic.h, 3))
                    .arg(fmtDouble(it.value().basePosition.ecef.X, 3))
                    .arg(fmtDouble(it.value().basePosition.ecef.Y, 3))
                    .arg(fmtDouble(it.value().basePosition.ecef.Z, 3));

        html += "</table>";

        // "To" table
        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:20px; border:1px solid #ccc;'>";
        html += QString("<tr style='background-color:#f0f0f0;'><th colspan='6'>To : %1</th></tr>") .arg(it.value().to);

        html += "<tr>"
                "<th colspan='2'>Grid (UTM)</th>"
                "<th colspan='2'>Geodetic</th>"
                "<th colspan='2'>ECEF</th>"
                "</tr>";
        html += QString(
                    "<tr>"
                    "<td >Easting</td><td>%1 m</td>"
                    "<td>Latitude</td><td>%4°</td>"
                    "<td>X</td><td>%7 m</td>"
                    "</tr>"
                    "<tr>"
                    "<td>Northing</td><td>%2 m</td>"
                    "<td>Longitude</td><td>%5°</td>"
                    "<td>Y</td><td>%8 m</td>"
                    "</tr>"
                    "<tr>"
                    "<td>Orthometric H<br>(EGM08)</td><td>%3 m</td>"
                    "<td>Ellipsoidal h</td><td>%6 m</td>"
                    "<td>Z</td><td>%9 m</td>"
                    "</tr>")
                    .arg(fmtDouble(it.value().roverPosition.easting, 3))
                    .arg(fmtDouble(it.value().roverPosition.northing, 3))
                    .arg(fmtDouble(it.value().roverPosition.orthometric, 3))
                    .arg(fmtDouble(it.value().roverPosition.geodetic.lat, 9))
                    .arg(fmtDouble(it.value().roverPosition.geodetic.lon, 9))
                    .arg(fmtDouble(it.value().roverPosition.geodetic.h, 3))
                    .arg(fmtDouble(it.value().roverPosition.ecef.X, 3))
                    .arg(fmtDouble(it.value().roverPosition.ecef.Y, 3))
                    .arg(fmtDouble(it.value().roverPosition.ecef.Z, 3));

        html += "</table>";

        html+="<br>";

        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:20px; border:1px solid #ccc;'>";
        html += "<tr style='background-color:#f0f0f0;'><th colspan='8'>Vector</th></tr>";

        html += "<tr>"
                "<th colspan='2'>Grid (UTM)</th>"
                "<th colspan='2'>Local (ENU)</th>"
                "<th colspan='2'>Geodetic</th>"
                "<th colspan='2'>Global (ECEF)</th>"
                "</tr>";

        html += QString(
                    "<tr>"
                    "<td>ΔEasting</td><td>%1 m</td>"
                    "<td>ΔE</td><td>%2 m</td>"
                    "<td>Azimuth</td><td>%3°</td>"
                    "<td>ΔX</td><td>%4 m</td>"
                    "</tr>"

                    "<tr>"
                    "<td>ΔNorthing</td><td>%5 m</td>"
                    "<td>ΔN</td><td>%6 m</td>"
                    "<td>Baseline</td><td>%7 m</td>"
                    "<td>ΔY</td><td>%8 m</td>"
                    "</tr>"

                    "<tr>"
                    "<td>ΔH (ortho)</td><td>%9 m</td>"
                    "<td>ΔU</td><td>%10 m</td>"
                    "<td>Δh (ellip)</td><td>%11 m</td>"
                    "<td>ΔZ</td><td>%12 m</td>"
                    "</tr>"
                    )
                    .arg(fmtDouble(it.value().roverPosition.easting  - it.value().basePosition.easting, 3))
                    .arg(fmtDouble(it.value().dE, 3))
                    .arg(fmtDouble(it.value().geodeticAzDeg, 4))
                    .arg(fmtDouble(it.value().roverPosition.ecef.X - it.value().basePosition.ecef.X, 3))

                    .arg(fmtDouble(it.value().roverPosition.northing - it.value().basePosition.northing, 3))
                    .arg(fmtDouble(it.value().dN, 3))
                    .arg(fmtDouble(it.value().ellipsoidDistance, 3))
                    .arg(fmtDouble(it.value().roverPosition.ecef.Y - it.value().basePosition.ecef.Y, 3))

                    .arg(fmtDouble(it.value().roverPosition.orthometric - it.value().basePosition.orthometric, 3))
                    .arg(fmtDouble(it.value().dU, 3))
                    .arg(fmtDouble(it.value().deltaHeight, 3))
                    .arg(fmtDouble(it.value().roverPosition.ecef.Z - it.value().basePosition.ecef.Z, 3));

        html += "</table>";

        html += "</div>";
        html += "<br/>";


        html += "<div style='width:100%; page-break-inside:avoid;  margin-bottom:40px;'>";
        html += "<p style='text-align:left; font-size:12pt; font-weight:bold;'>Standard Errors</p>";

        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:20px; border:1px solid #ccc;'>";
        html += "<tr style='background-color:#f0f0f0;'><th colspan='8'>Standard Errors (σ)</th></tr>";

        html += "<tr>"
                "<th colspan='2'>Grid (UTM)</th>"
                "<th colspan='2'>Local (ENU)</th>"
                "<th colspan='2'>Geodetic</th>"
                "<th colspan='2'>Global (ECEF)</th>"
                "</tr>";

        html += QString(
                    "<tr>"
                    "<td>σ ΔEasting</td><td>%1 m</td>"
                    "<td>σ ΔE</td><td>%2 m</td>"
                    "<td>σ Azimuth</td><td>%3</td>"
                    "<td>σ ΔX</td><td>%4 m</td>"
                    "</tr>"

                    "<tr>"
                    "<td>σ ΔNorthing</td><td>%5 m</td>"
                    "<td>σ ΔN</td><td>%6 m</td>"
                    "<td>σ Distance</td><td>%7 m</td>"
                    "<td>σ ΔY</td><td>%8 m</td>"
                    "</tr>"

                    "<tr>"
                    "<td>—</td><td>—</td>"
                    "<td>σ ΔU</td><td>%9 m</td>"
                    "<td>—</td><td>—</td>"
                    "<td>σ ΔZ</td><td>%10 m</td>"
                    "</tr>"
                    )
                    .arg(fmtDouble(it.value().sigma_dE, 3))
                    .arg(fmtDouble(it.value().sigma_dE, 3))
                    .arg(it.value().sigma_az_DMS)
                    .arg(fmtDouble(it.value().sigma_dX, 3))

                    .arg(fmtDouble(it.value().sigma_dN, 3))
                    .arg(fmtDouble(it.value().sigma_dN, 3))
                    .arg(fmtDouble(it.value().sigma_dist, 3))
                    .arg(fmtDouble(it.value().sigma_dY, 3))

                    .arg(fmtDouble(it.value().sigma_dU, 3))
                    .arg(fmtDouble(it.value().sigma_dZ, 3));

        html += "</table>";

        html += "</div>";
        html += "<br/><br/>";

        html += "<div style='width:100%; page-break-inside:avoid;  margin-bottom:40px;'>";
        html += "<p style='text-align:left; font-size:12pt; font-weight:bold;'>Covariance Matrix (ECEF, m²)</p>";

        //------Convariance metrix
        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:20px; border:1px solid #ccc;'>";
        html +=
            "<tr>"
            "<th></th>"
            "<th>X</th>"
            "<th>Y</th>"
            "<th>Z</th>"
            "</tr>"

            "<tr>"
            "<th> X </th>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[0][0],10) + "</td>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[0][1],10) + "</td>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[0][2],10) + "</td>"
            "</tr>"

            "<tr>"
            "<th> Y </th>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[1][0],10) + "</td>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[1][1],10) + "</td>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[1][2],10) + "</td>"
            "</tr>"

            "<tr>"
            "<th> Z </th>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[2][0],10) + "</td>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[2][1],10) + "</td>"
            "<td>" + fmtDouble(it.value().cov_dXYZ[2][2],10) + "</td>"
            "</tr>";

        html += "</table>";

        html += "</div>";
        html += "<br/><br/>";

        QJsonObject baseData =  rinexData.value(QFileInfo(it.value().basePath).fileName()).toObject();
        QJsonObject roverData =  rinexData.value(QFileInfo(it.value().roverPath).fileName()).toObject();

        if(baseData.isEmpty() || roverData.isEmpty()){
            continue;
        }

        QString bMarker = dashIfEmpty(baseData.value("marker_name").toString());
        QString rMarker = dashIfEmpty(roverData.value("marker_name").toString());

        QString bMarkerNumber = dashIfEmpty(baseData.value("marker_number").toString());
        QString rMarkerNumber = dashIfEmpty(roverData.value("marker_number").toString());

        QString bMarkerType = dashIfEmpty(baseData.value("marker_type").toString());
        QString rMarkerType = dashIfEmpty(roverData.value("marker_type").toString());

        QString bObserver = dashIfEmpty(baseData.value("observer").toString());
        QString rObserver = dashIfEmpty(roverData.value("observer").toString());

        QString bReceiver = dashIfEmpty(baseData.value("receiver_type").toString());
        QString rReceiver = dashIfEmpty(roverData.value("receiver_type").toString());

        QString bAntenna = dashIfEmpty(baseData.value("antenna_type").toString());
        QString rAntenna = dashIfEmpty(roverData.value("antenna_type").toString());

        QString bPoleStr = jsonTriple(baseData.value("poleHEN").toObject(),"H", "E", "N", 3);
        QString rPoleStr = jsonTriple(roverData.value("poleHEN").toObject(),"H", "E", "N", 3);

        QString bPosStr = jsonTriple(baseData.value("posxyz").toObject(),"X", "Y", "Z", 3);
        QString rPosStr = jsonTriple(roverData.value("posxyz").toObject(),"X", "Y", "Z", 3);


        html += "<br/><br/><br/><br/><br/>";

        html += "<div style='width:100%; page-break-inside:avoid;  margin-bottom:40px;'>";
        html += "<br/><br/>";
        html += "<p style='text-align:left; font-size:12pt; font-weight:bold;'>Occupations</p>";
        //Occupations
        html += "<table style='width:100%; border-collapse:collapse; margin-bottom:20px; border:1px solid #ccc;'>";
        html += "<tr>"
                "<th ></th>"
                "<th >From</th>"
                "<th >To</th>"
                "</tr>";

        html += QString(
                    "<tr>"
                    "<td >Point Id</td>"
                    "<td>%1</td>"
                    "<td>%2</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Data file</td>"
                    "<td>%3</td>"
                    "<td>%4</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Marker name</td>"
                    "<td>%5</td>"
                    "<td>%6</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Marker Number</td>"
                    "<td>%7</td>"
                    "<td>%8</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Marker Type</td>"
                    "<td>%9</td>"
                    "<td>%10</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Observer</td>"
                    "<td>%11</td>"
                    "<td>%12</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Receiver</td>"
                    "<td>%13</td>"
                    "<td>%14</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Antenna name</td>"
                    "<td>%15</td>"
                    "<td>%16</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Approx. Pos <br>(X, Y, Z)</td>"
                    "<td>%17</td>"
                    "<td>%18</td>"
                    "</tr>"

                    "<tr>"
                    "<td>Pole Pos <br>(H, E, N)</td>"
                    "<td>%19</td>"
                    "<td>%20</td>"
                    "</tr>"
                    )
                    .arg(it.value().from)
                    .arg(it.value().to)
                    .arg(it.value().basePath)
                    .arg(it.value().roverPath)
                    .arg(bMarker)
                    .arg(rMarker)
                    .arg(bMarkerNumber)
                    .arg(rMarkerNumber)
                    .arg(bMarkerType)
                    .arg(rMarkerType)
                    .arg(bObserver)
                    .arg(rObserver)
                    .arg(bReceiver)
                    .arg(rReceiver)
                    .arg(bAntenna)
                    .arg(rAntenna)
                    .arg(bPosStr)
                    .arg(rPosStr)
                    .arg(bPoleStr)
                    .arg(rPoleStr);

        html += "</table>";
        html+="<br>";
        if (!it.value().SatStatsMap.isEmpty()) {
            html += "<div style='page-break-before:always; page-break-inside:avoid; margin-bottom:20px;'>";
            html += "<p style='text-align:left; font-size:12pt; font-weight:bold;'>Tracking Summary</p>";
            int trackH = (it.value().SatStatsMap.size() * 25) + 60;
            html += generateTrackingSummarySvg(it.value().SatStatsMap, it.value().processingStart, it.value().processingStop, 550, trackH);
            html += "</div>";
        }

        if (!it.value().SatStatsMap.isEmpty()) {
            html += "<div style='page-break-before:always;'>";
            html += "<p style='text-align:left; font-size:12pt; font-weight:bold;'>Residuals</p>";

            for(auto satIt = it.value().SatStatsMap.begin(); satIt != it.value().SatStatsMap.end(); ++satIt) {
                html += "<div style='display:inline-block; page-break-inside:avoid; break-inside:avoid; border:1px solid #eee; padding:5px;'>";
                html += "<br><br><br>";
                html += generateResidualGraphSvg(satIt.value(), 550, 270);
                html += "</div>";
            }
            html += "</div>";
        }

        html += "<br/><br/>";

        html += "</div>";
        html += "<br/><br/>";
    }

    html += "</div></body></html>";

    return html;
}

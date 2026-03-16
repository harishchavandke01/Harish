#include "loopclosureutils.h"
#include <QVector3D>
#include <cmath>
#include <limits>
#include <QStack>
#include <QFileInfo>
#include <QSet>

LoopClosureUtils::LoopClosureUtils(QObject *parent): QObject{parent}
{}

void LoopClosureUtils::computeLoopClosures(ProjectContext *ctx)
{
    ctx->loopReport.loops.clear();

    QVector<ProjectBaseline> cleanBaselines;
    for (int i = 0; i < ctx->baselines.size(); ++i) {
        ProjectBaseline b = ctx->baselines[i];

        b.from = b.fromStationId;
        b.to = b.toStationId;

        if (b.baselineId.isEmpty() || b.baselineId.contains("/") || b.baselineId.contains("\\")) {
            b.baselineId = QString("B%1").arg(i + 1);
        }

        QString solLower = b.solutionType.toLower();
        if (solLower.contains("fixed")) b.solutionType = "Fixed";
        else if (solLower.contains("float")) b.solutionType = "Float";
        else if (!solLower.isEmpty()) b.solutionType = "Single";
        else b.solutionType = "Fixed";
        cleanBaselines.append(b);
    }

    detectLoops(cleanBaselines, ctx->loopReport.loops);
    for(auto &loop : ctx->loopReport.loops){
        computeLoopMetrics(ctx, cleanBaselines, loop);
    }
    computeSummary(ctx->loopReport);
    std::sort(ctx->loopReport.loops.begin(), ctx->loopReport.loops.end(), [](const LoopInfo &a, const LoopInfo &b){
        return a.ppm > b.ppm;
    });
}

void LoopClosureUtils::computeLoopMetrics(ProjectContext *ctx, const QVector<ProjectBaseline> &baselines, LoopInfo &loop)
{
    QVector3D mis(0, 0, 0);
    double length = 0;
    loop.vectors.clear();

    QMap<QString, QVector<ProjectBaseline>> nodeEdges;
    for (QString &id : loop.baselineIds) {
        for (const auto &b : baselines) {
            if (b.baselineId == id) {
                nodeEdges[b.from].append(b);
                nodeEdges[b.to].append(b);
                break;
            }
        }
    }

    if (nodeEdges.isEmpty()) return;
    QString startNode = nodeEdges.keys().first();
    QString currentNode = startNode;
    QSet<QString> usedBaselines;
    QStringList loopNodes;
    loopNodes.append(startNode);

    while (true) {
        bool moved = false;
        for (ProjectBaseline &b : nodeEdges[currentNode]) {
            if (usedBaselines.contains(b.baselineId)) continue;
            usedBaselines.insert(b.baselineId);
            double dir = 1.0;
            QString nextNode;
            if (b.from == currentNode) {
                dir = 1.0;
                nextNode = b.to;
            }
            else {
                dir = -1.0;
                nextNode = b.from;
            }

            if (nextNode != startNode) {
                loopNodes.append(nextNode);
            }
            mis += QVector3D(b.dX * dir, b.dY * dir, b.dZ * dir);
            length += b.length;

            LoopInfo::VectorInfo vInfo;
            vInfo.baselineId = b.baselineId;
            vInfo.from = b.from;
            vInfo.to = b.to;
            vInfo.length = b.length;
            vInfo.startTime = b.startTime.addSecs(5*3600 + 30 *60);
            vInfo.solutionType = b.solutionType;
            loop.vectors.append(vInfo);
            currentNode = nextNode;
            moved = true;
            break;
        }

        if (!moved || currentNode == startNode) break;
    }
    loop.loopId = loopNodes.join("-");
    qDebug()<<"loop.loopId = "<<loop.loopId;

    loop.misclosureXYZ = mis;
    qDebug()<<"\nloop misclosure = "<<loop.misclosureXYZ;
    loop.error3D = mis.length();
    loop.length = length;
    loop.ppm = (length > 0) ? (loop.error3D / length) * 1e6 : 0.0;
    loop.passed = (loop.ppm <= 1.0);

    qDebug()<<"\nCurrent Node : "<<currentNode<<" loop.ppm = "<<loop.ppm<<" loop.error3D = "<<loop.error3D<<" loop.length = "<<loop.length;
    double dX = mis.x();
    double dY = mis.y();
    double dZ = mis.z();

    double lat = 0.0;
    double lon = 0.0;

    for (auto it = ctx->stations.begin(); it != ctx->stations.end(); ++it) {
        if (it.value().stationId == startNode) {
            lat = it.value().geo.lat * M_PI / 180.0;
            lon = it.value().geo.lon * M_PI / 180.0;
            break;
        }
    }

    double dE = -std::sin(lon)*dX + std::cos(lon)*dY;
    double dN = -std::sin(lat)*std::cos(lon)*dX - std::sin(lat)*std::sin(lon)*dY + std::cos(lat)*dZ;
    double dU =  std::cos(lat)*std::cos(lon)*dX + std::cos(lat)*std::sin(lon)*dY + std::sin(lat)*dZ;

    loop.horizError = std::sqrt(dE*dE + dN*dN);
    loop.vertError = dU;
    qDebug()<<"std error using h and v = "<<std::sqrt(loop.horizError*loop.horizError + loop.vertError *loop.vertError);
}

void LoopClosureUtils::computeSummary(LoopReport &report)
{
    report.numLoops = report.loops.size();
    report.numPassed = 0;
    report.numFailed = 0;

    if (report.numLoops == 0) return;

    double sumLength = 0, sum3D = 0, sumHoriz = 0, sumVert = 0, sumPPM = 0;
    report.best3D = std::numeric_limits<double>::max();
    report.bestHoriz = std::numeric_limits<double>::max();
    report.bestVert  = std::numeric_limits<double>::max();
    report.bestPPM = std::numeric_limits<double>::max();

    report.worst3D = 0;
    report.worstHoriz = std::numeric_limits<double>::min();
    report.worstVert = std::numeric_limits<double>::min();
    report.worstPPM = 0;

    for(auto &l : report.loops) {
        if(l.passed) report.numPassed++;
        else report.numFailed++;

        report.best3D = std::min(report.best3D, l.error3D);
        report.bestHoriz = std::min(report.bestHoriz, l.horizError);
        report.bestVert = std::min(report.bestVert, l.vertError);
        report.bestPPM = std::min(report.bestPPM, l.ppm);

        report.worst3D = std::max(report.worst3D, l.error3D);
        report.worstHoriz = std::max(report.worstHoriz, l.horizError);
        report.worstVert = std::max(report.worstVert, l.vertError);
        report.worstPPM = std::max(report.worstPPM, l.ppm);

        sumLength += l.length;
        sum3D += l.error3D;
        sumHoriz += l.horizError;
        sumVert += std::abs(l.vertError);
        sumPPM += l.ppm;
    }

    report.avgLength = sumLength / report.numLoops;
    report.avg3D = sum3D / report.numLoops;
    report.avgHoriz = sumHoriz / report.numLoops;
    report.avgVert = sumVert / report.numLoops;
    report.avgPPM = sumPPM / report.numLoops;

    double sqLength = 0, sq3D = 0, sqHoriz = 0, sqVert = 0, sqPPM = 0;
    for(auto &l : report.loops) {
        sqLength += std::pow(l.length - report.avgLength, 2);
        sqPPM += std::pow(l.ppm - report.avgPPM, 2);

        sq3D += std::pow(l.error3D, 2);
        sqHoriz += std::pow(l.horizError, 2);
        sqVert += std::pow(l.vertError, 2);
    }
    int N = report.numLoops;

    report.stdLength = std::sqrt(sqLength / N);
    report.std3D = std::sqrt(sq3D / N);
    report.stdHoriz = std::sqrt(sqHoriz / N);
    report.stdVert = std::sqrt(sqVert / N);
    report.stdPPM = std::sqrt(sqPPM / N);
}

void LoopClosureUtils::detectLoops(const QVector<ProjectBaseline> &baselines, QVector<LoopInfo> &loops)
{
    QSet<QString> foundLoops;
    QMap<QString, QVector<int>> adj;
    for (int i = 0; i < baselines.size(); i++) {
        adj[baselines[i].from].append(i);
        adj[baselines[i].to].append(i);
    }

    QStringList stations = adj.keys();

    for (QString &stationA : stations) {
        const QVector<int> &edgesA = adj[stationA];
        for (int j = 0; j < edgesA.size(); ++j) {
            for (int k = j + 1; k < edgesA.size(); ++k) {
                int edge1Idx = edgesA[j];
                int edge2Idx = edgesA[k];

                const ProjectBaseline &b1 = baselines[edge1Idx];
                const ProjectBaseline &b2 = baselines[edge2Idx];

                QString stationB = (b1.from == stationA) ? b1.to : b1.from;
                QString stationC = (b2.from == stationA) ? b2.to : b2.from;
                if (stationB == stationC) continue;

                const QVector<int> &edgesB = adj[stationB];
                for (int edge3Idx : edgesB) {
                    if (edge3Idx == edge1Idx || edge3Idx == edge2Idx) continue;

                    const ProjectBaseline &b3 = baselines[edge3Idx];
                    QString otherNode = (b3.from == stationB) ? b3.to : b3.from;
                    if (otherNode == stationC) {

                        QStringList ids = {b1.baselineId, b2.baselineId, b3.baselineId};
                        ids.sort();
                        QString signature = ids.join("|");

                        if (!foundLoops.contains(signature)) {
                            foundLoops.insert(signature);

                            LoopInfo loop;
                            loop.baselineIds << b1.baselineId << b2.baselineId << b3.baselineId;
                            loops.append(loop);
                        }
                    }
                }
            }
        }
    }
}


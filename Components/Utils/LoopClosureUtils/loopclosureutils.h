#ifndef LOOPCLOSUREUTILS_H
#define LOOPCLOSUREUTILS_H

#include <QObject>
#include "../../Context/projectcontext.h"

class LoopClosureUtils : public QObject
{
    Q_OBJECT
public:
    static void  computeLoopClosures(ProjectContext *ctx);
    explicit LoopClosureUtils(QObject *parent = nullptr);

private:
    static void detectLoops(const QVector<ProjectBaseline> &baselines, QVector<LoopInfo> &loops);
    // static void computeLoopMetrics(const QVector<ProjectBaseline> &baselines,LoopInfo &loop);
    static void computeLoopMetrics(ProjectContext *ctx, const QVector<ProjectBaseline> &baselines, LoopInfo &loop);
    static bool findPath( const QString &start, const QString &end, const QVector<ProjectBaseline> &baselines, const QSet<int> &treeEdges, QVector<int> &path);
    static void computeSummary( LoopReport &report);
signals:
};

#endif // LOOPCLOSUREUTILS_H

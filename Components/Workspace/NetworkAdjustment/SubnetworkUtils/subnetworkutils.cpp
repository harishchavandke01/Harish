#include "subnetworkutils.h"

SubnetworkUtils::SubnetworkUtils(QObject *parent): QObject{parent}
{}

QString SubnetworkUtils::find(QMap<QString, QString> &parent, const QString &x)
{
    if (parent[x] != x)
        parent[x] = find(parent, parent[x]);
    return parent[x];
}

int SubnetworkUtils::detectAndStore(ProjectContext *ctx)
{
    ctx->subnetworks.clear();
    if (!ctx || ctx->stations.isEmpty()) return 0;

    QMap<QString, QString> parent;
    for (auto it = ctx->stations.constBegin(); it != ctx->stations.constEnd(); ++it) {
        QString sid = it.value().stationId;
        if (!parent.contains(sid))
            parent[sid] = sid;
    }

    for (ProjectBaseline &bl : ctx->baselines) {
        if (!ctx->stations.contains(bl.from) || !ctx->stations.contains(bl.to))
            continue;

        QString fromId = ctx->stations[bl.from].stationId;
        QString toId   = ctx->stations[bl.to].stationId;

        QString ra = find(parent, fromId);
        QString rb = find(parent, toId);
        if (ra != rb) parent[ra] = rb;
    }
    QMap<QString, QSet<QString>> groups;
    for (auto it = ctx->stations.constBegin(); it != ctx->stations.constEnd(); ++it) {
        QString sid = it.value().stationId;
        QString root = find(parent, sid);
        groups[root].insert(it.key());
    }

    int idx = 1;
    QStringList roots = groups.keys();
    roots.sort();

    for (QString &root : roots) {
        const QSet<QString> &uids = groups[root];
        SubnetworkInfo info;
        info.index = idx;
        info.stationUIDs = uids;

        for (int i = 0; i < ctx->baselines.size(); ++i) {
            const ProjectBaseline &bl = ctx->baselines[i];
            if (uids.contains(bl.from) && uids.contains(bl.to))
                info.baselineIndices.append(i);
        }

        QSet<QString> fixedStationIds;
        for (const QString &uid : uids) {
            const auto &st = ctx->stations[uid];
            if (st.isFixed)
                fixedStationIds.insert(st.stationId);
        }

        info.isConstrained = !fixedStationIds.isEmpty();
        info.hasResult = ctx->adjustmentResult.hasSubnetworkResult(idx);
        ctx->subnetworks.append(info);
        ++idx;
    }
    return ctx->subnetworks.size();
}

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

    // ── Step 1: Initialise union-find — every station is its own root ─────
    QMap<QString, QString> parent;
    for (auto it = ctx->stations.constBegin();
         it != ctx->stations.constEnd(); ++it)
        parent[it.key()] = it.key();

    // ── Step 2: Union stations connected by baselines ─────────────────────
    for (const ProjectBaseline &bl : ctx->baselines) {
        if (!parent.contains(bl.from) || !parent.contains(bl.to))
            continue;
        QString ra = find(parent, bl.from);
        QString rb = find(parent, bl.to);
        if (ra != rb) parent[ra] = rb;
    }

    // ── Step 3: Group station uids by root ────────────────────────────────
    QMap<QString, QSet<QString>> groups;
    for (auto it = ctx->stations.constBegin();
         it != ctx->stations.constEnd(); ++it)
        groups[find(parent, it.key())].insert(it.key());

    // ── Step 4: Build SubnetworkInfo for each group ───────────────────────
    int idx = 1;
    // Sort group roots for deterministic ordering
    QStringList roots = groups.keys();
    roots.sort();

    for (const QString &root : roots) {
        const QSet<QString> &uids = groups[root];

        SubnetworkInfo info;
        info.index       = idx;
        info.stationUIDs = uids;

        // Find baselines belonging to this subnetwork
        for (int i = 0; i < ctx->baselines.size(); ++i) {
            const ProjectBaseline &bl = ctx->baselines[i];
            if (uids.contains(bl.from) && uids.contains(bl.to))
                info.baselineIndices.append(i);
        }

        // Identify fixed points
        for (const QString &uid : uids) {
            if (ctx->stations.contains(uid) && ctx->stations[uid].isFixed)
                info.fixedUIDs.append(uid);
        }
        info.isConstrained = !info.fixedUIDs.isEmpty();

        // Preserve hasResult flag from a previous adjustment run
        info.hasResult = ctx->adjustmentResult.hasSubnetworkResult(idx);

        ctx->subnetworks.append(info);
        ++idx;
    }

    return ctx->subnetworks.size();
}

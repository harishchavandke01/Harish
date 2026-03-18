#ifndef SUBNETWORKUTILS_H
#define SUBNETWORKUTILS_H

#include <QObject>
#include "../../../Context/projectcontext.h"


// Detects connected components of the baseline graph and populates
// projectContext->subnetworks. Called every time AdjustNetworkDialog opens.
class SubnetworkUtils : public QObject
{
    Q_OBJECT
public:
    explicit SubnetworkUtils(QObject *parent = nullptr);
    static int detectAndStore(ProjectContext *ctx);

private:
    // Union-Find helper with path compression
    static QString find(QMap<QString, QString> &parent, const QString &x);

signals:
};

#endif // SUBNETWORKUTILS_H

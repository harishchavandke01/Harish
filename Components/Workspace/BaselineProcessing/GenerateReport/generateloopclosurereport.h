#ifndef GENERATELOOPCLOSUREREPORT_H
#define GENERATELOOPCLOSUREREPORT_H

#include <QWidget>
#include "../../../Context/projectcontext.h"

class GenerateLoopClosureReport : public QWidget
{
    Q_OBJECT
public:
    explicit GenerateLoopClosureReport(QWidget *parent = nullptr);
    static bool savePDF( const ProjectContext *ctx, const QString &filePath );

private:
    static QString buildHTML(const LoopReport &report);

signals:
};

#endif // GENERATELOOPCLOSUREREPORT_H

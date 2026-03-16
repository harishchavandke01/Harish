// #include "chartview.h"
// #include <QtCharts/QChart>
// #include <QtCharts/QValueAxis>
// #include <QtCharts/QLegendMarker>
// #include <QToolTip>
// #include <cmath>
// #include <QSet>

// static constexpr double ARROW_REL_SCALE = 0.028;
// static constexpr double ARROW_MIN_LENGTH  = 0.12;
// static constexpr double ARROW_WING_FACTOR = 0.30;

// static QPointF offsetFromPoint(
//     const QPointF &from,
//     const QPointF &to,
//     double offset)
// {
//     double vx = to.x() - from.x();
//     double vy = to.y() - from.y();
//     double L  = std::hypot(vx, vy);
//     if (L < 1e-9) return from;

//     double ux = vx / L;
//     double uy = vy / L;

//     return QPointF(
//         from.x() + ux * offset,
//         from.y() + uy * offset
//         );
// }

// static void addArrowHead(QChart *chart,
//                          const QPointF &from,
//                          const QPointF &to,
//                          QVector<QLineSeries*> &storage)
// {
//     double vx = to.x() - from.x();
//     double vy = to.y() - from.y();
//     double L  = std::hypot(vx, vy);
//     if (L < 1e-6) return;

//     double ux = vx / L;
//     double uy = vy / L;

//     double arrowLen = std::max(ARROW_MIN_LENGTH, L * ARROW_REL_SCALE);
//     arrowLen = std::min(arrowLen, L * 0.18);

//     double wing = arrowLen * ARROW_WING_FACTOR;

//     QPointF ortho(-uy, ux);

//     QPointF base(
//         to.x() - ux * arrowLen,
//         to.y() - uy * arrowLen
//         );

//     QPointF p1(
//         base.x() + ortho.x() * wing,
//         base.y() + ortho.y() * wing
//         );

//     QPointF p2(
//         base.x() - ortho.x() * wing,
//         base.y() - ortho.y() * wing
//         );

//     QLineSeries *arrow = new QLineSeries();
//     arrow->append(p1);
//     arrow->append(to);
//     arrow->append(p2);

//     QPen pen(QColor(40,40,40));
//     pen.setWidthF(1.3);
//     pen.setCapStyle(Qt::RoundCap);
//     pen.setJoinStyle(Qt::RoundJoin);
//     arrow->setPen(pen);

//     chart->addSeries(arrow);
//     arrow->attachAxis(chart->axes(Qt::Horizontal).first());
//     arrow->attachAxis(chart->axes(Qt::Vertical).first());

//     for (auto *m : chart->legend()->markers(arrow))
//         m->setVisible(false);

//     storage.append(arrow);
// }

// ChartView::ChartView(QChart *chart, QWidget *parent) : QChartView(chart, parent), m_chart(chart)
// {
//     clearAdjustedOverlay();
//     setRenderHint(QPainter::Antialiasing);
//     setDragMode(NoDrag);
//     setInteractive(true);

//     m_highlightSeries = new QScatterSeries(this);
//     QColor transparentFill(0, 0, 0, 1);
//     m_highlightSeries->setBrush(transparentFill);
//     m_highlightSeries->setPen(QPen(QColor(0, 0, 0), 2));
//     m_highlightSeries->setMarkerSize(15);
//     m_highlightSeries->setPointsVisible(true);
//     m_highlightSeries->setUseOpenGL(false);

//     m_baseSeries = new QScatterSeries();
//     m_baseSeries->setName("Bases");
//     m_baseSeries->setColor(Qt::red);
//     m_baseSeries->setMarkerSize(12);

//     m_roverSeries = new QScatterSeries();
//     m_roverSeries->setName("Stations");
//     m_roverSeries->setColor(Qt::blue);
//     m_roverSeries->setMarkerSize(12);

//     m_adjSeries  = nullptr;

//     m_chart->addSeries(m_baseSeries);
//     m_chart->addSeries(m_roverSeries);
//     m_chart->addSeries(m_highlightSeries);
//     m_chart->createDefaultAxes();

//     m_chart->setTheme(QChart::ChartThemeLight);

//     auto *axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
//     auto *axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());

//     axisX->setTickCount(12);
//     axisY->setTickCount(12);

//     axisX->setMinorTickCount(3);
//     axisY->setMinorTickCount(3);

//     axisX->setGridLineVisible(true);
//     axisY->setGridLineVisible(true);

//     axisX->setMinorGridLineVisible(true);
//     axisY->setMinorGridLineVisible(true);

//     axisX->setGridLinePen(QPen(QColor(220,220,220)));
//     axisY->setGridLinePen(QPen(QColor(220,220,220)));

//     axisX->setMinorGridLinePen(QPen(QColor(240,240,240)));
//     axisY->setMinorGridLinePen(QPen(QColor(240,240,240)));

//     m_chart->axes(Qt::Horizontal).first()->setTitleText("Easting (m)");
//     m_chart->axes(Qt::Vertical).first()->setTitleText("Northing (m)");

//     m_tooltipHideTimer = new QTimer(this);
//     m_tooltipHideTimer->setSingleShot(true);
//     m_tooltipHideTimer->setInterval(2500);
//     connect(m_tooltipHideTimer, &QTimer::timeout,
//             [](){ QToolTip::hideText(); });

//     connect(m_highlightSeries, &QScatterSeries::hovered, this, &ChartView::onPointHovered);
//     connect(m_highlightSeries, &QScatterSeries::clicked, this, &ChartView::onPointClicked);

//     for (auto *m : m_chart->legend()->markers(m_highlightSeries))
//         m->setVisible(false);
// }

// ChartView::~ChartView()
// {
//     clearChart();
// }

// QString ChartView::pointKey(const QPointF &p, int prec) const
// {
//     return QString::number(p.x(), 'f', prec) + "," + QString::number(p.y(), 'f', prec);
// }

// void ChartView::clearChart()
// {
//     m_baseSeries->clear();
//     m_roverSeries->clear();
//     m_highlightSeries->clear();
//     m_pointToIds.clear();
//     m_stationCache.clear();

//     for (QLineSeries *s : m_baselineSeries) {
//         if (!s) continue;
//         m_chart->removeSeries(s);
//         delete s;
//     }
//     m_baselineSeries.clear();
// }

// void ChartView::drawChart(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines)
// {
//     disconnect(m_baseSeries, nullptr, this, nullptr);
//     disconnect(m_roverSeries, nullptr, this, nullptr);
//     clearChart();

//     m_stationCache = stations;
//     m_pointToIds.clear();

//     double minX =  1e18, maxX = -1e18;
//     double minY =  1e18, maxY = -1e18;
//     QMap<QString, QPointF> uniqueLocations;
//     QMap<QString, bool> locationIsBase;

//     for (auto it = stations.begin(); it != stations.end(); ++it)
//     {
//         const QString &uid = it.value().uid;
//         const ProjectStation &st = it.value();

//         if (!std::isfinite(st.easting) || !std::isfinite(st.northing))
//             continue;

//         QPointF p(st.easting, st.northing);
//         QString pk = pointKey(p);
//         m_pointToIds[pk].append(uid);
//         uniqueLocations[pk] = p;
//         if (!locationIsBase.contains(pk)) {
//             locationIsBase[pk] = st.isBase;
//         } else if (st.isBase) {
//             locationIsBase[pk] = true;
//         }

//         minX = std::min(minX, p.x());
//         maxX = std::max(maxX, p.x());
//         minY = std::min(minY, p.y());
//         maxY = std::max(maxY, p.y());
//     }
//     for (auto it = uniqueLocations.begin(); it != uniqueLocations.end(); ++it) {
//         QString key = it.key();
//         QPointF p = it.value();
//         bool isBase = locationIsBase.value(key);

//         if (isBase)
//             m_baseSeries->append(p);
//         else
//             m_roverSeries->append(p);
//     }

//     connect(m_baseSeries, &QScatterSeries::hovered, this, &ChartView::onPointHovered);
//     connect(m_roverSeries, &QScatterSeries::hovered, this, &ChartView::onPointHovered);
//     connect(m_baseSeries, &QScatterSeries::clicked, this, &ChartView::onPointClicked);
//     connect(m_roverSeries, &QScatterSeries::clicked, this, &ChartView::onPointClicked);

//     for (const ProjectBaseline &bl : baselines)
//     {
//         if (!stations.contains(bl.from) || !stations.contains(bl.to))
//             continue;

//         const auto &A = stations[bl.from];
//         const auto &B = stations[bl.to];

//         QPointF pA(A.easting, A.northing);
//         QPointF pB(B.easting, B.northing);
//         if (QLineF(pA, pB).length() < 1e-4) continue;

//         QLineSeries *line = new QLineSeries();
//         line->append(pA);
//         line->append(pB);

//         // QPen pen(Qt::black);
//         // pen.setWidthF(1.8);

//         QPen pen(QColor(40,40,40));
//         pen.setWidthF(1.6);
//         pen.setCapStyle(Qt::RoundCap);
//         line->setPen(pen);

//         m_chart->addSeries(line);
//         line->attachAxis(m_chart->axes(Qt::Horizontal).first());
//         line->attachAxis(m_chart->axes(Qt::Vertical).first());

//         for (auto *m : m_chart->legend()->markers(line))
//             m->setVisible(false);

//         m_baselineSeries.append(line);
//         QString tip = QString("<b>%1 → %2</b><br>Length: %3 m").arg(A.stationId).arg(B.stationId).arg(bl.length, 0, 'f', 3);

//         connect(line, &QLineSeries::hovered, this, [this, tip](const QPointF &, bool entered)
//                 {
//                     if (entered) {
//                         m_tooltipHideTimer->stop();
//                         QToolTip::showText(QCursor::pos(), tip, this->viewport());
//                     } else {
//                         m_tooltipHideTimer->start();
//                     }
//                 });

//         addArrowHead(m_chart, pA, pB, m_baselineSeries);
//     }

//     if (minX < maxX && minY < maxY) {
//         double padX = std::max(1.0, (maxX - minX) * 0.1);
//         double padY = std::max(1.0, (maxY - minY) * 0.1);
//         m_chart->axes(Qt::Horizontal).first()->setRange(minX - padX, maxX + padX);
//         m_chart->axes(Qt::Vertical).first()->setRange(minY - padY, maxY + padY);
//     }
// }

// // ── Add after the existing drawChart() method ──────────────────────────

// void ChartView::clearAdjustedOverlay()
// {
//     for (QLineSeries *s : m_adjBaselineSeries) {
//         if (!s) continue;
//         m_chart->removeSeries(s);
//         delete s;
//     }
//     m_adjBaselineSeries.clear();

//     for (QLineSeries *s : m_correctionVectors) {
//         if (!s) continue;
//         m_chart->removeSeries(s);
//         delete s;
//     }
//     m_correctionVectors.clear();

//     if (m_adjSeries) {
//         m_adjSeries->clear();
//     }
// }

// void ChartView::drawChartAdjusted(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines,const QMap<QString, QVector3D>       &adjustedECEF)
// {
//     if (adjustedECEF.isEmpty()) {
//         drawChart(stations, baselines);
//         return;
//     }
//     // Draw original network first (slightly faded)
//     drawChart(stations, baselines);

//     // Make existing baseline lines thinner/gray
//     for (QLineSeries *s : m_baselineSeries) {
//         if (!s) continue;
//         QPen p = s->pen();
//         p.setColor(QColor(180, 180, 180));
//         p.setWidthF(1.0);
//         s->setPen(p);
//     }
//     // Make original markers gray/smaller
//     m_baseSeries->setColor(QColor(180, 100, 100));
//     m_roverSeries->setColor(QColor(100, 100, 180));
//     m_baseSeries->setMarkerSize(9);
//     m_roverSeries->setMarkerSize(9);

//     // ── Build adjusted station map (ECEF corrections → E/N) ────────────
//     // Reuse UTM easting/northing from existing stations as the approx position;
//     // corrections from adjustedECEF are in ECEF metres.
//     // For display purposes we approximate the correction in the local ENU frame:
//     //   ΔE ≈ ΔX * (-sinLon) + ΔY * cosLon
//     //   ΔN ≈ ΔX * (-sinLat*cosLon) + ΔY * (-sinLat*sinLon) + ΔZ * cosLat
//     // This is only for visualisation arrows; the chart draws positions in UTM.

//     QMap<QString, QPointF> adjPoints; // uid → adjusted (easting, northing)

//     for (auto it = stations.constBegin(); it != stations.constEnd(); ++it) {
//         const QString       &uid = it.key();
//         const ProjectStation &st  = it.value();

//         if (!adjustedECEF.contains(uid)) {
//             // Station not in result — use original
//             adjPoints[uid] = QPointF(st.easting, st.northing);
//             continue;
//         }

//         const QVector3D &adj = adjustedECEF[uid];

//         // Compute correction in ENU (approximate, for display only)
//         double latR = qDegreesToRadians(st.geo.lat);
//         double lonR = qDegreesToRadians(st.geo.lon);

//         double dX = adj.x() - st.ecef.X;
//         double dY = adj.y() - st.ecef.Y;
//         double dZ = adj.z() - st.ecef.Z;

//         double dE = -sin(lonR)           * dX  +  cos(lonR)           * dY;
//         double dN = -sin(latR)*cos(lonR)  * dX  + -sin(latR)*sin(lonR) * dY  +  cos(latR) * dZ;

//         adjPoints[uid] = QPointF(st.easting + dE, st.northing + dN);
//     }

//     // ── Adjusted positions series ───────────────────────────────────────
//     if (!m_adjSeries) {
//         m_adjSeries = new QScatterSeries();
//         m_adjSeries->setName("Adjusted");
//     }
//     m_adjSeries->clear();
//     m_adjSeries->setColor(QColor("#00b894"));
//     m_adjSeries->setMarkerSize(13);
//     m_adjSeries->setBorderColor(Qt::white);

//     for (auto it = adjPoints.constBegin(); it != adjPoints.constEnd(); ++it)
//         m_adjSeries->append(it.value());

//     m_chart->addSeries(m_adjSeries);
//     m_adjSeries->attachAxis(m_chart->axes(Qt::Horizontal).first());
//     m_adjSeries->attachAxis(m_chart->axes(Qt::Vertical).first());

//     // ── Correction vector arrows (original → adjusted) ──────────────────
//     for (auto it = stations.constBegin(); it != stations.constEnd(); ++it) {
//         const QString       &uid = it.key();
//         const ProjectStation &st  = it.value();

//         QPointF orig(st.easting, st.northing);
//         QPointF adj = adjPoints.value(uid, orig);

//         if (QLineF(orig, adj).length() < 1e-4) continue; // no visible correction

//         QLineSeries *vec = new QLineSeries();
//         vec->append(orig);
//         vec->append(adj);
//         QPen pen(QColor("#fdcb6e"));
//         pen.setWidthF(1.5);
//         pen.setStyle(Qt::DotLine);
//         vec->setPen(pen);

//         m_chart->addSeries(vec);
//         vec->attachAxis(m_chart->axes(Qt::Horizontal).first());
//         vec->attachAxis(m_chart->axes(Qt::Vertical).first());

//         for (auto *mk : m_chart->legend()->markers(vec))
//             mk->setVisible(false);

//         m_correctionVectors.append(vec);
//     }

//     // ── Adjusted baseline lines (green) ─────────────────────────────────
//     for (const ProjectBaseline &bl : baselines) {
//         if (!adjPoints.contains(bl.from) || !adjPoints.contains(bl.to)) continue;

//         QPointF pA = adjPoints[bl.from];
//         QPointF pB = adjPoints[bl.to];
//         if (QLineF(pA, pB).length() < 1e-4) continue;

//         QLineSeries *line = new QLineSeries();
//         line->append(pA);
//         line->append(pB);
//         QPen pen(QColor("#00b894"));
//         pen.setWidthF(1.8);
//         line->setPen(pen);

//         m_chart->addSeries(line);
//         line->attachAxis(m_chart->axes(Qt::Horizontal).first());
//         line->attachAxis(m_chart->axes(Qt::Vertical).first());

//         for (auto *mk : m_chart->legend()->markers(line))
//             mk->setVisible(false);

//         m_adjBaselineSeries.append(line);
//     }
// }

// void ChartView::highlightStation(const QString &uid)
// {
//     if (!m_stationCache.contains(uid))
//         return;

//     const auto &st = m_stationCache[uid];
//     if (!std::isfinite(st.easting) || !std::isfinite(st.northing))
//         return;

//     QPointF p(st.easting, st.northing);

//     if (m_chart->series().contains(m_highlightSeries)) {
//         m_chart->removeSeries(m_highlightSeries);
//     }

//     m_highlightSeries->clear();
//     m_highlightSeries->append(p);

//     m_chart->addSeries(m_highlightSeries);
//     m_highlightSeries->attachAxis(m_chart->axes(Qt::Horizontal).first());
//     m_highlightSeries->attachAxis(m_chart->axes(Qt::Vertical).first());
// }


// void ChartView::clearHighlight()
// {
//     m_highlightSeries->clear();
// }

// void ChartView::onPointHovered(const QPointF &pt, bool entered)
// {
//     if (!entered) {
//         m_tooltipHideTimer->start();
//         return;
//     }
//     m_tooltipHideTimer->stop();

//     QStringList uids = m_pointToIds.value(pointKey(pt));
//     if (uids.isEmpty())
//         return;

//     QString tip;
//     QSet<QString> printedStationIds;

//     for (QString &uid : uids) {
//         const auto &st = m_stationCache[uid];
//         if (printedStationIds.contains(st.stationId))
//             continue;
//         tip += QString("<b>%1</b><br>E: %2<br>N: %3<br>").arg(st.stationId).arg(st.easting, 0, 'f', 3).arg(st.northing, 0, 'f', 3);
//         printedStationIds.insert(st.stationId);
//     }

//     QToolTip::showText(QCursor::pos(), tip.trimmed(), viewport());
// }

// void ChartView::onPointClicked(const QPointF &pt){
//     QStringList uids = m_pointToIds.value(pointKey(pt));
//     if(uids.isEmpty())
//         return;
//     for(QString &uid : uids){
//         highlightStation(uid);
//     }
//     emit pointClicked(uids);
// }

// void ChartView::wheelEvent(QWheelEvent *event)
// {
//     const qreal factor = 1.2;
//     if (event->angleDelta().y() > 0)
//         m_chart->zoom(factor);
//     else
//         m_chart->zoom(1.0 / factor);
//     event->accept();
// }

// void ChartView::mousePressEvent(QMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         m_panning = true;
//         m_lastPos = event->pos();
//         setCursor(Qt::ClosedHandCursor);
//     }
//     QChartView::mousePressEvent(event);
// }

// void ChartView::mouseMoveEvent(QMouseEvent *event)
// {
//     if (m_panning) {
//         QPointF delta = event->pos() - m_lastPos;
//         m_chart->scroll(-delta.x(), delta.y());
//         m_lastPos = event->pos();
//     }
//     QChartView::mouseMoveEvent(event);
// }

// void ChartView::mouseReleaseEvent(QMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         m_panning = false;
//         setCursor(Qt::ArrowCursor);
//     }
//     QChartView::mouseReleaseEvent(event);
// }

// void ChartView::mouseDoubleClickEvent(QMouseEvent *)
// {
//     m_chart->zoomReset();
//     emit doubleClicked();
// }






#include "chartview.h"
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>
#include <QToolTip>
#include <cmath>
#include <QSet>
#include <QPainter>
#include <QPolygonF>

ChartView::ChartView(QChart *chart, QWidget *parent) : QChartView(chart, parent), m_chart(chart)
{
    clearAdjustedOverlay();
    setRenderHint(QPainter::Antialiasing);
    setDragMode(NoDrag);
    setInteractive(true);

    m_highlightSeries = new QScatterSeries(this);
    QColor transparentFill(0, 0, 0, 1);
    m_highlightSeries->setBrush(transparentFill);
    m_highlightSeries->setPen(QPen(QColor(0, 0, 0), 2));
    m_highlightSeries->setMarkerSize(15);
    m_highlightSeries->setPointsVisible(true);
    m_highlightSeries->setUseOpenGL(false);

    m_baseSeries = new QScatterSeries();
    m_baseSeries->setName("Bases");
    m_baseSeries->setColor(Qt::red);
    m_baseSeries->setMarkerSize(12);

    m_roverSeries = new QScatterSeries();
    m_roverSeries->setName("Stations");
    m_roverSeries->setColor(Qt::blue);
    m_roverSeries->setMarkerSize(12);

    m_adjSeries  = nullptr;

    m_chart->addSeries(m_baseSeries);
    m_chart->addSeries(m_roverSeries);
    m_chart->addSeries(m_highlightSeries);
    m_chart->createDefaultAxes();

    m_chart->setTheme(QChart::ChartThemeLight);

    auto *axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
    auto *axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());

    axisX->setTickCount(12);
    axisY->setTickCount(12);

    axisX->setMinorTickCount(3);
    axisY->setMinorTickCount(3);

    axisX->setGridLineVisible(true);
    axisY->setGridLineVisible(true);

    axisX->setMinorGridLineVisible(true);
    axisY->setMinorGridLineVisible(true);

    axisX->setGridLinePen(QPen(QColor(220,220,220)));
    axisY->setGridLinePen(QPen(QColor(220,220,220)));

    axisX->setMinorGridLinePen(QPen(QColor(240,240,240)));
    axisY->setMinorGridLinePen(QPen(QColor(240,240,240)));

    m_chart->axes(Qt::Horizontal).first()->setTitleText("Easting (m)");
    m_chart->axes(Qt::Vertical).first()->setTitleText("Northing (m)");

    m_tooltipHideTimer = new QTimer(this);
    m_tooltipHideTimer->setSingleShot(true);
    m_tooltipHideTimer->setInterval(2500);
    connect(m_tooltipHideTimer, &QTimer::timeout,
            [](){ QToolTip::hideText(); });

    connect(m_highlightSeries, &QScatterSeries::hovered, this, &ChartView::onPointHovered);
    connect(m_highlightSeries, &QScatterSeries::clicked, this, &ChartView::onPointClicked);

    for (auto *m : m_chart->legend()->markers(m_highlightSeries))
        m->setVisible(false);
}

ChartView::~ChartView()
{
    clearChart();
}

QString ChartView::pointKey(const QPointF &p, int prec) const
{
    return QString::number(p.x(), 'f', prec) + "," + QString::number(p.y(), 'f', prec);
}

void ChartView::clearChart()
{
    m_baseSeries->clear();
    m_roverSeries->clear();
    m_highlightSeries->clear();
    m_pointToIds.clear();
    m_stationCache.clear();

    for (QLineSeries *s : m_baselineSeries) {
        if (!s) continue;
        m_chart->removeSeries(s);
        delete s;
    }
    m_baselineSeries.clear();
}

void ChartView::drawChart(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines)
{
    disconnect(m_baseSeries, nullptr, this, nullptr);
    disconnect(m_roverSeries, nullptr, this, nullptr);
    clearChart();

    m_stationCache = stations;
    m_pointToIds.clear();

    double minX =  1e18, maxX = -1e18;
    double minY =  1e18, maxY = -1e18;
    QMap<QString, QPointF> uniqueLocations;
    QMap<QString, bool> locationIsBase;

    for (auto it = stations.begin(); it != stations.end(); ++it)
    {
        const QString &uid = it.value().uid;
        const ProjectStation &st = it.value();

        if (!std::isfinite(st.easting) || !std::isfinite(st.northing))
            continue;

        QPointF p(st.easting, st.northing);
        QString pk = pointKey(p);
        m_pointToIds[pk].append(uid);
        uniqueLocations[pk] = p;
        if (!locationIsBase.contains(pk)) {
            locationIsBase[pk] = st.isBase;
        } else if (st.isBase) {
            locationIsBase[pk] = true;
        }

        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }
    for (auto it = uniqueLocations.begin(); it != uniqueLocations.end(); ++it) {
        QString key = it.key();
        QPointF p = it.value();
        bool isBase = locationIsBase.value(key);

        if (isBase)
            m_baseSeries->append(p);
        else
            m_roverSeries->append(p);
    }

    connect(m_baseSeries, &QScatterSeries::hovered, this, &ChartView::onPointHovered);
    connect(m_roverSeries, &QScatterSeries::hovered, this, &ChartView::onPointHovered);
    connect(m_baseSeries, &QScatterSeries::clicked, this, &ChartView::onPointClicked);
    connect(m_roverSeries, &QScatterSeries::clicked, this, &ChartView::onPointClicked);

    for (const ProjectBaseline &bl : baselines)
    {
        if (!stations.contains(bl.from) || !stations.contains(bl.to))
            continue;

        const auto &A = stations[bl.from];
        const auto &B = stations[bl.to];

        QPointF pA(A.easting, A.northing);
        QPointF pB(B.easting, B.northing);
        if (QLineF(pA, pB).length() < 1e-4) continue;

        QLineSeries *line = new QLineSeries();
        line->append(pA);
        line->append(pB);

        QPen pen(QColor(40,40,40));
        pen.setWidthF(1.3);
        pen.setCapStyle(Qt::RoundCap);
        line->setPen(pen);

        m_chart->addSeries(line);
        line->attachAxis(m_chart->axes(Qt::Horizontal).first());
        line->attachAxis(m_chart->axes(Qt::Vertical).first());

        for (auto *m : m_chart->legend()->markers(line))
            m->setVisible(false);

        m_baselineSeries.append(line);
        QString tip = QString("<b>%1 → %2</b><br>Length: %3 m").arg(A.stationId).arg(B.stationId).arg(bl.length, 0, 'f', 3);

        connect(line, &QLineSeries::hovered, this, [this, tip](const QPointF &, bool entered)
                {
                    if (entered) {
                        m_tooltipHideTimer->stop();
                        QToolTip::showText(QCursor::pos(), tip, this->viewport());
                    } else {
                        m_tooltipHideTimer->start();
                    }
                });
    }

    if (minX < maxX && minY < maxY) {
        double padX = std::max(1.0, (maxX - minX) * 0.1);
        double padY = std::max(1.0, (maxY - minY) * 0.1);
        m_chart->axes(Qt::Horizontal).first()->setRange(minX - padX, maxX + padX);
        m_chart->axes(Qt::Vertical).first()->setRange(minY - padY, maxY + padY);
    }
}

void ChartView::clearAdjustedOverlay()
{
    for (QLineSeries *s : m_adjBaselineSeries) {
        if (!s) continue;
        m_chart->removeSeries(s);
        delete s;
    }
    m_adjBaselineSeries.clear();

    for (QLineSeries *s : m_correctionVectors) {
        if (!s) continue;
        m_chart->removeSeries(s);
        delete s;
    }
    m_correctionVectors.clear();

    if (m_adjSeries) {
        m_adjSeries->clear();
    }
}

void ChartView::drawChartAdjusted(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines,const QMap<QString, QVector3D>       &adjustedECEF)
{
    if (adjustedECEF.isEmpty()) {
        drawChart(stations, baselines);
        return;
    }
    drawChart(stations, baselines);
    for (QLineSeries *s : m_baselineSeries) {
        if (!s) continue;
        QPen p = s->pen();
        p.setColor(QColor(180, 180, 180));
        p.setWidthF(1.0);
        s->setPen(p);
    }
    m_baseSeries->setColor(QColor(180, 100, 100));
    m_roverSeries->setColor(QColor(100, 100, 180));
    m_baseSeries->setMarkerSize(9);
    m_roverSeries->setMarkerSize(9);

    QMap<QString, QPointF> adjPoints;

    for (auto it = stations.constBegin(); it != stations.constEnd(); ++it) {
        const QString &uid = it.key();
        const ProjectStation &st  = it.value();

        if (!adjustedECEF.contains(uid)) {
            adjPoints[uid] = QPointF(st.easting, st.northing);
            continue;
        }
        const QVector3D &adj = adjustedECEF[uid];

        double latR = qDegreesToRadians(st.geo.lat);
        double lonR = qDegreesToRadians(st.geo.lon);

        double dX = adj.x() - st.ecef.X;
        double dY = adj.y() - st.ecef.Y;
        double dZ = adj.z() - st.ecef.Z;

        double dE = -sin(lonR) * dX  +  cos(lonR) * dY;
        double dN = -sin(latR)*cos(lonR)  * dX  + -sin(latR)*sin(lonR) * dY + cos(latR) * dZ;

        adjPoints[uid] = QPointF(st.easting + dE, st.northing + dN);
    }

    if (!m_adjSeries) {
        m_adjSeries = new QScatterSeries();
        m_adjSeries->setName("Adjusted");
    }
    m_adjSeries->clear();
    m_adjSeries->setColor(QColor(0, 184, 148));
    m_adjSeries->setMarkerSize(13);
    m_adjSeries->setBorderColor(Qt::white);

    for (auto it = adjPoints.constBegin(); it != adjPoints.constEnd(); ++it)
        m_adjSeries->append(it.value());

    m_chart->addSeries(m_adjSeries);
    m_adjSeries->attachAxis(m_chart->axes(Qt::Horizontal).first());
    m_adjSeries->attachAxis(m_chart->axes(Qt::Vertical).first());

    for (auto it = stations.constBegin(); it != stations.constEnd(); ++it) {
        const QString       &uid = it.key();
        const ProjectStation &st  = it.value();

        QPointF orig(st.easting, st.northing);
        QPointF adj = adjPoints.value(uid, orig);

        if (QLineF(orig, adj).length() < 1e-4) continue;

        QLineSeries *vec = new QLineSeries();
        vec->append(orig);
        vec->append(adj);
        QPen pen(QColor("#fdcb6e"));
        pen.setWidthF(1.5);
        pen.setStyle(Qt::DotLine);
        vec->setPen(pen);

        m_chart->addSeries(vec);
        vec->attachAxis(m_chart->axes(Qt::Horizontal).first());
        vec->attachAxis(m_chart->axes(Qt::Vertical).first());

        for (auto *mk : m_chart->legend()->markers(vec))
            mk->setVisible(false);

        m_correctionVectors.append(vec);
    }

    for (const ProjectBaseline &bl : baselines) {
        if (!adjPoints.contains(bl.from) || !adjPoints.contains(bl.to)) continue;

        QPointF pA = adjPoints[bl.from];
        QPointF pB = adjPoints[bl.to];
        if (QLineF(pA, pB).length() < 1e-4) continue;

        QLineSeries *line = new QLineSeries();
        line->append(pA);
        line->append(pB);
        QPen pen(QColor(0, 184, 148));
        pen.setWidthF(1.3);
        pen.setCapStyle(Qt::RoundCap);
        line->setPen(pen);

        m_chart->addSeries(line);
        line->attachAxis(m_chart->axes(Qt::Horizontal).first());
        line->attachAxis(m_chart->axes(Qt::Vertical).first());

        for (auto *mk : m_chart->legend()->markers(line))
            mk->setVisible(false);

        m_adjBaselineSeries.append(line);
    }
}

void ChartView::highlightStation(const QString &uid)
{
    if (!m_stationCache.contains(uid))
        return;

    const auto &st = m_stationCache[uid];
    if (!std::isfinite(st.easting) || !std::isfinite(st.northing))
        return;

    QPointF p(st.easting, st.northing);

    if (m_chart->series().contains(m_highlightSeries)) {
        m_chart->removeSeries(m_highlightSeries);
    }

    m_highlightSeries->clear();
    m_highlightSeries->append(p);

    m_chart->addSeries(m_highlightSeries);
    m_highlightSeries->attachAxis(m_chart->axes(Qt::Horizontal).first());
    m_highlightSeries->attachAxis(m_chart->axes(Qt::Vertical).first());
}


void ChartView::clearHighlight()
{
    m_highlightSeries->clear();
}

void ChartView::onPointHovered(const QPointF &pt, bool entered)
{
    if (!entered) {
        m_tooltipHideTimer->start();
        return;
    }
    m_tooltipHideTimer->stop();

    QStringList uids = m_pointToIds.value(pointKey(pt));
    if (uids.isEmpty())
        return;

    QString tip;
    QSet<QString> printedStationIds;

    for (QString &uid : uids) {
        const auto &st = m_stationCache[uid];
        if (printedStationIds.contains(st.stationId))
            continue;
        tip += QString("<b>%1</b><br>E: %2<br>N: %3<br>").arg(st.stationId).arg(st.easting, 0, 'f', 3).arg(st.northing, 0, 'f', 3);
        printedStationIds.insert(st.stationId);
    }

    QToolTip::showText(QCursor::pos(), tip.trimmed(), viewport());
}

void ChartView::onPointClicked(const QPointF &pt){
    QStringList uids = m_pointToIds.value(pointKey(pt));
    if(uids.isEmpty())
        return;
    for(QString &uid : uids){
        highlightStation(uid);
    }
    emit pointClicked(uids);
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    const qreal factor = 1.2;
    if (event->angleDelta().y() > 0)
        m_chart->zoom(factor);
    else
        m_chart->zoom(1.0 / factor);
    event->accept();
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_lastPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPointF delta = event->pos() - m_lastPos;
        m_chart->scroll(-delta.x(), delta.y());
        m_lastPos = event->pos();
    }
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
    QChartView::mouseReleaseEvent(event);
}

void ChartView::mouseDoubleClickEvent(QMouseEvent *)
{
    m_chart->zoomReset();
    emit doubleClicked();
}

void ChartView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QChartView::drawForeground(painter, rect);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setClipRect(m_chart->plotArea());
    auto drawArrows = [&](const QVector<QLineSeries*> &lines) {
        for (QLineSeries *series : lines) {
            if (!series || series->count() < 2) continue;

            QPointF pA = m_chart->mapToPosition(series->at(0));
            QPointF pB = m_chart->mapToPosition(series->at(1));

            double dx = pB.x() - pA.x();
            double dy = pB.y() - pA.y();
            double length = std::hypot(dx, dy);

            if (length < 15.0) continue;

            double ux = -dx / length;
            double uy = -dy / length;
            QPointF tip = pB + QPointF(ux * 7.0, uy * 7.0);
            double arrowLength = 9.5;
            double arrowWidth  = 3.0;

            double ox = -uy;
            double oy = ux;

            QPointF base = tip + QPointF(ux * arrowLength, uy * arrowLength);
            QPointF p1 = base + QPointF(ox * arrowWidth, oy * arrowWidth);
            QPointF p2 = base - QPointF(ox * arrowWidth, oy * arrowWidth);

            painter->setPen(QPen(series->pen().color(), 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(series->pen().color());

            QPolygonF arrowHead;
            arrowHead << tip << p1 << p2;
            painter->drawPolygon(arrowHead);
        }
    };

    drawArrows(m_baselineSeries);
    drawArrows(m_adjBaselineSeries);
}

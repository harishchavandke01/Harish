#include "chartview.h"
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegendMarker>
#include <QToolTip>
#include <cmath>
#include <QSet>

static constexpr double ARROW_REL_SCALE   = 0.045;
static constexpr double ARROW_MIN_LENGTH  = 0.18;
static constexpr double ARROW_WING_FACTOR = 0.38;

static QPointF offsetFromPoint(
    const QPointF &from,
    const QPointF &to,
    double offset)
{
    double vx = to.x() - from.x();
    double vy = to.y() - from.y();
    double L  = std::hypot(vx, vy);
    if (L < 1e-9) return from;

    double ux = vx / L;
    double uy = vy / L;

    return QPointF(
        from.x() + ux * offset,
        from.y() + uy * offset
        );
}

static void addArrowHead(QChart *chart, const QPointF &from, const QPointF &to, QVector<QLineSeries*> &storage)
{
    double vx = to.x() - from.x();
    double vy = to.y() - from.y();
    double L  = std::hypot(vx, vy);
    if (L < 1e-6) return;

    double ux = vx / L;
    double uy = vy / L;

    double arrowLen = std::max(ARROW_MIN_LENGTH, L * ARROW_REL_SCALE);
    if (arrowLen > 0.2 * L) arrowLen = 0.2 * L;

    double wing = arrowLen * ARROW_WING_FACTOR;
    QPointF ortho(-uy, ux);

    QPointF p1(
        to.x() - ux * arrowLen + ortho.x() * wing,
        to.y() - uy * arrowLen + ortho.y() * wing
        );
    QPointF p2(
        to.x() - ux * arrowLen - ortho.x() * wing,
        to.y() - uy * arrowLen - ortho.y() * wing
        );

    QLineSeries *arrow = new QLineSeries();
    arrow->append(to);
    arrow->append(p1);
    arrow->append(to);
    arrow->append(p2);

    QPen pen(Qt::black);
    pen.setWidthF(1.0);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    arrow->setPen(pen);

    chart->addSeries(arrow);
    arrow->attachAxis(chart->axes(Qt::Horizontal).first());
    arrow->attachAxis(chart->axes(Qt::Vertical).first());

    for (auto *m : chart->legend()->markers(arrow))
        m->setVisible(false);
    storage.append(arrow);
}

ChartView::ChartView(QChart *chart, QWidget *parent) : QChartView(chart, parent), m_chart(chart)
{
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

    m_chart->addSeries(m_baseSeries);
    m_chart->addSeries(m_roverSeries);
    m_chart->addSeries(m_highlightSeries);
    m_chart->createDefaultAxes();

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

        QPen pen(Qt::black);
        pen.setWidthF(1.8);
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

        addArrowHead(m_chart, pA, pB, m_baselineSeries);
    }

    if (minX < maxX && minY < maxY) {
        double padX = std::max(1.0, (maxX - minX) * 0.1);
        double padY = std::max(1.0, (maxY - minY) * 0.1);
        m_chart->axes(Qt::Horizontal).first()->setRange(minX - padX, maxX + padX);
        m_chart->axes(Qt::Vertical).first()->setRange(minY - padY, maxY + padY);
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

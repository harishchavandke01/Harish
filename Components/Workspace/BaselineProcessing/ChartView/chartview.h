// #ifndef CHARTVIEW_H
// #define CHARTVIEW_H

// #include <QWidget>
// #include <QtCharts/QChartView>
// #include <QScatterSeries>
// #include <QLineSeries>
// #include <QMouseEvent>
// #include <QTimer>
// #include <QMap>
// #include "../../../Context/projectcontext.h"

// QT_BEGIN_NAMESPACE
// class QChart;
// class QScatterSeries;
// class QLineSeries;
// QT_END_NAMESPACE

// class ChartView : public QChartView
// {
//     Q_OBJECT
// public:
//     explicit ChartView(QChart *chart = nullptr, QWidget *parent = nullptr);
//     ~ChartView() override;
//     void drawChart(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines);

// public slots:
//     void highlightStation(const QString &stationId);
//     void clearHighlight();

// protected:
//     void wheelEvent(QWheelEvent *event) override;
//     void mousePressEvent(QMouseEvent *event) override;
//     void mouseMoveEvent(QMouseEvent *event) override;
//     void mouseReleaseEvent(QMouseEvent *event) override;
//     void mouseDoubleClickEvent(QMouseEvent *event) override;

// private:
//     QChart *m_chart = nullptr;
//     QScatterSeries *m_baseSeries = nullptr;
//     QScatterSeries *m_roverSeries = nullptr;
//     QScatterSeries *m_highlightSeries = nullptr;

//     QVector<QLineSeries*> m_baselineSeries;

//     bool m_panning = false;
//     QPoint m_lastPos;
//     QTimer *m_tooltipHideTimer = nullptr;

//     QMap<QString, ProjectStation> m_stationCache;
//     QMap<QString, QStringList> m_pointToIds;

//     QString pointKey(const QPointF &p, int prec = 5) const;
//     void onPointHovered(const QPointF &pt, bool entered);
//     void onPointClicked(const QPointF &pt);
//     void clearChart();

// signals:
//     void doubleClicked();
//     void pointClicked(const QStringList &uids);
// };

// #endif // CHARTVIEW_H





#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QScatterSeries>
#include <QLineSeries>
#include <QMouseEvent>
#include <QTimer>
#include <QMap>
#include <QVector3D>
#include "../../../Context/projectcontext.h"

QT_BEGIN_NAMESPACE
class QChart;
class QScatterSeries;
class QLineSeries;
QT_END_NAMESPACE

class ChartView : public QChartView
{
    Q_OBJECT
public:
    explicit ChartView(QChart *chart = nullptr, QWidget *parent = nullptr);
    ~ChartView() override;

    // Draws the baseline-processing network (original positions)
    void drawChart(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines);

    // Draws before/after overlay after network adjustment runs.
    // adjustedECEF maps station uid → adjusted ECEF QVector3D.
    // If adjustedECEF is empty, falls back to drawChart().
    void drawChartAdjusted(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline>      &baselines, const QMap<QString, QVector3D>      &adjustedECEF);

public slots:
    void highlightStation(const QString &uid);
    void clearHighlight();

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QChart  *m_chart      = nullptr;
    QScatterSeries  *m_baseSeries = nullptr;   // fixed / base stations  (red)
    QScatterSeries  *m_roverSeries= nullptr;   // free stations           (blue)
    QScatterSeries  *m_adjSeries  = nullptr;   // adjusted positions      (green)
    QScatterSeries  *m_highlightSeries = nullptr;

    QVector<QLineSeries*> m_baselineSeries;    // baseline connections (original)
    QVector<QLineSeries*> m_adjBaselineSeries; // baseline connections (adjusted)
    QVector<QLineSeries*> m_correctionVectors; // correction arrows

    bool m_panning = false;
    QPoint m_lastPos;
    QTimer *m_tooltipHideTimer = nullptr;

    QMap<QString, ProjectStation> m_stationCache;
    QMap<QString, QStringList>    m_pointToIds;

    QString pointKey(const QPointF &p, int prec = 5) const;

    void clearChart();
    void clearAdjustedOverlay();

    void onPointHovered(const QPointF &pt, bool entered);
    void onPointClicked(const QPointF &pt);

    // Internal helpers
    void addBaselineLines(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline>&baselines,
                          QVector<QLineSeries*> &storage, const QColor &color, qreal width, bool addArrows);

    void fitView(double minX, double maxX, double minY, double maxY);

signals:
    void doubleClicked();
    void pointClicked(const QStringList &uids);
};

#endif // CHARTVIEW_H

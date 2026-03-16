// #ifndef CHARTVIEW_H
// #define CHARTVIEW_H

// #include <QWidget>
// #include <QWidget>
// #include <QtCharts/QChartView>
// #include <QMouseEvent>
// #include <QScatterSeries>
// #include <QLineSeries>
// #include <QTimer>
// #include <QMap>
// #include "../../../Utils/utils.h"
// #include "../../../Utils/ProcessUtils/processutils.h"
// #include "../../../Context/projectcontext.h"

// QT_BEGIN_NAMESPACE
// class QChart;
// class QScatterSeries;
// class QLineSeries;
// QT_END_NAMESPACE

// struct NetworkEdge{
//     QString baseKey;
//     QString roverKey;
//     double length;
// };

// class ChartView : public QChartView
// {
//     Q_OBJECT
// public:
//     explicit ChartView(QChart *chartParam = nullptr, QWidget *parent = nullptr);
//     ~ChartView() override;
//     // void drawGraph(const QMap<QString, PosData>& _posData,QChart* _chart,QScatterSeries* _basesSeries, QScatterSeries* _roversSeries);
//     void drawGraph(const QMap<QString, PosData>& posData);
//     void drawStations(const QMap<QString, StationPosition> &stations);
//     void drawNetwork(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines, bool adjusted = false, const QMap<QString, QVector3D> *adjustedCorrections = nullptr );

// protected:
//     void wheelEvent(QWheelEvent *event) override;
//     void mousePressEvent(QMouseEvent *event) override;
//     void mouseMoveEvent(QMouseEvent *event) override;
//     void mouseReleaseEvent(QMouseEvent *event) override;
//     void mouseDoubleClickEvent(QMouseEvent *event) override;

// private:
//     bool m_panning = false;
//     QPoint m_lastPos;

//     QChart *m_chart = nullptr;
//     QScatterSeries *m_basesSeries = nullptr;
//     QScatterSeries *m_roversSeries = nullptr;

//     QList<QLineSeries*> connectionSeries;
//     QVector<QLineSeries*> arrowSeries;
//     QTimer *m_tooltipHideTimer = nullptr;

//     QMap<QString, PosData> posData;
//     QMap<QString, StationPosition> stationPositions;
//     QMap<QString, QString> roverToBaseMap;
//     QMap<QString, QStringList> pointToNames;

//     void updateChart();
//     void setUpStationPositions();
//     void drawBaseRoverConnections();
//     void clearConnections();

//     QVector<NetworkEdge> networkEdges;

// signals:
//     void doubleClicked();
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
    void drawChart(const QMap<QString, ProjectStation> &stations, const QVector<ProjectBaseline> &baselines);

public slots:
    void highlightStation(const QString &stationId);
    void clearHighlight();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QChart *m_chart = nullptr;
    QScatterSeries *m_baseSeries = nullptr;
    QScatterSeries *m_roverSeries = nullptr;
    QScatterSeries *m_highlightSeries = nullptr;

    QVector<QLineSeries*> m_baselineSeries;

    bool m_panning = false;
    QPoint m_lastPos;
    QTimer *m_tooltipHideTimer = nullptr;

    QMap<QString, ProjectStation> m_stationCache;
    QMap<QString, QStringList> m_pointToIds;

    QString pointKey(const QPointF &p, int prec = 5) const;
    void onPointHovered(const QPointF &pt, bool entered);
    void onPointClicked(const QPointF &pt);
    void clearChart();

signals:
    void doubleClicked();
    void pointClicked(const QStringList &uids);
};

#endif // CHARTVIEW_H






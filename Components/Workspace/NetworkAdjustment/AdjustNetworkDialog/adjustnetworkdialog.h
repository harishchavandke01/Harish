#ifndef ADJUSTNETWORKDIALOG_H
#define ADJUSTNETWORKDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>
#include "../../../Context/projectcontext.h"
#include "../AdjustOptions/adjustoptions.h"
#include "../../../Utils/customcheckbox.h"

class AdjustNetworkDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdjustNetworkDialog(ProjectContext*ctx, AdjustmentOptions &opts, QWidget *parent = nullptr);

    // Returns the subnetwork indices (1-based) the user checked for adjustment
    QVector<int> selectedSubnetworks() const;

private:
    ProjectContext    *projectContext;
    AdjustmentOptions &options;

    // ── Title bar ─────────────────────────────────────────────────────────
    QWidget*topBar;
    QLabel      *iconLabel;
    QLabel      *titleLabel;
    QPushButton *closeBtn;
    QPoint       dragStartPos;
    bool         dragging = false;

    // ── Tabs ──────────────────────────────────────────────────────────────
    QTabWidget *tabs;

    // Tab 1 — Subnetworks
    QLabel       *subnetSummaryLabel;
    QTableWidget *subnetTable;

    // Tab 2 — Weighting
    CustomCheckBox *useCovCheck;
    QDoubleSpinBox *aPrioriSpin;
    QDoubleSpinBox *sigmaHSpin;
    QDoubleSpinBox *sigmaVSpin;

    // ── Buttons ───────────────────────────────────────────────────────────
    QPushButton *cancelBtn;
    QPushButton *adjustBtn;

    // ── Helpers ───────────────────────────────────────────────────────────
    void buildTitleBar();
    void buildSubnetworkTab();
    void buildWeightingTab();
    void populateSubnetworkTable();

    enum SubnetCol {
        COL_CHECK  = 0,
        COL_NAME   = 1,
        COL_STNS   = 2,
        COL_MODE   = 3,
        COL_STATUS = 4,
        COL_COUNT  = 5
    };

private slots:
    void onAdjustClicked();
    void onCancelClicked();

protected:
    void showEvent(QShowEvent *event) override {
        QDialog::showEvent(event);
        move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
    }
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            QWidget *child = childAt(event->pos());
            dragging = child && (child == topBar || topBar->isAncestorOf(child));
            if (dragging) dragStartPos = event->globalPosition().toPoint() - pos();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (dragging && (event->buttons() & Qt::LeftButton))
            move(event->globalPosition().toPoint() - dragStartPos);
    }
    void mouseReleaseEvent(QMouseEvent *) override { dragging = false; }
};

#endif // ADJUSTNETWORKDIALOG_H

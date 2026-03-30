#ifndef COVARIANCEMATRIX_H
#define COVARIANCEMATRIX_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGuiApplication>
#include <QMouseEvent>
#include "../../../Context/projectcontext.h"
class CovarianceMatrix : public QDialog
{
    Q_OBJECT
public:
    explicit CovarianceMatrix(ProjectContext * _projectContext, QWidget *parent = nullptr);

private:
    ProjectContext *ctx;
    QWidget *topBar;
    QLabel *icon;
    QLabel *title;
    QPushButton *closeBtn;
    QPoint dragStartPos;
    bool dragging = false;

    QComboBox * cb;
    QLabel * header;
    QLabel *xxlabel;
    QLabel *xylabel;
    QLabel *xzlabel;
    QLabel *yylabel;
    QLabel *yzlabel;
    QLabel *zzlabel;

    QDoubleSpinBox *xxspin;
    QDoubleSpinBox *xyspin;
    QDoubleSpinBox *xzspin;
    QDoubleSpinBox *yyspin;
    QDoubleSpinBox *yzspin;
    QDoubleSpinBox *zzspin;
    QPushButton *saveBtn;
    QVector<ProjectBaseline> tempBaselines;
    QString currentBaselineId = "";
private slots:
    void onClose();
    void onSaveClicked();
    void onBaselineChanged(const QString &baselineId);

signals:

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

#endif // COVARIANCEMATRIX_H

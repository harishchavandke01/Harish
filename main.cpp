#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QScreen>
#include <QDebug>
#include <QStyleFactory>
#include <QSettings>

void Center(MainWindow *w) {
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = screenGeometry.x() + (screenGeometry.width() - w->width()) / 2;
    int y = screenGeometry.y() + (screenGeometry.height() - w->height()) / 2;
    w->move(x, y);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(233, 233, 233));
    lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
    lightPalette.setColor(QPalette::ToolTipText, Qt::black);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    QApplication::setPalette(lightPalette);

    QFile styles(":/styles/styles/styles.qss");
    styles.open(QFile::ReadOnly);
    QString styleQSS = QLatin1String(styles.readAll());
    a.setStyleSheet(styleQSS);

    MainWindow *w = new MainWindow();
    w->adjustSize();
    Center(w);
    w->show();

    return a.exec();
}

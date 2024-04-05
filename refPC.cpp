#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <opencv2/opencv.hpp>
#include <QWheelEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QRandomGenerator>
#include "mainwindow.h"

#include <lcms2.h>
#include <MagickWand.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set the application name
    app.setApplicationName("RandomReference");

    // Set the global stylesheet
    app.setStyleSheet("QPushButton { background-color: #d3d3d3; color: #000000; border-radius: 5px; padding: 5px; } QPushButton:hover { background-color: #3d3d3d; }");

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

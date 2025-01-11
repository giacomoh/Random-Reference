#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Provide BOTH an organization and an application name
    QCoreApplication::setOrganizationName("MyCompanyOrName");
    QCoreApplication::setOrganizationDomain("mydomain.com");  // optional
    QCoreApplication::setApplicationName("RandomReference");

    // Application-wide settings
    app.setApplicationName("RandomReference");
    app.setStyleSheet(
        "QPushButton { "
        "  background-color: #d3d3d3; "
        "  color: #000000; "
        "  border-radius: 5px; "
        "  padding: 5px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #3d3d3d; "
        "}"
    );

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

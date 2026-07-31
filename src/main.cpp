#include <QApplication>

#include "app/Application.h"

int main(int argc, char *argv[])
{
    QApplication qtApp(argc, argv);
    QApplication::setApplicationName(QStringLiteral("LitePaste"));
    QApplication::setOrganizationName(QStringLiteral("LitePaste"));

    Application app;
    app.start();

    return qtApp.exec();
}

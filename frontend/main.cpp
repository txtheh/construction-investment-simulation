#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Мегаполис"));
    QApplication::setOrganizationName(QStringLiteral("ConstructionSimulation"));
    app.setFont(QFont(QStringLiteral("Arial"), 10));

    MainWindow window;
    window.show();
    return app.exec();
}

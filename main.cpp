#include <QApplication>
#include "database_manager.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DatabaseManager dbManager;  // create instance

    if (!dbManager.connectToDatabase("inventory.db")) {
        qDebug() << "Failed to connect to database. Exiting.";
        return -1;
    }

    dbManager.setupTables();

    MainWindow w;
    w.show();

    return a.exec();
}

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QString>
#include <QSqlDatabase>

class DatabaseManager
{
public:
    bool connectToDatabase(const QString& path);
    void setupTables();

private:
    QSqlDatabase db;
};

#endif // DATABASE_MANAGER_H

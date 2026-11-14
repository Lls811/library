#ifndef LIBRARY_DATABASE_LIBRARYDATABASE_H
#define LIBRARY_DATABASE_LIBRARYDATABASE_H

// 数据库访问层：由队长和数据库负责人共同维护。
// 页面和业务模块不能直接编写 SQL，统一通过本类或 Repository 访问数据库。

#include "../common/domain.h"
#include <QList>
#include <QString>

class QSqlDatabase;

class LibraryDatabase
{
public:
    LibraryDatabase();
    ~LibraryDatabase();

    bool open();
    bool isReady() const { return ready; }
    QString lastError() const { return errorMessage; }
    bool load(QList<Book> &books, QList<Reader> &readers, QList<Loan> &loans,
              int &nextBookId, int &nextReaderId);
    bool save(const QList<Book> &books, const QList<Reader> &readers,
              const QList<Loan> &loans);

private:
    bool createSchema();
    bool execute(const QString &sql);
    void setError(const QString &message);

    QSqlDatabase *database = nullptr;
    bool ready = false;
    QString errorMessage;
};

#endif // LIBRARY_DATABASE_LIBRARYDATABASE_H

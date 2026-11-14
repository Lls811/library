#include "include/database/librarydatabase.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
// 连接名固定，避免 Qt 在同一进程中重复创建同名连接。
constexpr auto kConnectionName = "library_mvp_connection";
constexpr auto kBootstrapName = "library_bootstrap";
constexpr auto kDatabaseName = "library_mvp";
}

LibraryDatabase::LibraryDatabase() = default;

LibraryDatabase::~LibraryDatabase()
{
    // 先关闭连接，再删除句柄，最后通知 Qt 移除命名连接。
    if (database) {
        database->close();
        delete database;
        QSqlDatabase::removeDatabase(kConnectionName);
    }
}

void LibraryDatabase::setError(const QString &message)
{
    errorMessage = message;
}

bool LibraryDatabase::open()
{
    // 使用 QODBC 连接本机 MySQL，密码优先从环境变量读取。
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        setError("Qt QODBC 驱动不可用");
        return false;
    }

    // 使用环境变量可以在部署时替换密码；默认值仅用于当前课程作业环境。
    const auto password = qEnvironmentVariable("LIBRARY_DB_PASSWORD", "20070811");
    const auto connection = QString(
        "DRIVER={MySQL ODBC 8.0 Unicode Driver};SERVER=127.0.0.1;"
        "PORT=3306;UID=root;PWD=%1;OPTION=3;").arg(password);

    // 先连接 mysql 系统库创建业务库，再切换到 library_mvp。
    auto bootstrap = QSqlDatabase::addDatabase("QODBC", kBootstrapName);
    bootstrap.setDatabaseName(connection + "DATABASE=mysql;");
    if (!bootstrap.open()) {
        setError(bootstrap.lastError().text());
        bootstrap = QSqlDatabase();
        QSqlDatabase::removeDatabase(kBootstrapName);
        return false;
    }

    QSqlQuery createDatabase(bootstrap);
    const auto createSql = QString(
        "CREATE DATABASE IF NOT EXISTS %1 CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")
        .arg(kDatabaseName);
    if (!createDatabase.exec(createSql)) {
        setError(createDatabase.lastError().text());
        bootstrap.close();
        QSqlDatabase::removeDatabase(kBootstrapName);
        return false;
    }
    bootstrap.close();
    QSqlDatabase::removeDatabase(kBootstrapName);

    // 业务连接和 bootstrap 连接分开，避免在创建数据库后继续复用系统库连接。
    database = new QSqlDatabase(QSqlDatabase::addDatabase("QODBC", kConnectionName));
    database->setDatabaseName(connection + "DATABASE=" + kDatabaseName + ";");
    if (!database->open()) {
        setError(database->lastError().text());
        return false;
    }

    ready = createSchema();
    return ready;
}

bool LibraryDatabase::execute(const QString &sql)
{
    QSqlQuery query(*database);
    if (query.exec(sql)) return true;
    setError(query.lastError().text());
    return false;
}

bool LibraryDatabase::createSchema()
{
    // MVP 只初始化核心表；字段保持简单，方便后续继续扩展预约和权限表。
    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS books ("
        "id INT PRIMARY KEY, title VARCHAR(200) NOT NULL, author VARCHAR(100), "
        "category VARCHAR(100), total INT NOT NULL, available INT NOT NULL)",
        "CREATE TABLE IF NOT EXISTS readers ("
        "id INT PRIMARY KEY, name VARCHAR(100) NOT NULL, type VARCHAR(50), phone VARCHAR(40))",
        "CREATE TABLE IF NOT EXISTS loans ("
        "id INT AUTO_INCREMENT PRIMARY KEY, reader_id INT NOT NULL, book_id INT NOT NULL, "
        "borrow_date DATE NOT NULL, due_date DATE NOT NULL, return_date DATE NULL, "
        "returned TINYINT NOT NULL DEFAULT 0)"
    };

    for (const auto &statement : statements) {
        if (!execute(statement)) return false;
    }
    return true;
}

bool LibraryDatabase::load(QList<Book> &books, QList<Reader> &readers,
                           QList<Loan> &loans, int &nextBookId, int &nextReaderId)
{
    if (!ready) return false;

    QSqlQuery query(*database);
    if (!query.exec("SELECT id,title,author,category,total,available FROM books ORDER BY id")) {
        setError(query.lastError().text());
        return false;
    }

    // 只有第一条查询成功后才清空内存数据，避免读取失败时把演示数据丢掉。
    books.clear();
    readers.clear();
    loans.clear();
    nextBookId = 1000;
    nextReaderId = 1;

    // 读取图书的同时计算下一个可用编号，避免新增数据与已有主键冲突。
    while (query.next()) {
        books.append({query.value(0).toInt(), query.value(1).toString(),
                      query.value(2).toString(), query.value(3).toString(),
                      query.value(4).toInt(), query.value(5).toInt()});
        nextBookId = qMax(nextBookId, query.value(0).toInt() + 1);
    }

    if (!query.exec("SELECT id,name,type,phone FROM readers ORDER BY id")) {
        setError(query.lastError().text());
        return false;
    }
    while (query.next()) {
        readers.append({query.value(0).toInt(), query.value(1).toString(),
                        query.value(2).toString(), query.value(3).toString()});
        nextReaderId = qMax(nextReaderId, query.value(0).toInt() + 1);
    }

    if (!query.exec("SELECT reader_id,book_id,borrow_date,due_date,return_date,returned "
                   "FROM loans ORDER BY id")) {
        setError(query.lastError().text());
        return false;
    }
    // 归还日期允许为空，QVariant 转换为空日期后由 returned 字段决定状态。
    while (query.next()) {
        loans.append({query.value(0).toInt(), query.value(1).toInt(),
                      query.value(2).toDate(), query.value(3).toDate(),
                      query.value(4).toDate(), query.value(5).toBool()});
    }
    return true;
}

bool LibraryDatabase::save(const QList<Book> &books, const QList<Reader> &readers,
                           const QList<Loan> &loans)
{
    if (!ready) return true;

    // 保存采用幂等写入：重复保存不会重复创建图书和读者记录。
    QSqlQuery query(*database);
    // 图书和读者使用主键 upsert：存在则更新，不存在则插入。
    query.prepare("INSERT INTO books(id,title,author,category,total,available) "
                  "VALUES(?,?,?,?,?,?) ON DUPLICATE KEY UPDATE "
                  "title=VALUES(title),author=VALUES(author),category=VALUES(category),"
                  "total=VALUES(total),available=VALUES(available)");
    for (const auto &book : books) {
        query.addBindValue(book.id); query.addBindValue(book.title);
        query.addBindValue(book.author); query.addBindValue(book.category);
        query.addBindValue(book.total); query.addBindValue(book.available);
        if (!query.exec()) {
            setError(query.lastError().text());
            return false;
        }
        query.finish();
    }

    query.prepare("INSERT INTO readers(id,name,type,phone) VALUES(?,?,?,?) "
                  "ON DUPLICATE KEY UPDATE name=VALUES(name),type=VALUES(type),phone=VALUES(phone)");
    for (const auto &reader : readers) {
        query.addBindValue(reader.id); query.addBindValue(reader.name);
        query.addBindValue(reader.type); query.addBindValue(reader.phone);
        if (!query.exec()) {
            setError(query.lastError().text());
            return false;
        }
        query.finish();
    }

    // 借阅记录没有依赖自增 id 的领域模型，因此用读者、图书和借书日期定位记录。
    for (const auto &loan : loans) {
        QSqlQuery find(*database);
        find.prepare("SELECT id FROM loans WHERE reader_id=? AND book_id=? AND borrow_date=?");
        find.addBindValue(loan.readerId); find.addBindValue(loan.bookId); find.addBindValue(loan.borrowDate);
        if (!find.exec()) {
            setError(find.lastError().text());
            return false;
        }

        if (find.next()) {
            QSqlQuery update(*database);
            update.prepare("UPDATE loans SET due_date=?,return_date=?,returned=? WHERE id=?");
            update.addBindValue(loan.dueDate);
            update.addBindValue(loan.returned ? QVariant(loan.returnDate) : QVariant());
            update.addBindValue(loan.returned); update.addBindValue(find.value(0));
            if (!update.exec()) { setError(update.lastError().text()); return false; }
        } else {
            QSqlQuery insert(*database);
            insert.prepare("INSERT INTO loans(reader_id,book_id,borrow_date,due_date,return_date,returned) "
                           "VALUES(?,?,?,?,?,?)");
            insert.addBindValue(loan.readerId); insert.addBindValue(loan.bookId);
            insert.addBindValue(loan.borrowDate); insert.addBindValue(loan.dueDate);
            insert.addBindValue(loan.returned ? QVariant(loan.returnDate) : QVariant());
            insert.addBindValue(loan.returned);
            if (!insert.exec()) { setError(insert.lastError().text()); return false; }
        }
    }
    return true;
}
